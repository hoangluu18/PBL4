import cv2
import numpy as np

# Đọc ảnh nền và ảnh hiện tại
background = cv2.imread(r'D:\Code\pbl44\PBL4\PBL4_Server_Backend_API\462562060_2303502776664157_7655114754085549995_n.png')
current_image = cv2.imread(r'D:\Code\pbl44\PBL4\PBL4_Server_Backend_API\cos.png')



# Kiểm tra xem ảnh có được nạp thành công không
if background is None:
    print("Không thể đọc được ảnh nền. Vui lòng kiểm tra đường dẫn.")
elif current_image is None:
    print("Không thể đọc được ảnh hiện tại. Vui lòng kiểm tra đường dẫn.")
else:
    # Đảm bảo hai ảnh có cùng kích thước bằng cách thay đổi kích thước của current_image theo ảnh background
    current_image_resized = cv2.resize(current_image, (background.shape[1], background.shape[0]))

    # Chuyển cả hai ảnh sang thang xám
    background_gray = cv2.cvtColor(background, cv2.COLOR_BGR2GRAY)
    current_gray = cv2.cvtColor(current_image_resized, cv2.COLOR_BGR2GRAY)
    print(current_gray)

    print("0000000")

    print(background_gray)

    # Tính toán sự khác biệt giữa ảnh nền và ảnh hiện tại
    difference = cv2.absdiff(background_gray, current_gray)

    # Áp dụng ngưỡng để tạo ảnh nhị phân (binary image) từ sự khác biệt
    _, thresholded = cv2.threshold(difference, 30, 255, cv2.THRESH_BINARY)

    # Đếm số điểm ảnh khác biệt
    different_pixels = np.sum(thresholded > 0)
    print("Số điểm ảnh khác biệt:", different_pixels)

    # Xác định ngưỡng để quyết định có vật thể hay không (giả sử ngưỡng là 1000)
    if different_pixels > 8000:
        print("Có vật thể xuất hiện.")
    else:
        print("Không có vật thể.")
