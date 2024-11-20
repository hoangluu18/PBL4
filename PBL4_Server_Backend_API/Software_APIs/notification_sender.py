import firebase_admin
from fastapi import APIRouter
from firebase_admin import credentials, db
import fastapi
import uvicorn
from pydantic import BaseModel

# app = fastapi.FastAPI()
noti_router = APIRouter()
# Khởi tạo Firebase
cred = credentials.Certificate(r"D:\Code\pbl44\PBL4\PBL4_Server_Backend_API\token.json") # path to your token.json
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://ahrumiki-default-rtdb.asia-southeast1.firebasedatabase.app/'
})

# Định nghĩa cấu trúc JSON
class ResultModel(BaseModel):
    result: str
    isFull: bool
# Endpoint upload_result
@noti_router.post("/api/upload/result")
def upload_result(data: ResultModel):
    ref = db.reference(data.result)
    increment_bin_status(ref)
    if data.isFull:
        bin_full(ref, True)
    else:
        bin_full(ref, False)

    print(f"Kết quả đã được cập nhật: {data.result}")
    return {"message": f"Data received for {data.result}"}

# Hàm xử lý
def increment_bin_status(ref: db.Reference):
    current_value = ref.child('Emptiness').get()
    if current_value is None:
        current_value = 0
    updated_value = current_value + 1
    ref.child('Emptiness').set(updated_value)
    print(f"Dữ liệu đã được cập nhật: Emptiness = {updated_value}")

def bin_full(ref: db.Reference, isFull: bool):
    ref.child('isFull').set(isFull)
    print(f"Thùng rác đầy")

# Khởi động server
# if __name__ == '__main__':
#     uvicorn.run(app, host="0.0.0.0", port=8000)
