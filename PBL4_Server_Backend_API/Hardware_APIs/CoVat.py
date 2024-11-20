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
# app = FastAPI()
IsHaveObject_router = APIRouter()
UPLOAD_FOLDER = './uploadObject'
model_path = r'D:\Code\pbl44\PBL4\PBL4_Server_Backend_API\GarbageDetection\garbage_classification_binary_model.h5' #model phan loai co vat hay khong ?

# Khung hình mới nhất được lưu ở đây
latest_frames = []
stop_event = threading.Event()  # Sự kiện để dừng cả server và hiển thị frame

classification_results = []
final_classification_result = None # Kết quả phân loại cuối cùng
model_inception = load_model(model_path)


@IsHaveObject_router.post("/upload")
async def upload_image(request: Request):
    # global latest_frame
    global latest_frames
    # Đọc dữ liệu từ file ảnh được tải lên
    res = "chuaCo"
    print(res)
    if request.headers.get('Content-Type') == 'image/jpeg':
        contents = await request.body()

        # Chuyển bytes thành ảnh và lưu lại dưới dạng numpy array
        image = Image.open(io.BytesIO(contents))
        frame = cv2.cvtColor(np.array(image), cv2.COLOR_RGB2BGR)
        latest_frames.append(frame)

        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
        image_path = os.path.join(UPLOAD_FOLDER, f"frame_{timestamp}.jpg")
        cv2.imwrite(image_path, frame)

        predicted_category, probability = predict_waste_category(image_path)
        print(predicted_category, probability)
        classification_results.append((predicted_category, probability))

        res = predicted_category

    return {"message": res}

@IsHaveObject_router.get("/result")
def get_result():
    global classification_results, final_classification_result, latest_frames

    # Chỉ trả về kết quả nếu đã nhận đủ 3 frame
    if len(classification_results) >= 3 :
        # Đếm số lần xuất hiện của mỗi loại category
        categories = [result[0] for result in classification_results]
        category_count = Counter(categories)

        # Lấy ra loại rác xuất hiện nhiều nhất
        most_common_category = category_count.most_common(1)[0][0]

        # Lọc ra các kết quả thuộc loại rác xuất hiện nhiều nhất
        relevant_results = [result for result in classification_results if result[0] == most_common_category]

        # Tìm ra kết quả có xác suất (probability) cao nhất trong số này
        best_result = max(relevant_results, key=lambda x: x[1])

        # Lưu kết quả cuối cùng (category và xác suất cao nhất)
        final_classification_result = best_result

        # Xóa dữ liệu cũ
        classification_results.clear()
        latest_frames.clear()
        print(final_classification_result)
        return {
            "result": final_classification_result[0],
            "probability": float(final_classification_result[1])  # Chuyển numpy.float32 thành float
        }
    else:
        return {"message": "Not enough frames yet"}

# Nhan dien rac
def predict_waste_category(image_path):
    # Load và xử lý ảnh
    img = Image.open(image_path)
    img = img.resize((384, 512))  # Điều chỉnh kích thước ảnh theo kích thước đầu vào của mô hình
    img_array = keras_image.img_to_array(img)
    img_array = np.expand_dims(img_array, axis=0) / 255.0  # Chuẩn hóa ảnh

    # Dự đoán loại rác
    prediction = model_inception.predict(img_array)[0][0]
    if prediction < 0.5:
        return "IsHaveObject", prediction
    else:
        return "NoThings", prediction

    # waste_categories = [ 'glass', 'metal', 'paper', 'plastic']
    # predicted_category_index = np.argmax(prediction)
    # predicted_category = waste_categories[predicted_category_index]
    # # glass', 'metal', 'paper', 'plastic
    # # Lấy xác suất dự đoán của loại rác
    # probability = prediction[0][predicted_category_index]
    # return predicted_category, probability

def show_frame():
    # global latest_frame
    global latest_frames
    # Tạo cửa sổ để hiển thị frame
    cv2.namedWindow("ESP32-CAM Stream", cv2.WINDOW_NORMAL)
    cv2.setWindowProperty("ESP32-CAM Stream", cv2.WND_PROP_TOPMOST, 1)  # Đặt cửa sổ lên trên cùng

    while not stop_event.is_set():  # Kiểm tra sự kiện dừng

        if latest_frames:
            for frame in latest_frames:
                # Hiển thị frame mới nhất
                cv2.imshow("ESP32-CAM Stream", frame)

                # Đợi một khoảng thời gian để không làm quá nhanh
                if cv2.waitKey(500) & 0xFF == 27:  # Nhấn ESC để thoát
                    stop_event.set()
                    break
            latest_frames.clear()
        else:
            cv2.waitKey(1)

    cv2.destroyAllWindows()


def run_server():
    config = uvicorn.Config(IsHaveObject_router, host="0.0.0.0", port=8000, loop="asyncio")
    server = uvicorn.Server(config)

    # Chạy server trong một luồng riêng
    server_thread = threading.Thread(target=server.run)
    server_thread.start()

    # Đợi cho đến khi stop_event được đặt
    stop_event.wait()

    # Khi stop_event được kích hoạt, tắt server
    server.should_exit = True
    server_thread.join()  # Chờ server thoát hoàn toàn


def signal_handler(sig, frame):
    print("Exiting gracefully...")
    stop_event.set()  # Đánh dấu sự kiện dừng
    sys.exit(0)


if __name__ == "__main__":
    # Đăng ký xử lý tín hiệu để thoát khi nhận tín hiệu ngắt
    signal.signal(signal.SIGINT, signal_handler)

    # Tạo thread để chạy server
    server_thread = threading.Thread(target=run_server)
    server_thread.start()

    # Gọi hàm hiển thị frame trên cửa sổ
    show_frame()

    # Đợi server kết thúc
    server_thread.join()
