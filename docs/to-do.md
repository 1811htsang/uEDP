# To-do list

Đây là nơi lưu trữ các công việc cần làm liên quan đến dự án μEDP cũng như các ghi chú và ý tưởng phát triển trong tương lai.

## ANCHOR - Hướng dẫn sử dụng Comment Anchor

- SECTION cho từng phiên bản sẽ có dạng `//  SEC... - Sample vX.X.X`.

Áp dụng tương tự với close section là `//  !SECTION`, tức phải có 2 space.

- SECTION cho các đoạn note nhỏ hơn sẽ có dạng

<!-- SEC... - Sample L<X>, X for line number of task above this section

// !SEC... 
-->

Tức là có 1 space trước dấu `//` và 1 space trước dấu `!SECTION`, phần L[X] là số dòng của task phía trên section này.

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

### Phiên bản 1.1.2: The Kconfig-Docker Integration

- [x] Sửa lỗi memrp để cho phép redirect cấu hình vào pipeline của itnlog và loại bỏ printf
- [x] Bổ sung Kconfig để hỗ trợ cấu hình các tính năng của lõi μEDP một cách dễ dàng thông qua một giao diện cấu hình trực quan, giúp người dùng có thể tùy chỉnh các thông số của hệ thống mà không cần phải chỉnh sửa trực tiếp trong code.
- [x] Bổ sung tài liệu thiết kế chi tiết cho Kconfig để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của tính năng này trong việc hỗ trợ việc cấu hình hệ thống một cách dễ dàng và trực quan hơn.
- [x] Bổ sung cấu hình Docker để hỗ trợ việc chạy môi trường phát triển và testing của lõi μEDP trên các nền tảng khác nhau một cách dễ dàng và nhất quán, giúp người dùng có thể triển khai và kiểm thử hệ thống một cách nhanh chóng mà không gặp phải các vấn đề về môi trường phát triển.
- [x] Bổ sung video để hướng dẫn sử dụng Kconfig để giúp người dùng hiểu rõ hơn về cách thức hoạt động và cách sử dụng của tính năng này trong việc hỗ trợ việc cấu hình hệ thống một cách dễ dàng và trực quan hơn.
- [x] Ra mắt phiên bản 1.1.2 của lõi μEDP với đầy đủ tính năng Kconfig và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.1.3: The Out-Context Execution Service

- [x] Refine tài liệu hướng dẫn sử dụng OCE service để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của tính năng này trong việc hỗ trợ việc xử lý các sự kiện quan trọng một cách kịp thời và hiệu quả hơn.
- [x] Bổ sung thiết kế nhúng SCB vào lõi μEDP để hỗ trợ việc quản lý các dịch vụ OCE một cách hiệu quả và linh hoạt hơn, giúp đảm bảo rằng các sự kiện quan trọng được xử lý kịp thời mà không cần phải chờ đến lượt của task scheduler.
- [x] Hoàn thiện API cho linked list để hỗ trợ nền tảng cho các tính năng.
- [x] Bổ sung test cho llist để đảm bảo tính ổn định và hiệu quả của cơ chế linked list trong việc quản lý các sự kiện và dữ liệu liên quan đến OCE service.
- [x] Bổ sung Kconfig cho tự động gen hàm thực thi Core.
- [x] Hoàn thiện thiết kế ocesvc để hỗ trợ việc quản lý các dịch vụ OCE một cách hiệu quả và linh hoạt hơn, giúp đảm bảo rằng các sự kiện quan trọng được xử lý kịp thời mà không cần phải chờ đến lượt của task scheduler.
- [x] Triển khai thiết kế với cơ chế OCE có dispatch, register và unregister để hỗ trợ việc đăng ký và hủy đăng ký các sự kiện cần xử lý trong OCE service, giúp đảm bảo rằng các sự kiện quan trọng được xử lý kịp thời mà không cần phải chờ đến lượt của task scheduler.
- [x] Bổ sung testing cho cơ chế OCE để đảm bảo tính ổn định và hiệu quả của cơ chế này trong việc xử lý các sự kiện quan trọng một cách kịp thời và hiệu quả hơn.
- [x] Ra mắt phiên bản 1.1.3 của lõi μEDP với đầy đủ tính năng OCE service và tài liệu hướng dẫn sử dụng.

### Phiên bản 1.1.4: The 1.1.3a Release

- [x] Bổ sung state machine attribute cho task norm để chỉ định máy trạng thái và hình thức quản lý directly từ task pointer selector, để hỗ trợ việc quản lý các trạng thái của task một cách hiệu quả và linh hoạt hơn, giúp đảm bảo rằng các task được xử lý theo đúng logic và trạng thái của chúng trong hệ thống.
- [x] Bổ sung API tương ứng để thực hiện get/set state machine attribute cho task norm, nhằm hỗ trợ việc quản lý các trạng thái của task một cách hiệu quả và linh hoạt hơn.
- [x] Ra mắt phiên bản 1.1.4 của lõi μEDP.

### Phiên bản 1.1.5: The 1.1.3b Release

//  SECTION - v1.1.5

- [x] Triển khai thiết kế FCR (Fatal Code Return) kèm tài liệu để hỗ trợ việc định danh các lỗi nghiêm trọng trong μEDP với bảng mã lỗi và các hành động xử lý tương ứng, giúp đảm bảo rằng các lỗi nghiêm trọng được xử lý một cách hiệu quả và an toàn trong hệ thống.
- [x] Bổ sung extension Anchor Comment để hỗ trợ việc đánh dấu các vị trí quan trọng trong code và tài liệu, giúp người phát triển dễ dàng theo dõi và quản lý các phần quan trọng của hệ thống.
- [x] Hoàn thiện injection của FCR vào các API để đảm bảo rằng các lỗi nghiêm trọng được phát hiện và xử lý một cách hiệu quả trong hệ thống, đồng thời cung cấp thông tin chi tiết về lỗi và các hành động xử lý tương ứng. //NOTE - Minh đã được review code và bổ sung các anchor để đánh dấu tiếp tục hoàn thiện mục này. Sau khi hoàn thành thì line này sẽ được đánh dấu là done.
- [x] Bổ sung name attribute cho rprintf để phục vụ μE-LS.
- [x] Bổ sung tài liệu thiết kế FCR. //NOTE - VN done, EN done.
- [x] Bổ sung API filling task ID và source ID attribute của message (follow commit 6728c..) để hỗ trợ việc định danh các task và nguồn gốc của các tin nhắn trong hệ thống, giúp đảm bảo rằng các tin nhắn được xử lý một cách chính xác và hiệu quả. //REVIEW - Minh đã được review code và chuyển đổi từ đề xuất setup task ID đặc biệt như ISR, Start sang cho phép người dùng tự chọn Task ID. Tuy nhiên, cần đảm bảo về việc định danh.
- [x] Remove API `internal_uedp_msg_pool_panic` khỏi lõi μEDP và thay thế bằng cơ chế FCR để xử lý các lỗi nghiêm trọng trong hệ thống, nhằm đảm bảo rằng các lỗi được xử lý một cách hiệu quả và an toàn mà không cần phải sử dụng các API đặc biệt. //NOTE - Nghĩa là đã có FCR thay thế cho API này nên cần được loại bỏ.
- [x] Đánh giá tính năng mexecjn (chain) cho OCE service để hỗ trợ việc cho phép thay đổi thứ tự danh sách các sự kiện cần xử lý trong OCE service, giúp đảm bảo rằng các sự kiện quan trọng được xử lý theo đúng thứ tự ưu tiên và logic của hệ thống. //NOTE - Ngoài ra cân nhắc việc remove việc sử dụng ID để định danh các dịch vụ OCE do đã sử dụng llist để quản lý các dịch vụ OCE, giúp giảm thiểu sự phụ thuộc vào các ID và tăng tính linh hoạt trong việc quản lý các dịch vụ OCE. Do đó, việc bổ sung mexecjn cần được cân nhắc kỹ lưỡng để đảm bảo rằng cơ chế này không gây ra xung đột hoặc phức tạp hóa việc quản lý các dịch vụ OCE trong hệ thống. Minh sẽ cần review lại OCESVC và trình bày các đề xuất mới về mexecjn.
- [x] Hoàn thiện phản hồi vòng review cuối cùng về ocesvc.mexecjn và ID-remove để đưa vào lộ trình phát triển ở các phiên bản sau.
- [x] Ra mắt phiên bản 1.1.5 của lõi μEDP với đầy đủ tính năng FCR, các bản cập nhật và tài liệu hướng dẫn sử dụng.

//REVIEW - Giữa 2 phiên bản 1.1.5 và 1.2.0 sẽ cần được thống nhất phân tách thêm 3 phiên bản 1.1.6, 1.1.7, 1.1.8 để hoàn thiện các submodule cơ sở hạ tầng được dự trù trong phiên bản 1.2.0, bao gồm các tính năng PLD/μE-LS, PLTF.TSD/TLC.

//  !SECTION

### Phiên bản 1.1.6: The 1.1.5a Release

//  SECTION - v1.1.6

//NOTE - Phiên bản này được lựa chọn để triển khai các vấn đề còn tồn đọng từ phiên bản 1.1.5, các đề xuất dự trù để hoàn thiện tính năng cũ.

//NOTE - Ở phiên bản này sẽ bắt đầu bổ sung việc phân nhánh phát triển tính năng theo từng phiên bản số hiệu hoặc tên gọi đặc biệt để tránh việc lẫn lộn các tính năng của từng phiên bản với nhau. Cân nhắc bổ sung việc phân tách nhánh `feat` theo từng người phát triển để tránh xung đột khi merge code vào nhánh chính `main`.

- [x] Bổ sung tài liệu mô tả thiết kế kiến trúc (ver eng) để hỗ trợ cộng đồng global trong việc tiếp cận và phát triển dự án μEDP, bao gồm các thông tin về kiến trúc hệ thống, các module chính, các giao diện lập trình ứng dụng (API) và các hướng dẫn phát triển chi tiết.
- [x] Triển khai tài liệu thiết kế các tính năng từ KwDI sang PLTF để hỗ trợ việc phát triển và kiểm thử các tính năng của lõi μEDP một cách dễ dàng và hiệu quả hơn. //NOTE - Đã trình bày với phiên bản 1.0 và cung cấp các đề xuất mới để cập nhật cho phiên bản 1.2.0.
- [X] Triển khai sửa đổi thiết kế ocesvc.id sang ocesvc.dbugid để phản ánh tính chất debug ID của các dịch vụ OCE follow tài liệu thiết kế PLD/μE-LS.
- [x] Triển khai loại bỏ toàn bộ API liên quan đến dbugid, chỉ giữ lại khai báo trong `ocesvc_t`, ngoài ra, loại bỏ vòng O(N) liên quan đến việc tìm ID gán cho services, các khai báo đến việc quản lý dbugid như `id_counter`, `ocesvc_has_id`, `ocesvc_find_free_id` và các sử dụng của chúng trong API hiện tại.
- [X] Cân nhắc về việc bổ sung dpool GDA kèm tài liệu liên đới DMP, D2MP và PLD/μE-LS trong quản lý dữ liệu toàn cục đối với truyền tham chiếu -> Bổ sung tài liệu `review/dmp-gda.md` //LINK docs/uels-syntax.md:745
- [x] Vòng 2 thảo luận về việc triển khai thiết kế dpool GDA kèm tài liệu liên đới DMP, D2MP và PLD/μE-LS trong quản lý dữ liệu toàn cục đối với truyền tham chiếu.
- [x] Thống nhất và bắt đầu khiển khai API cho dpool GDA kèm tài liệu liên đới DMP, D2MP và PLD/μE-LS trong quản lý dữ liệu toàn cục đối với truyền tham chiếu. //NOTE - Các task liên đới đến PLD/μE-LS sẽ được Sang triển khai sau khi Minh hoàn thiện API kèm tài liệu sửa đổi DMP, D2MP.
- [x] Bổ sung thiết kế API trong dpool GDA kèm API tương ứng.
- [x] Revert lại sửa đổi của phiên bản 1.1.4 bị mất liên quan đến attribute HSMC (`tsm` và `fsm`) của task norm, kèm API get/set.
- [x] Bổ sung các khai báo FCR thiếu với API của GDA.
- [x] Remove các khai báo task NORM và POLL mặc định không sử dụng.

<!-- SECTION - L178
Xem `uedp_core.h` và các tag FIXME trong workspace nhánh feat để hiểu rõ hơn yêu cầu của task này. Cần kiểm tra các tài liệu và mã nguồn liên đới như PLD/μE-LS, pltf.pyspec/testspec để đảm bảo rằng các khai báo task này không còn được sử dụng trong các tính năng hiện tại và tương lai của lõi μEDP.

#LINK - pltf/testspec/ustab/gnnerate_ustab.py:13
#LINK - /workspaces/uEDP/sources/core/inc/uedp_core.h:29
#LINK - /workspaces/uEDP/sources/core/inc/uedp_core.h:57
#LINK - pltf/testspec/cfparsers/dotcfg_cfp.py:37

#NOTE - 260826 đã thực hiện revert lại các sửa đổi của task để Minh sửa lại theo đúng yêu cầu của task này.

#STATUS - task đã hoàn thành.

// !SECTION 
-->

- [x] Thay đổi triển khai pyspec.usrinp để cho phép mỗi task có thể khai báo việc sử dụng FSM/TSM riêng biệt kèm số lượng trạng thái khác nhau.

<!-- SECTION - L191
Kiểm tra trong `usrinp_pspec` để nắm rõ thông tin của task và thực hiện thay đổi tương ứng trên `tnorm_pspec`.

#LINK - pltf/pyspec/usrinp_pspec.py:27
#LINK - pltf/pyspec/tnorm_pspec.py:56
#LINK - pltf/pyspec/tnorm_pspec.py:84

#STATUS - task đã hoàn thành.

// !SECTION 
-->

- [x] Sửa đổi và cập nhật thiết kế DMP, D2MP tương ứng với sự xuất hiện của dpool GDA. //NOTE - Ở task, yêu cầu cụ thể là sửa đổi tài liệu tương ứng.
- [ ] Sửa đổi và cập nhật thiết kế PLD/μE-LS tương ứng với sự thay đổi của pyspec.usrinp và pyspec.tnorm //NOTE - Ở task này, yêu cầu cụ thể là bổ sung sửa đổi tài liệu tương ứng.
- [ ] Bổ sung sửa đổi và cập nhật tài liệu `user-manual` đang thiếu các thông tin về các tính năng mới được triển khai trong phiên bản 1.1.5 và 1.1.6, bao gồm các hướng dẫn sử dụng chi tiết và các ví dụ minh họa cụ thể để giúp người dùng hiểu rõ hơn về cách thức hoạt động và cách sử dụng của các tính năng này trong việc phát triển ứng dụng trên nền tảng μEDP.
- [ ] Bổ sung các tag của Comment Anchor extension vào các comment doxygen-type cũ để hỗ trợ việc đánh dấu các vị trí quan trọng trong code và tài liệu.

<!-- NOTE - Lưu ý cho task bên trên
Theo dõi ở uedp_core.h nhánh feat để nắm rõ các tag ANCHOR được comment theo format kết hợp cùng doxygen-type.
-->

- [ ] Thực hiện rebase các task đã hoàn thành liên quan đến thiết kế syntax PLD/μE-LS (phía nhánh docs) (trước khi triển khai source code phân giải logic như testspec.*, pyspec.*) theo đề xuất phân tách v1.2.0 thành các phiên bản 1.1.6, 1.1.7, 1.1.8.

<!-- NOTE - Expectation b4 BST task
Dự kiến trước khi task BSW bắt đầu thực thi thì PLD/μE-LS sẽ được hoàn thiện với khả năng tự động hóa việc phân giải logic từ các cấu hình YAML sang các hàm thực thi trong lõi μEDP.

Điều này đảm bảo:

1. Minh sẽ thực hiện việc kiểm thử và đánh giá khả năng sinh code của PLD/μE-LS song song với việc perform BST trước khi chuyển sang triển khai TLC và TSD
2. Lộ trình phân tách đảm bảo 1.1.6 sẽ hoàn thiện các tính năng PLD/μE-LS, 1.1.7 sẽ hoàn thiện TLD và 1.1.8 sẽ hoàn thiện TLC, từ đó đảm bảo 1.2.0 sẽ có đầy đủ các tính năng cơ sở hạ tầng cần thiết để triển khai sang thiết kế Test Strategy và Test Matrix cho các phạm vi test, re-test và regression test.
3. Hỗ trợ việc triển khai testing với các phiên bản sau đó một cách dễ dàng và hiệu quả hơn, đồng thời đảm bảo hỗ trợ cho các nhân sự mới tham gia vào dự án có thể nhanh chóng nắm bắt và thực hiện các công việc liên quan.
-->

- [ ] Bổ sung BST (Basic Software Test) cho phiên bản 1.1.5 để bảo vệ tạm thời các tính năng được phát triển pre-1.2.0 trước khi áp dụng PLTF và TSD/TLC trong kiểm thử.
- [ ] Bổ sung tài liệu thiết kế chi tiết cho Pub/Sub engine để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của tính năng này trong việc hỗ trợ việc phát triển các ứng dụng phức tạp với nhiều tác vụ tương tác với nhau một cách linh hoạt hơn.
- [ ] Thiết kế và triển khai Publish-Subscribe (Pub/Sub) engine để một sự kiện có thể phát tới nhiều task đã đăng ký.

//REVIEW - Cần cân nhắc để Minh chủ trì phiên bản này do đã có nền tảng placeholder từ baseline gốc của AK-EDP, có thể so sánh với các thiết kế sẵn có như MQTT, RabbitMQ, Kafka, v.v. để đưa ra các đề xuất cải tiến và tối ưu hóa cho Pub/Sub engine của μEDP, đồng thời đảm bảo rằng các tính năng mới được triển khai một cách hiệu quả và ổn định.
//NOTE - Nhưng cũng cần lưu ý rằng, các thiết kế cần được đánh giá dưới góc nhìn ứng dụng cho hệ thống nhúng, không nên áp dụng trực tiếp các thiết kế từ các hệ thống lớn hơn mà không cân nhắc đến các hạn chế về tài nguyên và hiệu suất của hệ thống nhúng.

- [ ] Ra mắt phiên bản 1.1.6 của lõi μEDP với đầy đủ tính năng PLD/μE-LS, Pub/Sub engine và tài liệu hướng dẫn sử dụng.

//  !SECTION

### Phiên bản 1.2.0: The Infrastructure Preparation for μE-OS

//  SECTION - v1.2.0

- [x] Chỉnh sửa lại pyspec (Python Specifier - Bộ chỉ định tham số Python) cũ từ tính năng KwDI để đưa vào sử dụng kết hợp với PLTF.
- [x] Triển khai testspec (Test Specifier - Bổ chỉ định tham số test) mới chứa cfparsers (config parsers) với việc tích hợp pipeline từ pyspec được điều chỉnh và template.
- [x] Bổ sung bộ điều khiển chung (tsgen) để tích hợp pyspec + testspec tự động cho PLTF.
- [x] Triển khai thiết kế lên Docker.
- [x] Bổ sung triển khai thiết kế với Docker Compose để tự động build, chỉ định services, containers và các thông số môi trường cần thiết cho việc triển khai và kiểm thử lõi μEDP trên các nền tảng khác nhau một cách dễ dàng và nhất quán.
- [x] Kiểm tra và đánh giá thiết kế syntax YAML của PLD/μE-LS cho Task và HSMC để đảm bảo rằng cú pháp được thiết kế một cách hợp lý, dễ đọc và dễ hiểu, đồng thời hỗ trợ việc mô tả logic của các tính năng và dịch vụ trong lõi μEDP một cách hiệu quả.
- [x] Triển khai single-call trên cfparsers (testspec.cfpcall) với concentrate import để tránh repetitive call và tăng tốc độ xử lý khi parse các cấu hình logic của μE-LS.
- [x] Thiết kế syntax SII cho YAML.
- [x] Thiết kế syntax PPLP cho YAML.
- [x] Thiết kế syntax APE cho YAML.
- [x] Thiết kế syntax OCE cho YAML. //NOTE - Bổ sung cân nhắc OCE-execjn (chain) cho các phiên bản sau. Đã được review và loại bỏ, đưa vào thiết kế AOCE.
- [x] Review và refine các syntax SII, PPLP, APE, OCE cho YAML để đảm bảo rằng cú pháp được thiết kế một cách hợp lý, dễ đọc và dễ hiểu, đồng thời hỗ trợ việc mô tả logic của các tính năng và dịch vụ trong lõi μEDP một cách hiệu quả.
- [x] Thiết kế ustab.cvert và ustab.gnnerate để redirect kconfig data sang task-oriented YAML nhằm hỗ trợ việc đối chiếu và quản lý các ký hiệu, hằng số và định danh trong lõi μEDP và μE-LS một cách hiệu quả và nhất quán, giúp giảm thiểu lỗi và tăng tính nhất quán trong việc triển khai các tính năng của lõi μEDP. //FIXME - Ở thời điểm hiện tại, kết quả từ gnnerate chưa hỗ trợ được việc tự động gán anchor cho các tag (ankorpin từng được thiết kế để hỗ trợ việc này nhưng chưa triển khai được do hạn chế của PyYAML, ruamel.yaml). Cần cân nhắc triển khai ustab.ankorpin trong các phiên bản sau nếu cần thiết để hỗ trợ việc tự động gán anchor cho các tag trong YAML. Hiện tại, chỉ có thể sử dụng ustab.xportstax để export toàn bộ config sang YAML. Cần cân nhắc triển khai ustab.ankorpin trong các phiên bản sau nếu cần thiết để hỗ trợ việc tự động gán anchor cho các tag trong YAML.
- [x] Bổ sung thiết kế ustab.xportstax để hỗ trợ mapping toàn bộ config vào file YAML của μE-LS, giúp giảm thiểu lỗi và tăng tính nhất quán trong việc triển khai các tính năng của lõi μEDP. //NOTE - Loại bỏ ustab.ankorpin do không thể triển khai được và hạn chế của PyYAML, ruamel.yaml. Tạm thời sử dụng ustab.xportstax để export toàn bộ config sang YAML. Cân nhắc triển khai ustab.ankorpin trong các phiên bản sau.
- [x] Hoàn thiện việc kết nối ustab vào docker compose để tự động build và kiểm thử các tính năng của lõi μEDP trên các nền tảng khác nhau một cách dễ dàng và nhất quán.
- [x] Sửa đổi quyền truy cập đồng bộ để tránh lỗi khi create/remove file in/out Docker.
- [x] Sửa đổi lại thiết kế của phần `escal` để remove duplicate khi enable/disable các tính năng của μE-LS, nhằm đảm bảo rằng các tính năng được quản lý một cách hiệu quả và tránh tình trạng trùng lặp trong việc kích hoạt hoặc vô hiệu hóa các tính năng của lõi μEDP. //NOTE - bổ sung tài liệu để phản ánh ngược lại các thay đổi trong thiết kế của phần `escal` và cách thức quản lý các tính năng của μE-LS một cách hiệu quả.
- [x] Kiểm tra tài liệu `uels-syntax.md` để thêm task vào to-do list nhằm đảm bảo rằng các tính năng của μE-LS được triển khai một cách hiệu quả và nhất quán, đồng thời hỗ trợ việc phát triển và kiểm thử các tính năng của lõi μEDP một cách dễ dàng và hiệu quả hơn.
- [x] Triển khai rewrite mẫu app/lstaxizier-test.yaml để bắt đầu triển khai thiết kế lstaxer.vlid.
- [x] Triển khai thiết kế với lstaxer.model để hỗ trợ pydantic model validation cho các cấu hình logic của μE-LS, giúp giảm thiểu lỗi và tăng tính nhất quán trong việc triển khai các tính năng của lõi μEDP.
- [x] Hoàn thiện thiết kế với pydantic_model.logic/resc/misc để làm nền tảng data validation cho pydantic model validation, giúp giảm thiểu lỗi và tăng tính nhất quán trong việc triển khai các tính năng của lõi μEDP.
- [x] Bổ sung lstaxer.symresolv để tạo thành biểu đồ phân giải ký hiệu (SRM - Symbol Resolution Map) cho việc tìm kiếm và quản lý các references (anchors - aliases).
- [x] Triển khai các validation strategy với lstaxer.vlid để hỗ trợ việc kiểm tra tính hợp lệ của các cấu hình logic của μE-LS, giúp giảm thiểu lỗi và tăng tính nhất quán trong việc triển khai các tính năng của lõi μEDP.
- [x] Triển khai thiết kế các subcomponent của lstaxer.vlid để hỗ trợ việc kiểm tra tính hợp lệ của các cấu hình logic của μE-LS, bao gồm 5 chiến lược đề ra trong mã nguồn lstaxer.vlid.
- [x] Đánh dấu các submodule của lstaxer.vlid để triển khai trong tương lai, bao gồm 2 tính năng là UST và PRE.
- [x] Bổ sung thêm help cho các tùy chọn trong Kconfig.
- [x] Sửa đổi triển khai tag `task` thành `tnorm` và `tpoll` để phân biệt tránh nhầm lẫn cú pháp giữa non-HSMC tnorm và tpoll. //STATUS - Đã sửa đổi tài liệu bổ sung kèm mã nguồn.
- [x] Bổ sung triển khai template cho app.c với các khai báo jinja2 reusable.
- [x] Thay đổi triển khai __init__.py để hỗ trợ thống nhất triển khai các submodule của pltf.
- [x] Bổ sung triển khai tsgen.gda để hỗ trợ việc generate các cấu hình global data của μE-LS từ các mô tả logic trong PLD, giúp giảm thiểu lỗi và tăng tính nhất quán trong việc triển khai các tính năng của lõi μEDP.
- [x] Bổ sung thiết kế module để tinh gọn các khai báo NULL không sử dụng trong khai báo YAML của người dùng nhằm giảm thiểu lỗi và tăng tính nhất quán trong việc triển khai các tính năng của lõi μEDP.
- [x] Sửa đổi triển khai lstaxer.strucjec để bao quát toàn bộ cấu trúc actvobj, fsm, tsm, của tlist.
- [x] Bổ sung triển khai thiết kế lstaxer.lukupmodel để đưa các cấu hình post-validated vào pydantic model để chuẩn hóa toàn bộ logic triển khai trước khi translate thành mã C, giúp giảm thiểu lỗi và tăng tính nhất quán trong việc triển khai các tính năng của lõi μEDP.
- [x] Đưa cân nhắc triển khai hoạt động của cfparsers hoặc pyspec vào post-validation của lstaxer.vlid để phối hợp lstaxer.kre8 generate các cấu hình logic của μE-LS từ các mô tả logic trong PLD, giúp giảm thiểu lỗi và tăng tính nhất quán trong việc triển khai các tính năng của lõi μEDP.

<!-- REVIEW
1. Bổ sung cân nhắc phân tách PLD/μE-LS thành SA (semi-automation) và FA (full-automation) trên 2 phiên bản 1.1.7, 1.1.8 hay 1.1.8, 1.1.9.
2. Cân nhắc đưa triển khai kết quả sau khi cleanup NULL vào pydantic model để chuẩn hóa toàn bộ logic triển khai trước khi translate thành mã C.
3. Cần review lại thiết kế cú pháp ở khu vực data để quản lý chi tiết vấn đề chuyển data-as-parameter thành data-as-global đối với các tham số truyền vào có sizeof() hoặc length nặng hoặc multiple param trong 1 function call.
-->

- [x] Đưa calling của testspec.generator vào cùng phase với lstaxer.kre8 để cùng giai đoạn cấu hình sang mã nguồn.
- [x] Triển khai thiết kế lstaxer.kre8 để hỗ trợ việc generate các cấu hình logic của μE-LS từ các mô tả logic trong PLD, giúp giảm thiểu lỗi và tăng tính nhất quán trong việc triển khai các tính năng của lõi μEDP.
- [ ] Bổ sung rewrite cú pháp giải quyết vấn đề ambiguous data tag lên apps/lstaxizer.yaml để thực hiện regression test cho pipeline.
- [ ] Cân nhắc đưa lstaxer.nullremov vào pipeline chung của PLD/μE-LS để lstaxer.lukupmodel giảm tải các parsing.
- [ ] Triển khai BST cho pipeline PLD/μE-LS trên phần cứng thật để kiểm tra khả năng sinh code và thực thi các cấu hình logic của μE-LS từ các mô tả logic trong PLD.

<!-- STATUS
Trong thiết kế trước đó:
- testspec.generator được triển khai để sinh ra toàn bộ các cấu hình khai báo bắt buộc cho apps/lstaxizer.yaml
- testspec.ustab được dùng để sinh ra bộ cấu hình tham chiếu cho logic. 
- Các triển khai chưa được thêm vào chính là lstaxer.vlid (chứa pipeline từ lstaxer.strucjec đến lstaxer.symresolv) và sử dụng lstaxer.pydantic_model để mapping cấu hình sang parse-able state.

Hiện tại: 
- testspec.generator được phân tách thành 2 phần là generator.pre_logicdef và generator.post_logicdef để phân biệt các cấu hình được sinh ra trước khi định nghĩa logic và sau khi định nghĩa logic. 
- lstaxer.lukupmodel được bổ sung để triển khai đưa toàn bộ các cấu hình YAML thành parse-able state pydantic model. 
- lstaxer.kre8 được hoàn thiện triển khai cùng với generator.pos_logicdef để phối hợp triển khai sinh ra các mã nguồn C hoàn chỉnh.
- Vấn đề nhập nhằng trong data đã được giải quyết với sự phân tách triệt để cú pháp và ràng buộc logic.

Một số vấn đề còn tồn đọng:
- Chưa review code style của các triển khai mới.
- lstaxer.nullremov chưa được triển khai vào pipeline chung của PLD/μE-LS.
- Chưa bổ sung BST cho pipeline này lên hệ thống phần cứng thực tế.
-->

<!-- NOTE - Expectation for pipeline
Cần dự trù hoàn thành toàn bộ pipeline và các vấn đề tồn đọng của PLD/μE-LS để bắt đầu rebase các task thuộc về phiên bản 1.2.0 sang các phiên bản 1.1.7, 1.1.8, 1.1.9 để hoàn thiện các submodule cơ sở hạ tầng được dự trù trong phiên bản 1.2.0, bao gồm các tính năng PLTF.TSD/TLC.
-->

- [ ] Bổ sung phần tài liệu trình bày về hỗ trợ file inclusion nâng cao của YAML và các hạn chế của YAML trong triển khai khai thác remote-file alias. //LINK docs/uels-syntax.md:118
- [ ] Tìm hiểu các giải pháp trong việc thực thi remote-file alias trên YAML để hỗ trợ rebuilt ustab.ankorpin đưa vào phiên bản 1.1.7 hoặc 1.1.8 để hỗ trợ việc tự động gán anchor cho các tag trong YAML.
- [ ] Thực hiện chore filename để thống nhất các module riêng biệt của PLTF.

<!-- SECTION - Idea cho remote-file alias
# LINK docs/uels-syntax.md:118
# LINK pltf/testspec/lstaxer/lukupmodel_lstaxer.py:5
# LINK pltf/testspec/lstaxer/lukupmodel_lstaxer.py:706

Dựa trên triển khai và kết quả trả về của yaml.parse(), Event sẽ hỗ trợ việc get/set parameter như anchor với `MappingStartEvent(anchor='tnorm1-ctrl', tag=None, implicit=True)` khi trước đó chúng ta gặp các ScalarEvent như `ScalarEvent(anchor=None, tag=None, implicit=(False, True), value='1'`. 

Tuy nhiên file inclusion của YAML chỉ đảm bảo việc thêm vào chứ không hề đảm bảo việc remote-alias từ file khác.
Vì vậy, một ràng buộc chủ chốt phải có chính là thực hiện write toàn bộ file A với alias vào file B trước khi thực hiện bất kỳ thay đổi nào trên file B.

Ngoài ra, hiện tại trong thiết kế của lstaxer.ustab sẽ sinh ra các incremental indexed item dạng '1', '2', '3', ... để phục vụ việc mapping các anchor và alias.

Do đó, một ý tưởng có thể cân nhắc chính là:

Khi chúng ta có 1 file A cần remote-alias từ file B:
1. Chúng ta thực hiện parse file A để trích xuất event và xác định các incremental indexed item cần thiết.
2. Chúng ta thực hiện gắn anchor vào các MappingStartEvent vào sau các ScalarEvent có value là incremental indexed item.
3. Chúng ta thực hiện yaml.dump lại chính EventStream này vào chính file A để cập nhật các anchor và alias.
4. Chúng ta thực hiện write toàn bộ file A với alias vào file B trước khi thực hiện bất kỳ thay đổi nào trên file B.

# STATUS - Tuy nhiên, ở thời điểm hiện tại chưa có kiểm tra lại tính khả thi này nên ustab.ankorpin sẽ chưa được triển khai trước khi có kết quả kiểm tra khả thi. Nếu kết quả kiểm tra khả thi thì sẽ bổ sung ustab.ankorpin vào sau phiên bản 1.1.6 khi pipeline được chuẩn hóa và hoàn thiện hơn.

# !SECTION
-->

- [ ] Thực hiện rewrite giới thiệu về cú pháp YAML của μE-LS để làm rõ cách thức hoạt động tương ứng trên mã nguồn thiết kế. //LINK docs/uels-syntax.md:199
- [ ] Bổ sung tài liệu triển khai thiết kế UST (Unified Symbol Table - testspec.ustab) để hỗ trợ việc đối chiếu và quản lý các ký hiệu, hằng số và định danh trong lõi μEDP và μE-LS một cách hiệu quả và nhất quán, giúp giảm thiểu lỗi và tăng tính nhất quán trong việc triển khai các tính năng của lõi μEDP.
- [ ] Review lại thiết kế PLD (Parse-able Logic Descriptor) với các triển khai hiện có để đánh giá tính khả thi và hiệu quả của việc sử dụng PLD trong việc mô tả logic của các tính năng và dịch vụ trong lõi μEDP một cách dễ đọc và dễ hiểu, đồng thời hỗ trợ việc tự động sinh mã nguồn C từ các mô tả logic này.
- [ ] Mở rộng PLD với TSD (Test Scenario Descriptor) để hỗ trợ việc mô tả các kịch bản kiểm thử một cách dễ đọc và dễ hiểu, hướng tới việc tự động sinh mã nguồn C từ các mô tả kịch bản kiểm thử này, giúp giảm thiểu lỗi và tăng tính nhất quán trong việc triển khai các kịch bản kiểm thử cho lõi μEDP.
- [ ] Hoàn thiện thiết kế TLC (Test Level Coverager) để cho phép chỉ định mức kiểm tra từ ut (unit), ct (component), st (system) và it (integration) nhằm đảm bảo rằng các tính năng của lõi μEDP được kiểm thử đầy đủ và hiệu quả trên các mức độ khác nhau của hệ thống.

<!-- NOTE - Expectation for PLTF.TSD/TLC
Đưa 3 task bên trên sang phiên bản 1.2.1 để tránh quá tải cho phiên bản 1.2.0 và đảm bảo rằng các tính năng mới được triển khai một cách hiệu quả và ổn định.
-->

- [ ] Thêm tài liệu thiết kế đưa smoltcp vào μEDP để bổ sung khả năng xử lý mạng tương thích hướng sự kiện, giúp mở rộng khả năng của lõi μEDP trong việc xử lý các ứng dụng mạng và giao tiếp với các thiết bị khác trong hệ thống.
- [ ] Thêm tài liệu thiết kế chi tiết bootloader - μDB (Device Bootloader) để hỗ trợ việc khởi động và quản lý các thiết bị trong hệ thống một cách hiệu quả và linh hoạt hơn, giúp đảm bảo rằng các thiết bị được khởi động và quản lý một cách an toàn và hiệu quả.
- [ ] Bổ sung các hạng mục bổ sung tài liệu thiết kế từ μEDP (μEDP) sang μE-OS với nâng cấp thiết kế bộ điều phối sử dụng phần cứng như NVIC - các bộ quản lý ngắt để tối ưu hiệu suất và giảm độ trễ trong việc xử lý các sự kiện thời gian thực.
- [ ] Hoàn thiện thiết kế chi tiết cho logic sử dụng của bộ điều phối phần cứng như NVIC và xử lý vấn đề về ISR nesting & preemption để đảm bảo hệ thống hoạt động ổn định và hiệu quả khi xử lý các sự kiện thời gian thực.

<!-- NOTE - Expectation for μE-OS Documentation
Cần cân nhắc tách phần bổ sung tài liệu này sang phiên bản 1.2.2 hoặc phiên bản 1.1.6, 1.1.7 để tránh quá tải cho phiên bản 1.2.0 và đảm bảo rằng các tính năng mới được triển khai một cách hiệu quả và ổn định.
-->

- [ ] Ra mắt phiên bản 1.2.0 của lõi μEDP với đầy đủ tài liệu thiết kế chi tiết cho bộ điều phối phần cứng và xử lý ISR nesting & preemption, chuẩn bị cho việc chuyển đổi sang μE-OS.

//  !SECTION

### Phiên bản 1.2.1: The Safe Input Filter

//  SECTION - v1.2.1

//NOTE - Cân nhắc rebase các task từ v1.2.0 sang v1.2.1 theo lộ trình và đề xuất dự kiến

- [ ] Bổ sung tài liệu chi tiết cho thiết kế Safe Input Filter nhằm đảm bảo các tín hiệu đầu vào từ lõi được xử lý an toàn ở pool EXTAL trước khi được chuyển vào pool nội bộ của lõi, nhằm đảm bảo tính ổn định và an toàn của hệ thống khi xử lý các tín hiệu từ bên ngoài.
- [ ] Refine tính năng SOCI với OCE để đảm bảo rằng các tín hiệu đầu vào từ lõi được xử lý an toàn ở pool EXTAL trước khi được chuyển vào pool nội bộ của lõi, đồng thời đảm bảo rằng các sự kiện quan trọng được xử lý kịp thời mà không cần phải chờ đến lượt của task scheduler.
- [ ] Hoàn thiện thiết kế Safe Input Filter.
- [ ] Ra mắt phiên bản 1.2.1 của lõi μEDP với đầy đủ tính năng Safe Input Filter và tài liệu hướng dẫn sử dụng.

//  !SECTION

### Phiên bản 1.2.2: The I/O Mapping Shell

- [ ] Bổ sung các hạng mục bổ sung tài liệu thiết kế IOMS (I/O Mapping Shell) để cung cấp cơ chế gán lệnh vào 1 chân GPIO cụ thể để hỗ trợ việc kích hoạt các chức năng của lõi μEDP thông qua các tín hiệu vật lý, giúp mở rộng khả năng tương tác với phần cứng và hỗ trợ các ứng dụng yêu cầu tương tác thời gian thực.
- [ ] Hoàn thiện thiết kế chi tiết cho logic sử dụng như viot (virtual I/O table), idempotency signal, pipeline command để thống nhất cơ chế gán lệnh vào chân GPIO và đảm bảo tính ổn định và hiệu quả khi sử dụng IOMS để kích hoạt các chức năng của lõi μEDP thông qua các tín hiệu vật lý.
- [ ] Bổ sung tài liệu phát triển chức năng Priority Degradation của μE-OS để giảm mức ưu tiên của command được kích hoạt thông qua IOMS sau một khoảng thời gian nhất định để tránh tình trạng task bị chiếm dụng quá lâu do các tín hiệu vật lý liên tục kích hoạt cùng một chức năng, giúp đảm bảo tính ổn định và hiệu quả của hệ thống khi xử lý các tín hiệu vật lý.
- [ ] Ra mắt phiên bản 1.2.2 của lõi μEDP với đầy đủ tài liệu thiết kế chi tiết cho IOMS và tính năng viot, idempotency cùng với tài liệu hướng dẫn sử dụng, tài liệu phát triển cho chức năng Priority Degradation của μE-OS để hỗ trợ việc sử dụng IOMS một cách hiệu quả và ổn định trong các ứng dụng yêu cầu tương tác thời gian thực thông qua các tín hiệu vật lý.

### Phiên bản 1.2.3: AMP/HELF support for multicore systems

//  SECTION - v1.2.3

<!-- NOTE - Warning on compatibility issue
Cần lưu ý về vấn đề API tương thích và nền tảng phần cứng. 
Ví dụ, với STM32H723 đơn nhân, chúng ta không cần tính năng AMP/HELF, nhưng với ESP32S3 thì vốn dĩ đã có sẵn API theo hệ FreeRTOS nên việc triển khai AMP/HELF sẽ cần một API trung gian để đảm bảo tính tương thích giữa các nền tảng phần cứng khác nhau.
-->

- [ ] Bổ sung các tài liệu tham khảo về AMP/SMP để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của các kiến trúc đa lõi trong việc hỗ trợ việc phát triển các ứng dụng phức tạp với nhiều tác vụ tương tác với nhau một cách linh hoạt hơn.
- [ ] Đưa vào các tài liệu thiết kế chi tiết cho việc triển khai AMP/HELF (Hardware Eventization with Logic Fragmentation).
- [ ] Bổ sung entry cho task list để bắt đầu công việc.
- [ ] Ra mắt phiên bản 1.2.3 của lõi μEDP với đầy đủ tài liệu thiết kế chi tiết cho việc triển khai AMP/HELF và tài liệu hướng dẫn sử dụng, nhằm đảm bảo rằng các tính năng mạng được tích hợp một cách hiệu quả và ổn định trong hệ thống, đồng thời hỗ trợ việc phát triển các ứng dụng mạng và giao tiếp với các thiết bị khác trong hệ thống một cách dễ dàng và hiệu quả hơn.

//  !SECTION

### Phiên bản 1.2.4: The integration of smoltcp

//  SECTION - v1.2.4

<!-- NOTE - Warning on compatibility issue
smoltcp là network stack của Rust nên sẽ cần API trung gian và các công cụ hỗ trợ để tích hợp vào lõi μEDP. Ngoài ra, với nền tảng ESP32 hiện tại thì đã có nhà phát triển thực hiện kiểm thử việc tích hợp smoltcp vào ESP32-P4, nên có thể follow các hướng dẫn và tài liệu của họ để triển khai tích hợp smoltcp vào ESP-IDF trước khi tích hợp vào lõi μEDP trên kit ESP32-S3.

Tuy nhiên, theo khuyến cáo của nhà phát triển, `Other RISC-V SoCs should work; the classic Xtensa ESP32 needs a different Rust target and hasn’t been done.` chỉ có nghĩa là các SoC RISC-V khác sẽ hoạt động, nhưng ESP32 cổ điển cần một target Rust khác và chưa được thực hiện.

Điều này sẽ dẫn đến việc khi làm việc trên ESP32 sẽ có 2 hướng triển khai khác nhau: một là sử dụng API của ESP-IDF để tích hợp smoltcp, hai là sử dụng API trung gian của μEDP để tích hợp smoltcp. Cần cân nhắc việc triển khai API trung gian của μEDP để đảm bảo tính tương thích giữa các nền tảng phần cứng khác nhau và hỗ trợ việc phát triển các ứng dụng mạng một cách dễ dàng và hiệu quả hơn.

Do đó, cần tái thử nghiệm lại quá trình tích hợp smoltcp vào ESP-IDF để đánh giá hiệu quả trước khi chuyển sang đưa smoltcp vào lõi μEDP.

Ngoài ra cần lưu ý rằng, smoltcp chỉ là một network stack hỗ trợ TCP/IP ở tầng 3 - Network và 4 - Transport chứ không phải hỗ trợ các kết nối mạng như WiFi. Do đó, việc mapping xuống tầng 2 - Data Link sẽ cần một API trung gian để tích hợp với các driver phần cứng của ESP32-S3, ví dụ như driver WiFi hoặc driver Ethernet. Cần cân nhắc việc triển khai API trung gian của μEDP để đảm bảo tính tương thích giữa các nền tảng phần cứng khác nhau và hỗ trợ việc phát triển các ứng dụng mạng một cách dễ dàng và hiệu quả hơn.

Hoặc có thể tận dụng BSP của từng phần cứng làm điểm cốt lõi để μEDP có API tương tác điều khiển hoặc forward các tác vụ mạng sang một nhân xử lý khác nếu có hỗ trợ AMP/HELF.
-->

- [ ] Review tổng quan thiết kế smoltcp để đánh giá tính khả thi và đề xuất bổ sung task list lộ trình thiết kế API cho smoltcp vào μEDP, nhằm đảm bảo rằng các tính năng mạng được tích hợp một cách hiệu quả và ổn định trong hệ thống.
- [ ] Bổ sung tài liệu thiết kế chi tiết cho việc tích hợp smoltcp vào μEDP để hỗ trợ việc phát triển các ứng dụng mạng và giao tiếp với các thiết bị khác trong hệ thống một cách dễ dàng và hiệu quả hơn, đồng thời đảm bảo rằng các tính năng mạng được tích hợp một cách hiệu quả và ổn định trong hệ thống.
- [ ] Ra mắt phiên bản 1.2.4 của lõi μEDP với đầy đủ tài liệu thiết kế chi tiết cho việc tích hợp smoltcp vào μEDP và tài liệu hướng dẫn sử dụng, nhằm đảm bảo rằng các tính năng mạng được tích hợp một cách hiệu quả và ổn định trong hệ thống, đồng thời hỗ trợ việc phát triển các ứng dụng mạng và giao tiếp với các thiết bị khác trong hệ thống một cách dễ dàng và hiệu quả hơn.

//  !SECTION

## Lộ trình phát triển μEDP sang μE-OS

μEDP (μEDP) sẽ bắt đầu chuyển đổi thành μE-OS với thiết kế mới và các tính năng nâng cao như HAS (Hardware Accelerated Scheduling), TIM (Tickless Idle Mode), uvfs (Micro-Virtual File System), compmng (Component Manager), SHA (Safe Heap Allocation), ESD (Execution Space Division), MPU/MMU Integration, SSI (Secure Signal Injection), AOCE (Advance Out-Context Execution), DIOMS (Degradable IOMS), ...

Các hạng mục bổ sung tài liệu thiết kế từ μEDP (μEDP) sang μE-OS sẽ được cập nhật chi tiết hơn khi tiến trình chuyển đổi bắt đầu.

Phiên bản sẽ được tách thành 1 repository mới với tên gọi μE-OS để phản ánh rõ hơn về mục tiêu của dự án là một hệ điều hành nhúng nhẹ, và sẽ tiếp tục phát triển theo lộ trình đã đề ra với các tính năng mới và cải tiến dựa trên thiết kế của HyperPanelOS, RTOS.
