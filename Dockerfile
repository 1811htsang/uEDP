# Sử dụng Python Slim (dựa trên Debian)
FROM python:3.13-slim

# Cài đặt kconfiglib
RUN pip install --no-cache-dir kconfiglib

# Cài đặt các gói cần thiết
WORKDIR /workspace

# Cài đặt biểu tượng màu sắc cho terminal
ENV TERM=xterm-256color

# Sao chép mã nguồn vào container
COPY . /workspace

# Gọi menuconfig khi container được chạy
CMD ["python", "uedp.py", "menuconfig"]