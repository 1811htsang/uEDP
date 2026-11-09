cmake_minimum_required(VERSION 3.10)
project(ciedpc_test C)

# 1. Định nghĩa Biến Nền tảng (Mặc định là LINUX nếu không chỉ định)
set(PLATFORM "LINUX" CACHE STRING "Target platform: LINUX, STM32, ESP32")

# 2. Danh sách file nguồn LÕI (Không đổi dù sang nền tảng nào)
set(CORE_SOURCES
  ../../core/src/ciedpc_msg.c
  ../../core/src/ciedpc_task.c
  ../../core/src/ciedpc_timer.c
  ../../pal/service/memrp/pal_memrp.c
  ../../common/container/fifo/fifo.c
)

# 3. Cấu hình file nguồn theo NỀN TẢNG (PAL Implementation)
if(PLATFORM STREQUAL "LINUX")
  message(STATUS "Configuring for Linux Simulation...")
  set(PAL_SOURCES 
      ../../pal/arch/linux/linux_arch.c
  )
  # Thêm thư viện pthread cho Linux
  set(PLATFORM_LIBS pthread)
  add_definitions(-DCIEDPC_PLATFORM_LINUX)
elseif(PLATFORM STREQUAL "STM32")
  # Hiện tại chưa có implementation cho STM32, sẽ thêm sau
else()
  message(FATAL_ERROR "Unsupported platform: ${PLATFORM}")
endif()

# 4. Gom tất cả file nguồn lại
set(SOURCES
  test.c
  ${CORE_SOURCES}
  ${PAL_SOURCES}
)

# 5. Tạo mục tiêu thực thi
add_executable(${PROJECT_NAME} ${SOURCES})

# 6. Cấu hình Include Directories
target_include_directories(${PROJECT_NAME} PRIVATE
  ../../core/inc/
  ../../common/container/fifo/
  ../../pal/
  ../../pal/arch/linux/   # Cần include cho từng arch cụ thể
  ../../pal/service/memrp/
  ./
)

# 7. Liên kết thư viện (Chỉ có hiệu lực trên Linux)
if(PLATFORM_LIBS)
  target_link_libraries(${PROJECT_NAME} ${PLATFORM_LIBS})
endif()

# Cấu hình Debug mode
set(CMAKE_BUILD_TYPE Debug)