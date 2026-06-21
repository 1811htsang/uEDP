# To-do list

Đây là nơi lưu trữ các công việc cần làm liên quan đến dự án μEDP cũng như các ghi chú và ý tưởng phát triển trong tương lai.

## Công việc cần làm

### Phiên bản 1.0.0: The Foundation

- [x] Hoàn thiện phân tích lõi tham chiếu AKEDP hiện có để đưa ra tài liệu phân tích & đề xuất cải tiến.
- [x] Remove các obsolete code và thay thế bằng cấu trúc thư mục + cấu trúc code mới.
- [x] Hoàn thiện thông tin core gốc
- [x] Hoàn thiện message pool implementation và tích hợp vào lõi μEDP.
- [x] Hoàn thiện FSM, TSM làm cơ sở để implement vào task driver.
- [x] Hoàn thiện task driver implementation và tích hợp vào lõi μEDP.
- [x] Hoàn thiện timer driver implementation và tích hợp vào lõi μEDP.
- [x] Hoàn thiện ISR Bridge implementation và tích hợp vào lõi μEDP.
- [x] Thiết kế testing chức năng trên task, timer, isr, message pool, tsm.
- [x] Bổ sung thiết kế testing MSG allocation cho task.
- [x] Bổ sung thiết kế testing FSM cho task.
- [x] Sửa đổi bổ sung thiết kế hàm khởi tạo FSM để tránh cyclic dependency giữa FSM, message pool và task driver.
- [x] Bổ sung API cho tương tác với task poll.
- [x] Bổ sung driver memrp để thực hiện memory profiling và tối ưu hóa memory footprint của lõi μEDP.
- [x] Viết tài liệu hướng dẫn sử dụng và phát triển lõi μEDP.
- [x] Kiểm tra các data types sử dụng nhằm thu gọn memory footprint.
- [x] Bổ sung linting và code formatting để đảm bảo codebase sạch sẽ và dễ đọc.

### Phiên bản 1.0.1: The Launch

- [x] Bổ sung template code cho phần app layer để làm ví dụ cho việc phát triển ứng dụng trên nền tảng μEDP.
- [x] Import thiết kế vào STM32 nhằm thử nghiệm thực tế trên phần cứng.
- [x] Bổ sung RAM profiling cho memrp để đánh giá hiệu quả sử dụng bộ nhớ của lõi μEDP trên đa nền tảng.
- [x] Import các template code vào lại source code để ra mắt phiên bản 1.0.1 của lõi μEDP.
- [x] Bổ sung tài liệu để hướng dẫn phát triển với nền tảng μEDP trên STM32, bao gồm hướng dẫn cài đặt môi trường phát triển, cấu hình phần cứng và ví dụ code.
- [x] Loại bỏ user-manual bản PDF do tốn thời gian căn chỉnh ngắt trang và thay thế tạm thời bằng markdown file để dễ dàng cập nhật và chỉnh sửa trong quá trình phát triển.
- [x] Bổ sung các tài liệu phân tích và thiết kế chi tiết cho các module của lõi μEDP từ lỗi AKEDP, bao gồm FSM, TSM, task driver, timer driver và ISR bridge đã thất lạc vào thời điểm hoàn thiện phiên bản 1.0.0.
- [x] Ra mắt phiên bản 1.0.1 của lõi μEDP với đầy đủ tài liệu hướng dẫn sử dụng và phát triển trên STM32.

### Phiên bản 1.0.2: The Internal Logger

- [x] Hoàn thiện test case để thống nhất định dạng và quy trình testing cho các module của lõi μEDP. Đã thực hiện điều này trong quá trình phát triển Core.
- [x] Bổ sung thiết kế 1 Internal Logger (itnlog) để thay thế printf debugging trong để hỗ trợ kit không có cổng UART.
- [x] Triển khai thiết kế logger với inline snapshot để lưu trực tiếp tsk/sig/fsm/tsm/msg và khai thác ring buffer để lưu log nội bộ ở runtime.
- [x] Sửa lỗi thiếu exit_critical trong timer khi timer_set
- [x] Loại bỏ hỗ trợ cho việc specify phân vùng trên pool để tránh xung đột thiết kế khi import vào ESP32.
- [x] Chuyển đổi các global variable từ uninitialize sang initialize để tránh vấn đề về memory footprint và phân vùng bộ nhớ.
- [x] Chuyển các size specifier của pool sang uedp_core để thống nhất và dễ dàng quản lý cấu hình pool size cho người dùng.
- [x] Bổ sung 1 file `core_Cfg.h` ở `app/config` để người dùng có thể cấu hình các thông số của lõi μEDP như pool size, task count, timer count, v.v. một cách dễ dàng mà không cần phải chỉnh sửa trực tiếp trong source code của lõi.
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
- [x] Bổ sung mục tài liệu để phân tích thiết kế dump log nằm ngoài core μEDP để tận dụng out-context execution nhằm đảm bảo các tính chất của EDP.
- [x] Ra mắt phiên bản 1.0.2 của lõi μEDP với đầy đủ test case và tài liệu hướng dẫn sử dụng internal logger.

### Phiên bản 1.0.3: The Plug-N-Play Logging Pipeline

- [x] Sửa đổi UART_DMA_TX để thử nghiệm printf debugging trên STM32, nếu hiệu quả thì có thể giữ lại như một tùy chọn cho người dùng, nếu không hiệu quả thì sẽ loại bỏ và tập trung vào phát triển internal logger.
- [x] Bổ sung rprintf (redirect printf) để hỗ trợ chuyển hướng output của printf ra ngoài console của STM32. Đối với ESP32 đã có hỗ trợ sẵn nên không cần thiết kế thêm.
- [x] Bổ sung xprintf (eXtended printf) để hỗ trợ định dạng log nâng cao, bao gồm timestamp, task ID, signal ID, v.v. nhằm cung cấp thông tin chi tiết hơn trong các log được ghi lại.
- [x] Bổ sung logdp (log dispatcher) để tự động định tuyến log đến các đích khác nhau dựa trên mức độ ưu tiên hoặc loại log, ví dụ như gửi log quan trọng đến UART và log thông thường đến internal logger.
- [x] Bổ sung tài liệu thiết kế chi tiết cho các tính năng logdp, xprintf và rprintf để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của các tính năng này trong việc hỗ trợ việc phân tích hiệu suất và xử lý sự cố hiệu quả hơn.
- [x] Bổ sung tài liệu trình bày về giới hạn mà μEDP có thể được sử dụng trong hệ thống nhúng và khi nào nên cân nhắc sử dụng một hệ điều hành nhúng đầy đủ như FreeRTOS thay vì μEDP, nhằm giúp người dùng hiểu rõ hơn về phạm vi ứng dụng và lựa chọn phù hợp cho dự án của họ.
- [X] Import tài liệu yêu cầu phần mềm của mô hình QP/C của Miro Samek để làm tài liệu tham khảo cho việc phát triển các tính năng tracing trong μEDP.
- [x] Bổ sung tài liệu đối chiếu thiết kế của μEDP với mô hình chuẩn QP/C của Miro Samek để làm rõ các điểm tương đồng và khác biệt trong kiến trúc và cách tiếp cận.
- [x] Ra mắt phiên bản 1.0.3 của lõi μEDP với các tính năng redirect printf và xprintf, cùng với tài liệu hướng dẫn sử dụng và phân tích thiết kế chi tiết về giới hạn sử dụng của μEDP và đối chiếu với mô hình QP/C.

### Phiên bản 1.1.0: The Empty Priority Escalation Update

- [x] Đổi tên μEDP thành μEDP (micro-EDP) với các API tương đồng để phản ánh rõ hơn về mục tiêu của dự án là một lõi điều phối nhẹ cho các hệ thống nhúng.
- [x] Bổ sung tài liệu trình bày về cơ chế Priority Escalation và Scheduling Policy của μEDP để làm rõ cách thức hoạt động và lợi ích của cơ chế này trong việc xử lý các tình huống khẩn cấp và đảm bảo hiệu suất của hệ thống.
- [x] Nâng cấp thiết kế phân phối task với API cho phép thực hiện cơ chế Priority Escalation để cho phép một task có thể tạm thời tăng độ ưu tiên của mình khi cần thiết và hoàn trả độ ưu tiên về mức ban đầu sau khi hoàn thành công việc khẩn cấp.
- [x] Bổ sung ô nhớ lưu trữ mức ưu tiên hiện tại, rename ô nhớ cũ thành ô nhớ base priority để tránh nhầm lẫn và đảm bảo rằng task có thể hoàn trả độ ưu tiên về mức ban đầu một cách chính xác sau khi hoàn thành công việc khẩn cấp.
- [x] Bổ sung cờ nhớ `urgent_pending` để đánh dấu rằng task đang trong trạng thái tăng ưu tiên tạm thời và cần được xử lý ngay lập tức, nhằm đảm bảo rằng các task quan trọng được xử lý kịp thời và hiệu quả.
- [x] Thêm tài liệu thiết kế của PPLP (Plug-N-Play Logging Pipeline) do thiếu sót chỉ có tài liệu hướng dẫn sử dụng.
- [x] Thêm tài liệu để phân biệt giữa task polling và OCE service với các ví dụ minh họa cụ thể để làm rõ sự khác biệt trong cách thức hoạt động và ứng dụng của hai cơ chế này trong việc xử lý các tác vụ và sự kiện trong hệ thống.
- [x] Bổ sung testing cho cơ chế Priority Escalation để đảm bảo tính ổn định và hiệu quả của cơ chế này trong việc xử lý các tình huống khẩn cấp và đảm bảo hiệu suất của hệ thống.
- [x] Sửa lỗi sai điều kiện reset priority trong scheduler.
- [x] Bổ sung mức ưu tiên base khi tìm mức ưu tiên mới cho task trong cơ chế Priority Escalation để đảm bảo rằng task không bị lệch khỏi dãy giá trị ưu tiên hợp lệ và tránh tình trạng task bị mất quyền truy cập vào các tài nguyên quan trọng trong hệ thống.
- [x] Ra mắt phiên bản 1.1.0 của lõi μEDP với đầy đủ tính năng Priority Escalation cơ bản và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.1.1: The S-LnF (Safe LIFO-nested FIFO) Mechanism

- [x] Bổ sung tài liệu thiết kế chi tiết cho cơ chế S-LnF (Safe LIFO-nested FIFO) để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của cơ chế này trong việc xử lý các tin nhắn khẩn cấp một cách an toàn và hiệu quả trong hệ thống.
- [x] Triển khai thiết kế trên FIFO API để hỗ trợ việc xử lý các tin nhắn khẩn cấp một cách an toàn và hiệu quả, đồng thời đảm bảo rằng các tin nhắn được xử lý theo thứ tự ưu tiên một cách công bằng và hiệu quả trong hệ thống.
- [x] Tích hợp cơ chế S-LnF vào urgent call của task kèm theo self-post messaging để đảm bảo rằng các tin nhắn khẩn cấp được xử lý ngay lập tức mà không phải chờ đợi các tin nhắn cũ trong task queue, đồng thời vẫn đảm bảo rằng các tin nhắn khẩn cấp được xử lý theo thứ tự ưu tiên một cách công bằng và hiệu quả trong hệ thống.
- [x] Bổ sung testing cho cơ chế S-LnF để đảm bảo tính ổn định và hiệu quả của cơ chế này trong việc xử lý các tin nhắn khẩn cấp một cách an toàn và hiệu quả trong hệ thống.
- [x] Sửa lỗi thiếu xóa bit ưu tiên khẩn cấp khi urgent call được xử lý xong để đảm bảo rằng task có thể trở về trạng thái bình thường sau khi hoàn thành công việc khẩn cấp và tránh tình trạng task bị giữ mãi ở trạng thái ưu tiên cao một cách không cần thiết.
- [x] Rename cơ chế PE gốc thành non=S-LnF APE (non-supported LIFO-nested FIFO Atomic Priority Escalation) để làm rõ tính chất Atomic của cơ chế PE gốc nhưng không hỗ trợ LIFO-nested FIFO, đồng thời phân biệt rõ hơn với cơ chế S-LnF mới được bổ sung, trình bày chi tiết trong tài liệu thiết kế để làm rõ sự khác biệt giữa hai cơ chế này và lý do tại sao cơ chế S-LnF được bổ sung để đảm bảo tính an toàn và hiệu quả trong việc xử lý các tin nhắn khẩn cấp trong hệ thống.
- [x] Rename cơ chế PE mới thành S-LnF APE (Safe LIFO-nested FIFO Atomic Priority Escalation) để làm rõ tính chất an toàn và hỗ trợ LIFO-nested FIFO của cơ chế này, đồng thời phân biệt rõ hơn với cơ chế PE gốc.
- [x] Import lại README của v1.1.0 để sửa đổi và bổ sung thông tin về cơ chế S-LnF và các tính năng mới của phiên bản 1.1.1, đồng thời làm rõ hơn về các cải tiến và lợi ích của cơ chế S-LnF trong việc xử lý các tin nhắn khẩn cấp một cách an toàn và hiệu quả trong hệ thống.
- [x] Ra mắt phiên bản 1.1.1 của lõi μEDP với đầy đủ tính năng S-LnF và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.1.2: The Kconfig Configuration

- [ ] Sửa lỗi memrp để cho phép redirect cấu hình vào pipeline của itnlog và loại bỏ printf
- [ ] Bổ sung Kconfig để hỗ trợ cấu hình các tính năng của lõi μEDP một cách dễ dàng thông qua một giao diện cấu hình trực quan, giúp người dùng có thể tùy chỉnh các thông số của hệ thống mà không cần phải chỉnh sửa trực tiếp trong code.
- [ ] Bổ sung tài liệu thiết kế chi tiết cho Kconfig để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của tính năng này trong việc hỗ trợ việc cấu hình hệ thống một cách dễ dàng và trực quan hơn.
- [ ] Ra mắt phiên bản 1.1.2 của lõi μEDP với đầy đủ tính năng Kconfig và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.1.3: The Out-Context Execution Service

- [ ] Bổ sung cơ chế OCE đơn giản để làm khung hỗ trợ cho AOCE (Advance OCE) trong tương lai. Cơ chế này tích hợp sẵn khi task scheduler hoàn thành 1 vòng lặp lịch thì thoát ra và tự động chạy OCE service để xử lý các sự kiện đã đăng ký, giúp đảm bảo rằng các sự kiện quan trọng được xử lý kịp thời mà không cần phải chờ đến lượt của task scheduler. Tuy nhiên ở AOCE sẽ bổ sung try-catch để đảm bảo OCE thực thi khi không có task nào sẵn sàng chạy, nhằm tối ưu hiệu suất và giảm độ trễ trong việc xử lý các sự kiện thời gian thực.
- [ ] Triển khai thiết kế với cơ chế OCE có dispatch, register và unregister để hỗ trợ việc đăng ký và hủy đăng ký các sự kiện cần xử lý trong OCE service, giúp đảm bảo rằng các sự kiện quan trọng được xử lý kịp thời mà không cần phải chờ đến lượt của task scheduler.
- [ ] Refine tài liệu hướng dẫn sử dụng OCE service để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của tính năng này trong việc hỗ trợ việc xử lý các sự kiện quan trọng một cách kịp thời và hiệu quả hơn.
- [ ] Bổ sung testing cho cơ chế OCE để đảm bảo tính ổn định và hiệu quả của cơ chế này trong việc xử lý các sự kiện quan trọng một cách kịp thời và hiệu quả hơn.
- [ ] Ra mắt phiên bản 1.1.3 của lõi μEDP với đầy đủ tính năng OCE service và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.1.4: The I/O Mapping Shell

- [ ] Bổ sung các hạng mục bổ sung tài liệu thiết kế IOMS (I/O Mapping Shell) để cung cấp cơ chế gán lệnh vào 1 chân GPIO cụ thể để hỗ trợ việc kích hoạt các chức năng của lõi μEDP thông qua các tín hiệu vật lý, giúp mở rộng khả năng tương tác với phần cứng và hỗ trợ các ứng dụng yêu cầu tương tác thời gian thực.
- [ ] Hoàn thiện thiết kế chi tiết cho logic sử dụng như viot (virtual I/O table), idempotency signal, pipeline command để thống nhất cơ chế gán lệnh vào chân GPIO và đảm bảo tính ổn định và hiệu quả khi sử dụng IOMS để kích hoạt các chức năng của lõi μEDP thông qua các tín hiệu vật lý.
- [ ] Bổ sung tài liệu phát triển chức năng Priority Degradation của μE-OS để giảm mức ưu tiên của command được kích hoạt thông qua IOMS sau một khoảng thời gian nhất định để tránh tình trạng task bị chiếm dụng quá lâu do các tín hiệu vật lý liên tục kích hoạt cùng một chức năng, giúp đảm bảo tính ổn định và hiệu quả của hệ thống khi xử lý các tín hiệu vật lý.
- [ ] Ra mắt phiên bản 1.1.4 của lõi μEDP với đầy đủ tài liệu thiết kế chi tiết cho IOMS và tính năng viot, idempotency cùng với tài liệu hướng dẫn sử dụng, tài liệu phát triển cho chức năng Priority Degradation của μE-OS để hỗ trợ việc sử dụng IOMS một cách hiệu quả và ổn định trong các ứng dụng yêu cầu tương tác thời gian thực thông qua các tín hiệu vật lý.

### Phiên bản 1.1.5: The Hardware Accelerated Scheduling

- [ ] Bổ sung các hạng mục bổ sung tài liệu thiết kế từ μEDP (μEDP) sang μE-OS với nâng cấp thiết kế bộ điều phối sử dụng phần cứng như NVIC - các bộ quản lý ngắt để tối ưu hiệu suất và giảm độ trễ trong việc xử lý các sự kiện thời gian thực.
- [ ] Hoàn thiện thiết kế chi tiết cho logic sử dụng của bộ điều phối phần cứng như NVIC và xử lý vấn đề về ISR nesting & preemption để đảm bảo hệ thống hoạt động ổn định và hiệu quả khi xử lý các sự kiện thời gian thực.
- [ ] Ra mắt phiên bản 1.1.5 của lõi μEDP với đầy đủ tài liệu thiết kế chi tiết cho bộ điều phối phần cứng và xử lý ISR nesting & preemption, chuẩn bị cho việc chuyển đổi sang μE-OS.

### Phiên bản 1.2.0: The Pub/Sub Engine

- [ ] Bổ sung tài liệu thiết kế chi tiết cho Pub/Sub engine để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của tính năng này trong việc hỗ trợ việc phát triển các ứng dụng phức tạp với nhiều tác vụ tương tác với nhau một cách linh hoạt hơn.
- [ ] Thiết kế và triển khai Publish-Subscribe (Pub/Sub) engine để một sự kiện có thể phát tới nhiều task đã đăng ký.
- [ ] Ra mắt phiên bản 1.2.0 của lõi μEDP với đầy đủ tính năng Pub/Sub engine và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.2.1: The Safe Out-Core Interaction

- [ ] Bổ sung tài liệu chi tiết cho thiết kế Safe Out-Core Interaction nhằm đảm bảo các tín hiệu đầu vào từ lõi được xử lý an toàn ở pool EXTAL trước khi được chuyển vào pool nội bộ của lõi, nhằm đảm bảo tính ổn định và an toàn của hệ thống khi xử lý các tín hiệu từ bên ngoài.
- [ ] Refine tính năng SOCI với OCE để đảm bảo rằng các tín hiệu đầu vào từ lõi được xử lý an toàn ở pool EXTAL trước khi được chuyển vào pool nội bộ của lõi, đồng thời đảm bảo rằng các sự kiện quan trọng được xử lý kịp thời mà không cần phải chờ đến lượt của task scheduler.
- [ ] Hoàn thiện thiết kế Safe Out-Core Interaction.
- [ ] Ra mắt phiên bản 1.2.1 của lõi μEDP với đầy đủ tính năng Safe Out-Core Interaction và tài liệu hướng dẫn sử dụng.

Sau phiên bản này, μEDP (μEDP) sẽ bắt đầu chuyển đổi thành μE-OS với thiết kế mới và các tính năng nâng cao như HAS (Hardware Accelerated Scheduling), TIM (Tickless Idle Mode), uvfs (Micro-Virtual File System), compmng (Component Manager), SHA (Safe Heap Allocation), ESD (Execution Space Division), MPU/MMU Integration, SSI (Secure Signal Injection), AOCE (Advance Out-Context Execution), DIOMS (Degradable IOMS), ...

Các hạng mục bổ sung tài liệu thiết kế từ μEDP (μEDP) sang μE-OS sẽ được cập nhật chi tiết hơn khi tiến trình chuyển đổi bắt đầu.

Phiên bản sẽ được tách thành 1 repository mới với tên gọi μE-OS để phản ánh rõ hơn về mục tiêu của dự án là một hệ điều hành nhúng nhẹ, và sẽ tiếp tục phát triển theo lộ trình đã đề ra với các tính năng mới và cải tiến dựa trên thiết kế của HyperPanelOS, RTOS.
