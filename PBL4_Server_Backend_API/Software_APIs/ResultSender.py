import firebase_admin
from firebase_admin import credentials, db
from fastapi import FastAPI, APIRouter
from PBL4_Server_Backend_API.Model.ResultModel import ResultModel

# Constants
FIREBASE_CREDENTIALS_PATH = r"D:\Code\pbl44\PBL4\PBL4_Server_Backend_API\token.json"
FIREBASE_DB_URL = 'https://ahrumiki-default-rtdb.asia-southeast1.firebasedatabase.app/'

# Initialize FastAPI
app = FastAPI()
Result_Sender_router = APIRouter()

# Initialize Firebase
cred = credentials.Certificate(FIREBASE_CREDENTIALS_PATH)
firebase_admin.initialize_app(cred, {
    'databaseURL': FIREBASE_DB_URL
})

# API Endpoints
@Result_Sender_router.post("")
def upload_result(data: ResultModel):
    """Handle result upload to Firebase."""
    ref = db.reference(data.result)
    ref.set({
        'isFull': ResultModel.isFull
    })

    print(f"Kết quả đã được cập nhật: {data.result}")
    return {"message": f"Data received for {data.result}"}

