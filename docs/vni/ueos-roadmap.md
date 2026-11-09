# Lộ trình phát triển của μE-OS

Tài liệu này trình bày lộ trình phát triển của hệ điều hành nhúng μE-OS, bao gồm các phiên bản dự kiến và các tính năng chính sẽ được triển khai trong từng phiên bản. Lộ trình này có thể được điều chỉnh và cập nhật dựa trên tiến trình phát triển và phản hồi từ cộng đồng người dùng.

## Phiên bản 1.2.2: The Secure Signal Injection

- [ ] Bổ sung tài liệu thiết kế SSI (Secure Signal Injection) để bổ trợ việc bảo vệ hệ thống khỏi các tín hiệu độc hại hoặc không mong muốn được truyền vào lõi thông qua các API như task_post_isr, nhằm đảm bảo tính ổn định và an toàn của hệ thống khi xử lý các tín hiệu từ bên ngoài.
- [ ] Thiết kế SSI (Secure Signal Injection) để bổ trợ việc bảo vệ hệ thống khỏi các tín hiệu độc hại hoặc không mong muốn.
- [ ] Ra mắt phiên bản 1.2.2 của μE-OS với đầy đủ tính năng SSI (Secure Signal Injection) và tài liệu hướng dẫn sử dụng.

## Phiên bản 1.2.3: The Tickless Idle Mode

- [ ] Tích hợp Tickless Idle Mode vào PAL để tạm dừng tick hệ thống khi CPU ở trạng thái idle và không còn timer chờ xử lý.
- [ ] Bổ sung tài liệu thiết kế chi tiết cho Tickless Idle Mode để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của tính năng này trong việc tối ưu hóa hiệu suất và tiết kiệm năng lượng cho các hệ thống nhúng.
- [ ] Ra mắt phiên bản 1.2.3 của μE-OS với đầy đủ tính năng Tickless Idle Mode và tài liệu hướng dẫn sử dụng.

## Phiên bản 1.3.0: The Task-as-File Control Interface

- [ ] Bổ sung tài liệu thiết kế chi tiết cho tính năng Task-as-File Control Interface để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của tính năng này trong việc hỗ trợ việc quản lý tác vụ một cách trực quan và linh hoạt hơn thông qua một giao diện giống như hệ thống file ảo.
- [ ] Thiết kế uvfs (Task-as-File Control Interface) như một giao diện để quản lý tác vụ như 1 hệ thống file ảo, cho phép người dùng tương tác với task, timer và message pool thông qua các lệnh giống như thao tác trên file system.
- [ ] Ra mắt phiên bản 1.3.0 của μE-OS với đầy đủ tính năng Task-as-File Control Interface và tài liệu hướng dẫn sử dụng.

## Phiên bản 1.3.1: The Component Manager

- [ ] Xây dựng Component Manager để đóng gói các driver và module thành các Components tái sử dụng được.
- [ ] Bổ sung tài liệu thiết kế chi tiết cho Component Manager để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của tính năng này trong việc hỗ trợ việc quản lý và tái sử dụng các driver và module một cách hiệu quả hơn.
- [ ] Ra mắt phiên bản 1.3.1 của μE-OS với đầy đủ tính năng Component Manager và tài liệu hướng dẫn sử dụng.

## Phiên bản 1.3.2: The Task Counter

- [ ] Bổ sung tài liệu thiết kế chi tiết cho Task Counter để phát hiện task bị treo và kích hoạt safe response nhằm đảm bảo tính ổn định và an toàn của hệ thống khi có sự cố xảy ra với các tác vụ.
- [ ] Hoàn thiện Task Counter để phát hiện task bị treo và kích hoạt safe response.
- [ ] Ra mắt phiên bản 1.3.2 của μE-OS với đầy đủ tính năng Task Counter và tài liệu hướng dẫn sử dụng.

## Phiên bản 1.4.0: The Safe Heap Allocation

- [ ] Bổ sung tài liệu thiết kế thuật toán và thư viện cho Safe Heap Allocation để đảm bảo tính ổn định và an toàn của hệ thống khi thực hiện các thao tác cấp phát bộ nhớ động, đặc biệt là trong các tình huống có nhiều tác vụ cùng truy cập vào pool bộ nhớ.
- [ ] Thiết kế và triển khai thuật toán Safe Heap Allocation để hỗ trợ cấp phát bộ nhớ động một cách an toàn. Thuật toán này có thể sử dụng như First-fit hoặc Best-fit với cơ chế Coalescing để giảm fragmentation và đảm bảo hiệu quả sử dụng bộ nhớ, đồng thời có cơ chế bảo vệ để tránh các lỗi như double free hoặc memory leak.
- [ ] Bổ sung tài liệu thuật toán First-fit / Best-fit với Coalescing để làm rõ cách thức hoạt động, lợi ích và cách sử dụng của thuật toán này trong việc hỗ trợ việc cấp phát bộ nhớ động một cách an toàn và hiệu quả hơn.
- [ ] Ra mắt phiên bản 1.4.0 của μE-OS với đầy đủ tính năng Safe Heap Allocation và tài liệu hướng dẫn sử dụng.

## Phiên bản 1.4.1: The Execution Space Division

- [ ] Bổ sung tài liệu thiết kế chi tiết Execution Space Division nhằm phân chia không gian thực thi giữa các task, timer và ISR để đảm bảo tính ổn định và hiệu quả của hệ thống khi xử lý các sự kiện thời gian thực, đồng thời hỗ trợ việc phát triển các ứng dụng phức tạp với nhiều tác vụ tương tác với nhau một cách linh hoạt hơn.
- [ ] Thiết kế Execution Space Division để phân chia không gian thực thi giữa các task, timer và ISR, đảm bảo rằng các tác vụ có thể hoạt động một cách độc lập.
- [ ] Ra mắt phiên bản 1.4.1 của μE-OS với đầy đủ tính năng Execution Space Division và tài liệu hướng dẫn sử dụng.

## Phiên bản 1.4.2: The MM/PU Integration

- [ ] Bổ sung tài liệu thiết kế 1 MMU (Memory Management Unit) hoặc MPU (Memory Protection Unit) để hỗ trợ việc bảo vệ bộ nhớ và phân vùng bộ nhớ một cách hiệu quả hơn, nhằm đảm bảo tính ổn định và an toàn của hệ thống khi xử lý các tác vụ có yêu cầu cao về bảo mật hoặc khi có nhiều tác vụ cùng truy cập vào pool bộ nhớ.
- [ ] Hoàn thiện thiết kế MMU/MPU để hỗ trợ việc bảo vệ bộ nhớ và phân vùng bộ nhớ một cách hiệu quả hơn.
- [ ] Ra mắt phiên bản 1.4.2 của μE-OS với đầy đủ tính năng MMU/MPU và tài liệu hướng dẫn sử dụng.

## Phiên bản 1.4.3: The μE-OS Transition

- [ ] Bổ sung tài liệu đối chiếu lộ trình thiết kế của μE-OS với lộ trình phát triển của HyperPanelOS để làm rõ các điểm tương đồng và khác biệt trong cách tiếp cận phát triển hệ điều hành nhúng.
- [ ] Tìm hiểu và phân tích thiết kế của HyperPanelOS để rút ra các bài học kinh nghiệm và áp dụng vào thiết kế của μE-OS.
- [ ] Ra mắt phiên bản 1.4.3 của μE-OS với tài liệu đối chiếu thiết kế của μE-OS với HyperPanelOS.

## Sau phiên bản 1.4.3

Các tính năng và hạng mục bổ sung sẽ được xác định và lên kế hoạch chi tiết hơn dựa trên tiến trình phát triển của μE-OS và phản hồi từ cộng đồng người dùng. Điều này phụ thuộc vào việc hoàn thiện các tính năng đã đề ra trong lộ trình và đánh giá hiệu quả của chúng trong thực tế, cũng như việc nghiên cứu và áp dụng các công nghệ mới có thể hỗ trợ cho sự phát triển của hệ điều hành nhúng.
