import uuid
from fastapi import FastAPI, UploadFile, File, Request, Response
import os
from datetime import datetime
from fastapi.responses import JSONResponse
import tensorflow as tf
import numpy as np
# from tensorflow.keras.preprocessing import image
from tensorflow.keras.models import load_model

from PIL import Image
from tensorflow.keras.preprocessing import image as keras_image
from tensorflow.keras.models import load_model  # Import load_model
app = FastAPI()

UPLOAD_FOLDER = './uploads'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)



@app.post("/api/upload/image/")
async def upload_file(file: UploadFile = File(...)):
    file.filename = f"{uuid.uuid4()}.jpg"

    # Đọc nội dung file
    contents = await file.read()

    imageDir = r"D:\Code\PBL4\Server_Backend_API\wwwroot\Images"

    os.makedirs(imageDir, exist_ok=True)

    file_path = os.path.join(imageDir, file.filename)

    with open(file_path, "wb") as f:
        f.write(contents)

    return {"filename": file.filename}

def load_and_preprocess_image(image_path):
  """Tải ảnh, resize và chuẩn hóa."""
  # img = image.load_img(image_path, target_size=(224, 224))  # Resize ảnh
  # img_array = image.img_to_array(img)  # Chuyển ảnh thành mảng numpy
  # img_array = np.expand_dims(img_array, axis=0)  # Thêm chiều batch
  # img_array /= 255.  # Chuẩn hóa giá trị pixel về [0, 1]
  img = Image.open(image_path)
  img = img.resize((384, 512))  # Điều chỉnh kích thước ảnh theo kích thước đầu vào của mô hình
  img_array = keras_image.img_to_array(img)
  img_array = np.expand_dims(img_array, axis=0) / 255.0  # Chuẩn hóa ảnh
  return img_array

@app.post("/upload")
async def upload_file(request: Request):
    # Kiểm tra Content-Type của request
    if request.headers.get('Content-Type') == 'image/jpeg':
        # Lấy dữ liệu từ request
        file_content = await request.body()

        # Tạo tên file với timestamp
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        file_name = f'captured_image_{timestamp}.jpg'
        file_path = os.path.join(UPLOAD_FOLDER, file_name)

        # Lưu file vào hệ thống
        with open(file_path, 'wb') as f:
            f.write(file_content)
        img_test = load_and_preprocess_image(file_path)
        model_path = 'E:/Workspace/PBL4/PBL4_Server_Backend_API/ModelAI/garbage_classification_model_inception_50.h5'
        # model_path = 'E:/Workspace/PBL4/PBL4_Server_Backend_API/ModelAI/garbage_classification_model_inception.h5'

        myModel = load_model(model_path)

        # Dự đoán kết quả
        # predictions = myModel.predict(img_test)

        # Lấy nhãn dự đoán
        # predicted_class = np.argmax(predictions)
        # print(predicted_class)

        # Lấy danh sách các lớp (nếu cần)
        # class_names = ['metal', 'paper', 'plastic', 'white-glass']  # Thay đổi cho phù hợp với các lớp của bạn

        # In ra kết quả
        # print("Kết quả dự đoán:", class_names[predicted_class])
        # print("Xác suất:", predictions[0])

        model_inception = load_model(model_path)  # Load model đã lưu đúng cách
        predicted_category, probability = predict_waste_category(file_path, model_inception)
        print("Predicted waste category:", predicted_category)
        print("Probability:", probability)

        # Trả về status code 200 (OK)
        return JSONResponse(content={"message": "File received and saved"}, status_code=200)
    else:
        # Trả về status code 415 (Unsupported Media Type) khi Content-Type không phải 'image/jpeg'
        return JSONResponse(content={"message": "Unsupported Media Type"}, status_code=415)


def predict_waste_category(image_path, model):
    # Load và xử lý ảnh
    img = Image.open(image_path)
    img = img.resize((384, 512))  # Điều chỉnh kích thước ảnh theo kích thước đầu vào của mô hình
    img_array = keras_image.img_to_array(img)
    img_array = np.expand_dims(img_array, axis=0) / 255.0  # Chuẩn hóa ảnh

    # Dự đoán loại rác
    prediction = model.predict(img_array)
    waste_categories = [ 'glass', 'metal', 'paper', 'plastic']
    predicted_category_index = np.argmax(prediction)
    predicted_category = waste_categories[predicted_category_index]
    # glass', 'metal', 'paper', 'plastic
    # Lấy xác suất dự đoán của loại rác
    probability = prediction[0][predicted_category_index]
    return predicted_category, probability

#uvicorn ImageAPIs:app --host 0.0.0.0 --port 8000