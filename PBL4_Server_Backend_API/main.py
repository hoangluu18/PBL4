import numpy as np
import tf


def process(image):
    data=np.asarray(image)
    data=data/255.0
    data=tf.image.resize(data,[224,224])
    return data

#Return Prediction Result
def predict(tensor):
    predictions=model(np.array([tensor]))
    res=np.argmax(predictions)
    return res