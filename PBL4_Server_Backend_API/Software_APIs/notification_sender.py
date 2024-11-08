import firebase_admin
from fastapi import FastAPI
from firebase_admin import credentials, db

cred = credentials.Certificate(r"D:\Code\pbl44\PBL4\PBL4_Server_Backend_API\token.json")

firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://ahrumiki-default-rtdb.asia-southeast1.firebasedatabase.app/'
})

app = FastAPI()

# ref1 = db.reference('Bin_Paper')
# ref2 = db.reference('Bin_Plastic')
# ref3 = db.reference('Bin_Metal')
# ref4 = db.reference('Bin_Glass')


@app.post("/result/")
def increment_bin_status(ref : db.reference):
    data = request.get_json()
    current_value = ref.child('Emptiness').get()
    if current_value is None:
        current_value = 0
    updated_value = current_value + 1
    ref.child('Emptiness').set(updated_value)
    print(f"Dữ liệu đã được cập nhật: Emptiness = {updated_value}")

@app.post("/result/")
def bin_full(ref : db.reference):
    ref.child('isFull').set(True)
    print(f"Thung Rac Full")

def bin_empty(ref : db.reference):
    ref.child('isFull').set(False)
    print(f"Thung Rac Empty")
increment_bin_status(ref2)
bin_full(ref3)
bin_empty(ref4)