# 1. Khai báo phiên bản tối thiểu
cmake_minimum_required(VERSION 3.16)

# ANCHOR - Định nghĩa Biến Nền tảng (Phải đặt trước project() để xử lý Toolchain nếu cần)
set(PLAT "TEST" CACHE STRING "Target: TEST, LINUX, STM32F103, STM32H723, ESP32S3")

# Set Toolchain compiler TRƯỚC LỆNH project() nếu cross-compiling cho STM32
if (PLAT STREQUAL "STM32F103" OR PLAT STREQUAL "STM32H723")
  set(CMAKE_SYSTEM_NAME Generic)
  set(CMAKE_C_COMPILER arm-none-eabi-gcc)
  set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
  set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
endif()

project(uedp C ASM)

# ANCHOR - Bật xuất compile_commands.json cho VS Code IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# ANCHOR - Thu thập file nguồn chung
file(GLOB_RECURSE CORE_SRCS 
  "sources/core/src/*.c"
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

# ANCHOR - Khởi tạo các biến tích lũy
set(PAL_PLATFORM_SRCS "")
set(EXTRA_MCU_SRCS "")
set(PLATFORM_LIBS "")
set(MCU_INCLUDE_DIRS "")

# ANCHOR - Cấu hình theo NỀN TẢNG
if (PLAT STREQUAL "TEST")
  message(STATUS "[INFO]: Configuring for Test Build...")
  add_compile_definitions(TEST_MODE)
elseif(PLAT STREQUAL "LINUX")
  message(STATUS "[INFO]: Configuring for Linux Simulation...")
  set(PAL_PLATFORM_SRCS "sources/pal/arch/linux/linux.c")
  set(PLATFORM_LIBS pthread m)
  add_compile_definitions(LINUX_PLATFORM)
elseif(PLAT STREQUAL "STM32F103")
  set(MCU_FLAGS "-mcpu=cortex-m3 -mthumb -Os -ffunction-sections -fdata-sections")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MCU_FLAGS}" CACHE STRING "" FORCE)
  set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${MCU_FLAGS}" CACHE STRING "" FORCE)
  set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../STM32F103C8TX_FLASH.ld")

  set(MCU_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/../Core/Inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32F1xx_HAL_Driver/Inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/CMSIS/Device/ST/STM32F1xx/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/CMSIS/Include"
  )

  file(GLOB HAL_AND_CORE_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/../Core/Src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Drivers/STM32F1xx_HAL_Driver/Src/*.c"
  )

  file(GLOB STARTUP_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/../Core/Startup/*.s"
  )

  set(EXTRA_MCU_SRCS 
    ${HAL_AND_CORE_SRCS}
    ${STARTUP_SRC}
  )

  message(STATUS "[INFO]: Configuring for STM32F103 Build...")
  set(PAL_PLATFORM_SRCS "sources/pal/arch/stm32f103/stm32f103.c")
  add_compile_definitions(STM32F103_PLATFORM STM32F103xB USE_HAL_DRIVER)
elseif(PLAT STREQUAL "ESP32S3")
  message(STATUS "[INFO]: Configuring for ESP32S3 Standalone Build...")

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
      "sources/common/encryp/libcrc8"
      "sources/common/xprintf"
      "sources/core/inc"
      "sources/pal"
      "sources/pal/service/memrp"
      "sources/pal/service/logdp"
      "sources/pal/service/rprintf"
      "sources/pal/arch/esp32s3"
      "sources/app/config"
      "sources/app/declaration"
    PRIV_INCLUDE_DIRS
      "sources/core/src"
    REQUIRES
      # NOTE - Tự động thêm các thư viện cần thiết cho ESP32S3
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

else()
  message(FATAL_ERROR "[ERROR]: Unsupported platform: ${PLAT}")
endif()

# ANCHOR - Gom TẤT CẢ file nguồn
set(SOURCES
  ${CORE_SRCS}
  ${COMMON_XPRINTF_SRCS}
  ${APP_SRCS}
  ${PAL_SERVICE_SRCS}
  ${PAL_PLATFORM_SRCS}
  ${EXTRA_MCU_SRCS}
)

# ANCHOR - Tạo mục tiêu build
if (PLAT STREQUAL "TEST")
  add_library(${PROJECT_NAME} OBJECT ${SOURCES})
else()
  add_executable(${PROJECT_NAME} ${SOURCES})
endif()

# ANCHOR - Cấu hình Include Directories chung & riêng
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
  ${MCU_INCLUDE_DIRS}
)

# ANCHOR - Liên kết thư viện
if(PLATFORM_LIBS)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${PLATFORM_LIBS})
endif()

# ANCHOR - Linker Options cho STM32F103
if(PLAT STREQUAL "STM32F103")
  target_link_options(${PROJECT_NAME} PRIVATE
    -mcpu=cortex-m3
    -mthumb
    "-T${LINKER_SCRIPT}"
    --specs=nano.specs                  # Đặt duy nhất 1 lần tại đây
    --specs=nosys.specs
    -Wl,-Map=${PROJECT_NAME}.map,--cref
    -Wl,--gc-sections
    -static
    -Wl,--start-group -lc -lm -Wl,--end-group
  )
endif()