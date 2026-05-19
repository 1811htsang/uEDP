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
- [ ] Bổ sung logic thiết kế chống ghi đè với checksum trên từng ô log để đảm bảo tính toàn vẹn của log khi sử dụng cơ chế ghi đè vòng tròn.
- [ ] Bổ sung tài liệu hướng dẫn sử dụng internal logger, bao gồm cách cấu hình, cách sử dụng API để ghi log và các rule để đảm bảo log được ghi chính xác và có thể phân tích hiệu quả.
- [ ] Ra mắt phiên bản 1.0.2 của lõi CIEDPC với đầy đủ test case và tài liệu hướng dẫn sử dụng internal logger.

### Phiên bản 1.0.3

- [ ] Thiết kế getter để truyền dữ liệu từ internal logger ra ngoài màn hình hoặc UART.
- [ ] Bổ sung các triển khai lớp internal của printf để hỗ trợ việc xuất log
- [ ] Bổ sung tài liệu trình bày về giới hạn mà CIEDPC có thể được sử dụng trong hệ thống nhúng và khi nào nên cân nhắc sử dụng một hệ điều hành nhúng đầy đủ thay vì CIEDPC, nhằm giúp người dùng hiểu rõ hơn về phạm vi ứng dụng và lựa chọn phù hợp cho dự án của họ.
- [ ] Ra mắt phiên bản 1.0.3 của lõi CIEDPC với đầy đủ tính năng getter cho internal logger và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.0.4

- [ ] Bổ sung tài liệu đối chiếu thiết kế của CIEDPC với mô hình chuẩn QP/C của Miro Samek để làm rõ các điểm tương đồng và khác biệt trong kiến trúc và cách tiếp cận.
- [ ] Bổ sung các triển khai lớp internal của printf để hỗ trợ việc xuất log
- [ ] Triển khai itnlog (Internal Logger) nâng cao để lưu log nội bộ trên Flash hoặc Backup Data Register của RTC, với cơ chế ghi đè vòng tròn và bảo vệ chống ghi đè bằng checksum để đảm bảo tính toàn vẹn của log.
- [ ] Ra mắt phiên bản 1.0.4 của lõi CIEDPC với đầy đủ tính năng nâng cao cho internal logger và tài liệu đối chiếu thiết kế với QP/C.

### Phiên bản 1.0.5

- [ ] Bổ sung tài liệu đối chiếu lộ trình thiết kế của CIEDPC với lộ trình phát triển của HyperPanelOS để làm rõ các điểm tương đồng và khác biệt trong cách tiếp cận phát triển hệ điều hành nhúng.
- [ ] Nâng cấp Priority Scheduling để xử lý trường hợp có nhiều task cùng độ ưu tiên.
- [ ] Bổ sung tài liệu trình bày về cơ chế Priority Escalation và Scheduling Policy của CIEDPC để làm rõ cách thức hoạt động và lợi ích của cơ chế này trong việc xử lý các tình huống khẩn cấp và đảm bảo hiệu suất của hệ thống.
- [ ] Ra mắt phiên bản 1.0.5 của lõi CIEDPC với đầy đủ tính năng, tài liệu hướng dẫn sử dụng và tài liệu chi tiết về Priority Scheduling nhằm chuẩn bị cho việc phát triển các tính năng nâng cao hơn trong phiên bản 1.1.0.

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
- [ ] Thiết kế Virtual File System (VFS) layer để task có thể thao tác với Flash, EEPROM hoặc RAM như file.
- [ ] Xây dựng Component Manager để đóng gói các driver và module thành các CIEDPC Components tái sử dụng được.

### Phiên bản 1.5.x

- [ ] Hoàn thiện Task Guard với cơ chế heartbeat định kỳ để phát hiện task bị treo và kích hoạt safe response.
- [ ] Bổ sung Secure Signal Bridge với kiểm tra CRC và range check cho dữ liệu đi vào từ task_post_isr.
- [ ] Thiết kế Memory Protection Logic để giả lập phân vùng bộ nhớ và tận dụng MPU nếu phần cứng hỗ trợ.

### Sau phiên bản 1.5.x

Sau phiên bản này, CIEDPC (uEDP) sẽ bắt đầu chuyển đổi thành uE-OS với thiết kế sử dụng 2 lõi điều phối sử dụng phần cứng như NVIC - các bộ quản lý ngắt và phần mềm để tối ưu hiệu suất và giảm độ trễ trong việc xử lý các sự kiện thời gian thực. Các hạng mục bổ sung tài liệu thiết kế từ CIEDPC (uEDP) sang uE-OS sẽ được cập nhật chi tiết hơn khi tiến trình chuyển đổi bắt đầu.
