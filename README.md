# ZERO HTTP – Web Server viết từ con số 0 bằng C++

Một **Web Server** được xây dựng hoàn toàn từ đầu bằng **C/C++**, sử dụng trực tiếp các hàm
socket chuẩn (`socket()`, `bind()`, `listen()`, `accept()`) — không dùng framework web có sẵn.

Server có thể:

- Lắng nghe kết nối từ trình duyệt (Chrome, Edge, Firefox…) trên cổng **8080**.
- Đọc và phân tích cú pháp **HTTP Request** (lấy ra Method, URL, phiên bản HTTP, headers, body).
- Trả về (**HTTP Response**) các file tĩnh có thật trên ổ cứng: `.html`, `.css`, `.js`, ảnh `.png/.jpg`… dưới dạng nhị phân cho trình duyệt hiển thị (request `GET`).
- Hỗ trợ **upload file** lên server qua request `POST` (`multipart/form-data`).
- Trả về trang **404 Not Found** khi file không tồn tại.
- Phục vụ **nhiều người dùng cùng lúc** bằng **thread pool** (mỗi request được xử lý bởi một luồng riêng).
- **Mã hóa dữ liệu bằng TLS/SSL** (HTTPS) qua thư viện OpenSSL — phần *Optional* của đề tài.

> Server chạy trên **HTTPS** với địa chỉ https://localhost:8080.

---

## 1. Yêu cầu môi trường

- **Hệ điều hành**: Linux/macOS (server dùng socket POSIX)
- **CMake** ≥ 3.20
- **Trình biên dịch** hỗ trợ **C++20** (g++ ≥ 10 hoặc clang++).
- **Git**
- **OpenSSL**
- **vcpkg**

Cài các gói hệ thống cần thiết:

```bash
# Fedora / RHEL
sudo dnf install gcc-c++ cmake git openssl-devel

# Ubuntu / Debian
sudo apt install build-essential cmake git libssl-dev

# macOS (Homebrew)
brew install cmake git openssl
```

---

## 2. Chạy dự án

### Bước 1 — Tải mã nguồn và vcpkg

```bash
git clone git@github.com:BinhPhuc/HTTPServer.git
cd HTTPServer

# Tải và cài đặt vcpkg (chỉ làm 1 lần)
git clone https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh        # macOS/Linux
```

### Bước 2 — Tạo chứng chỉ TLS cho HTTPS

Server dùng HTTPS nên cần một cặp chứng chỉ. Script bên dưới tự tạo sẵn:

```bash
./script/gen_certs.sh
```

Lệnh này sinh ra thư mục `certs/` gồm:
- `cert.pem`, `key.pem` — server tự động nạp khi chạy.
- `rootCA.pem` — chứng chỉ gốc để **import vào trình duyệt** (xem Bước 4).

### Bước 3 — Build và chạy server

```bash
./script/build_and_run.sh
```

Script sẽ tự động: cấu hình CMake + vcpkg → tải `nlohmann-json`, `spdlog` → build → chạy `./build/server`.

### Bước 4 — Truy cập và test

**Cách nhanh nhất bằng `curl`** (dùng `-k` để bỏ qua cảnh báo chứng chỉ tự ký):

```bash
# GET trang chủ (HTML)
curl -k https://localhost:8080/index.html

# GET file tĩnh (CSS / JS / ảnh)
curl -k https://localhost:8080/style.css

# Gọi API trả JSON
curl -k https://localhost:8080/api/users

# Test 404 với file không tồn tại
curl -k https://localhost:8080/khong-ton-tai.html

# Upload file lên server (POST /upload)
curl -k -F "file=@public/style.css" https://localhost:8080/upload
```

File upload thành công sẽ được lưu vào thư mục `public/` (tên file được thêm timestamp để tránh trùng).

**Bằng trình duyệt (Chrome/Edge):**

1. Import `certs/rootCA.pem` vào trình duyệt để không bị cảnh báo bảo mật
2. Mở `https://localhost:8080/index.html`

> Nếu không muốn import chứng chỉ, bạn vẫn có thể dùng Postman với uncheck ```SSL certificate verification``` trong setting

---

## 3. Chạy thủ công (không dùng script)

```bash
# Tạo chứng chỉ TLS (nếu chưa có)
./script/gen_certs.sh

# Cấu hình và build
mkdir -p build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . -j$(nproc)

# Chạy server
./server
```

---

## 4. Cấu trúc thư mục

```
HTTPServer/
├── public/          # Web root: các file html, css, js, ảnh được server trả về
├── certs/           # Chứng chỉ TLS (sinh ra bởi script/gen_certs.sh)
├── include/         # File header (.hpp)
├── src/             # Mã nguồn (.cpp)
│   ├── network/     # Server socket + định tuyến (ApiRouter)
│   ├── handler/     # Phân tích request, dựng response
│   ├── thread_pool/ # Thread pool xử lý đa luồng
│   ├── tls/         # Mã hóa TLS/SSL (OpenSSL)
│   └── main.cpp     # Điểm bắt đầu chương trình
├── script/          # Script tiện ích (build, run, gen_certs)
├── logs/            # Log server (server.log)
├── CMakeLists.txt   # Cấu hình build
└── vcpkg.json       # Khai báo thư viện phụ thuộc
```

Một vài tham số cấu hình nằm trong `include/utils/Constants.hpp`:

- `PORT = 8080` — cổng lắng nghe.
- `SERVER_ROOT_FOLDER = "public"` — thư mục web root.
- `MAX_UPLOAD_SIZE = 10MB` — kích thước file upload tối đa.

---

## 5. Thêm thư viện mới

Dự án dùng **vcpkg** để quản lý thư viện.

1. Thêm tên thư viện vào mảng `dependencies` trong [`vcpkg.json`](vcpkg.json).
2. Thêm `find_package(...)` và `target_link_libraries(...)` tương ứng trong [`CMakeLists.txt`](CMakeLists.txt).
3. Build lại: `./script/build_and_run.sh` — vcpkg sẽ tự tải thư viện ở lần build đầu.

> Lưu ý: tên gói trong `vcpkg.json` có thể khác tên dùng trong `find_package`. Hãy tham khảo tài liệu của thư viện.

---

## 6. Hỗ trợ IDE (LSP / clangd)

Script build tự sinh `compile_commands.json` và tạo symlink ở thư mục gốc, giúp các editor
(Neovim, VSCode…) có gợi ý code, go-to-definition và bắt lỗi chính xác.

```
# Manual setup (if needed)
# Generate compile_commands.json
cd build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
         -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake

# Create symlink at project root
cd ..
ln -sf build/compile_commands.json .
```