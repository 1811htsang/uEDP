# To-do list

Đây là nơi lưu trữ các công việc cần làm liên quan đến dự án CIEDPC cũng như các ghi chú và ý tưởng phát triển trong tương lai.

## Ghi chú phát triển

- Bổ sung lộ trình phát triển để đưa CIEDPC từ core EDP thành 1 OS hoàn chỉnh theo tiêu chuẩn Semantic Versioning.
- Bổ sung các đối chiếu về đánh giá thiết kế của CIEDPC với mô hình chuẩn là QP/C của Miro Samek.
- Bổ sung tài liệu để mention quá trình chuyển đổi thiết kế từ dependent của EPCB-vn AKEDP sang CIEDPC
- Bổ sung tài liệu phân tích thiết kế của HyperPanelOS và QP/C chi tiết hơn để rút ra các bài học kinh nghiệm và áp dụng vào thiết kế của CIEDPC.
- Quy ước: thay đổi tài liệu có thể được phát hành như patch version nếu ảnh hưởng trực tiếp đến release deliverable hoặc cách sử dụng hệ thống.

## Công việc cần làm

### Phiên bản 1.0.0

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

### Phiên bản 1.0.1

- [x] Bổ sung template code cho phần app layer để làm ví dụ cho việc phát triển ứng dụng trên nền tảng CIEDPC.
- [x] Import thiết kế vào STM32 nhằm thử nghiệm thực tế trên phần cứng.
- [x] Bổ sung RAM profiling cho memrp để đánh giá hiệu quả sử dụng bộ nhớ của lõi CIEDPC trên đa nền tảng.
- [x] Import các template code vào lại source code để ra mắt phiên bản 1.0.1 của lõi CIEDPC.
- [x] Bổ sung tài liệu để hướng dẫn phát triển với nền tảng CIEDPC trên STM32, bao gồm hướng dẫn cài đặt môi trường phát triển, cấu hình phần cứng và ví dụ code.
- [x] Loại bỏ user-manual bản PDF do tốn thời gian căn chỉnh ngắt trang và thay thế tạm thời bằng markdown file để dễ dàng cập nhật và chỉnh sửa trong quá trình phát triển.
- [x] Bổ sung các tài liệu phân tích và thiết kế chi tiết cho các module của lõi CIEDPC từ lỗi AKEDP, bao gồm FSM, TSM, task driver, timer driver và ISR bridge đã thất lạc vào thời điểm hoàn thiện phiên bản 1.0.0.
- [x] Ra mắt phiên bản 1.0.1 của lõi CIEDPC với đầy đủ tài liệu hướng dẫn sử dụng và phát triển trên STM32.

### Phiên bản 1.0.2

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
- [x] Bổ sung mục tài liệu để phân tích thiết kế dump log nằm ngoài core CIEDPC-uEDP để tận dụng out-context execution nhằm đảm bảo các tính chất của EDP.
- [x] Ra mắt phiên bản 1.0.2 của lõi CIEDPC với đầy đủ test case và tài liệu hướng dẫn sử dụng internal logger.

### Phiên bản 1.0.3

- [x] Sửa đổi UART_DMA_TX để thử nghiệm printf debugging trên STM32, nếu hiệu quả thì có thể giữ lại như một tùy chọn cho người dùng, nếu không hiệu quả thì sẽ loại bỏ và tập trung vào phát triển internal logger.
- [x] Bổ sung rprintf (redirect printf) để hỗ trợ chuyển hướng output của printf ra ngoài console của STM32. Đối với ESP32 đã có hỗ trợ sẵn nên không cần thiết kế thêm.
- [x] Bổ sung xprintf (eXtended printf) để hỗ trợ định dạng log nâng cao, bao gồm timestamp, task ID, signal ID, v.v. nhằm cung cấp thông tin chi tiết hơn trong các log được ghi lại.
- [x] Bổ sung logdp (log dispatcher) để tự động định tuyến log đến các đích khác nhau dựa trên mức độ ưu tiên hoặc loại log, ví dụ như gửi log quan trọng đến UART và log thông thường đến internal logger.
- [ ] Bổ sung tài liệu thiết kế chi tiết cho các tính năng logdp, xprintf và rprintf để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của các tính năng này trong việc hỗ trợ việc phân tích hiệu suất và xử lý sự cố hiệu quả hơn.
- [ ] Bổ sung tài liệu trình bày về giới hạn mà CIEDPC có thể được sử dụng trong hệ thống nhúng và khi nào nên cân nhắc sử dụng một hệ điều hành nhúng đầy đủ thay vì CIEDPC, nhằm giúp người dùng hiểu rõ hơn về phạm vi ứng dụng và lựa chọn phù hợp cho dự án của họ.
- [ ] Bổ sung tài liệu đối chiếu thiết kế của CIEDPC với mô hình chuẩn QP/C của Miro Samek để làm rõ các điểm tương đồng và khác biệt trong kiến trúc và cách tiếp cận.
- [ ] Ra mắt phiên bản 1.0.3 của lõi CIEDPC với các tính năng redirect printf và xprintf, cùng với tài liệu hướng dẫn sử dụng và phân tích thiết kế chi tiết về giới hạn sử dụng của CIEDPC và đối chiếu với mô hình QP/C.

### Phiên bản 1.0.4

- [ ] Bổ sung tài liệu đối chiếu lộ trình thiết kế của CIEDPC với lộ trình phát triển của HyperPanelOS để làm rõ các điểm tương đồng và khác biệt trong cách tiếp cận phát triển hệ điều hành nhúng.
- [ ] Nâng cấp Priority Scheduling để xử lý trường hợp có nhiều task cùng độ ưu tiên.
- [ ] Bổ sung tài liệu trình bày về cơ chế Priority Escalation và Scheduling Policy của CIEDPC để làm rõ cách thức hoạt động và lợi ích của cơ chế này trong việc xử lý các tình huống khẩn cấp và đảm bảo hiệu suất của hệ thống.
- [ ] Ra mắt phiên bản 1.0.4 của lõi CIEDPC với đầy đủ tính năng, tài liệu hướng dẫn sử dụng và tài liệu chi tiết về Priority Scheduling nhằm chuẩn bị cho việc phát triển các tính năng nâng cao hơn trong phiên bản 1.1.0.

### Phiên bản 1.1.0

- [ ] Nâng cấp thiết kế phân phối task với API cho phép thực hiện cơ chế Priority Escalation để cho phép một task có thể tạm thời tăng độ ưu tiên của mình khi cần thiết và hoàn trả độ ưu tiên về mức ban đầu sau khi hoàn thành công việc khẩn cấp.
- [ ] Bổ sung các hạng mục bổ sung tài liệu thiết kế từ CIEDPC (uEDP) sang uE-OS với nâng cấp thiết kế bộ điều phối sử dụng phần cứng như NVIC - các bộ quản lý ngắt để tối ưu hiệu suất và giảm độ trễ trong việc xử lý các sự kiện thời gian thực.
- [ ] Rename CIEDPC thành uEDP (micro-EDP) để phản ánh rõ hơn về mục tiêu của dự án là một lõi điều phối nhẹ cho các hệ thống nhúng và bắt đầu quá trình nâng cấp thiết kế lên uE-OS với thiết kế sử dụng 2 lõi điều phối sử dụng phần cứng như NVIC - các bộ quản lý ngắt và phần mềm để tối ưu hiệu suất và giảm độ trễ trong việc xử lý các sự kiện thời gian thực.
- [ ] Ra mắt phiên bản 1.1.0 của lõi CIEDPC với đầy đủ tính năng Priority Escalation và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.1.1

- [ ] Tích hợp ciedpc_spy (Software Tracing) vào Scheduler và Message Pool để xuất trace phục vụ phân tích luồng xử lý.
- [ ] Bổ sung tài liệu hướng dẫn sử dụng ciedpc_spy để giúp người dùng hiểu cách cấu hình và sử dụng công cụ này để phân tích hiệu suất và luồng xử lý của hệ thống.
- [ ] Hoàn thiện thiết kế chi tiết cho logic sử dụng của bộ điều phối phần cứng như NVIC và xử lý vấn đề về ISR nesting & preemption để đảm bảo hệ thống hoạt động ổn định và hiệu quả khi xử lý các sự kiện thời gian thực.
- [ ] Ra mắt phiên bản 1.1.1 của lõi CIEDPC với đầy đủ tính năng ciedpc_spy và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.2.x

- [ ] Thiết kế và triển khai Publish-Subscribe (Pub/Sub) engine để một sự kiện có thể phát tới nhiều task đã đăng ký.
- [ ] Hỗ trợ Multicasting để gửi một tin nhắn đến một nhóm ID cụ thể dựa trên ref_count của Message.
- [ ] Thiết kế Zero-copy Bridge để chuyển quyền sở hữu vùng nhớ giữa các pool mà không cần sao chép dữ liệu lớn.

### Phiên bản 1.3.x

- [ ] Tích hợp Tickless Idle Mode vào PAL để tạm dừng tick hệ thống khi CPU ở trạng thái idle và không còn timer chờ xử lý.
- [ ] Bổ sung Dynamic Priority Tuning để thay đổi độ ưu tiên của task tại runtime khi xảy ra tình huống khẩn cấp.
- [ ] Xây dựng CPU Load Monitor để tính toán % tải CPU dựa trên thời gian thực thi task và idle time.

### Phiên bản 1.4.x

- [ ] Tích hợp Kconfig system với kconfiglib để quản lý cấu hình Core, Pool size và TSM settings qua menuconfig.
- [ ] Thiết kế uvfs như một giao diện để quản lý tác vụ như 1 hệ thống file ảo, cho phép người dùng tương tác với task, timer và message pool thông qua các lệnh giống như thao tác trên file system.
- [ ] Xây dựng Component Manager để đóng gói các driver và module thành các CIEDPC Components tái sử dụng được.

### Phiên bản 1.5.x

- [ ] Hoàn thiện Task Guard với cơ chế heartbeat định kỳ để phát hiện task bị treo và kích hoạt safe response.
- [ ] Bổ sung Secure Signal Bridge với kiểm tra CRC và range check cho dữ liệu đi vào từ task_post_isr.
- [ ] Thiết kế Memory Protection Logic để giả lập phân vùng bộ nhớ và tận dụng MPU nếu phần cứng hỗ trợ.
- [ ] Bổ sung tài liệu thiết kế chi tiết cho Task Guard, Secure Signal Bridge và Memory Protection Logic để làm rõ cách thức hoạt động và lợi ích của các tính năng này trong việc nâng cao độ tin cậy và bảo mật của hệ thống.

### Sau phiên bản 1.5.x

Sau phiên bản này, CIEDPC (uEDP) sẽ bắt đầu chuyển đổi thành uE-OS với thiết kế sử dụng 2 lõi điều phối sử dụng phần cứng như NVIC - các bộ quản lý ngắt và phần mềm để tối ưu hiệu suất và giảm độ trễ trong việc xử lý các sự kiện thời gian thực. Các hạng mục bổ sung tài liệu thiết kế từ CIEDPC (uEDP) sang uE-OS sẽ được cập nhật chi tiết hơn khi tiến trình chuyển đổi bắt đầu.
