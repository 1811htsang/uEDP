# @file CMakeLists.txt
# @author Shang Huang
# @brief CMake configuration cho UEDP-uEDP import on ESP-IDF
# @version 1.0
# @date 2026-05-23
# @copyright MIT License

# @brief Định nghĩa các file nguồn chung cho tất cả kiến trúc
file(
  GLOB_RECURSE CORE_SRCS
  "sources/core/src/*.c"
)
file(
  GLOB COMMON_CONTAINER_FIFO_SRCS
  "sources/common/container/fifo/*.c" 
)
file(
  GLOB APP_SRCS
  "sources/app/app.c"
)
file(
  GLOB_RECURSE PAL_SERVICE_MEMRP_SRCS
  "sources/pal/service/memrp/*.c"
)
file(
  GLOB PAL_CORE_SRCS 
  "sources/pal/*.c"
)

# @brief CHỈ CHỌN file nguồn cho kiến trúc ESP32-WR32
# @attention Nếu có nhiều kiến trúc, bạn có thể sử dụng điều kiện để chọn file nguồn tương ứng
#        hoặc dựa theo tên thư mục ở pal/arch để chọn file nguồn phù hợp
file(
  GLOB PAL_ARCH_SRCS 
  "sources/pal/arch/<arch_spec>/*.c"
)

# @brief Đăng ký Component với hệ thống build của ESP-IDF
# @attention Bạn cần thay thế <arch_spec> bằng tên thư mục thực tế chứa file nguồn cho kiến trúc
# @note Có thể thêm các thư viện phụ thuộc khác của ESP-IDF vào phần REQUIRES
# @note Tra cứu tại https://github.com/espressif/esp-idf/tree/master/components 
#       để biết thêm các component có sẵn mà điền vào
idf_component_register(
  SRCS
    ${APP_SRCS}
    ${COMMON_CONTAINER_FIFO_SRCS}
    ${CORE_SRCS}
    ${PAL_SERVICE_MEMRP_SRCS}
    ${PAL_ARCH_SRCS}
    ${PAL_CORE_SRCS}
  INCLUDE_DIRS
    "sources/common/container/fifo"
    "sources/core/inc"
    "sources/pal"
    "sources/pal/service/memrp"
    "sources/pal/arch/<arch_spec>"
    "sources/app/config"
    "sources/app/declaration"
  PRIV_INCLUDE_DIRS
    "sources/core/src"
  REQUIRES
    app_update                  # Cần cho OTA
    esp_app_format              # Cần cho OTA
    bootloader_support          # Cần cho OTA
    esp_timer                   # Cần cho uedp_timer_tick
    driver                      # Cần cho GPIO/UART/I2C
    freertos                    # Cần cho task/sleep
    log                         # Cần cho logging
  WHOLE_ARCHIVE
)

# @brief Định nghĩa Macro toàn cục để Code biết đang chạy trên ESP32
target_compile_definitions(${COMPONENT_LIB} PUBLIC UEDP)