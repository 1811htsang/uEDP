cmake_minimum_required(VERSION 3.10)
project(uedp C)

# ANCHOR - Định nghĩa Biến Nền tảng
set(PLAT "TEST" CACHE STRING "Target: TEST, LINUX, STM32F103, STM32H723, ESP32S3")

# ANCHOR - Thu thập file nguồn chung
file(GLOB_RECURSE CORE_SRCS "sources/core/src/*.c"
  "sources/common/container/fifo/*.c"
  "sources/common/container/llist/*.c"
  "sources/common/container/ring_buffer/*.c"
  "sources/common/encryp/libcrc8/*.c"
)
file(GLOB COMMON_XPRINTF_SRCS "sources/common/xprintf/*.c")
file(GLOB APP_SRCS "sources/app/app.c")
file(GLOB_RECURSE PAL_SERVICE_SRCS
  "sources/pal/service/memrp/*.c"
  "sources/pal/service/logdp/*.c"
  "sources/pal/service/rprintf/*.c"
)

# ANCHOR - Cấu hình file nguồn theo NỀN TẢNG
set(PAL_PLATFORM_SRCS "") # Khởi tạo biến trống

if (PLAT STREQUAL "TEST")
  message(STATUS "[INFO]: Configuring for Test Build...")
  add_definitions(-DTEST_MODE)
elseif(PLAT STREQUAL "LINUX")
  message(STATUS "[INFO]: Configuring for Linux Simulation...")
  set(PAL_PLATFORM_SRCS "sources/pal/arch/linux/linux.c")
  set(PLATFORM_LIBS pthread m) # Thêm m (math) nếu cần
  add_definitions(-DLINUX_PLATFORM)
elseif(PLAT STREQUAL "STM32F103")
  # TASK - Add CMakeList.txt for BSP of STM32
  message(STATUS "[INFO]: Configuring for STM32F103 Build...")
  set(PAL_PLATFORM_SRCS "sources/pal/arch/stm32f103/stm32f103.c")
  add_definitions(-DSTM32F103_PLATFORM)
elseif(PLAT STREQUAL "STM32H723")
  message(STATUS "[INFO]: Configuring for STM32H723 Build...")
  set(PAL_PLATFORM_SRCS "sources/pal/arch/stm32h723/stm32h723.c")
  add_definitions(-DSTM32_PLATFORM)
elseif(PLAT STREQUAL "ESP32S3")
  message(STATUS "[INFO]: Configuring for ESP32S3 Build...")
  set(PAL_PLATFORM_SRCS "sources/pal/arch/esp32s3/esp32s3.c")
  add_definitions(-DESP32_PLATFORM)
else()
  message(FATAL_ERROR "[ERROR]: Unsupported platform: ${PLAT}")
endif()

# ANCHOR - Gom TẤT CẢ file nguồn lại (Sửa lỗi tên biến ở đây)
set(SOURCES
  ${CORE_SRCS}
  ${COMMON_CONTAINER_SRCS}
  ${COMMON_XPRINTF_SRCS}
  ${APP_SRCS}
  ${PAL_SERVICE_SRCS}
  ${PAL_PLATFORM_SRCS}
  # STUB - If later have main.c for test, can add here
)

# ANCHOR - Tạo mục tiêu thực thi
add_executable(${PROJECT_NAME} ${SOURCES})

# ANCHOR - Cấu hình Include Directories
target_include_directories(${PROJECT_NAME} PRIVATE
  "sources/common/container/fifo/"
  "sources/common/container/llist/"
  "sources/common/container/ring_buffer/"
  "sources/common/encryp/libcrc8/"

  "sources/common/xprintf/"
  
  "sources/core/inc/"

  "sources/pal/"
  "sources/pal/arch/linux/"
  "sources/pal/arch/stm32f103/"
  "sources/pal/arch/stm32h723/"
  "sources/pal/arch/esp32s3/"
  "sources/pal/service/memrp/"
  "sources/pal/service/logdp/"
  "sources/pal/service/rprintf/"

  "sources/app/config/"
  "sources/app/declaration/"
  
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

# ANCHOR - Liên kết thư viện
if(PLATFORM_LIBS)
  target_link_libraries(${PROJECT_NAME} ${PLATFORM_LIBS})
endif()

# ANCHOR - Cấu hình Debug mode
set(CMAKE_BUILD_TYPE Debug)

# ANCHOR - Cấu hình Compiler cho từng nền tảng
if (PLAT STREQUAL "STM32")
  # Ví dụ: set(CMAKE_C_COMPILER arm-none-eabi-gcc)
  set(CMAKE_C_COMPILER arm-none-eabi-gcc)
elseif (PLAT STREQUAL "ESP32")
  # Ví dụ: set(CMAKE_C_COMPILER xtensa-esp32-elf-gcc)
  idf_component_register(
    SRCS
      ${CORE_SRCS}
      ${COMMON_CONTAINER_SRCS}
      ${COMMON_XPRINTF_SRCS}
      ${APP_SRCS}
      ${PAL_SERVICE_SRCS}
      ${PAL_PLATFORM_SRCS}
    INCLUDE_DIRS
      "sources/common/container/fifo"
      "sources/common/container/llist"
      "sources/common/container/ring_buffer"
      "sources/core/inc"
      "sources/pal"
      "sources/pal/service/memrp"
      "sources/pal/service/logdp"
      "sources/pal/service/rprintf"
      "sources/pal/arch/<arch_spec>"
      "sources/app/config"
      "sources/app/declaration"
    PRIV_INCLUDE_DIRS
      "sources/core/src"
    REQUIRES
      # NOTE - Tự động thêm các thư viện cần thiết cho ESP32
      app_update                  # Cần cho OTA
      esp_app_format              # Cần cho OTA
      bootloader_support          # Cần cho OTA
      esp_timer                   # Cần cho uedp_timer_tick
      driver                      # Cần cho GPIO/UART/I2C
      freertos                    # Cần cho task/sleep
      log                         # Cần cho logging
    WHOLE_ARCHIVE
  )

  # NOTE - Định nghĩa Macro toàn cục để Code biết đang chạy trên ESP32
  target_compile_definitions(${COMPONENT_LIB} PUBLIC UEDP)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
