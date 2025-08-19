# Hai file này là **thư viện header dạng single-file** rất phổ biến do **Sean Barrett (nothings)** viết, được dùng để đọc/ghi ảnh mà không cần phụ thuộc vào các thư viện phức tạp như libpng, libjpeg.

## 1. `stb_image.h`

👉 Dùng để **đọc (load)** các định dạng ảnh phổ biến (PNG, JPG, BMP, TGA, GIF, PSD, HDR, PIC, PNM, …) thành dữ liệu thô (mảng `unsigned char*`).

* Hàm thường dùng:

  ```c
  int stbi_load(const char *filename, int *x, int *y, int *channels_in_file, int desired_channels);
  void stbi_image_free(void *retval_from_stbi_load);
  ```

  * `filename`: tên file ảnh cần đọc
  * `x, y`: chiều rộng, chiều cao ảnh
  * `channels_in_file`: số kênh trong file gốc (3 = RGB, 4 = RGBA, …)
  * `desired_channels`: muốn load ra bao nhiêu kênh (ví dụ ép JPG (3 kênh) thành RGBA (4 kênh))

---

## 2. `stb_image_write.h`

👉 Dùng để **ghi (save)** ảnh ra các định dạng phổ biến như PNG, JPG, BMP, TGA, HDR.

* Hàm thường dùng:

  ```c
  int stbi_write_png(const char *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
  int stbi_write_jpg(const char *filename, int w, int h, int comp, const void *data, int quality);
  ```

  * `filename`: tên file ảnh output
  * `w, h`: chiều rộng, chiều cao
  * `comp`: số kênh (1=grayscale, 3=RGB, 4=RGBA)
  * `data`: mảng pixel (unsigned char\*)
  * `quality`: chất lượng (chỉ với JPG, từ 1–100)

---

## 3. Cách dùng

* Chỉ cần tải file `.h` về và đặt cùng thư mục project.
* Trong **một file .c/.cpp duy nhất**, bạn phải định nghĩa:

  ```c
  #define STB_IMAGE_IMPLEMENTATION
  #include "stb_image.h"

  #define STB_IMAGE_WRITE_IMPLEMENTATION
  #include "stb_image_write.h"
  ```
* Sau đó bạn có thể `#include "stb_image.h"` và `#include "stb_image_write.h"` ở các file khác mà không cần `#define` nữa.

---

👉 Tóm lại:

* `stb_image.h` = đọc ảnh (PNG/JPG/BMP/...)
* `stb_image_write.h` = ghi ảnh (PNG/JPG/BMP/...)

---


