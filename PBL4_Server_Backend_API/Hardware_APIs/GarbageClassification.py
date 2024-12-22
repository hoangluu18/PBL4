import io
import cv2
import numpy as np
from fastapi import FastAPI, Request, APIRouter
import uvicorn
import threading
import signal
import sys
import os
from datetime import datetime
from collections import Counter
from PIL import Image
from tensorflow.keras.preprocessing import image as keras_image
from tensorflow.keras.models import load_model
import Global
# Constants
UPLOAD_FOLDER = './uploads'
MODEL_PATH = r'D:\Code\pbl44\PBL4\PBL4_Server_Backend_API\GarbageDetection\garbage_classification_model_inception_v5.h5'
WASTE_CATEGORIES = ['metal', 'other', 'paper', 'plastic'] #Class other o dau?


latest_frames = []
classification_results = []
final_classification_result = None
stop_event = threading.Event()

# Load Model
model_inception = load_model(MODEL_PATH)

# FastAPI Router
GarbageClassification_router = APIRouter()


# Utility Functions
def save_frame(contents):
    image = Image.open(io.BytesIO(contents))
    frame = cv2.cvtColor(np.array(image), cv2.COLOR_RGB2BGR)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    image_path = os.path.join(UPLOAD_FOLDER, f"frame_{timestamp}.jpg")
    cv2.imwrite(image_path, frame)
    return frame, image_path


def predict_waste_category(image_path):
    img = Image.open(image_path)
    img = img.resize((384, 512))
    img_array = keras_image.img_to_array(img)
    img_array = np.expand_dims(img_array, axis=0) / 255.0
    prediction = model_inception.predict(img_array)
    predicted_category_index = np.argmax(prediction)
    predicted_category = WASTE_CATEGORIES[predicted_category_index]
    probability = prediction[0][predicted_category_index]
    return predicted_category, probability


def get_final_result():
    global classification_results
    categories = [result[0] for result in classification_results]
    category_count = Counter(categories)
    most_common_category = category_count.most_common(1)[0][0]
    relevant_results = [result for result in classification_results if result[0] == most_common_category]
    best_result = max(relevant_results, key=lambda x: x[1])
    return best_result


# API Endpoints
@GarbageClassification_router.post("/upload")
async def upload_image(request: Request):
    global latest_frames, classification_results

    if request.headers.get('Content-Type') == 'image/jpeg':
        contents = await request.body()
        frame, image_path = save_frame(contents)
        Global.lastestImage = image_path
        print(Global.lastestImage)
        latest_frames.append(frame)
        predicted_category, probability = predict_waste_category(image_path)
        classification_results.append((predicted_category, probability))
        print("----------", predicted_category)
        print("\nprobability", probability)
    else: print("ko in")
    return {"message": "Frame received"}


@GarbageClassification_router.get("/result")
def get_result():
    global classification_results, final_classification_result, latest_frames

    if len(classification_results) >= 3:
        final_classification_result = get_final_result()

        classification_results.clear()
        latest_frames.clear()
        print("----->>>>",final_classification_result[0])
        return {
            "result": final_classification_result[0],
            "probability": float(final_classification_result[1])
        }

    return {"message": "Not enough frames yet"}


# Display Frame
def show_frame():
    global latest_frames

    cv2.namedWindow("ESP32-CAM Stream", cv2.WINDOW_NORMAL)
    cv2.setWindowProperty("ESP32-CAM Stream", cv2.WND_PROP_TOPMOST, 1)

    while not stop_event.is_set():
        if latest_frames:
            for frame in latest_frames:
                cv2.imshow("ESP32-CAM Stream", frame)
                if cv2.waitKey(500) & 0xFF == 27:
                    stop_event.set()
                    break
            latest_frames.clear()
        else:
            cv2.waitKey(1)

    cv2.destroyAllWindows()


# Server Management
def run_server():
    config = uvicorn.Config(GarbageClassification_router, host="0.0.0.0", port=8000, loop="asyncio")
    server = uvicorn.Server(config)
    server_thread = threading.Thread(target=server.run)
    server_thread.start()
    stop_event.wait()
    server.should_exit = True
    server_thread.join()


def signal_handler(sig, frame):
    print("Exiting gracefully...")
    stop_event.set()
    sys.exit(0)


if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)
    server_thread = threading.Thread(target=run_server)
    server_thread.start()
    server_thread.join()