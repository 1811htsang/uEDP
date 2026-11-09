# Review: Thiết kế FFI/API cho tích hợp smoltcp (Rust) vào μEDP (C)

<!-- STATUS - PROPOSAL: thiết kế FFI/API chi tiết, chưa có dòng code nào được viết -->

- Ngày viết: 2026-09-23
- Liên quan: [`smoltcp.md`](./smoltcp.md) (review kiến trúc tổng quan smoltcp — tài liệu này KHÔNG lặp lại phần đó, chỉ tham chiếu), [`docs/to-do.md`](../to-do.md) (mục v1.2.4 "The integration of smoltcp")
- Phạm vi: dựa trên đọc trực tiếp mã nguồn crate `smoltcp` (v0.14.0, edition 2024) vendor tại `smoltcp/` trong repo, đối chiếu với API C hiện có của μEDP (`sources/core/inc/`, `sources/pal/`).
- Tiền đề: `smoltcp.md` §7 đã ghi rõ yêu cầu cần có lldriver Ethernet/WiFi ở tầng Data Link — tài liệu này **không** thiết kế lldriver đó (ngoài phạm vi), chỉ thiết kế đúng ranh giới hợp đồng (contract) mà một lldriver tương lai phải hiện thực để cắm vào FFI layer mô tả dưới đây.

## 1. Mục tiêu

Trả lời câu hỏi: μEDP (viết bằng C, static-allocation, single-core non-preemptive scheduler) gọi vào smoltcp (viết bằng Rust, generic/GAT-heavy, closure-based) bằng cách nào, cụ thể là những hàm `extern "C"` nào cần viết, kiểu dữ liệu nào cần chuyển đổi thủ công, và smoltcp nên được "lắp" vào đâu trong vòng đời chạy của μEDP.

## 2. Vì sao không thể bind trực tiếp — 4 rào cản cụ thể từ mã nguồn

### 2.1. `phy::Device` dùng GAT — không thể expose generic qua FFI

`phy::mod.rs` định nghĩa:

```rust
pub trait Device {
    type RxToken<'a>: RxToken where Self: 'a;
    type TxToken<'a>: TxToken where Self: 'a;
    fn receive(&mut self, timestamp: Instant) -> Option<(Self::RxToken<'_>, Self::TxToken<'_>)>;
    fn transmit(&mut self, timestamp: Instant) -> Option<Self::TxToken<'_>>;
    fn capabilities(&self) -> DeviceCapabilities;
}
```

`RxToken<'a>`/`TxToken<'a>` là generic associated type — trait này không dyn-compatible, không thể "để C tự implement một Device nào đó rồi truyền vào". Ngược lại: phải viết **một struct Rust cụ thể** (`EdpFfiDevice`, xem §4) implement `Device`, và struct đó nội bộ gọi ra các hàm C (`extern "C"` function pointer do lldriver đăng ký) để lấy/gửi khung dữ liệu thật.

### 2.2. `RxToken`/`TxToken::consume()` nhận closure — không qua được FFI

```rust
pub trait RxToken { fn consume<R, F: FnOnce(&[u8]) -> R>(self, f: F) -> R; }
pub trait TxToken { fn consume<R, F: FnOnce(&mut [u8]) -> R>(self, len: usize) -> R; }
```

`Interface::poll()` tự gọi `consume()` nội bộ trong quá trình parse/emit gói tin — closure `f` này do chính smoltcp cung cấp, không phải do μEDP viết. Vì vậy đây **không phải** điểm cần FFI hoá — chỉ cần token cụ thể (`EdpRxToken`/`EdpTxToken`, xem §4.2) sở hữu con trỏ+độ dài buffer thô (nhận từ callback C) và trả slice cho closure nội bộ dùng. C không bao giờ nhìn thấy hay gọi `consume()`.

### 2.3. `tcp::Socket`/`udp::Socket` có 2 nhóm API — chỉ 1 nhóm FFI-friendly

`socket/tcp.rs`, `socket/udp.rs` có cả `send()/recv()` (nhận closure `FnOnce(&mut [u8]) -> (usize, R)`) lẫn `send_slice()/recv_slice()/peek_slice()` (nhận thẳng `&[u8]`/`&mut [u8]`, trả `Result<usize, Error>`). **Chỉ nhóm `*_slice()` được wrap cho FFI** — đây chính là các hàm phù hợp với C, không cần closure.

### 2.4. Kiểu địa chỉ/wire là enum không `#[repr(C)]` — không transmute được

`wire/ip.rs`: `IpAddress::Ipv4(Ipv4Address)` / `Ipv6(Ipv6Address)` (payload 4 byte vs 16 byte, discriminant ẩn, không có `#[repr(...)]`); tương tự `IpCidr`, `IpListenEndpoint`, `HardwareAddress` (`phy/mod.rs`, gồm `Ethernet(EthernetAddress)`/`Ip`). Layout các enum này **không được Rust đảm bảo ổn định qua ABI** — không thể `#[repr(C)]` trực tiếp lên type của smoltcp (không sở hữu crate đó để sửa) và không an toàn khi transmute sang struct C tự định nghĩa.

**Ngược lại**, các type sau AN TOÀN để marshal trực tiếp vì là newtype phẳng quanh mảng byte hoặc số nguyên, không có discriminant:

- `Ipv4Address`/`Ipv6Address` (alias `core::net::Ipv4Addr`/`Ipv6Addr`) → `[u8; 4]` / `[u8; 16]`.
- `EthernetAddress` → `[u8; 6]`.
- `Instant` (`time.rs`) → `i64` (micros kể từ epoch tuỳ ý).
- `Duration` (`time.rs`) → `u64` (micros).
- `SocketHandle` (`iface/socket_set.rs`) → `Copy` newtype quanh `usize`, nhưng **phải giữ opaque** ở phía C (xem §5.4) vì trường bên trong là `private` — không được C tự dựng giá trị, chỉ được truyền lại nguyên vẹn giá trị mà `edp_socket_set_add_*` đã trả về.

Kết luận mục 2: cần một lớp Rust `#[no_mangle] extern "C"` (gọi là **FFI shim crate**, ví dụ `edp-smoltcp-ffi`, tách khỏi crate `smoltcp` vendor — không sửa trực tiếp source vendor) đứng giữa, tự tay viết converter cho từng enum ở trên và bọc lại toàn bộ API bằng slice/con trỏ/số nguyên thuần.

## 3. Cấu trúc crate đề xuất

```structure
smoltcp/                  <-- vendor, KHÔNG sửa
edp-smoltcp-ffi/          <-- crate mới, thư viện Rust, crate-type = ["staticlib"]
  Cargo.toml              <-- dependency path = "../smoltcp", default-features = false,
                              features = ["medium-ethernet", "proto-ipv4", "socket-tcp", "socket-udp"]
  src/
    lib.rs                <-- #![no_std], khai báo panic_handler nếu cần cho target embedded
    device.rs              <-- EdpFfiDevice, EdpRxToken, EdpTxToken (§4)
    iface.rs                <-- wrapper Interface (§5.1-5.3)
    sockset.rs              <-- wrapper SocketSet + storage tĩnh (§5.4-5.5)
    tcp.rs                   <-- wrapper tcp::Socket (§5.6)
    udp.rs                   <-- wrapper udp::Socket (§5.7)
    types.rs                 <-- struct C tagged cho IpAddress/IpCidr/IpListenEndpoint/HardwareAddress (§6)
    time.rs                  <-- Instant/Duration <-> i64/u64 (§7)
sources/pal/net/edp_net.h    <-- header C khai báo các extern "C" trên, do người dùng μEDP include
```

Lý do tách crate riêng thay vì sửa `smoltcp/` trực tiếp: giữ khả năng `cargo update` vendor sau này mà không mất patch; giữ đúng ranh giới license (0BSD của smoltcp không đổi, code shim mới có thể theo license MIT của μEDP).

## 4. Thiết kế `Device`: cầu nối GAT-token ↔ C callback

### 4.1. Hợp đồng C mà lldriver phải hiện thực (đây là "chân cắm" cho Ethernet/WiFi thật sau này)

```c
// sources/pal/net/edp_net.h — struct này KHÔNG đổi khi lldriver thay đổi
typedef struct edp_netdev_ops_t {
    /* Trả về con trỏ+độ dài khung nhận được, hoặc size=0 nếu không có gì để nhận.
       Con trỏ do driver sở hữu, chỉ cần sống tới khi edp_netdev_ops_t.rx_release được gọi. */
    ui8* (*rx_poll)(void* ctx, ui16* out_len);
    void (*rx_release)(void* ctx, ui8* buf);

    /* Cấp một buffer trống độ dài >= len để FFI layer ghi khung cần gửi vào,
       sau đó driver tự gửi đi khi edp_netdev_ops_t.tx_submit được gọi. */
    ui8* (*tx_alloc)(void* ctx, ui16 len);
    void (*tx_submit)(void* ctx, ui8* buf, ui16 len);

    ui16       mtu;             /* DeviceCapabilities::max_transmission_unit */
    ui8        hwaddr[6];       /* EthernetAddress */
    void*      ctx;             /* con trỏ ngữ cảnh riêng của driver, FFI layer không đụng vào */
} edp_netdev_ops_t;
```

Đây chính là ranh giới "sạch" nói ở đầu tài liệu: lldriver Ethernet/WiFi tương lai chỉ cần implement 4 hàm này cho phần cứng thật (ESP32-S3 WiFi, MAC Ethernet STM32H723, …) — không cần biết gì về Rust/smoltcp.

### 4.2. Struct Rust `EdpFfiDevice` (nội bộ FFI shim, không lộ ra C)

```rust
pub struct EdpFfiDevice { ops: *const edp_netdev_ops_t }

pub struct EdpRxToken { buf: *mut u8, len: u16, ops: *const edp_netdev_ops_t }
pub struct EdpTxToken { ops: *const edp_netdev_ops_t }

impl RxToken for EdpRxToken {
    fn consume<R, F: FnOnce(&[u8]) -> R>(self, f: F) -> R {
        let slice = unsafe { core::slice::from_raw_parts(self.buf, self.len as usize) };
        let r = f(slice);
        unsafe { ((*self.ops).rx_release)((*self.ops).ctx, self.buf) };
        r
    }
}
impl TxToken for EdpTxToken {
    fn consume<R, F: FnOnce(&mut [u8]) -> R>(self, len: usize) -> R {
        let buf = unsafe { ((*self.ops).tx_alloc)((*self.ops).ctx, len as u16) };
        let slice = unsafe { core::slice::from_raw_parts_mut(buf, len) };
        let r = f(slice);
        unsafe { ((*self.ops).tx_submit)((*self.ops).ctx, buf, len as u16) };
        r
    }
}

impl Device for EdpFfiDevice {
    type RxToken<'a> = EdpRxToken;
    type TxToken<'a> = EdpTxToken;
    fn receive(&mut self, _ts: Instant) -> Option<(EdpRxToken, EdpTxToken)> {
        let mut len: u16 = 0;
        let buf = unsafe { ((*self.ops).rx_poll)((*self.ops).ctx, &mut len) };
        if buf.is_null() || len == 0 { return None; }
        Some((EdpRxToken { buf, len, ops: self.ops }, EdpTxToken { ops: self.ops }))
    }
    fn transmit(&mut self, _ts: Instant) -> Option<EdpTxToken> {
        Some(EdpTxToken { ops: self.ops })
    }
    fn capabilities(&self) -> DeviceCapabilities {
        let mut caps = DeviceCapabilities::default();
        caps.max_transmission_unit = unsafe { (*self.ops).mtu as usize };
        caps.medium = Medium::Ethernet;
        caps
    }
}
```

Ghi chú `#![deny(unsafe_code)]` của crate `smoltcp` vendor không áp dụng cho crate shim mới (`edp-smoltcp-ffi`) — `unsafe` là bắt buộc và đúng chỗ ở lớp biên FFI này (deref con trỏ thô từ C), cần review kỹ 4 hàm trên là nơi duy nhất chứa `unsafe` của toàn thiết kế.

## 5. API `extern "C"` — Interface / SocketSet / Socket

### 5.1. Khởi tạo Interface

```c
typedef struct edp_iface_t edp_iface_t;   /* opaque, forward-declare only trong header C */

edp_iface_t* edp_iface_create(const edp_netdev_ops_t* ops, const ui8 hwaddr[6], i64 now_us);
void         edp_iface_destroy(edp_iface_t* iface);
RETR_STAT    edp_iface_add_ipv4(edp_iface_t* iface, ui8 a, ui8 b, ui8 c, ui8 d, ui8 prefix_len);
```

`edp_iface_create` nội bộ gọi `Config::new(HardwareAddress::Ethernet(...))` → `Interface::new(config, &mut device, Instant::from_micros(now_us))`, `Box::leak` (hoặc, nếu build `no_std` không có `alloc`, đặt trong static `MaybeUninit` — xem §8 về static allocation) kết quả thành con trỏ trả về C dưới dạng `edp_iface_t*` opaque.

### 5.2. Vòng lặp poll — điểm tích hợp chính với scheduler μEDP

```c
bool edp_iface_poll(edp_iface_t* iface, edp_sockset_t* set, i64 now_us);
i64  edp_iface_poll_delay_us(edp_iface_t* iface, edp_sockset_t* set, i64 now_us); /* -1 nếu không giới hạn */
```

`edp_iface_poll` gọi thẳng `Interface::poll(Instant::from_micros(now_us), &mut device, &mut sockets)`, trả `bool` y hệt smoltcp (`true` nếu có socket đổi trạng thái đáng xử lý tiếp). `edp_iface_poll_delay_us` bọc `Interface::poll_delay()` — giá trị này dùng để μEDP quyết định khi nào gọi lại `edp_iface_poll` (xem §9 về nơi đặt lời gọi này trong scheduler).

### 5.3. Địa chỉ IP hiện tại (đọc, phục vụ debug/log)

```c
ui16 edp_iface_get_ipv4_count(edp_iface_t* iface);
void edp_iface_get_ipv4_at(edp_iface_t* iface, ui16 idx, ui8 out_addr[4], ui8* out_prefix_len);
```

### 5.4. SocketSet — quản lý bằng storage tĩnh, opaque handle

```c
typedef struct edp_sockset_t edp_sockset_t;
typedef ui32 edp_sockhandle_t;   /* opaque: giá trị nội bộ Rust, C KHÔNG được tự tạo, chỉ truyền lại */

edp_sockset_t* edp_sockset_create(void* storage, ui16 storage_slots);
```

`storage`/`storage_slots` trỏ tới một mảng `SocketStorage` tĩnh cấp phát phía Rust static (`static mut EDP_SOCK_STORAGE: [MaybeUninit<SocketStorage>; N]`) — μEDP chỉ cần khai báo `N` (số socket tối đa) tại compile-time qua một macro cấu hình (xem §8), không cần biết layout thật của `SocketStorage`. `SocketSet::new(&mut storage[..])` dùng `ManagedSlice::Borrowed` — không đụng heap.

`SocketHandle` là newtype riêng tư quanh `usize` — thiết kế `edp_sockhandle_t` chỉ là `usize` được cast nguyên vẹn qua `transmute` một chiều (Rust → C khi trả về từ `add`, C → Rust khi truyền vào `get`/`remove`); không có API C nào cho phép dựng handle từ số tuỳ ý.

### 5.5. Thêm/gỡ socket — 1 hàm non-generic cho mỗi loại socket cụ thể

Vì `SocketSet::add<T: AnySocket>()` là generic, cần 2 cặp hàm cụ thể hoá thủ công (không thể sinh 1 hàm C chung):

```c
edp_sockhandle_t edp_sockset_add_tcp(edp_sockset_t* set, edp_tcp_socket_t* sock);
edp_sockhandle_t edp_sockset_add_udp(edp_sockset_t* set, edp_udp_socket_t* sock);
void             edp_sockset_remove(edp_sockset_t* set, edp_sockhandle_t h);
```

`edp_tcp_socket_t`/`edp_udp_socket_t` (opaque) được tạo trước bằng `edp_tcp_socket_create`/`edp_udp_socket_create` (§5.6/5.7) rồi mới `add` vào set — theo đúng thứ tự mã nguồn `examples/loopback.rs` (`tcp::Socket::new(rx,tx)` → `sockets.add(socket)`).

### 5.6. TCP socket — chỉ wrap các hàm `*_slice`

```c
typedef struct edp_tcp_socket_t edp_tcp_socket_t;

edp_tcp_socket_t* edp_tcp_socket_create(void* rx_buf, ui16 rx_len, void* tx_buf, ui16 tx_len);
RETR_STAT edp_tcp_listen(edp_sockset_t* set, edp_sockhandle_t h, ui16 port);
RETR_STAT edp_tcp_connect(edp_iface_t* iface, edp_sockset_t* set, edp_sockhandle_t h,
                           const edp_ip_endpoint_t* remote, ui16 local_port);
bool      edp_tcp_can_send(edp_sockset_t* set, edp_sockhandle_t h);
bool      edp_tcp_can_recv(edp_sockset_t* set, edp_sockhandle_t h);
i32       edp_tcp_send_slice(edp_sockset_t* set, edp_sockhandle_t h, const ui8* data, ui16 len); /* trả số byte đã gửi, -1 nếu lỗi */
i32       edp_tcp_recv_slice(edp_sockset_t* set, edp_sockhandle_t h, ui8* out_buf, ui16 max_len);
bool      edp_tcp_is_active(edp_sockset_t* set, edp_sockhandle_t h);
void      edp_tcp_close(edp_sockset_t* set, edp_sockhandle_t h);
```

`rx_buf`/`tx_buf` do phía C cấp (static array, đúng triết lý μEDP) — Rust dựng `SocketBuffer::new(unsafe { slice::from_raw_parts_mut(rx_buf, rx_len) })` từ đó, không tự cấp phát.

`edp_tcp_socket_create` trả socket **chưa nằm trong set** — caller phải gọi `edp_sockset_add_tcp` ngay sau đó để lấy `edp_sockhandle_t` trước khi gọi bất kỳ hàm nào khác ở trên (mọi hàm còn lại thao tác qua `set`+`handle`, không giữ con trỏ `edp_tcp_socket_t*` sau khi đã add — giống hệt quy tắc borrow của `SocketSet::get_mut::<tcp::Socket>(handle)` trong Rust gốc).

### 5.7. UDP socket

```c
typedef struct edp_udp_socket_t edp_udp_socket_t;

edp_udp_socket_t* edp_udp_socket_create(void* rx_meta_buf, ui16 rx_meta_cnt, void* rx_buf, ui16 rx_len,
                                         void* tx_meta_buf, ui16 tx_meta_cnt, void* tx_buf, ui16 tx_len);
RETR_STAT edp_udp_bind(edp_sockset_t* set, edp_sockhandle_t h, ui16 port);
i32       edp_udp_send_slice(edp_sockset_t* set, edp_sockhandle_t h, const ui8* data, ui16 len,
                              const edp_ip_endpoint_t* dst);
i32       edp_udp_recv_slice(edp_sockset_t* set, edp_sockhandle_t h, ui8* out_buf, ui16 max_len,
                              edp_ip_endpoint_t* out_src);
```

`udp::PacketMetadata`/`UdpMetadata` cũng cần buffer tĩnh riêng (`rx_meta_buf`/`tx_meta_buf`) giống `tcp::SocketBuffer` — smoltcp UDP dùng ring buffer 2 lớp (data + metadata) để giữ được biên gói tin và địa chỉ nguồn/đích cho từng datagram.

## 6. Kiểu địa chỉ — tagged struct C thay cho enum Rust

```c
typedef enum edp_ip_addr_kind_t { EDP_IP_V4 = 0, EDP_IP_V6 = 1 } edp_ip_addr_kind_t;

typedef struct edp_ip_addr_t {
    edp_ip_addr_kind_t kind;
    ui8 bytes[16];   /* IPv4 dùng 4 byte đầu, còn lại bỏ qua */
} edp_ip_addr_t;

typedef struct edp_ip_endpoint_t {
    edp_ip_addr_t addr;
    ui16          port;
} edp_ip_endpoint_t;
```

Rust shim tự viết converter 2 chiều (`From<edp_ip_addr_t> for IpAddress`, ngược lại) — không transmute, luôn qua `match`/constructor tường minh, đúng như nhận định ở §2.4. Đây là phần code "tốn dòng nhưng không rủi ro" của FFI shim — thuần copy byte theo `kind`, không có logic phức tạp.

## 7. Thời gian: `Instant`/`Duration` (i64/u64 micro) ↔ `uedp_timer_get_systick()` (ui32)

`time.rs`: `Instant` bọc `i64` micro giây kể từ một epoch tuỳ ý (không nhất thiết Unix epoch); `Duration` bọc `u64` micro giây — cả hai đã là số nguyên phẳng, marshal trực tiếp qua FFI không cần converter.

Vấn đề thật nằm ở phía μEDP: `uedp_timer_get_systick()` (`sources/core/inc/uedp_timer.h`) trả `ui32`, đơn vị tick — tài liệu timer hiện có không cam kết rõ tick = 1ms hay đơn vị khác (tuỳ cấu hình `pal_timer_get_freq()`/kiến trúc). Đề xuất cầu nối, đặt trong `sources/pal/net/edp_net_time.c` (C thuần, không phải Rust):

```c
static inline i64 edp_net_now_us(void) {
    /* Giả định 1 tick = 1ms (xác nhận lại với từng arch trước khi dùng thật) */
    return (i64)uedp_timer_get_systick() * 1000;
}
```

**Rủi ro cần chốt trước khi code**: nếu tick không cố định 1ms trên mọi arch (STM32F103/STM32H723/ESP32S3/LINUX), hàm trên sai đơn vị âm thầm — cần xác nhận với tài liệu PAL timer từng arch, hoặc thêm một hằng số cấu hình `UEDP_NET_TICK_US` per-PLAT thay vì hardcode `1000`. `ui32` tick cũng sẽ tràn sau ~49 ngày ở 1ms/tick — với `i64` micro của smoltcp thì không tràn trong phạm vi thực tế, nhưng phép nhân `* 1000` phải thực hiện sau khi đã cast lên `i64` (như viết ở trên), không được nhân ở kiểu `ui32` trước rồi mới cast (sẽ tràn số ở phép nhân chứ không phải ở kiểu đích).

## 8. Static allocation — khớp triết lý μEDP, không cần `alloc`/`std`

`Cargo.toml` của `edp-smoltcp-ffi` đặt `default-features = false`, chỉ bật:

```toml
[dependencies.smoltcp]
path = "../smoltcp"
default-features = false
features = ["medium-ethernet", "proto-ipv4", "socket-tcp", "socket-udp"]
```

Không bật `std`/`alloc`/`proto-ipv6`/`socket-raw`/`socket-icmp`/... — giữ đúng tối thiểu cho Ethernet+IPv4+TCP+UDP nhúng, khớp yêu cầu thực tế v1.2.4 (`docs/to-do.md`). Tất cả buffer (`SocketBuffer`, `SocketStorage` cho `SocketSet`, metadata ring cho UDP) đều nhận slice từ mảng `static`/global phía gọi (C hoặc Rust static) — không có `Box`/`Vec` nào được cấp phát runtime, nhất quán với Task table/GDA/PSE của μEDP core đã cố định tại compile-time.

Số lượng socket tối đa (`N` ở §5.4) nên là một macro cấu hình đặt cạnh các macro cấu hình PLTF khác (ví dụ `UEDP_NET_MAX_SOCKETS`, default 4) — quyết định cụ thể (macro ở đâu, sinh từ `lstaxizer.yaml` hay hardcode `app_cfg.h`) nên để ngỏ cho vòng review tích hợp với `pycdscriptor`, không thuộc phạm vi FFI thuần tài liệu này.

## 9. Điểm tích hợp vào vòng đời μEDP

`smoltcp.md` §7 đã nêu 2 phương án (OCE service, hoặc gần `uedp_task_scheduler()`). Sau khi đọc kỹ API, khuyến nghị cụ thể hoá như sau:

- **Không phù hợp làm OCE service** (`uedp_ocesvc.h`): OCE chạy FCFS, mỗi service có `handler`/`context` chạy 1 lần rồi chuyển `OCESVC_STATE_COMPLETED` — không khớp bản chất "poll liên tục, có delay khuyến nghị" của `Interface::poll()`/`poll_delay()`.
- **Phù hợp hơn**: một `task_poll_t` (`uedp_task.h`) riêng, gọi là ví dụ `TASK_NET_POLL`, với action gọi `edp_iface_poll()` mỗi khi được scheduler kích hoạt, và dùng `edp_iface_poll_delay_us()` để tự quyết định khoảng nghỉ trước lần poll kế — logic này khớp tự nhiên với mô hình "tác vụ polling" đã có sẵn của μEDP (`task_poll_t` khác `task_norm_t` chính ở việc không chờ message mà tự polling định kỳ, theo tên gọi).
- Việc bơm dữ liệu ứng dụng (ví dụ app logic muốn gửi/nhận TCP) nên đi qua `uedp_msg_t`/`task_norm_t` bình thường: app task gửi message chứa payload tới `TASK_NET_POLL`, `TASK_NET_POLL` gọi `edp_tcp_send_slice`; chiều ngược lại, sau mỗi `edp_iface_poll()` nếu `edp_tcp_can_recv()` thì `TASK_NET_POLL` đọc rồi `uedp_msg_alloc`+`uedp_task_norm_post_msg` gửi payload cho task đích — giữ nguyên nguyên tắc điều hướng qua message queue hiện có của μEDP, không để logic mạng gọi thẳng logic ứng dụng.
- Câu hỏi để ngỏ (không chốt trong tài liệu này): `TASK_NET_POLL` có nên tự sinh từ codegen `pycdscriptor` (như mọi task khác trong `lstaxizer.yaml`) hay là task viết tay cố định trong PAL/BSP layer? Nghiêng về phương án sau ở giai đoạn đầu (task viết tay, không qua codegen) vì mạng là hạ tầng PAL-level, không phải logic ứng dụng — nhưng cần xác nhận với người review trước khi triển khai.

## 10. Build system — Cargo ↔ CMake

`CMakeLists.txt` hiện tại hoàn toàn không biết về Rust/Cargo. Đề xuất bridge tối thiểu, có điều kiện (không ảnh hưởng build khi không bật):

```cmake
option(UEDP_NET_SMOLTCP "Enable smoltcp network stack via Rust FFI" OFF)
if (UEDP_NET_SMOLTCP)
  find_program(CARGO_EXECUTABLE cargo REQUIRED)
  set(EDP_SMOLTCP_FFI_DIR ${CMAKE_SOURCE_DIR}/edp-smoltcp-ffi)
  set(EDP_SMOLTCP_FFI_LIB ${EDP_SMOLTCP_FFI_DIR}/target/release/libedp_smoltcp_ffi.a)
  add_custom_command(
    OUTPUT ${EDP_SMOLTCP_FFI_LIB}
    COMMAND ${CARGO_EXECUTABLE} build --release --manifest-path ${EDP_SMOLTCP_FFI_DIR}/Cargo.toml
    WORKING_DIRECTORY ${EDP_SMOLTCP_FFI_DIR}
    COMMENT "Building edp-smoltcp-ffi (Rust)"
  )
  add_custom_target(edp_smoltcp_ffi_build DEPENDS ${EDP_SMOLTCP_FFI_LIB})
  add_library(edp_smoltcp_ffi STATIC IMPORTED)
  set_target_properties(edp_smoltcp_ffi PROPERTIES IMPORTED_LOCATION ${EDP_SMOLTCP_FFI_LIB})
  add_dependencies(edp_smoltcp_ffi edp_smoltcp_ffi_build)
  target_include_directories(uedp PRIVATE sources/pal/net)
  target_link_libraries(uedp edp_smoltcp_ffi)
endif()
```

Điểm cần chốt riêng (không giải trong tài liệu này, chỉ nêu để review):

1. **Cross-compile target**: build cho STM32F103/STM32H723/ESP32S3 cần Cargo target tương ứng (`thumbv7m-none-eabi`, `thumbv7em-none-eabihf`, `xtensa-esp32s3-none-elf` hoặc `riscv32imc-...` tuỳ chip) — mỗi target cần cấu hình `.cargo/config.toml` riêng và (với ESP32 Xtensa) toolchain đặc thù ngoài Rust stable chính thức, đúng như cảnh báo đã có ở `docs/to-do.md` v1.2.4.
2. **PLAT=LINUX trước tiên**: nên prototype FFI này trên `PLAT=LINUX` với một `edp_netdev_ops_t` giả lập (ví dụ đọc/ghi qua TAP device hoặc UDP loopback nội bộ) trước khi đụng tới cross-compile — nhất quán với cách TSD/TLC cũng nhắm `PLAT=LINUX` trước.
3. **Panic handler cho `no_std`**: build embedded thật cần `#[panic_handler]` — nên tái sử dụng `pal_sys_fatal()` (đã có, weak, overridable) làm đích gọi từ panic handler Rust, tránh có 2 cơ chế fatal-error riêng biệt trong cùng firmware.

## 11. Câu hỏi mở cần chốt với người review trước khi viết code

1. **Vị trí `TASK_NET_POLL`** (§9): viết tay trong PAL/BSP hay sinh từ `pycdscriptor`? Ảnh hưởng trực tiếp tới việc có cần mở rộng `uels-syntax.md` hay không.
2. **Đơn vị tick thật của từng arch** (§7): cần xác nhận `uedp_timer_get_systick()` có luôn là 1ms trên STM32F103/STM32H723/ESP32S3/LINUX hay khác nhau — quyết định cách viết `edp_net_now_us()`.
3. **`UEDP_NET_MAX_SOCKETS` đặt ở đâu** (§8): macro tay trong `app_cfg.h`, hay field mới trong `lstaxizer.yaml` do `pycdscriptor` sinh ra?
4. **Cross-compile Rust cho ESP32 Xtensa** (§10.1): dùng nhánh `esp-rs` (fork LLVM riêng) hay chờ hỗ trợ chính thức — quyết định này nằm ngoài phạm vi FFI, nhưng chặn được việc build thật trên ESP32-S3 (không chặn thiết kế FFI, không chặn prototype LINUX).
5. **`edp_netdev_ops_t` có đủ cho WiFi không** (§4.1): thiết kế hiện tại giả định thuần Ethernet-media (khung đã có sẵn header L2 đầy đủ do driver cung cấp/tiêu thụ) — WiFi driver thật (ESP-IDF) có thể cần thêm thông tin ngoài khung (RSSI, kênh, trạng thái kết nối AP) không thuộc phạm vi `phy::Device` của smoltcp; các thông tin đó nên quản lý ở một API riêng (BSP WiFi-control) tách khỏi `edp_netdev_ops_t`, không nhét vào đây.

## 12. Kết luận

Ranh giới FFI khả thi và cụ thể: một crate shim Rust (`edp-smoltcp-ffi`, `no_std`, static-only) bọc smoltcp bằng các hàm `extern "C"` thao tác trên slice/con trỏ/số nguyên thuần (§5), với 1 struct `Device` cụ thể cầu nối GAT-token sang 4 hàm callback C mà lldriver tương lai hiện thực (§4.1) — đây chính là ranh giới sạch cho phần Ethernet/WiFi lldriver mà `smoltcp.md` đã lưu ý, không cần giải quyết ở đây. Converter tay cho enum địa chỉ (§6), bridge đơn vị thời gian (§7), static storage cho socket (§8), và một task-poll riêng thay vì OCE service (§9) là các quyết định thiết kế chính. Phần rủi ro/không chắc chắn lớn nhất không nằm ở tầng FFI (đã rõ ràng) mà ở tầng build cross-compile cho Xtensa ESP32 (§10.1) và xác nhận đơn vị tick per-arch (§7/§11.2) — cả hai nên chốt trước khi bắt đầu implement, nhưng không chặn việc viết crate shim + prototype trên `PLAT=LINUX`.
