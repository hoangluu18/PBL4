from fastapi import FastAPI
from Software_APIs.ResultSender import Result_Sender_router
from Hardware_APIs.GarbageClassification import GarbageClassification_router
from Hardware_APIs.ObjectDetection import ObjectDetection_router


app = FastAPI()

app.include_router(Result_Sender_router, prefix='/result')
app.include_router(GarbageClassification_router, prefix='/garbage')
app.include_router(ObjectDetection_router, prefix='/object')

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)