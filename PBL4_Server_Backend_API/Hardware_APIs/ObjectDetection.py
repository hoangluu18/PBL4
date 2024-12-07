import os
import io
import sys
import signal
import threading
from datetime import datetime
from collections import Counter

import cv2
import numpy as np
from PIL import Image
from fastapi import FastAPI, Request, APIRouter
from tensorflow.keras.models import load_model
from tensorflow.keras.preprocessing import image as keras_image
import uvicorn

# Cấu hình và đường dẫn
UPLOAD_FOLDER = './uploadObject'
UPLOAD_FOLDER_HAVE = os.path.join(UPLOAD_FOLDER, 'IsHave')
UPLOAD_FOLDER_NO = os.path.join(UPLOAD_FOLDER, 'NoHave')
MODEL_PATH = r'D:\Code\pbl44\PBL4\PBL4_Server_Backend_API\GarbageDetection\garbage_classification_binary_model.h5'

# Khởi tạo model
model_inception = load_model(MODEL_PATH)

# Các biến toàn cục
latest_frames = []
classification_results = []
final_classification_result = None
stop_event = threading.Event()

# Khởi tạo router và app
ObjectDetection_router = APIRouter()
app = FastAPI()
app.include_router(ObjectDetection_router)

# Tiện ích xử lý ảnh
def save_image(frame, folder, prefix="frame"):
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    image_path = os.path.join(folder, f"{prefix}_{timestamp}.jpg")
    cv2.imwrite(image_path, frame)
    return image_path

def predict_waste_category(image_path):
    img = Image.open(image_path).resize((384, 512))
    img_array = keras_image.img_to_array(img) / 255.0
    img_array = np.expand_dims(img_array, axis=0)

    prediction = model_inception.predict(img_array)[0][0]
    category = "IsHaveObject" if prediction < 0.5 else "NoThings"
    probability = 1 - prediction
    return category, probability

# API endpoint
@ObjectDetection_router.post("")
async def upload_image(request: Request):
    global latest_frames, classification_results

    if request.headers.get('Content-Type') == 'image/jpeg':
        contents = await request.body()
        image = Image.open(io.BytesIO(contents))
        frame = cv2.cvtColor(np.array(image), cv2.COLOR_RGB2BGR)
        latest_frames.append(frame)

        image_path = save_image(frame, UPLOAD_FOLDER)
        predicted_category, probability = predict_waste_category(image_path)
        classification_results.append((predicted_category, probability))

        if predicted_category == 'IsHaveObject':
            save_image(frame, UPLOAD_FOLDER_HAVE)
        else:
            save_image(frame, UPLOAD_FOLDER_NO)

        os.remove(image_path)
        return {"status": 1}

    return {"status": 0}

@ObjectDetection_router.get("")
def get_result():
    global classification_results, final_classification_result, latest_frames

    if len(classification_results) >= 3:
        categories = [result[0] for result in classification_results]
        most_common_category = Counter(categories).most_common(1)[0][0]

        relevant_results = [result for result in classification_results if result[0] == most_common_category]
        best_result = max(relevant_results, key=lambda x: x[1])

        final_classification_result = best_result
        classification_results.clear()
        latest_frames.clear()

        return {
            "result": final_classification_result[0],
            "probability": float(final_classification_result[1])
        }

    return {"message": "Not enough frames yet"}

# Hiển thị frame
def show_frame():
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

# Chạy server
def run_server():
    config = uvicorn.Config(app, host="0.0.0.0", port=8000, loop="asyncio")
    server = uvicorn.Server(config)

    server_thread = threading.Thread(target=server.run)
    server_thread.start()

    stop_event.wait()
    server.should_exit = True
    server_thread.join()

# Xử lý tín hiệu thoát
def signal_handler(sig, frame):
    print("Exiting gracefully...")
    stop_event.set()
    sys.exit(0)

if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)
    server_thread = threading.Thread(target=run_server)
    server_thread.start()
    server_thread.join()
