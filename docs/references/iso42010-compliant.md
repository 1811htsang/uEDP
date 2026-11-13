# Tài liệu trình bày các hạng mục mà μEDP tuân thủ theo chuẩn ISO/IEC/IEEE 42010:2022 về kiến trúc phần mềm

Tài liệu ISO/IEC/IEEE 42010:2022 (Software, systems and enterprise — Architecture description). Đây là "kim chỉ nam", là tiêu chuẩn quốc tế cao nhất và chính thống nhất quy định cách thức mô tả, thiết kế và tư liệu hóa một kiến trúc hệ thống (từ phần mềm, hệ thống nhúng đến kiến trúc doanh nghiệp).

## Các khái niệm cốt lõi trong ISO/IEC/IEEE 42010:2022

### Entity of Interest (EoI)

EoI là một thực thể (entity) mà kiến trúc của nó được mô tả trong một kiến trúc phần mềm. EoI có thể là một hệ thống, một phần mềm, một hệ thống nhúng, hoặc thậm chí là một tổ chức.

Trong μEDP, EoI chính là kết quả của firmware phần mềm được triển khai trên các thiết bị nhúng (embedded devices) mà μEDP đang phát triển và kiểm thử. Các EoI này có thể bao gồm các hệ thống nhúng, phần mềm điều khiển, hoặc các thành phần phần mềm khác cho phép hệ thống nhúng chạy đúng chức năng và đáp ứng các yêu cầu kỹ thuật trên phần cứng như STM32F103, STM32F407, ESP32, Raspberry Pi, và các thiết bị nhúng khác.

### Architecture Description Language (ADL)

ADL là ngôn ngữ mô tả kiến trúc, được sử dụng để biểu diễn các thành phần, mối quan hệ và cấu trúc của một kiến trúc phần mềm. ADL giúp các nhà phát triển và kiến trúc sư phần mềm hiểu rõ hơn về cách mà các thành phần của hệ thống tương tác với nhau.

Trong μEDP, ADL chính là PLD/μE-LS - (Parse-able Logic Descriptor/Micro Embedded Logical Syntaxizer) cho phép mô tả hoàn chỉnh logic hoạt động với tính năng HSMC chủ đạo của μEDP. Với sự phát triển ở các phiên bản kế cận, PLD/μE-LS sẽ được mở rộng để hỗ trợ mô tả các tính năng phức tạp của μEDP như PPLP, OCE, SIF, PSE, ...

### Architecture Description Framework (ADF)

ADF là khung mô tả kiến trúc, cung cấp một cấu trúc và phương pháp để tổ chức và trình bày các mô tả kiến trúc. ADF giúp đảm bảo rằng các mô tả kiến trúc được tổ chức một cách logic và dễ hiểu để sử dụng ADL.

Trong μEDP, PLTF (Portable Local Test Framework) chính là cung cấp một ADF cho phép tổ chức và trình bày các mô tả kiến trúc của μEDP. PLTF cung cấp một cấu trúc logic để mô tả các thành phần, mối quan hệ và cấu trúc của μEDP, giúp các nhà phát triển và kiến trúc sư phần mềm hiểu rõ hơn về cách mà các thành phần của μEDP tương tác với nhau sử dụng kconfiglib, jinja2, pyyaml, và các công cụ khác để tạo ra các mô tả kiến trúc có thể đọc được và có thể phân giải thành mã nguồn thực thi trên các thiết bị nhúng. PLTF cũng cung cấp các công cụ để kiểm tra và xác minh các mô tả kiến trúc, đảm bảo rằng chúng đáp ứng các yêu cầu kỹ thuật và chức năng của μEDP.

### Model Kind (MK)

MK là các loại mô hình dùng để giải quyết các khía cạnh khác nhau của kiến trúc phần mềm. MK giúp phân loại các mô hình kiến trúc dựa trên các khía cạnh mà chúng tập trung vào, chẳng hạn như cấu trúc, hành vi, hoặc các quan điểm khác nhau của hệ thống.

Trong μEDP, MK chính là các khối được mô tả trong PLD/μE-LS, bao gồm các khối tsm, fsm, exec, poll, oce, pplp, isr.

## Correspondence & Rules

Mô tả mối quan hệ và sự nhất quán giữa các thành phần kiến trúc khác nhau, đảm bảo rằng các mô tả kiến trúc là nhất quán và có thể được phân tích một cách logic. UST chính là công cụ để kiểm tra sự nhất quán giữa các mô tả kiến trúc khác nhau trong μEDP, đảm bảo rằng các mô tả kiến trúc là chính xác và có thể được sử dụng để tạo ra mã nguồn thực thi trên các thiết bị nhúng.

### Architecture Viewpoint

Đây là một quan điểm hoặc góc nhìn cụ thể về kiến trúc phần mềm, tập trung vào một khía cạnh hoặc mối quan tâm cụ thể của hệ thống. Mỗi viewpoint cung cấp một cách tiếp cận khác nhau để hiểu và phân tích kiến trúc phần mềm. Ví dụ, với kconfiglib là vấn đề quan tâm về cấu hình và tài nguyên, PLD là góc nhìn về logic và hành vi, và PLTF là góc nhìn về tổ chức và trình bày các mô tả kiến trúc kiểm thử.

## Triết lý thiết kế

### Seperation of Concerns

Với ISO 42010, mục 5.2.3, *Kiến trúc phải giải quyết được các Concerns (mối bận tâm) của Stakeholders (các bên liên quan). Nếu trộn lẫn mọi thứ, hệ thống sẽ thất bại.*

Do đó, μEDP đã giải quyết

1. Bận tâm về đồng bộ dữ liệu thì có GDA (Global Data Area)
2. Bận tâm về logic điều khiển thì có PLD/μE-LS (Parse-able Logic Descriptor/Micro Embedded Logical Syntaxizer)
3. Bận tâm về thời gian thực thi và tính khẩn cấp thì có S-LnF APE (Safe-LIFO nested FIFO Atomic Priority Escalation)
4. Bận tâm về kiểm thử thì có PLTF (Portable Local Test Framework)

Điều này phản ánh việc μEDP tuân thủ theo chuẩn ISO/IEC/IEEE 42010:2022 và đảm bảo có sự ràng buộc người dùng vào việc mô tả các mối quan tâm của họ một cách rõ ràng và có cấu trúc, từ đó giúp cải thiện khả năng đọc hiểu và bảo trì mã nguồn.
