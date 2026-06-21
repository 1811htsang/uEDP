/**
 * @file linux_arch.c
 * @author Shang Huang
 * @brief Implementation of Linux Architecture Abstraction Layer for UEDP
 * @version 0.1
 * @date 2026-04-20
 * @copyright MIT License
 */

 /**
  * @brief Định nghĩa _POSIX_C_SOURCE để sử dụng các tính năng POSIX như clock_gettime và nanosleep
  * @attention Việc định nghĩa này phải được thực hiện trước khi bao gồm bất kỳ header nào để đảm bảo các hàm 
  *            và kiểu dữ liệu cần thiết được khai báo đúng cách
  */
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 199309L

/**
 * @brief Khai báo thư viện sử dụng
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "linux_arch.h"
#include "uedp_core.h"
#include "uedp_task.h"

/**
 * @brief Đảm bảo uedp_timer_tick được biết đến
 */

extern void uedp_timer_tick(void);

/**
 * @brief Khai báo biến mutex để hỗ trợ cơ chế khóa trong môi trường đa luồng của Linux
 */

static pthread_mutex_t uedp_mutex;
static pthread_mutexattr_t mutex_attr;
static ui32 start_tick_ms = 0;

/* --- IMPLEMENTATION CHO UEDP_CORE.H --- */

void uedp_core_init(void) {
    pal_core_init();
    /* Khởi tạo các thành phần khác của Core nếu cần */
}

/**
 * @brief Implementation cho uedp_core.h
 */

void pal_core_init(void) {
    pal_linux_init_env();
}

void pal_enter_critical(void) {
    pthread_mutex_lock(&uedp_mutex);
}

void pal_exit_critical(void) {
    pthread_mutex_unlock(&uedp_mutex);
}

ui8 pal_math_get_highest_bit32(ui32 mask) {
    if (mask == 0) return 0xFF;
    /* 31 - CLZ của 32-bit mang lại vị trí bit cao nhất (0-15) */
    return (ui8)(31 - __builtin_clz((uint32_t)mask));
}

ui32 pal_sys_get_tick(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ui32 current_ms = (ui32)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    return current_ms - start_tick_ms;
}

void pal_sys_reset(void) {
    printf("[System] Linux Simulation: Performing System Reset...\n");
    pal_linux_cleanup();
    /* Trong mô phỏng, reset thường là khởi động lại tiến trình hoặc thoát */
    exit(0); 
}

void pal_sys_fatal(const char* file, ui32 line, const char* msg) {
    fprintf(stderr, "\n[FATAL ERROR] %s\n", msg);
    fprintf(stderr, "Location: %s:%u\n", file, line);
    pal_linux_cleanup();
    abort(); // Tạo core dump để debug
}

/**
 * @brief Implementation cho linux_arch.h
 * @note Có thể xóa hoặc bổ sung các hàm khác tùy theo nhu cầu của dự án
 */

void pal_linux_init_env(void) {
    /* 1. Khởi tạo Mutex hỗ trợ khóa lồng nhau (Recursive) */
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&uedp_mutex, &mutex_attr);

    /* 2. Đăng ký xử lý tín hiệu hệ thống Ctrl+C */
    signal(SIGINT, pal_signal_handler);

    /* 3. Lưu mốc thời gian bắt đầu */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    start_tick_ms = (ui32)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void pal_linux_simulate_interrupt(ui8 task_id, ui8 signal) {
    /* Giả lập việc nạp tín hiệu từ ngoại vi vào Bridge của Core */
    uedp_task_norm_post_isr(task_id, signal);
}

/* Hàm này phải match với pthread callback */
void* pal_linux_simulate_tick_thread(void* arg) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000000; // 1ms

    while (1) {
        nanosleep(&ts, NULL);
        /* Gọi nhịp đập của Core */
        uedp_timer_tick(); 
    }
    return NULL;
}

void pal_linux_cleanup(void) {
    pthread_mutex_destroy(&uedp_mutex);
    pthread_mutexattr_destroy(&mutex_attr);
}

void pal_signal_handler(int signum) {
    if (signum == SIGINT) {
        printf("\n[System] Caught SIGINT (Ctrl+C), exiting gracefully...\n");
        pal_linux_cleanup();
        exit(0);
    }
}