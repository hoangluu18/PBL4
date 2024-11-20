import firebase_admin
from fastapi import APIRouter
from firebase_admin import credentials, db
import fastapi
from pydantic import BaseModel
MAX_DISTANCE = 15
app = fastapi.FastAPI()
noti_router = APIRouter()
# Khởi tạo Firebase
cred = credentials.Certificate(r"D:\Code\pbl44\PBL4\PBL4_Server_Backend_API\token.json") # path to your token.json
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://ahrumiki-default-rtdb.asia-southeast1.firebasedatabase.app/'
})

# Định nghĩa cấu trúc JSON
class ResultModel(BaseModel):
    result: str
    emptiness: float

# Endpoint upload_result
@noti_router.post("/api/upload/result")
def upload_result(data: ResultModel):
    ref = db.reference(data.result)
    data.emptiness = cal(data.emptiness)
    ref.set({
        'emptiness': data.emptiness
    })

    print(f"Kết quả đã được cập nhật: {data.result}")
    return {"message": f"Data received for {data.result}"}

# Hàm xử lý
# xu li sau
def cal(data : float):
    return data * 100 / MAX_DISTANCE

# Khởi động server
# if __name__ == '__main__':
#     uvicorn.run(app, host="0.0.0.0", port=8000)
