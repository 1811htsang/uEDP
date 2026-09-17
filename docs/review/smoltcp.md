# Tài liệu tìm hiểu: smoltcp

> Tài liệu này chỉ nhằm mục đích tìm hiểu smoltcp như một dự án độc lập (kiến trúc, tính năng,
> cách triển khai logic hướng sự kiện) để làm nền tảng đánh giá trước khi thiết kế tích hợp vào
> μEDP. Việc thiết kế API trung gian, ánh xạ tầng Data Link xuống driver WiFi/Ethernet của
> ESP32-S3, và các quyết định tích hợp cụ thể được để lại cho tài liệu thiết kế riêng (mục
> "Bổ sung tài liệu thiết kế chi tiết cho việc tích hợp smoltcp vào μEDP" trong `docs/to-do.md`).

## 1. Giới thiệu chung

[smoltcp](https://github.com/smoltcp-rs/smoltcp) (`smoltcp-rs/smoltcp`, license 0BSD) là một
network stack TCP/IP viết bằng Rust, tự mô tả là *"standalone, event-driven TCP/IP stack that is
designed for bare-metal, real-time systems"*. Hai mục tiêu thiết kế chính được nêu rõ trong README:

- Đơn giản và bền vững (simplicity and robustness).
- Anti-goal: tránh các kỹ thuật tính toán phức tạp lúc biên dịch (macro/type trick kiểu
  Rust nâng cao), kể cả khi phải đánh đổi hiệu năng - ưu tiên dễ đọc/dễ bảo trì hơn là "khéo léo".

Đặc điểm nền tảng quan trọng nhất đối với μEDP: smoltcp không cần cấp phát heap (`no heap
allocation at all`), biên dịch được trên Rust ổn định (không cần nightly, trừ khi bật thêm feature
`alloc`), và có thể chạy hoàn toàn `no_std` (bare-metal). Toàn bộ kích thước buffer/bảng nội bộ
được cấu hình tĩnh lúc biên dịch (qua Cargo feature dạng `<name>-<value>` hoặc biến môi trường
`SMOLTCP_<value>`), không có con số nào được cấp phát động lúc runtime - đúng triết lý mà μEDP
cũng đang theo đuổi (Task table, GDP, PSE... đều là bảng tĩnh khai báo lúc biên dịch).

## 2. Kiến trúc phân lớp

smoltcp tách thành 4 module chính, xếp chồng lên nhau:

```text
┌─────────────────────────────────────────┐
│ socket   - TCP/UDP/ICMP/Raw/DHCPv4/DNS  │  <- API mà ứng dụng gọi trực tiếp
├─────────────────────────────────────────┤
│ iface  - Interface: routing, poll()     │  <- "bộ não" điều phối, chạy vòng lặp sự kiện
├─────────────────────────────────────────┤
│ wire   - Parse/serialize các giao thức  │  <- không giữ state, chỉ đọc/ghi định dạng gói tin
├─────────────────────────────────────────┤
│ phy    - Device trait (token-based)     │  <- cầu nối xuống driver phần cứng thật
└─────────────────────────────────────────┘
```

### 2.1 `phy` - lớp thiết bị vật lý

`phy::Device` là trait duy nhất người tích hợp cần tự implement để nối smoltcp với phần cứng thật
(driver Ethernet/WiFi/loopback...):

```rust
pub trait Device {
  type RxToken<'a>: RxToken where Self: 'a;
  type TxToken<'a>: TxToken where Self: 'a;
  fn receive(&mut self, timestamp: Instant) -> Option<(Self::RxToken<'_>, Self::TxToken<'_>)>;
  fn transmit(&mut self, timestamp: Instant) -> Option<Self::TxToken<'_>>;
  fn capabilities(&self) -> DeviceCapabilities;
}
```

Điểm thiết kế đáng chú ý: `receive()`/`transmit()` không trả trực tiếp buffer, mà trả về 1
token - một giá trị nhỏ cho phép "tiêu thụ" đúng 1 gói tin. Việc thật sự đọc/ghi dữ liệu chỉ
xảy ra khi token được dùng (gọi `.consume()`), không phải lúc gọi `receive()`/`transmit()`. Cách
làm này giúp tránh copy dữ liệu thừa và cho phép implementation tuỳ biến hoàn toàn cách cấp phát
buffer bên dưới - khớp với hướng tiếp cận của μEDP là để tầng PAL tự quyết định cách cấp phát,
tầng lõi chỉ định nghĩa "hợp đồng" (interface).

### 2.2 `wire` - lớp phân tích/tạo gói tin

Chứa các struct để đọc (parse) và ghi (emit) từng loại gói tin (`EthernetFrame`, `Ipv4Packet`,
`Ipv6Packet`, `TcpPacket`, `UdpPacket`, `ArpPacket`, `Icmpv4Packet`...). Lớp này không giữ
trạng thái (stateless) - chỉ là các hàm/struct thao tác trực tiếp trên slice byte, tương tự cách
`sources/pal/service/rprintf`/`xprintf` của μEDP chỉ format chuỗi mà không giữ state riêng.

### 2.3 `iface::Interface` - bộ điều phối trung tâm

Đây là phần quan trọng nhất để hiểu cách smoltcp triển khai mô hình hướng sự kiện - xem mục 3.

### 2.4 `socket` + `SocketSet` - API cho tầng ứng dụng

`SocketSet` là một tập hợp (giống pool tĩnh) chứa các socket đã cấp `SocketHandle`. Mỗi socket
(`TcpSocket`, `UdpSocket`, `IcmpSocket`, `RawSocket`, `Dhcpv4Socket`, `DnsSocket`) tự quản lý buffer
gửi/nhận riêng và một máy trạng thái giao thức riêng (ví dụ `TcpSocket` có đầy đủ state machine
TCP: `LISTEN`, `SYN-SENT`, `ESTABLISHED`, `TIME-WAIT`...). Ứng dụng không gọi socket trực tiếp kiểu
blocking `read()`/`write()` như POSIX - xem mục 4.

## 3. Vòng lặp sự kiện: `Interface::poll()`

Đây chính là câu trả lời cho "smoltcp triển khai logic xử lý mạng hướng sự kiện như thế nào":
không có thread nền, không có callback ngắt riêng cho từng gói tin, không có blocking I/O.
Toàn bộ engine chỉ xoay quanh một hàm duy nhất ứng dụng phải tự gọi lặp lại:

```rust
pub fn poll(&mut self, timestamp: Instant, device: &mut D, sockets: &mut SocketSet) -> bool
```

Mỗi lần gọi, `poll()` thực hiện tuần tự 3 giai đoạn:

1. Egress: duyệt qua từng socket trong `SocketSet`, với socket nào có dữ liệu đang chờ gửi thì
   đóng gói (theo đúng giao thức: TCP/UDP/ICMP...) và đẩy xuống `Device::transmit()`.
2. Maintenance: xử lý các việc nền không thuộc về 1 socket cụ thể - làm mới ARP/Neighbor cache,
   trả lời ARP request, gia hạn DHCP lease, gửi lại Router Solicitation, dọn timer nội bộ (ví dụ
   retransmission timeout của TCP)...
3. Ingress: đọc gói tin từ `Device::receive()`, parse qua tầng `wire`, tìm đúng socket đích
   trong `SocketSet` theo cổng/địa chỉ, cập nhật state machine và buffer của socket đó.

Hàm trả về `true` nếu trạng thái của bất kỳ socket nào đã thay đổi (có dữ liệu mới, đổi state,
đóng kết nối...) - ứng dụng dùng giá trị này để biết có nên "đánh thức" logic xử lý của mình hay
không, giống hệt tinh thần giá trị trả về của `uedp_task_norm_post_isr()`/kết quả
`uedp_timer_tick()` báo hiệu "có việc cần xử lý" trong μEDP.

Điểm mấu chốt cho một hệ nhúng không có preemptive scheduling (hoặc `poll()` được gọi ngay
trong main loop cùng nhiều việc khác): thay vì `poll()` xử lý TOÀN BỘ gói tin đang chờ trong 1 lần
gọi (có thể mất thời gian không xác định trước, phá vỡ tính real-time), smoltcp cung cấp thêm 3
hàm mức thấp hơn để chia nhỏ công việc:

- `poll_egress()` - chỉ gửi các gói đang chờ, đảm bảo lượng công việc bị chặn trên (bounded work).
- `poll_maintenance()` - chỉ chạy phần việc nền.
- `poll_ingress_single()` - chỉ xử lý đúng 1 gói tin đến, trả về biết còn gói nào đang chờ
  trong hàng đợi của device hay không (để gọi lại nếu cần), và biết state có đổi hay không - cho
  phép ứng dụng chèn các việc khác (yield, xử lý task khác) giữa các gói tin, tương tự cách
  `uedp_task_scheduler()` của μEDP chỉ lấy đúng 1 message mỗi vòng lặp thay vì xử lý cạn hàng
  đợi của 1 task trong 1 lần gọi.

Để biết khi nào nên gọi `poll()` lần tiếp theo (thay vì gọi liên tục tốn CPU, hoặc chờ quá lâu
làm chậm phản hồi timeout/retransmission nội bộ), smoltcp cung cấp:

- `poll_at(timestamp, sockets) -> Option<Instant>`: thời điểm tuyệt đối cần gọi `poll()` tiếp theo
  (ví dụ: khi nào 1 timer TCP retransmission sẽ hết hạn).
- `poll_delay(timestamp, sockets) -> Option<Duration>`: tương đương nhưng ở dạng khoảng thời gian
  còn lại.

Ứng dụng bare-metal điển hình sẽ kết hợp giá trị này với cơ chế "ngủ tới khi có 1 trong 2 điều kiện
xảy ra trước": (a) đến đúng thời điểm `poll_at()` yêu cầu, hoặc (b) có ngắt phần cứng báo "có gói
tin mới đến" - đây chính là mô hình 1 vòng lặp trung tâm dựa trên tick/deadline mà μEDP cũng đang
dùng cho `uedp_timer_tick()`/`uedp_task_scheduler()`, nên về mặt triết lý, việc nhúng
`Interface::poll()` vào vòng lặp chính của μEDP là hợp lý (chi tiết cách nối 2 vòng lặp này với
nhau thuộc phạm vi tài liệu thiết kế tích hợp riêng, không phải tài liệu này).

## 4. Mô hình socket: polling, không blocking

Khác với socket POSIX (`read()`/`write()` blocking chờ dữ liệu), socket của smoltcp được thao tác
theo kiểu non-blocking, tự kiểm tra trạng thái trước khi gọi:

```rust
if socket.can_recv() {
  let data = socket.recv(|buffer| { /* đọc buffer, trả về (số byte đã dùng, giá trị) */ });
}
if socket.can_send() {
  socket.send_slice(b"hello")?;
}
```

Ứng dụng thường kiểm tra các cờ này ngay sau mỗi lần `poll()` trả về `true` (có thay đổi state) - một vòng lặp điển hình là: `poll()` → duyệt qua từng socket cần quan tâm, kiểm tra
`can_recv()`/`may_send()`/`is_active()` → xử lý dữ liệu tương ứng → lặp lại. Mô hình này không cần
thread riêng cho mỗi kết nối (khác hẳn mô hình 1-thread-per-connection kiểu POSIX/BSD socket
truyền thống), phù hợp trực tiếp với môi trường 1 vòng lặp scheduler duy nhất như μEDP.

## 5. Tính năng hỗ trợ (tính đến bản mới nhất tại thời điểm viết tài liệu này, v0.13.0)

| Lớp | Đã hỗ trợ | Chưa hỗ trợ / hạn chế |
| --- | --- | --- |
| Media | Ethernet II, ARP (gratuitous request/reply, rate-limit 1/s, cache hết hạn sau 1 phút), IP thô, IEEE 802.15.4 (chỉ data frame) | 802.3/802.1Q, Jumbo frame |
| IPv4 | Checksum, TTL cấu hình theo socket (mặc định 64), default gateway, routing qua bảng CIDR, fragmentation + reassembly | IPv4 options (bị bỏ qua âm thầm) |
| IPv6 | Hop-limit theo socket, routing, hop-by-hop header, ICMPv6 parameter problem cho next-header lạ | ICMPv6 parameter problem cho hop-by-hop option lạ |
| 6LoWPAN | RFC6282 (nén header), fragmentation theo RFC4944, nén/giải nén UDP header và extension header | Uncompressed IPv6 extension header |
| IP multicast | IGMPv1/v2 | - |
| ICMP | ICMPv4 + ICMPv6 (checksum, echo reply tự động, ICMP socket lắng nghe port-unreachable), NDISC (Neighbor Advertisement, Router Solicitation) | ICMPv4 parameter problem, Router Advertisement tự sinh, Redirect message |
| UDP | Đầy đủ trên cả IPv4/IPv6, tự sinh ICMP destination-unreachable khi cổng không ai lắng nghe | - |
| TCP | Checksum, đàm phán MSS, window scaling, gửi nhiều gói không chờ ACK, reassembly out-of-order (tối đa 4-32 khoảng hở), keep-alive, RTO theo ước lượng RTT (tăng gấp đôi mỗi lần), time-wait cố định 10s, user timeout, delayed ACK, thuật toán Nagle, congestion control CUBIC/Reno | Selective ACK (SACK), chống silly window syndrome, timestamping, urgent pointer (bị bỏ qua), PLPMTU |
| Socket type | `socket-raw`, `socket-udp`, `socket-tcp`, `socket-icmp`, `socket-dhcpv4`, `socket-dns` (đều bật mặc định, có thể tắt từng cái qua Cargo feature) | - |

Điểm cần lưu ý khi đánh giá tích hợp: bảng trên cho thấy smoltcp không phải một TCP/IP stack
"đầy đủ" theo chuẩn RFC ở mọi khía cạnh (thiếu SACK có thể ảnh hưởng hiệu năng khi mất gói trên
mạng lossy, thiếu PLPMTU nghĩa là phải tự cấu hình MTU thủ công/an toàn thay vì auto-discover) -
nhưng đây là đánh đổi có chủ đích để giữ engine nhỏ gọn, đúng tinh thần "smol" trong tên gọi.

## 6. Cấu hình tĩnh, không cấp phát động

Toàn bộ kích thước bảng/buffer nội bộ (số địa chỉ IP tối đa trên 1 interface, số route tối đa, số
entry neighbor cache, số buffer reassembly, số kết quả DNS giữ lại...) được set lúc biên dịch,
qua 1 trong 2 cách:

- Cargo feature dạng `<name>-<value>` (ví dụ `iface-max-addr-count-3`).
- Biến môi trường `SMOLTCP_<VALUE>` lúc build (ví dụ `SMOLTCP_IFACE_MAX_ADDR_COUNT=3 cargo build`),
  biến môi trường được ưu tiên hơn Cargo feature nếu cả 2 cùng đặt.

Cách làm này tương đương tinh thần Kconfig của μEDP (`UEDP_TIMER_MAX_NODES`,
`UEDP_GDP_MAX_SLOTS`...) - số lượng slot cố định, không có `malloc`/`Box` nào phát sinh lúc runtime
nếu không bật feature `alloc`/`std`. Đây là điểm tương thích triết lý quan trọng nhất giữa smoltcp
và μEDP, và là lý do smoltcp được cân nhắc thay vì một TCP/IP stack C truyền thống khác.

## 7. Điểm cần lưu ý khi đối chiếu với μEDP (chỉ ghi nhận, chưa thiết kế)

- Ngôn ngữ khác nhau: smoltcp viết bằng Rust, μEDP viết bằng C - cần một lớp FFI/API trung gian
  (`cbindgen` hoặc viết tay `extern "C"` wrapper) để μEDP gọi được vào smoltcp. Đây là điểm phức
  tạp nhất, đã được `docs/to-do.md` ghi nhận là cần "API trung gian".
- smoltcp chỉ dừng ở tầng Network/Transport (layer 3-4): không tự xử lý tầng Data Link (layer
  2) như driver WiFi/Ethernet thật - cần tự viết `phy::Device` implementation nối xuống driver
  WiFi/Ethernet của ESP32-S3 (SDK ESP-IDF), như `docs/to-do.md` đã ghi nhận.
- Mô hình vòng lặp poll() rất khớp với vòng lặp scheduler hiện có của μEDP (mục 3) - về lý
  thuyết, việc nhúng 1 lời gọi `poll()`/`poll_at()` vào đâu đó gần `uedp_task_scheduler()` hoặc
  như 1 OCE service (`uedp_ocesvc`) là hướng tiếp cận hợp lý để đánh giá tiếp, nhưng cần thiết kế
  cụ thể ở tài liệu tích hợp riêng.
- Giấy phép 0BSD (permissive, tương tự MIT của μEDP) - không có rào cản pháp lý khi nhúng vào
  dự án MIT.
- Cộng đồng đã có báo cáo thử nghiệm tích hợp smoltcp vào ESP32-P4 qua ESP-IDF - nên tham khảo tài
  liệu/hướng dẫn của họ trước khi tự triển khai lại từ đầu trên ESP32-S3 (đã ghi trong
  `docs/to-do.md`, mục "Phiên bản 1.2.4").

## 8. Nguồn tham khảo

- Repo chính: <https://github.com/smoltcp-rs/smoltcp>
- Docs API: <https://docs.rs/smoltcp>
- Changelog: <https://github.com/smoltcp-rs/smoltcp/blob/main/CHANGELOG.md>
