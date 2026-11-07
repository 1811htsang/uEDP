# Tài liệu trình bày nguyên tắc đánh giá mức độ lỗi

Thông thường trong embedded / automotive middleware, mức độ lỗi được đánh giá theo 3 câu hỏi:

## Các câu hỏi đánh giá mức độ lỗi

### A. Có làm sai dữ liệu hoặc hỏng trạng thái hệ thống không?

Ví dụ: `TIMER linked list corrupted`

Nếu linked list đã corrupt:

- scheduler không còn đáng tin cậy
- timer có thể chạy sai
- không thể tự phục hồi

Có thể kết luận là **FATAL**.

### B. Có thể tiếp tục hoạt động an toàn không?

Ví dụ: `GDP name not found`

Một datapoint không tồn tại:

- hệ thống vẫn chạy
- chỉ mất một chức năng nhỏ

Có thể kết luận là **WARN**.

### C. Có phải lỗi lập trình (programming error) không?

Ví dụ:

```md
`MSG invalid pointer`
`SM null state handler`
`TASK invalid ID`
```

Cần phân biệt:

#### Recoverable

`MSG invalid param` do Caller truyền sai tham số.

Có thể:

```c
return  ERROR;
log();
```

Có thể kết luận là **ERROR**.

#### Non-recoverable

`SM null state handler` do FSM thiếu handler.

Nếu tiếp tục thì `state machine undefined` |

Có thể kết luận là **FATAL**

## Severity Matrix đề xuất

### WARN

Điều kiện:

- Chức năng giảm cấp (degraded)
- Hệ thống vẫn hoạt động
- Không mất dữ liệu
- Có thể retry

Ví dụ:

```md
`UEDP_FCR_GDP_NOT_FOUND`
`UEDP_FCR_OCE_REGISTRY_FULL`
`UEDP_FCR_TASK_PRI_EXHAUSTED`
```

### ERROR

Điều kiện:

- Thao tác hiện tại thất bại
- Nhưng hệ thống vẫn nhất quán
- Có thể tiếp tục chạy

Ví dụ:

```md
`UEDP_FCR_TASK_QUEUE_FULL`
`UEDP_FCR_MSG_INVALID_PARAM`
`UEDP_FCR_TIMER_POOL_EXHAUSTED`
`UEDP_FCR_GDP_DUPLICATE_NAME`
```

### FATAL

Điều kiện:

- Corrupted state
- Internal invariant broken
- Memory corruption
- Framework cannot guarantee correctness

Ví dụ:

```md
`UEDP_FCR_TIMER_CORRUPTED`
`UEDP_FCR_LIB_COLLAPSE`
`UEDP_FCR_MSG_POOL_EXHAUSTED`
`UEDP_FCR_MSG_ISR_FIFO_FULL`
```

## Action Matrix đề xuất

### LOG_ONLY

```md
`WARN`
`ERROR`
``

Ví dụ:

```md
`GDP_NOT_FOUND`
`INVALID_PARAM`
`DUPLICATE_NAME`
```

### RESET_TASK

Khi:`

- lỗi cục bộ
- chỉ 1 task bị hỏng
- reset task khôi phục được

Ví dụ:

```md
`TASK_QUEUE_FULL`
```

Action:

```md
`reset queue`
`restart task`
```

### SYS_PANIC

Khi:

- memory corruption
- scheduler corruption
- framework integrity broken

Ví dụ:

```md
`TIMER_CORRUPTED`
`LIB_COLLAPSE`
`SM_NULL_HANDLER`
```

## Cách đánh giá chuẩn

Nên dùng ma trận 5 cấp:

```md
`LEVEL 0  INFO`
`LEVEL 1  WARN`
`LEVEL 2  ERROR`
`LEVEL 3  CRITICAL`
`LEVEL 4  FATAL`
```

và đánh giá theo 4 tiêu chí:

| Tiêu chí | Câu hỏi |
| --- | --- |
| Recoverability | Có tự phục hồi được không? |
| Scope | Ảnh hưởng 1 API hay toàn hệ thống? |
| Data Integrity | Dữ liệu còn tin cậy không? |
| Safety Impact | Có thể gây hành vi nguy hiểm không? |
