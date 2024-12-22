import datetime
import Global
from fastapi import FastAPI, HTTPException, APIRouter, Request
import firebase_admin
from firebase_admin import credentials, db, firestore
from Model.ResultModel import ResultModel

import cloudinary
import cloudinary.uploader

# Cấu hình Cloudinary
cloudinary.config(
    cloud_name="dsmmehsrb",
    api_key="844243638966347",
    api_secret="gzqMHr0HotBowpfoO8gJqaL4do0"
)

# Constants
FIREBASE_CREDENTIALS_PATH = r"D:\Code\pbl44\PBL4\PBL4_Server_Backend_API\token.json"
FIREBASE_DB_URL = 'https://ahrumiki-default-rtdb.asia-southeast1.firebasedatabase.app/'

# Initialize Firebase
cred = credentials.Certificate(FIREBASE_CREDENTIALS_PATH)
firebase_admin.initialize_app(cred, {
    'databaseURL': FIREBASE_DB_URL
})
TRASH_FULL_STATE = 1

trash = {"Paper","Plastic","Metal","Other"}

# Kết nối với Firestore
db_firestore = firestore.client()

# Khởi tạo FastAPI app
app = FastAPI()

# Router
Result_Sender_router = APIRouter()

# API Endpoints
@Result_Sender_router.post("/upload")
def upload_result(data: ResultModel):
    """Handle result upload to Firebase."""
    try:
        # Tham chiếu đến khóa 'count' trong Firebase

        # Lưu dữ liệu vào khóa tương ứng với `data.result`
        ref = db.reference(data.result)
        count = ref.child("count").get() or 0
        ref.child("count").set(count+1)

        image_url = upload_image_to_cloudinary(Global.lastestImage)
        if image_url:
            print(f"Ảnh đã upload lên Cloudinary, link: {image_url}")
        # Lưu dữ liệu vào Firestore
        doc_ref = db_firestore.collection("Trash_Information").document()
        doc_ref.set({
            "Type": data.result,
            "Time": datetime.datetime.now().isoformat() ,
            "imageURL": image_url
        })

        print(f"Kết quả đã được cập nhật: {data.result}")
        return {"message": f"Data received for {data.result}"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error occurred: {str(e)}")

@Result_Sender_router.post("/upload/state")
async def upload_result(request: Request):
    """Handle result upload to Firebase."""
    try:
        # Parse the JSON input from the request body
        input_data = await request.json()

        # Iterate through each key-value pair in the input JSON
        for trash_type, state in input_data.items():
            # Get a reference to the specific trash type in Firebase
            ref = db.reference(trash_type)
            # Set the state for this trash type
            ref.child('state').set(state == TRASH_FULL_STATE)


        return {"message": "States updated successfully"}

    except Exception as e:
        # Handle errors and return a 500 HTTP response
        print(str(e))
        raise HTTPException(status_code=500, detail=f"Error occurred: {str(e)}")



# Hàm upload ảnh lên Cloudinary
def upload_image_to_cloudinary(image_path):
    try:
        response = cloudinary.uploader.upload(image_path)
        return response['secure_url']  # Trả về link ảnh
    except Exception as e:
        print(f"Lỗi khi upload ảnh lên Cloudinary: {e}")
        return None


# Thêm router vào ứng dụng chính
app.include_router(Result_Sender_router)

