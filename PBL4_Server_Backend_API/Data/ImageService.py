import pyodbc
from ConnectDb import ConnectDb


class ImageService:
    def __init__(self):
        self.dbContext = ConnectDb()  # dbContext là một instance của ConnectDb
        self.dbContext.connect()    # Kết nối tới cơ sở dữ liệu khi khởi tạo

    def get_Image(self, filePath):
        try:
            query = "SELECT * FROM Images WHERE filePath = ?"
            self.dbContext.cursor.execute(query, (filePath,))
            result = self.dbContext.cursor.fetchall()  # Lấy tất cả kết quả trả về
            return result  # Trả về kết quả
        except pyodbc.Error as e:
            print(f"Lỗi khi lấy Image: {e}")
            return None

    def create_Image(self, filePath, dateCreated):
        try:
            query = "INSERT INTO Images (filePath, dateCreated) VALUES (?, ?)"
            self.dbContext.cursor.execute(query, (filePath, dateCreated))
            self.dbContext.conn.commit()
            print("Insert Image thành công.")
        except pyodbc.Error as e:
            print(f"Lỗi khi Insert Image: {e}")
        return None
    def fetch_Images(self):
        try:
            query = "SELECT * FROM Images"
            self.dbContext.cursor.execute(query)
            records = self.dbContext.cursor.fetchall()
            return records
        except pyodbc.Error as e:
            print(f"Lỗi khi lấy dữ liệu: {e}")
            return None
    def delete_Image(self, filePath):
        try:
            query = "DELETE FROM Images WHERE filePath=?"
            self.dbContext.cursor.execute(query, (filePath))
            self.dbContext.conn.commit()
            print("Delete Image thành công.")
        except pyodbc.Error as e:
            print(f"Lỗi khi Delete Image: {e}")
            return None

    def close(self):
        if self.dbContext.cursor:
            self.dbContext.cursor.close()
        if self.dbContext.conn:
            self.dbContext.conn.close()
        print("Kết nối đã được đóng.")