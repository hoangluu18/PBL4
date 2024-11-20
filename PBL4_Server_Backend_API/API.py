import uvicorn
from fastapi import FastAPI

# Import app instances from each module
from Software_APIs.notification_sender import app as software_app
from Hardware_APIs.test import app as hardware_app

# Create a new FastAPI instance
app = FastAPI()

# Include each app as a router with a unique prefix
app.include_router(software_app.router, prefix="/Software_APIs")
app.include_router(hardware_app.router, prefix="/Hardware_APIs")

if __name__ == '__main__':
    uvicorn.run(app, host="0.0.0.0", port=8000)
