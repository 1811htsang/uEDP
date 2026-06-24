# μEDP: micro Event-Driven Programming Framework (formerly CIEDPC)

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Language: C](https://img.shields.io/badge/Language-Pure%20C-blue.svg)
![Tools: GCC/GDB/CMake](https://img.shields.io/badge/Tools-GCC%20%7C%20GDB%20%7C%20CMake-lightgrey.svg)
![Platform: Agnostic](https://img.shields.io/badge/Platform-STM32%20|%20ESP32%20|%20Linux-green.svg)
![Status: Active Development](https://img.shields.io/badge/Status-Active%20Development-orange.svg)

**μEDP** (formerly CIEDPC) is a lightweight, high-performance, event-driven programming framework based on the **Active Object** model with a focus on real-time embedded systems. It is designed to be platform-agnostic, OS-centric feature-rich, and highly efficient, making it suitable for a wide range of applications in the embedded domain.

The core goal is to achieve **"Zero-Touch Porting"** — enabling the porting of application logic between platforms (STM32, ESP32, Linux) without changing the core source code.

Future development plans insist on embedding μEDP as the Kernel of a new μE-OS (micro Event-Driven Operating System) that will be built on top of the μEDP framework, providing additional OS-level features while maintaining the core principles of event-driven programming.

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
├── core/                 # Definition and implementation of the core logic of CIEDPC
│   ├── inc/              # Message, Task, Timer, Itnlog, FSM, TSM
│   │   └── uedp_core.h   # Definition of signals, constants and core data structures of μEDP
│   └── src/              # Implementation of scheduler logic, timer engine, message manager, FSM/TSM engine, etc.
├── pal/                  # BACKEND (Abstract Layer)
│   ├── pal_core.h        # Unified declaration for the entire PAL and system services
│   ├── services/         # Hardware Services (logdp, memrp, rprintf)
│   │   ├── logdp/        # pal_logdp.h contains the declaration for the Log Dispatcher service for routing logs
│   │   ├── memrp/        # pal_memrp.h contains the declaration for the memory profiling API
│   │   └── rprintf/      # pal_rprintf.h contains the declaration for the rprintf API for platform-specific implementation
│   └── arch/             # Implementation (Detailed source code for each chip)
│       ├── stm32f103/    # stm32_f103_arch.c/h contains the implementation functions for STM32F103
│       ├── stm32h723/    # stm32h723_arch.c/h contains the implementation functions for STM32H723
│       ├── esp32_wr32/   # esp32_wr32_arch.c/h contains the implementation functions for ESP32-WROOM-32
│       ├── esp32_s3/     # esp32_s3_arch.c/h contains the implementation functions for ESP32-S3
│       └── linux/        # linux_arch.c/h contains the implementation functions for Linux simulation
├── app/                  # Definition of application logic, including tasks and FSM created by the user
│   ├── config/           # Contains application configuration and user configuration
│   │   ├── app_cfg.h     # Contains required configurations such as task table, timer, signals, etc.
│   │   ├── core_cfg.h    # Contains required configurations for the core such as pool size, number of tasks, timers, etc.
│   │   └── pal_cfg.h     # Contains required configurations for the PAL such as pool size, number of services, etc.
│   ├── declaration/      # Main implementation of the user application logic
│   ├── interface/        # Definition and implementation of interfaces with external signals (task_if)
│   └── app.c             # Main implementation of the user application logic
└── common/               # Various utilities and common data structures used throughout the project
    ├── container/        # Data structures like FIFO, Ring Buffer, Linked List implemented in pure C
    └── xprintf/          # Custom xprintf library for advanced log formatting
```

---

## 📝 Documentation

Information about the API, memory pool planning, and porting guides to other MCUs can be found in the [User Manual](./docs/user-manual.md). Please note that some documentation is still in progress, and the current version may not cover all features or details. The documentation will be updated as the project progresses, and contributions from the community are welcome to help improve and expand the documentation.

A comparison analysis between the event-driven model (μEDP/CIEDPC) and RTOS is available in [μEDP vs FreeRTOS](./docs/uedp-vs-freertos.md).

A detailed analysis between the μEDP/CIEDPC and the QP/C framework is available in [μEDP vs QP/C](./docs/uedp-vs-qpc.md).

If you want to see the documentation in progress, switch to the `docs` branch to view the documents that are currently being drafted and updated. Please also note that the documentation is currently supporting Vietnamese, and English documentation will be added in the future as the project progresses or with contributions from the community.

---

## 🤝 Contributing

This project is developed by **Shang Huang (Huynh Thanh Sang)**. Contributions for bug reports or feature proposals are welcome via GitHub Issues.

**License:** MIT.

---

## Future Development Roadmap

All design roadmaps are stored in [to-do.md](./docs/to-do.md) to track the progress and development plans for new features, improvements, and related documentation. For more detailed updates, switch to the `docs` branch to view the documents that are currently being drafted and updated.
