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
