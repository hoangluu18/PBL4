import datetime
from fastapi import FastAPI, HTTPException, APIRouter
import firebase_admin
from firebase_admin import credentials, db, firestore
from PBL4_Server_Backend_API.Model.ResultModel import ResultModel

# Constants
FIREBASE_CREDENTIALS_PATH = r"D:\Code\pbl44\PBL4\PBL4_Server_Backend_API\token.json"
FIREBASE_DB_URL = 'https://ahrumiki-default-rtdb.asia-southeast1.firebasedatabase.app/'

# Initialize Firebase
cred = credentials.Certificate(FIREBASE_CREDENTIALS_PATH)
firebase_admin.initialize_app(cred, {
    'databaseURL': FIREBASE_DB_URL
})

# Kết nối với Firestore
db_firestore = firestore.client()

# Khởi tạo FastAPI app
app = FastAPI()

# Router
Result_Sender_router = APIRouter()

# API Endpoints
@Result_Sender_router.post("/")
def upload_result(data: ResultModel):
    """Handle result upload to Firebase."""
    try:
        # Lưu kết quả vào Firebase Realtime Database
        ref = db.reference(data.result)
        ref.set({
            'isFull': data.isFull
        })

        # Lưu dữ liệu vào Firestore
        doc_ref = db_firestore.collection("Trash_Information").document()
        doc_ref.set({
            "Type": data.result,
            "Time": datetime.datetime.now().isoformat()  # Sử dụng datetime.datetime thay vì datetime
        })

        print(f"Kết quả đã được cập nhật: {data.result}")
        return {"message": f"Data received for {data.result}"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error occurred: {str(e)}")

# Thêm router vào ứng dụng chính
app.include_router(Result_Sender_router)
