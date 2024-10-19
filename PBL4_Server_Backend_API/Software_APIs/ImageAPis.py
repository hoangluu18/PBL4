import uuid
from fastapi import FastAPI, UploadFile, File
from Data.ImageService import ImageService


app = FastAPI()



@app.get("/Image/")
def getImage(filePath):
    ImageSv = ImageService()




