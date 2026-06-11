# Tài liệu phân tích giới hạn sử dụng của uEDP và so sánh với FreeRTOS

Tài liệu này tập trung vào so sánh kỹ thuật và triết lý thiết kế để giúp kỹ sư hệ thống lựa chọn đúng công cụ cho đúng bài toán.

Lưu ý tên gọi:

- Trong source hiện tại, dự án đang dùng tên CIEDPC.
- Trong tài liệu roadmap, CIEDPC dự kiến đổi tên thành uEDP.
- Tài liệu này dùng tên uEDP để đồng bộ với định hướng uE-OS, nhưng API map sẽ tham chiếu tên CIEDPC hiện hành.

## 1. Tổng quan triết lý (Design Philosophy)

| Đặc điểm | uEDP (Active Object) | FreeRTOS (RTOS) |
| :--- | :--- | :--- |
| Mô hình | Event-Driven. Task chỉ chạy khi có sự kiện/message. | Sequential + Blocking. Task chạy liên tục hoặc bị block. |
| Cơ chế thực thi | Run-to-Completion (RTC). Không preempt tại mục logic phần mem của task. | Preemptive Scheduler. Task có thể bị ngắt ngang bởi task ưu tiên cao hơn. |
| Tổ chức stack | Single-stack theo luồng scheduler. | Multi-stack: mỗi task có stack riêng. |
| Tư duy lập trình | State-centric (FSM/TSM). | Thread-centric (task loop + delay/semaphore). |

## 2. So sánh khả năng kỹ thuật (Technical Capabilities)

### 2.1 Quản lý tác vụ và tài nguyên

uEDP:

- Tạo task theo bảng tĩnh (vd: `ciedpc_task_norm_create`, `ciedpc_task_poll_create`).
- Cấm blocking trong handler message-driven.
- Mô hình cho đổi trạng thái timer + transition (`ciedpc_timer_set` + FSM/TSM).

FreeRTOS:

- Tạo task bằng `xTaskCreate`/`xTaskCreateStatic`.
- Blocking là cơ chế mặc định (`vTaskDelay`, `xQueueReceive` with timeout, `xSemaphoreTake`).
- Phù hợp với lượng xử lý tuần tự dài và cần chờ tại điểm gọi.

### 2.2 IPC và đường dẫn dữ liệu

uEDP:

- Message-oriented qua pool tĩnh (blank/alloc/extal + ISR bridge).
- Hỗ trợ truyền theo value hoặc reference.
- Phù hợp cho dữ liệu nhỏ và sự kiện tần suất cao cần tính định trước bộ nhớ.

FreeRTOS:

- Queue, Semaphore, Mutex, EventGroup, StreamBuffer.
- Thường copy dữ liệu qua queue (deep copy), có thể tăng chi phí RAM và CPU nếu payload lớn.
- Hệ sinh thái middleware rộng (TCP/IP, TLS, file system, cloud SDK).

### 2.3 Timing, latency và jitter

uEDP:

- ISR bridge giữ ISR ngắn, đẩy việc xử lý về scheduler.
- Không có context switch theo task stack riêng -> giảm overhead scheduler.
- Độ trễ tổng thể phụ thuộc vào độ dài handler RTC đang chạy.

FreeRTOS:

- Latency phụ thuộc interrupt priority, critical section, context switch và số task sẵn sàng.
- Có khả năng preempt để ưu tiên task critical, nhưng đổi lại là chi phí scheduler/stack management.

### 2.4 Debug và memory profiling

uEDP:

- Một stack chính, dễ tính toán và theo dõi memory footprint tổng.
- Có thể profiling hệ thống qua PAL memrp (vd: `pal_memrp_get_sys_info`/`pal_memrp_report` tùy theo bản PAL).
- Itnlog + logdp/rprintf/xprintf giúp quan sát lượng sự kiện theo kiểu event-centric.

FreeRTOS:

- Cần tính stack cho từng task.
- Thường dùng `uxTaskGetStackHighWaterMark` để kiểm tra watermark và phòng stack overflow.
- Có nhiều công cụ ecosystem, nhưng effort setup profiling thường cao hơn trên dự án nhỏ.

## 3. Giới hạn của uEDP (uEDP Constraints)

### Giới hạn compute-bound trong RTC

- Nếu một handler chạy quá lâu (ví dụ xử lý FFT dài, parsing lớn trong 1 lần), toàn bộ event loop bị chậm.
- Giải pháp: cắt nhỏ thành nhiều bước state, mỗi message xử lý một lát cắt ngắn.

### Hạn chế thư viện bên thứ ba theo kiểu blocking

- Các thư viện cần delay dài, busy-wait hoặc lock cho dài không phù hợp trực tiếp.
- Giải pháp: tạo adapter bất đồng bộ dựa trên timer/event callback.
- Tuy nhiên cần lưu ý rằng, nếu khoảng delay nhỏ so với độ trễ RTC (thường thấy trên các HAL do NSX cung cấp) thì vẫn có thể được xử lý trong RTC mà không cần adapter phức tạp.

### Độ phức tạp thiết kế tăng theo số state

- Bài toán lớn cần kỹ năng modeling FSM/TSM để tránh state explosion.
- Giải pháp: phân cấp state machine, tách module và đặt quy ước signal rõ ràng.

### Không phải lựa chọn tối ưu cho bài toán "thread-heavy"

- Nếu cần nhiều luồng blocking độc lập, middleware RTOS-native hoặc SMP, chi phí adaptation của uEDP tăng nhanh.

## 4. Ma trận quyết định: Chọn uEDP hay FreeRTOS?

| Trường hợp sử dụng | uEDP | FreeRTOS |
| :--- | :---: | :---: |
| MCU RAM rất thấp (dưới 8KB) | Tốt | Hạn chế |
| Logic điều khiển state-heavy | Tốt | Cần kỷ luật code cao |
| Cần deterministic event flow gọn | Tốt | Phụ thuộc scheduler + preempt policy |
| Tích hợp WiFi/TCP-IP stack nặng | Cần effort porting | Tốt (ecosystem sẵn có) |
| Multi-thread true concurrent / multi-core RTOS features | Không phải mục tiêu chính | Tốt |
| Team quen lập trình blocking tuần tự | Khó tiếp cận ban đầu | Dễ tiếp cận hơn |

## 5. Bảng map API (FreeRTOS -> uEDP/CIEDPC)

| Tính năng | FreeRTOS API | uEDP Equivalent (CIEDPC hien tai) |
| :--- | :--- | :--- |
| Tạo task message-driven | `xTaskCreate`/`xTaskCreateStatic` | `ciedpc_task_norm_create` |
| Delay chờ đợi | `vTaskDelay` | `ciedpc_timer_set` + xử lý bảng FSM/TSM |
| Gửi dữ liệu giữa task | `xQueueSend` | `ciedpc_msg_alloc` + `ciedpc_task_norm_post_msg` |
| Bảo vệ vùng găng | `taskENTER_CRITICAL`/`taskEXIT_CRITICAL` | `pal_enter_critical`/`pal_exit_critical` |
| ISR gửi sự kiện | `xQueueSendFromISR` | `ciedpc_task_norm_post_isr` |

## 6. "Stack Watermark" là điểm khác biệt lớn

FreeRTOS:

- Mỗi task có stack riêng, cần sizing và theo dõi watermark để tránh overflow.
- Bài toán tuning stack thường tốn nhiều vòng đo/thử nghiệm.

uEDP:

- Một stack scheduler chính + bộ nhớ tính theo pool.
- Memory profiling hệ thống để quan sát tổng quan, giảm bài toán "đoạn stack từng task".
- Đây là lợi thế lớn cho dự án RAM nhỏ và đội ngũ ưu tiên tư duy đơn giản trong vận hành.

## 7. Kết luận ngắn gọn

Nên ưu tiên uEDP khi:

- Bài toán trung tâm là điều khiển theo sự kiện và trạng thái.
- Hệ thống cần bộ nhớ gọn, dễ dự đoán và ít blocking.
- Đội ngũ sẵn sàng đầu tư vào tư duy FSM/TSM.

Nên ưu tiên FreeRTOS khi:

- Cần hệ sinh thái middleware lớn, networking stack nặng và nhiều thư viện blocking sẵn có.
- Cần mô hình thread quen thuộc cho đội ngũ.
- Cần các tính năng RTOS đã trưởng thành trên nhiều dòng chip/SDK.

Tiêu chí quyết định cuối cùng không nằm ở "framework nào mạnh hơn", mà ở "bài toán hệ thống của bạn cần event-deterministic core hay thread-preemptive ecosystem".
