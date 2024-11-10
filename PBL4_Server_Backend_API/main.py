from fastapi import FastAPI
from Software_APIs.notification_sender import noti_router
from Hardware_APIs.test import test_router


app = FastAPI()

app.include_router(noti_router, prefix='/noti')
app.include_router(test_router, prefix='/test')

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)