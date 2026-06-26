# μEDP: micro Event-Driven Programming Framework (formerly CIEDPC)

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Language: C](https://img.shields.io/badge/Language-Pure%20C-blue.svg)
![Tools: GCC/GDB/CMake](https://img.shields.io/badge/Tools-GCC%20%7C%20GDB%20%7C%20CMake-lightgrey.svg)
![Platform: Agnostic](https://img.shields.io/badge/Platform-STM32%20|%20ESP32%20|%20Linux-green.svg)
![Status: Active Development](https://img.shields.io/badge/Status-Active%20Development-orange.svg)

**μEDP** (formerly CIEDPC) is a lightweight, high-performance, event-driven programming framework based on the **Active Object** model with a focus on real-time embedded systems. It is designed to be platform-agnostic, OS-centric feature-rich, and highly efficient, making it suitable for a wide range of applications in the embedded domain.

The core goal is to achieve **"Zero-Touch Porting"** — enabling the porting of application logic between platforms (STM32, ESP32, Linux) without changing the core source code.

Future development plans insist on embedding μEDP as the Kernel of a new μE-OS (micro Event-Driven Operating System) that will be built on top of the μEDP framework, providing additional OS-level features while maintaining the core principles of event-driven programming.

Video demonstrations of μEDP in action can be found at [docs/videos](./docs/videos/uedp-introduction-v112.webm).

Also, documentations refer to section [Documentation](#-documentation) for more details.

Feel free to star the projetct and contribute to its development. Your support is greatly appreciated!

---

## 🚀 Key Features - Released & Future Development

- **[SAD]** Separate Architecture Design: Clear layering between App Layer — μEDP Framework — PAL (Platform Abstraction Layer).
- **[PSP]** Priority Scheduling Policy: Priority-based multitasking scheduling using bitmasks, optimizing response time for events.
- **[APE]** Safe LIFO-nested FIFO Atomic Priority Escalation : Support for temporary priority escalation for urgent tasks, ensuring critical events are handled promptly without preemption issues using S-LnF (Safe LIFO-nested FIFO) mechanism.
- **[DMP]** Deterministic Memory Pooling: Minimizing fragmentation and ensuring deterministic behavior for real-time systems with automatic atomic void size scaling of memory pools.
- **[D2MP]** Data-to-Message Passing : Support for passing values and references (zero-copy), automatically adapting to 32/64-bit pointer sizes.
- **[HSMC]** Hybrid State Machine Control: Integration of mode management (TSM) and micrologic (FSM) for clear system organization.
- **[PPLP]** Plug-N-Play Logging Pipeline: Three-layer logging system `itnlog` → `logdp` → `rprintf/xprintf` supporting safe log collection and forwarding from Core to backend.
- **[MPS]** Modular Porting Support: Abstracted hardware access and services in the PAL, enabling easy porting to new platforms with predefined interfaces and configurations.
- **[OCE]** Out-Context Execution Service: Support for executing tasks in an out-of-context manner, allowing for flexible task management and execution.
- **[IOMS]** I/O Mapping Shell: A shell for mapping certain operations to I/O hardware, enabling command-based control and interaction with hardware components.
- **[PSE]** Pub/Sub Engine: A publish-subscribe mechanism for decoupled communication between tasks, allowing for flexible and scalable event handling.
- **[SOCI]** Safe Out-Core Interaction: Ensuring safe and efficient interaction between the Core infrastructure and the external data flow, preventing issues such as data corruption and ensuring Core stability.

---

## 🏗 System Architecture

```mermaid
graph LR
    subgraph App[Application Layer]
        Config[App Config app_cfg.h]
        Declaration[App Logic declaration/]
        Logic[Main App Logic app.c]
    end

    subgraph Core[μEDP Core]
        Task[Task Scheduler & Task Objects]
        Msg[Message Pools & Manager]
        Timer[Timer Service]
        SM[TSM/FSM Engine]
        Itnlog[Event Logger itnlog]
    end

    subgraph PAL[Platform Abstraction Layer]
        Arch[Architecture-Specific HAL]
        Logdp[Log Dispatcher logdp]
        Rprintf[redirectable printf]
        Memrp[Memory Profiler memrp]
    end

    PAL -->|Hardware Access| Core
    Core -->|Event-Driven API| App
    App -->|Configuration| Core
    App -->|Configuration| PAL
    App -->|Logging| PAL
    Core -->|Logging| PAL
```

---

## 📂 Directory Structure

```text
μEDP/
├── core/                        # Định nghĩa và triển khai logic chính của μEDP
│   ├── inc/                     # uedp_msg.h, uedp_task.h, uedp_timer.h, uedp_fsm.h, uedp_tsm.h
│   │   └── uedp_core.h          # Định nghĩa các tín hiệu, hằng số và cấu trúc dữ liệu cốt lõi của μEDP
│   └── src/                     # Triển khai logic scheduler, timer engine, message manager
├── pal/                         # BACKEND (Lớp trừu tượng)
│   ├── pal_core.h               # Khai báo thống nhất chung cho toàn bộ PAL và các dịch vụ hệ thống
│   ├── services/                # Hardware Services (Mapping phần cứng)
│   │   ├── logdp/               # pal_logdp.h chứa các khai báo API log để triển khai bộ dispatch log ra nhiều backend
│   │   ├── memrp/               # pal_memrp.h chứa các khai báo API memory profiling để triển khai trên từng nền tảng
│   │   └── rprintf/             # pal_rprintf.h chứa các khai báo API rprintf để triển khai trên từng nền tảng
│   └── arch/                    # Implementation (Mã nguồn chi tiết từng chip)
│       └── .../                 # Mỗi nền tảng sẽ có một thư mục riêng để triển khai
├── app/                         # Định nghĩa logic ứng dụng, bao gồm các tác vụ và FSM do người dùng tạo ra
│   ├── config/                  # Chứa cấu hình ứng dụng, Core và PAL
│   ├── declaration/             # Khai báo các thiết kế cho logic nghiệp vụ
│   ├── interface/               # Chứa các triển khai cho truyền tín hiệu từ ngoài vào Core
│   ├── kconfig/                 # Chứa các cấu hình cho ứng dụng bằng Kconfig
│   └── app.c                    # Implementation chính của logic hoạt động của ứng dụng người dùng
├── common/                      # Các tiện ích và cấu trúc dữ liệu chung được sử dụng trong toàn bộ dự án
│   ├── container/               # Các cấu trúc dữ liệu như FIFO, Ring Buffer, Linked List được triển khai thuần C
│   ├── kconfiglib/              # Chứa cấu hình thực thi Kconfig terminal
│   ├── pyspec/                  # Cấu hình python để sinh code từ Kconfig terminal
│   └── xprintf/                 # Thư viện xprintf sử dụng cho việc format chuỗi log và xuất ra nhiều backend khác nhau
└── test/                        # Các test case mẫu để kiểm tra các tính năng của μEDP
    ├── test01/                  # Test cơ bản với các tác vụ ISR và TSM 
    ├── test02/                  # Test với các tính năng như message pooling và memrp
    ├── test03/                  # Test với các tính năng như message pooling và memrp
    └── test04/                  # Test với tính năng itnlog
```

---

## 📝 Documentation

Information about the API, memory pool planning, and porting guides to other MCUs can be found in the [User Manual](./docs/user-manual.md). Please note that some documentation is still in progress, and the current version may not cover all features or details. The documentation will be updated as the project progresses, and contributions from the community are welcome to help improve and expand the documentation.

A comparison analysis between the event-driven model (μEDP/CIEDPC) and RTOS is available in [μEDP vs FreeRTOS](./docs/uedp-vs-freertos.md).

A detailed analysis between the μEDP/CIEDPC and the QP/C framework is available in [μEDP vs QP/C](./docs/uedp-vs-qpc.md).

If you want to see the documentation in progress, switch to the `docs` branch to view the documents that are currently being drafted and updated.

Please also note that the documentation is currently supporting Vietnamese, and English documentation will be added in the future as the project progresses or with contributions from the community.

---

## 🤝 Contributing

This project is developed by **Shang Huang (Huynh Thanh Sang)**. Contributions for bug reports or feature proposals are welcome via GitHub Issues.

**License:** MIT.

---

## Future Development Roadmap

All design roadmaps are stored in [to-do.md](./docs/to-do.md) to track the progress and development plans for new features, improvements, and related documentation. For more detailed updates, switch to the `docs` branch to view the documents that are currently being drafted and updated.
