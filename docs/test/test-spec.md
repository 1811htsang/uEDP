# Tài liệu trình bày các tiêu chuẩn và hướng dẫn kiểm thử cho lõi μEDP

Tài liệu này được tham khảo từ cấu trúc dự án kiểm thử của các dự án phần mềm lớn tại Bosch như một hướng dẫn có sửa đổi cho các dự án kiểm thử của lõi μEDP. Nó trình bày các tiêu chuẩn và hướng dẫn kiểm thử, bao gồm các khái niệm cơ bản, các phương pháp kiểm thử, các công cụ hỗ trợ kiểm thử, và các ví dụ minh họa.

> Lưu ý rằng tài liệu này không phải là tài liệu chính thức của Bosch được áp dụng ở bất kỳ dự án nào, mà chỉ là một hướng dẫn tham khảo dựa trên quá trình tham gia của tác giả trong các dự án kiểm thử của Bosch. Tài liệu này có thể được cập nhật và sửa đổi theo thời gian để phù hợp với các yêu cầu và thực tiễn kiểm thử của lõi μEDP.

## Kiến trúc tổng thể

Cấu trúc tổng thể của một dự án kiểm thử bao gồm các thành phần sau:

```asciidocs
                       μEDP Test Project                                                       
                       │                      │                                                
                       └── ftezsec            │ - Feature Test Section ────────┐               
                         ┌─┼── tezuit         │ - Test Suite                   │               
         multiple file ◄─┼─┼───┼── tezk       │ - Test Case                    │               
tezcal-<indentif>.yaml   └─┼───┼───┼── teztep │ - Test Step                    │               
                           │   │   └── ...    │                                │               
                           │   ├── tezk       │ - Testcase                     │               
                           │   │   ├── teztep │ - Test Step                    │               
                           │   │   └── ...    │                                │               
                           │   └── ...        │                                │               
                           ├── tezconf        │ - Test Project Configuration ──┼─► single file 
                           ├── prehook        │ - Pre Test Configuration ──────┤   ftezsec.yaml
                           ├── posthook       │ - Post Test Configuration ─────┘               
                           └── report         │ - Test Report ───────────────────► folder      
                                              │                                    tezrebort/  
```

Trong đó, `ftezsec` là thư mục chính của dự án kiểm thử, chứa các thư mục con như `tezuit` (Test Suite), `tezconf` (Test Project Configuration), `prehook` (Pre Test Configuration), `posthook` (Post Test Configuration), và `report` (Test Report). Mỗi thư mục con có thể chứa các tệp tin và thư mục con khác để tổ chức các testcase, test step, và các cấu hình kiểm thử.

Việc bổ sung các hook (prehook và posthook) cho phép thực hiện các thao tác chuẩn bị và dọn dẹp trước và sau khi chạy các testcase, giúp đảm bảo tính nhất quán và ổn định của quá trình kiểm thử thay vì phải gọi các hàm chuẩn bị và dọn dẹp trong từng testcase. Điều này giúp giảm thiểu sự phụ thuộc giữa các testcase và tăng tính tái sử dụng của các hàm chuẩn bị và dọn dẹp.

Mỗi `tezuit` (Test Suite) có thể chứa nhiều `tezk` (Test Case), và mỗi `tezk` có thể chứa nhiều `teztep` (Test Step). Các testcase và test step được tổ chức theo cấu trúc cây, giúp dễ dàng quản lý và thực hiện các kiểm thử phức tạp. Trong đó, `tezuit` chỉ quản lý mô tả mục tiêu kiểm thử và các `tezk` nào được thực hiện.

Với `tezk`, đây là vị trí quan trọng trong dự án kiểm thử, nơi chứa các testcase cụ thể và các bước kiểm thử chi tiết. Mỗi `tezk` có thể chứa nhiều `teztep`, đại diện cho các bước kiểm thử cụ thể, giúp đảm bảo rằng các testcase được thực hiện một cách chính xác và đầy đủ. Ở nhiều dự án khác nhau sẽ có các cách tổ chức khác nhau, nhưng trong dự án kiểm thử của lõi μEDP, `tezk` được coi là vị trí quan trọng nhất để quản lý các testcase và test step. Cụ thể

1. Mô tả mục tiêu kiểm thử: Mỗi `tezk` có thể chứa một mô tả chi tiết về mục tiêu kiểm thử, giúp người đọc hiểu rõ về mục đích và phạm vi của testcase. Điều này khác với `tezuit`, nơi chỉ quản lý mô tả mục tiêu kiểm thử và các `tezk` nào được thực hiện.
2. Tham số truyền vào: Mỗi `tezk` có thể định nghĩa các tham số truyền vào, giúp kiểm thử các trường hợp khác nhau của cùng một testcase. Điều này giúp tăng tính linh hoạt và khả năng tái sử dụng của testcase.
3. Thiết lập môi trường kiểm thử: Mỗi `tezk` có thể chứa các bước thiết lập môi trường kiểm thử, giúp đảm bảo rằng testcase được thực hiện trong một môi trường kiểm thử ổn định và đáng tin cậy. Điều này khác với `tezuit`, nơi chỉ quản lý mô tả mục tiêu kiểm thử và các `tezk` nào được thực hiện. Tuy nhiên, với đặc trưng dự án kiểm thử của lõi μEDP, các bước thiết lập môi trường kiểm thử vốn đã có thể được thực hiện bởi generation BSP của platform nên việc thiết lập môi trường kiểm thử trong `tezk` có thể được giảm bớt hoặc bỏ qua, giúp giảm thiểu sự phụ thuộc giữa các testcase và tăng tính tái sử dụng của các hàm chuẩn bị và dọn dẹp.
4. Thiết lập uutobj: Mỗi `tezk` có thể chứa các bước thiết lập uutobj (Unit Under Test Object), giúp đảm bảo rằng testcase được thực hiện trên đối tượng kiểm thử đúng và đầy đủ. Step này khác với bước số 3, vì bước số 3 là thiết lập môi trường kiểm thử, còn bước số 4 là thiết lập đối tượng kiểm thử cụ thể. Do đó, bước này giúp đảm bảo đối tượng kiểm thử đúng trong khi bước số 3 giúp đảm bảo môi trường kiểm thử đúng.
5. Thực hiện testcase: Đây là bước chứa các bước kiểm thử cụ thể và có implementation cụ thể.
6. Xác nhận kết quả: Mỗi `tezk` có thể chứa các bước xác nhận kết quả, giúp đảm bảo rằng testcase được thực hiện đúng và đầy đủ. Điều này khác với `tezuit`, nơi chỉ quản lý mô tả mục tiêu kiểm thử và các `tezk` nào được thực hiện.
7. Dọn dẹp uutobj > môi trường kiểm thử.

Vậy tại sao chúng ta nên có những bước như vậy trong `tezk` thay vì chỉ có `input` > `exec` > `output`?

> Bởi vì trong thực tế, các testcase thường phức tạp hơn và yêu cầu nhiều bước kiểm thử khác nhau để đảm bảo rằng chúng được thực hiện đúng và đầy đủ. Việc có các bước như thiết lập môi trường kiểm thử, thiết lập uutobj, xác nhận kết quả, và dọn dẹp giúp đảm bảo rằng testcase được thực hiện trong một môi trường kiểm thử ổn định và đáng tin cậy, đồng thời giúp giảm thiểu sự phụ thuộc giữa các testcase và tăng tính tái sử dụng của các hàm chuẩn bị và dọn dẹp.

## Đơn vị kiểm thử nhỏ nhất

Dựa trên kiến trúc mà chúng ta đã tham chiếu, đơn vị kiểm thử nhỏ nhất là `teztep` (Test Step) và có mặt trong báo cáo kiểm thử. Chúng cũng có một kiến trúc nhưng ở đơn vị nhỏ hơn là `mô tả > input > exec > output`. Do bản thân nó là đơn vị nhỏ nhất, nó chỉ có thể chứa một bước kiểm thử cụ thể và có implementation cụ thể.

Tuy nhiên, trong thực tế, các testcase thường phức tạp hơn và yêu cầu nhiều bước kiểm thử khác nhau để đảm bảo rằng chúng được thực hiện đúng và đầy đủ. Do đó, ở một số dự án, ranh giới giữa việc tổ chức giữa `tezk` và `teztep` có thể bị mờ nhạt. Một `teztep` có thể chứa nhiều bước kiểm thử khác nhau, nhưng nó vẫn được coi là một đơn vị kiểm thử nhỏ nhất vì nó chỉ có thể chứa một bước kiểm thử cụ thể và có implementation cụ thể. Trong khi đó, `tezk` có thể chứa nhiều `teztep`, giúp tổ chức các testcase phức tạp hơn và đảm bảo rằng chúng được thực hiện đúng và đầy đủ.

## Kho lưu trữ tham số kiểm thử

Việc kiểm thử không phải lúc nào cũng đảm bảo môi trường kiểm thử và các tham số được cung cấp đầy đủ và chính xác. Do đó, việc có một kho lưu trữ tham số kiểm thử giúp đảm bảo rằng các testcase được thực hiện trong một môi trường kiểm thử ổn định và đáng tin cậy, đồng thời giúp giảm thiểu sự phụ thuộc giữa các testcase và tăng tính tái sử dụng của các hàm chuẩn bị và dọn dẹp.

Một pattern có thể sử dụng và được nhìn nhận từ kiến trúc tổng quan của chúng ta chính là việc hỗ trợ mô tả các tham số.

Ví dụ:

```py
xVTask = 80
```

Chúng ta không thể biết được `xVTask` là gì, nó có thể là một tham số kiểm thử, một biến toàn cục, hay một hằng số. Trong khi đó, sử dụng một cấu trúc OOP lại giúp chúng ta mô tả rõ ràng hơn về tham số kiểm thử, bao gồm tên, giá trị, và mô tả về tham số đó. Ví dụ:

```py
class TestParam:
    def __init__(self, name, value):
        self.name = name
        self.value = value
        self.xplain = "Parameter for testing"

self.test_param = TestParam(
  "xVTask", 80,
  "Number of tasks to be created for testing the task management module"
)
```

Sau đó report có thể thể hiện parameter đó khi testcase được thực hiện, giúp người đọc hiểu rõ hơn về các tham số kiểm thử và cách chúng được sử dụng trong testcase. Điều này giúp tăng tính minh bạch và khả năng tái sử dụng của các testcase, đồng thời giúp giảm thiểu sự phụ thuộc giữa các testcase và tăng tính tái sử dụng của các hàm chuẩn bị và dọn dẹp.

## Tái sử dụng testcase và tham số hóa

Với một chức năng cần kiểm thử, nếu có nhiều điều kiện cần kiểm thử, chúng ta có thể tạo ra nhiều testcase khác nhau để kiểm thử các điều kiện đó. Tuy nhiên, việc tạo ra nhiều testcase khác nhau có thể dẫn đến việc lặp lại các bước kiểm thử giống nhau, gây tốn thời gian và công sức. Do đó, việc tái sử dụng testcase và tham số hóa giúp giảm thiểu sự lặp lại và tăng tính linh hoạt của các testcase.

Chúng ta có thể sử dụng các phương pháp như:

- Tham số hóa
- Loop
- Mảng / Danh sách (phụ thuộc vào ngôn ngữ lập trình)
- if/else

Đây là pattern rất phù hợp khi requirement có nhiều boundary/value combinations.

## Phân cấp kết quả

Khi một testcase được thực hiện, kết quả của nó có thể được phân cấp thành nhiều mức độ khác nhau, giúp người đọc hiểu rõ hơn về kết quả của testcase và cách nó được đánh giá. Tuy nhiên, để có được kết quả sau cùng, chúng ta cũng cần có một đánh giá rõ về kết quả của từng bước kiểm thử. Tham khảo từ kiến trúc tổng quan của chúng ta, chúng ta có thể phân cấp kết quả như sau:

- non: `teztep` không có kết quả, ví dụ như các bước in ra log, các bước chuẩn bị môi trường kiểm thử, các bước dọn dẹp môi trường kiểm thử, v.v.
- paz: `teztep` có kết quả đáp ứng đúng với yêu cầu kiểm thử, ví dụ như các bước kiểm thử cụ thể, các bước xác nhận kết quả, v.v.
- fel: `teztep` có kết quả không đáp ứng đúng với yêu cầu kiểm thử, ví dụ như các bước kiểm thử cụ thể, các bước xác nhận kết quả, v.v.
- err: `teztep` có lỗi trong quá trình thực hiện, ví dụ như các bước kiểm thử cụ thể, các bước xác nhận kết quả, v.v.
- inkon: `teztep` có kết quả không xác định, ví dụ như các bước kiểm thử cụ thể, các bước xác nhận kết quả, v.v.

Kết quả sau cùng của một testcase được đánh giá dựa trên kết quả của các bước kiểm thử có mức ưu tiên cao nhất. Nếu có bất kỳ bước kiểm thử nào có kết quả `fel` hoặc `err`, thì kết quả sau cùng của testcase sẽ là `fel` hoặc `err`. Nếu tất cả các bước kiểm thử đều có kết quả `paz`, thì kết quả sau cùng của testcase sẽ là `paz`. Nếu tất cả các bước kiểm thử đều có kết quả `non`, thì kết quả sau cùng của testcase sẽ là `non`. Nếu tất cả các bước kiểm thử đều có kết quả `inkon`, thì kết quả sau cùng của testcase sẽ là `inkon`.

Hay một cách đơn giản hơn: `non < paz < inkon < fel < err`.

## Tích hợp Unity và phân mức kiểm thử (ut / ct / st / it)

Unity (`sources/common/unity`, v2.7.2) là assertion và test runner cho artifact kiểm thử của μEDP. Unity chỉ được link vào executable kiểm thử; không được thêm vào production target, `app.c` hay các firmware target.

`lunity.h` là lớp bao (wrapper) đặt tên theo quy ước của dự án, không thay đổi hành vi của Unity:

| Macro trong `lunity.h` | Hàm Unity tương ứng | Vai trò |
| --- | --- | --- |
| `TEST_SETUP()` / `TEST_TEARDOWN()` | `setUp()` / `tearDown()` | Reset trạng thái trước/sau từng testcase |
| `TESTSUITE_SETUP()` / `TESTSUITE_TEARDOWN(n)` | `suiteSetUp()` / `suiteTearDown(n)` | Chuẩn bị/dọn dẹp toàn suite |
| `TEST_RESET()` / `TEST_VERIFY()` | `resetTest()` / `verifyTest()` | Chạy teardown và kiểm tra cuối testcase |

Lưu ý: các hàm trên là **hook do người dùng cung cấp**. `sources/common/unity/unity.c` chỉ khai báo chúng trong `unity.h`, không định nghĩa. Mỗi file test phải tự định nghĩa những hàm cần dùng; nếu thiếu, link sẽ lỗi.

Quy ước assertion: dùng `TEST_ASSERT_*` / `TEST_ASSERT_EQUAL_*` của Unity. Không tạo assertion framework riêng. Runner gọi `UNITY_BEGIN()`, `RUN_TEST(...)`, `UNITY_END()`, và giá trị trả về của `UNITY_END()` là kết quả để CTest đánh giá.

### Định nghĩa các mức kiểm thử

| Mức | Ý nghĩa | Trạng thái |
| --- | --- | --- |
| `ut` (unit) | Gọi trực tiếp một hàm do codegen sinh (ví dụ `{state}_onst`, `{state}_entry`, `{state}_exit`) với một `uedp_msg_t` dựng tay; assert side-effect. Không qua scheduler, không qua hàng đợi, không cần tick thread. | **Đề xuất**, chưa có runner riêng |
| `ct` (component) | Chạy TSM/FSM đầy đủ của một task qua scheduler, nhưng chặn (stub) các message gửi sang task khác. | **Đề xuất**, cần cơ chế stub cross-task |
| `st` (system) | Nhiều task, qua scheduler thật và PAL Linux thật. | **Đã có** (`tsd_runner` baseline) |
| `it` (integration) | Tích hợp với phần cứng hoặc subsystem thật (ví dụ STM32 thật). | **Ngoài phạm vi** `pycdscriptor`; cần quy trình riêng với board thật |

Hiện tại mọi kịch bản đều chạy như `st`, bất kể nhãn `level` khai báo. Nhãn chỉ đổi cách nhóm báo cáo, chưa đổi cách chạy. Việc để `level` điều khiển cách chạy cần quyết định trước vị trí của trường `level` (trên `tsd` hay `tlc`); xem [docs/review/tlc-impl-post-v123.md](../review/tlc-impl-post-v123.md).

Tích hợp CTest dự kiến: map mỗi mức vào thuộc tính `LABELS` của `add_test()` để chạy `ctest -L <level>`. Đây là thiết kế, chưa được triển khai trong `CMakeLists.txt`.
