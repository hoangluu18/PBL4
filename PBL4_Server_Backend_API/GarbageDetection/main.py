#Imports
import os,io
import numpy as np
from tensorflow import image
import tensorflow_hub as hub
from PIL import Image
import keras

#Load Model Architecture with weights
model = keras.models.load_model("garbage_classification_model_inception.h5", custom_objects={'KerasLayer': hub.KerasLayer})

#Process the image and return Tensor
#Process the image and return Tensor
def process(image):
    image = image.resize((512, 384))  # Resize the image to the original size
    data=np.asarray(image)
    data=data/255.0
    data=np.clip(data, 0, 1)  # Ensure values are within the valid range
    return data
#Return Prediction Result
def predict(tensor):
    predictions=model(np.array([tensor]))
    res=np.argmax(predictions)
    return res

def index():
    file_path = r"D:\Code\pbl44\PBL4\PBL4_Server_Backend_API\paper-ball-with-white-background.jpg"
    with open(file_path, 'rb') as file:
        image_bytes=file.read()
        image=Image.open(io.BytesIO(image_bytes))
        tensor = process(image)
        prediction=predict(tensor)
        data={"prediction":int(prediction)}
        print(data)

if __name__ == "__main__":
    index()