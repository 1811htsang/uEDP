# To-do list

Đây là nơi lưu trữ các công việc cần làm liên quan đến dự án CIEDPC cũng như các ghi chú và ý tưởng phát triển trong tương lai.

## Ghi chú phát triển

- Bổ sung lộ trình phát triển để đưa CIEDPC từ core EDP thành 1 OS hoàn chỉnh theo tiêu chuẩn Semantic Versioning.
- Bổ sung các đối chiếu về đánh giá thiết kế của CIEDPC với mô hình chuẩn là QP/C của Miro Samek.
- Bổ sung tài liệu để mention quá trình chuyển đổi thiết kế từ dependent của EPCB-vn AKEDP sang CIEDPC
- Bổ sung tài liệu phân tích thiết kế của HyperPanelOS và QP/C chi tiết hơn để rút ra các bài học kinh nghiệm và áp dụng vào thiết kế của CIEDPC.
- Quy ước: thay đổi tài liệu có thể được phát hành như patch version nếu ảnh hưởng trực tiếp đến release deliverable hoặc cách sử dụng hệ thống.

## Công việc cần làm

### Phiên bản 1.0.0: The Foundation

- [x] Hoàn thiện phân tích lõi tham chiếu AKEDP hiện có để đưa ra tài liệu phân tích & đề xuất cải tiến.
- [x] Remove các obsolete code và thay thế bằng cấu trúc thư mục + cấu trúc code mới.
- [x] Hoàn thiện thông tin core gốc
- [x] Hoàn thiện message pool implementation và tích hợp vào lõi CIEDPC.
- [x] Hoàn thiện FSM, TSM làm cơ sở để implement vào task driver.
- [x] Hoàn thiện task driver implementation và tích hợp vào lõi CIEDPC.
- [x] Hoàn thiện timer driver implementation và tích hợp vào lõi CIEDPC.
- [x] Hoàn thiện ISR Bridge implementation và tích hợp vào lõi CIEDPC.
- [x] Thiết kế testing chức năng trên task, timer, isr, message pool, tsm.
- [x] Bổ sung thiết kế testing MSG allocation cho task.
- [x] Bổ sung thiết kế testing FSM cho task.
- [x] Sửa đổi bổ sung thiết kế hàm khởi tạo FSM để tránh cyclic dependency giữa FSM, message pool và task driver.
- [x] Bổ sung API cho tương tác với task poll.
- [x] Bổ sung driver memrp để thực hiện memory profiling và tối ưu hóa memory footprint của lõi CIEDPC.
- [x] Viết tài liệu hướng dẫn sử dụng và phát triển lõi CIEDPC.
- [x] Kiểm tra các data types sử dụng nhằm thu gọn memory footprint.
- [x] Bổ sung linting và code formatting để đảm bảo codebase sạch sẽ và dễ đọc.

### Phiên bản 1.0.1: The Launch

- [x] Bổ sung template code cho phần app layer để làm ví dụ cho việc phát triển ứng dụng trên nền tảng CIEDPC.
- [x] Import thiết kế vào STM32 nhằm thử nghiệm thực tế trên phần cứng.
- [x] Bổ sung RAM profiling cho memrp để đánh giá hiệu quả sử dụng bộ nhớ của lõi CIEDPC trên đa nền tảng.
- [x] Import các template code vào lại source code để ra mắt phiên bản 1.0.1 của lõi CIEDPC.
- [x] Bổ sung tài liệu để hướng dẫn phát triển với nền tảng CIEDPC trên STM32, bao gồm hướng dẫn cài đặt môi trường phát triển, cấu hình phần cứng và ví dụ code.
- [x] Loại bỏ user-manual bản PDF do tốn thời gian căn chỉnh ngắt trang và thay thế tạm thời bằng markdown file để dễ dàng cập nhật và chỉnh sửa trong quá trình phát triển.
- [x] Bổ sung các tài liệu phân tích và thiết kế chi tiết cho các module của lõi CIEDPC từ lỗi AKEDP, bao gồm FSM, TSM, task driver, timer driver và ISR bridge đã thất lạc vào thời điểm hoàn thiện phiên bản 1.0.0.
- [x] Ra mắt phiên bản 1.0.1 của lõi CIEDPC với đầy đủ tài liệu hướng dẫn sử dụng và phát triển trên STM32.

### Phiên bản 1.0.2: The Internal Logger

- [x] Hoàn thiện test case để thống nhất định dạng và quy trình testing cho các module của lõi CIEDPC. Đã thực hiện điều này trong quá trình phát triển Core.
- [x] Bổ sung thiết kế 1 Internal Logger (itnlog) để thay thế printf debugging trong để hỗ trợ kit không có cổng UART.
- [x] Triển khai thiết kế logger với inline snapshot để lưu trực tiếp tsk/sig/fsm/tsm/msg và khai thác ring buffer để lưu log nội bộ ở runtime.
- [x] Sửa lỗi thiếu exit_critical trong timer khi timer_set
- [x] Loại bỏ hỗ trợ cho việc specify phân vùng trên pool để tránh xung đột thiết kế khi import vào ESP32.
- [x] Chuyển đổi các global variable từ uninitialize sang initialize để tránh vấn đề về memory footprint và phân vùng bộ nhớ.
- [x] Chuyển các size specifier của pool sang ciedpc_core để thống nhất và dễ dàng quản lý cấu hình pool size cho người dùng.
- [x] Bổ sung 1 file `core_Cfg.h` ở `app/config` để người dùng có thể cấu hình các thông số của lõi CIEDPC như pool size, task count, timer count, v.v. một cách dễ dàng mà không cần phải chỉnh sửa trực tiếp trong source code của lõi.
- [x] Bổ sung các khai báo PAL cho các kiến trúc STM32-F103, ESP32-S3, ESP32-WR32 để hỗ trợ việc import vào các nền tảng này và chuẩn bị cho việc phát triển đa nền tảng trong tương lai.
- [x] Bổ sung logic thiết kế bảo vệ toàn vẹn dữ liệu với hash hoặc checksum để đảm bảo tính toàn vẹn của dữ liệu khi truyền qua các API như task_post_isr, đặc biệt là khi truyền dữ liệu lớn hoặc nhạy cảm.
- [x] Bổ sung cơ chế threshhold để tự động kích hoạt việc xuất log ra ngoài khi có sự kiện quan trọng hoặc khi log đạt đến một mức độ nhất định, nhằm hỗ trợ việc phân tích hiệu suất và xử lý sự cố hiệu quả hơn.
- [x] Bổ sung log abstraction của PAL để hỗ trợ lưu log nội bộ trên Flash hoặc Backup Data Register của RTC trên các nền tảng phần cứng.
- [x] Bổ sung integration test cho internal logger để đảm bảo tính ổn định và hiệu quả của cơ chế logging nội bộ.
- [x] Thiết kế getter để truyền dữ liệu từ internal logger ra ngoài màn hình hoặc UART.
- [x] Bổ sung integration test trên kit LXP723ZGP1V2 để đánh giá hiệu quả của internal logger trong môi trường thực tế và đảm bảo tính ổn định khi hoạt động trên phần cứng.
- [x] Bổ sung PAL config ở `app/config` để người dùng có thể cấu hình các thông số liên quan đến PAL, phục vụ cho các service mới của PAL ở phiên bản 1.0.3.
- [x] Thêm PAL cho kit LXP723ZGP1V2 để hỗ trợ việc triển khai và testing internal logger trên nền tảng này.
- [x] Bổ sung tài liệu hướng dẫn sử dụng internal logger, bao gồm cách cấu hình, cách sử dụng API để ghi log và các rule để đảm bảo log được ghi chính xác và có thể phân tích hiệu quả.
- [x] Bổ sung mục tài liệu để phân tích thiết kế dump log nằm ngoài core CIEDPC-μEDP để tận dụng out-context execution nhằm đảm bảo các tính chất của EDP.
- [x] Ra mắt phiên bản 1.0.2 của lõi CIEDPC với đầy đủ test case và tài liệu hướng dẫn sử dụng internal logger.

### Phiên bản 1.0.3: The Plug-N-Play Logging Pipeline

- [x] Sửa đổi UART_DMA_TX để thử nghiệm printf debugging trên STM32, nếu hiệu quả thì có thể giữ lại như một tùy chọn cho người dùng, nếu không hiệu quả thì sẽ loại bỏ và tập trung vào phát triển internal logger.
- [x] Bổ sung rprintf (redirect printf) để hỗ trợ chuyển hướng output của printf ra ngoài console của STM32. Đối với ESP32 đã có hỗ trợ sẵn nên không cần thiết kế thêm.
- [x] Bổ sung xprintf (eXtended printf) để hỗ trợ định dạng log nâng cao, bao gồm timestamp, task ID, signal ID, v.v. nhằm cung cấp thông tin chi tiết hơn trong các log được ghi lại.
- [x] Bổ sung logdp (log dispatcher) để tự động định tuyến log đến các đích khác nhau dựa trên mức độ ưu tiên hoặc loại log, ví dụ như gửi log quan trọng đến UART và log thông thường đến internal logger.
- [x] Bổ sung tài liệu thiết kế chi tiết cho các tính năng logdp, xprintf và rprintf để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của các tính năng này trong việc hỗ trợ việc phân tích hiệu suất và xử lý sự cố hiệu quả hơn.
- [x] Bổ sung tài liệu trình bày về giới hạn mà CIEDPC có thể được sử dụng trong hệ thống nhúng và khi nào nên cân nhắc sử dụng một hệ điều hành nhúng đầy đủ như FreeRTOS thay vì CIEDPC, nhằm giúp người dùng hiểu rõ hơn về phạm vi ứng dụng và lựa chọn phù hợp cho dự án của họ.
- [X] Import tài liệu yêu cầu phần mềm của mô hình QP/C của Miro Samek để làm tài liệu tham khảo cho việc phát triển các tính năng tracing trong CIEDPC.
- [x] Bổ sung tài liệu đối chiếu thiết kế của CIEDPC với mô hình chuẩn QP/C của Miro Samek để làm rõ các điểm tương đồng và khác biệt trong kiến trúc và cách tiếp cận.
- [x] Ra mắt phiên bản 1.0.3 của lõi CIEDPC với các tính năng redirect printf và xprintf, cùng với tài liệu hướng dẫn sử dụng và phân tích thiết kế chi tiết về giới hạn sử dụng của CIEDPC và đối chiếu với mô hình QP/C.

### Phiên bản 1.1.0: The Escalation Update

- [ ] Đổi tên CIEDPC thành μEDP (micro-EDP) với các API tương đồng để phản ánh rõ hơn về mục tiêu của dự án là một lõi điều phối nhẹ cho các hệ thống nhúng.
- [ ] Bổ sung tài liệu trình bày về cơ chế Priority Escalation và Scheduling Policy của CIEDPC để làm rõ cách thức hoạt động và lợi ích của cơ chế này trong việc xử lý các tình huống khẩn cấp và đảm bảo hiệu suất của hệ thống.
- [ ] Nâng cấp thiết kế phân phối task với API cho phép thực hiện cơ chế Priority Escalation để cho phép một task có thể tạm thời tăng độ ưu tiên của mình khi cần thiết và hoàn trả độ ưu tiên về mức ban đầu sau khi hoàn thành công việc khẩn cấp.
- [ ] Ra mắt phiên bản 1.1.0 của lõi μEDP với đầy đủ tính năng Priority Escalation và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.1.1: The Hardware Accelerated Scheduling

- [ ] Bổ sung các hạng mục bổ sung tài liệu thiết kế từ CIEDPC (μEDP) sang μE-OS với nâng cấp thiết kế bộ điều phối sử dụng phần cứng như NVIC - các bộ quản lý ngắt để tối ưu hiệu suất và giảm độ trễ trong việc xử lý các sự kiện thời gian thực.
- [ ] Hoàn thiện thiết kế chi tiết cho logic sử dụng của bộ điều phối phần cứng như NVIC và xử lý vấn đề về ISR nesting & preemption để đảm bảo hệ thống hoạt động ổn định và hiệu quả khi xử lý các sự kiện thời gian thực.
- [ ] Ra mắt phiên bản 1.1.1 của lõi μEDP với đầy đủ tài liệu thiết kế chi tiết cho bộ điều phối phần cứng và xử lý ISR nesting & preemption, chuẩn bị cho việc chuyển đổi sang μE-OS.

### Phiên bản 1.2.0: The Pub/Sub Engine

- [ ] Bổ sung tài liệu thiết kế chi tiết cho Pub/Sub engine để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của tính năng này trong việc hỗ trợ việc phát triển các ứng dụng phức tạp với nhiều tác vụ tương tác với nhau một cách linh hoạt hơn.
- [ ] Thiết kế và triển khai Publish-Subscribe (Pub/Sub) engine để một sự kiện có thể phát tới nhiều task đã đăng ký.
- [ ] Ra mắt phiên bản 1.2.0 của lõi μEDP với đầy đủ tính năng Pub/Sub engine và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.2.1: The Safe Out-Core Interaction

- [ ] Bổ sung tài liệu chi tiết cho thiết kế Safe Out-Core Interaction nhằm đảm bảo các tín hiệu đầu vào từ lõi được xử lý an toàn ở pool EXTAL trước khi được chuyển vào pool nội bộ của lõi, nhằm đảm bảo tính ổn định và an toàn của hệ thống khi xử lý các tín hiệu từ bên ngoài.
- [ ] Hoàn thiện thiết kế Safe Out-Core Interaction.
- [ ] Ra mắt phiên bản 1.2.1 của lõi μEDP với đầy đủ tính năng Safe Out-Core Interaction và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.2.2: The Secure Signal Injection

- [ ] Bổ sung tài liệu thiết kế SSI (Secure Signal Injection) để bổ trợ việc bảo vệ hệ thống khỏi các tín hiệu độc hại hoặc không mong muốn được truyền vào lõi thông qua các API như task_post_isr, nhằm đảm bảo tính ổn định và an toàn của hệ thống khi xử lý các tín hiệu từ bên ngoài.
- [ ] Thiết kế SSI (Secure Signal Injection) để bổ trợ việc bảo vệ hệ thống khỏi các tín hiệu độc hại hoặc không mong muốn.
- [ ] Ra mắt phiên bản 1.2.2 của lõi μEDP với đầy đủ tính năng SSI (Secure Signal Injection) và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.2.3: The Tickless Idle Mode

- [ ] Tích hợp Tickless Idle Mode vào PAL để tạm dừng tick hệ thống khi CPU ở trạng thái idle và không còn timer chờ xử lý.
- [ ] Bổ sung tài liệu thiết kế chi tiết cho Tickless Idle Mode để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của tính năng này trong việc tối ưu hóa hiệu suất và tiết kiệm năng lượng cho các hệ thống nhúng.
- [ ] Ra mắt phiên bản 1.2.2 của lõi μEDP với đầy đủ tính năng Tickless Idle Mode và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.3.0: The Task-as-File Control Interface

- [ ] Bổ sung tài liệu thiết kế chi tiết cho tính năng Task-as-File Control Interface để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của tính năng này trong việc hỗ trợ việc quản lý tác vụ một cách trực quan và linh hoạt hơn thông qua một giao diện giống như hệ thống file ảo.
- [ ] Thiết kế uvfs (Task-as-File Control Interface) như một giao diện để quản lý tác vụ như 1 hệ thống file ảo, cho phép người dùng tương tác với task, timer và message pool thông qua các lệnh giống như thao tác trên file system.
- [ ] Ra mắt phiên bản 1.3.0 của lõi μEDP với đầy đủ tính năng Task-as-File Control Interface và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.3.1: The Kconfig Configuration

- [ ] Bổ sung Kconfig để hỗ trợ cấu hình các tính năng của lõi μEDP một cách dễ dàng thông qua một giao diện cấu hình trực quan, giúp người dùng có thể tùy chỉnh các thông số của hệ thống mà không cần phải chỉnh sửa trực tiếp trong code.
- [ ] Bổ sung tài liệu thiết kế chi tiết cho Kconfig để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của tính năng này trong việc hỗ trợ việc cấu hình hệ thống một cách dễ dàng và trực quan hơn.
- [ ] Ra mắt phiên bản 1.3.1 của lõi μEDP với đầy đủ tính năng Kconfig và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.3.2: The Component Manager

- [ ] Xây dựng Component Manager để đóng gói các driver và module thành các CIEDPC Components tái sử dụng được.
- [ ] Bổ sung tài liệu thiết kế chi tiết cho Component Manager để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của tính năng này trong việc hỗ trợ việc quản lý và tái sử dụng các driver và module một cách hiệu quả hơn.
- [ ] Ra mắt phiên bản 1.3.2 của lõi μEDP với đầy đủ tính năng Component Manager và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.3.3: The Task Counter

- [ ] Bổ sung tài liệu thiết kế chi tiết cho Task Counter để phát hiện task bị treo và kích hoạt safe response nhằm đảm bảo tính ổn định và an toàn của hệ thống khi có sự cố xảy ra với các tác vụ.
- [ ] Hoàn thiện Task Counter để phát hiện task bị treo và kích hoạt safe response.
- [ ] Ra mắt phiên bản 1.3.3 của lõi μEDP với đầy đủ tính năng Task Counter và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.4.0: The Safe Heap Allocation

- [ ] Bổ sung tài liệu thiết kế thuật toán và thư viện cho Safe Heap Allocation để đảm bảo tính ổn định và an toàn của hệ thống khi thực hiện các thao tác cấp phát bộ nhớ động, đặc biệt là trong các tình huống có nhiều tác vụ cùng truy cập vào pool bộ nhớ.
- [ ] Thiết kế và triển khai thuật toán Safe Heap Allocation để hỗ trợ cấp phát bộ nhớ động một cách an toàn. Thuật toán này có thể sử dụng như First-fit hoặc Best-fit với cơ chế Coalescing để giảm fragmentation và đảm bảo hiệu quả sử dụng bộ nhớ, đồng thời có cơ chế bảo vệ để tránh các lỗi như double free hoặc memory leak.
- [ ] Bổ sung tài liệu thuật toán First-fit / Best-fit với Coalescing để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của thuật toán này trong việc hỗ trợ việc cấp phát bộ nhớ động một cách an toàn và hiệu quả hơn.
- [ ] Ra mắt phiên bản 1.4.0 của lõi μEDP với đầy đủ tính năng Safe Heap Allocation và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.4.1: The Execution Space Division

- [ ] Bổ sung tài liệu thiết kế chi tiết Execution Space Division nhằm phân chia không gian thực thi giữa các task, timer và ISR để đảm bảo tính ổn định và hiệu quả của hệ thống khi xử lý các sự kiện thời gian thực, đồng thời hỗ trợ việc phát triển các ứng dụng phức tạp với nhiều tác vụ tương tác với nhau một cách linh hoạt hơn.
- [ ] Thiết kế Execution Space Division để phân chia không gian thực thi giữa các task, timer và ISR, đảm bảo rằng các tác vụ có thể hoạt động một cách độc lập.
- [ ] Ra mắt phiên bản 1.4.1 của lõi μEDP với đầy đủ tính năng Execution Space Division và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.4.2: The MM/PU Integration

- [ ] Bổ sung tài liệu thiết kế 1 MMU (Memory Management Unit) hoặc MPU (Memory Protection Unit) để hỗ trợ việc bảo vệ bộ nhớ và phân vùng bộ nhớ một cách hiệu quả hơn, nhằm đảm bảo tính ổn định và an toàn của hệ thống khi xử lý các tác vụ có yêu cầu cao về bảo mật hoặc khi có nhiều tác vụ cùng truy cập vào pool bộ nhớ.
- [ ] Hoàn thiện thiết kế MMU/MPU để hỗ trợ việc bảo vệ bộ nhớ và phân vùng bộ nhớ một cách hiệu quả hơn.
- [ ] Ra mắt phiên bản 1.4.2 của lõi μEDP với đầy đủ tính năng MMU/MPU và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.4.3: The μE-OS Transition

- [ ] Bổ sung tài liệu đối chiếu lộ trình thiết kế của μE-OS với lộ trình phát triển của HyperPanelOS để làm rõ các điểm tương đồng và khác biệt trong cách tiếp cận phát triển hệ điều hành nhúng.
- [ ] Tìm hiểu và phân tích thiết kế của HyperPanelOS để rút ra các bài học kinh nghiệm và áp dụng vào thiết kế của μE-OS.
- [ ] Ra mắt phiên bản 1.4.3 của lõi μEDP với tài liệu đối chiếu thiết kế của μE-OS với HyperPanelOS.

### Sau phiên bản 1.4.3

Sau phiên bản này, CIEDPC (μEDP) sẽ bắt đầu chuyển đổi thành μE-OS với thiết kế sử dụng 2 lõi điều phối sử dụng phần cứng như NVIC - các bộ quản lý ngắt và phần mềm để tối ưu hiệu suất và giảm độ trễ trong việc xử lý các sự kiện thời gian thực.

Các hạng mục bổ sung tài liệu thiết kế từ CIEDPC (μEDP) sang μE-OS sẽ được cập nhật chi tiết hơn khi tiến trình chuyển đổi bắt đầu.

Các phiên bản sẽ được tách thành 1 repository mới với tên gọi μE-OS để phản ánh rõ hơn về mục tiêu của dự án là một hệ điều hành nhúng nhẹ, và sẽ tiếp tục phát triển theo lộ trình đã đề ra với các tính năng mới và cải tiến dựa trên thiết kế của HyperPanelOS, RTOS.
