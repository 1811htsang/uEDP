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
- **[FCR]** Fatal Code Return: A mechanism for handling fatal errors and returning error codes, ensuring robust error handling and system stability.
- **[PLTF]** Portable Local Test Framework: A framework for writing and running tests with YAML syntax and a simple test runner, enabling easy testing and validation of application logic.
- **[PSE]** Pub/Sub Engine: A publish-subscribe mechanism for decoupled communication between tasks, allowing for flexible and scalable event handling.
- **[SIF]** Safe Input Filter: A mechanism for safely filtering and validating input data, ensuring that only valid and expected data is processed by the system.
- **[IOMS]** I/O Mapping Shell: A shell for mapping certain operations to I/O hardware, enabling command-based control and interaction with hardware components.

---

## From Framework to Kernel: The μE-OS Transition

Currently, μEDP is evolving into the kernel of μE-OS (micro Event-Driven Operating System). Version 1.2.0 will mark a major milestone: The Infrastructure Preparation. We are moving away from manual coding toward Model-Driven Development (MDD) using the μE-LS (Logical Syntax-izer).

μEDP will include the PLTF (Portable Local Test Framework), a robust infrastructure for testing and validating application logic with:

- μE-LS (Logical Syntax-izer): A YAML-based syntax to define Task behavior, Hybrid State Machines (TSM/FSM), and Global Data Areas (GDA).
- TLC (Test Level Coverager): Automated verification ranging from Unit (UT) and Component (CT) to System (ST) and Integration (IT) testing.
- UST (Unified Symbol Table): A centralized context engine that ensures consistency between Kconfig resources and Logic descriptors.

---

## 📝 Documentation

Information about the API, memory pool planning, and porting guides to other MCUs can be found in the [User Manual](./docs/user-manual.md). Please note that some documentation is still in progress, and the current version may not cover all features or details. The documentation will be updated as the project progresses, and contributions from the community are welcome to help improve and expand the documentation.

A comparison analysis between the event-driven model (μEDP/CIEDPC) and RTOS is available in [μEDP vs FreeRTOS](./docs/uedp-vs-freertos.md).

A detailed analysis between the μEDP/CIEDPC and the QP/C framework is available in [μEDP vs QP/C](./docs/uedp-vs-qpc.md).

If you want to see the documentation in progress, switch to the `docs` branch to view the documents that are currently being drafted and updated.

Please also note that the documentation is currently supporting Vietnamese, and English documentation will be added in the future as the project progresses or with contributions from the community. Currently, the documentation is quite limited to the resource of time and human to consilidate source code into a comprehensive document. However, the reminder in tasklist is always there to ensure the documentation task is not forgotten.

---

## 🤝 Contributing

This project is developed by **Shang Huang (Huynh Thanh Sang)**. Contributions for bug reports or feature proposals are welcome via GitHub Issues.

From v1.1.5, **Minminie06 (Nguyen Hoang Hai Minh)** has joined the project as a contributor, focusing on the development, refactor and documentation for any leftover features and improvements. Contributions from the community are always welcome, and we encourage you to submit pull requests for any enhancements or bug fixes.

**License:** MIT.
