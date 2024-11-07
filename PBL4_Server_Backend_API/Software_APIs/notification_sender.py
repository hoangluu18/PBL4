import firebase_admin
from firebase_admin import credentials, db

cred = credentials.Certificate(r"D:\Code\pbl44\PBL4\PBL4_Server_Backend_API\token.json")

firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://ahrumiki-default-rtdb.asia-southeast1.firebasedatabase.app/'
})

ref1 = db.reference('Bin_Paper')
ref2 = db.reference('Bin_Plastic')
ref3 = db.reference('Bin_Metal')
ref4 = db.reference('Bin_Glass')

def increment_bin_status(ref : db.reference):
    current_value = ref.child('Emptiness').get()
    if current_value is None:
        current_value = 0
    updated_value = current_value + 1
    ref.child('Emptiness').set(updated_value)
    print(f"Dữ liệu đã được cập nhật: Emptiness = {updated_value}")

def bin_full(ref : db.reference):
    ref.child('isFull').set(True)
    print(f"Thung Rac Full")

increment_bin_status(ref4)
bin_full(ref4)