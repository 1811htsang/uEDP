# Test Set Review: PSE, GDP and FCR

- Status: Draft for review
- Date: 2026-10-30
- Input: Proposed test set in `docs/vni/arch-design.md` (13 cases: 7 positive, 6 negative)
- Checked against: `sources/core/src/uedp_msg.c`, `sources/core/inc/uedp_msg.h`, `sources/core/src/uedp_fcr.c`, `sources/core/inc/uedp_fcr.h`, `sources/core/src/uedp_task.c`, `sources/core/inc/uedp_core.h`

## 1. Summary

Only 2 of the 13 proposed cases can be kept as written (`TC_POS_03`, `TC_NEG_04`). The other 11 need an expectation rewrite, are blocked by a code defect, or cover a feature or FCR code that does not exist in `sources/core`. The review also found seven defects in the code that the test set would expose. These should be fixed or tracked before the cases are added to CI.

| Verdict | Count | Cases |
| --- | --- | --- |
| KEEP | 2 | TC_POS_03, TC_NEG_04 |
| REWRITE (expectation is wrong) | 5 | TC_POS_01, TC_POS_05, TC_NEG_01, TC_NEG_02, TC_NEG_03 |
| REWRITE + BLOCKED (code defect) | 2 | TC_POS_04, TC_POS_06 |
| BLOCKED (code defect, expectation OK) | 1 | TC_POS_02 |
| REWRITE (feature missing or FCR code missing) | 1 | TC_NEG_05 |
| DROP (not in `sources/core`) | 2 | TC_POS_07, TC_NEG_06 |

## 2. Code Defects Found During Review

These are defects in the current source. Each one causes a proposed test to fail, or to pass for the wrong reason.

| ID | Defect | Location | Effect |
| --- | --- | --- | --- |
| B1 | `uedp_msg_register_topic()` and `uedp_msg_clear_topic()` loop over task IDs `0xE0..0xEF`, then test `id < 32`. Every valid task ID takes the error branch. | `uedp_msg.c:654-678` | No task can subscribe to a topic. Blocks TC_POS_01, 02, 04. |
| B2 | `uedp_msg_publish()` checks `sizeof(data) > 0`. `sizeof(data)` is the size of a pointer (`void*`), so the check is always true. The `memcpy` copies pointer bytes, not the payload. | `uedp_msg.c:689` onward | Published values are wrong. The "no payload" branch is unreachable. Blocks TC_POS_06. |
| B3 | `uedp_msg_publish()`, `register_topic()` and `clear_topic()` index `g_tpm_matrix[sig]` without checking `sig < UEDP_TBM_MAX_SIZE` (32). | `uedp_msg.c:686, 664, 678` | Out-of-bounds read/write on a bad topic ID. Relates to TC_NEG_01. |
| B4 | `uedp_msg_publish()` has no null check on `data`. Because `sizeof(data)` is always non-zero (B2), it always takes the payload branch and `memcpy`s from `NULL`. | `uedp_msg.c:689-695` | Undefined behaviour on `publish(sig, NULL)`. |
| B5 | `uedp_msg_t::ref_count` is declared `ui8`, but `uedp_msg_ref_inc()` guards overflow against `UINT16_MAX`. The guard never fires; 255 + 1 wraps to 0. | `uedp_msg.h:56`, `uedp_msg.c:191-194` | A message referenced more than 255 times has a wrong count. The next release either frees it while holders still use it, or it is never freed (leak). |
| B6 | `internal_msg_pse_post_data_out_range()` registers the GDP name `"publish"` every time and never unregisters it. A second call returns `UEDP_FCR_GDP_DUPLICATE_NAME`. This path is also only reached after `UEDP_FCR_MSG_POOL_EXHAUSTED` has already halted the system (see Section 3). | `uedp_msg.c:742` | The GDP fallback is effectively unreachable and not reusable. |
| B7 | `uedp_gdp_register()` stores the `name` pointer, not a copy. A name held in a temporary buffer dangles after the caller returns. | `uedp_msg.c:535` | Lookups fail or match wrong names. Document the lifetime rule or copy the name. |

B1 and B2 are the blockers. Without them, the positive publish/subscribe cases cannot pass.

## 3. FCR Behaviour That the Test Set Must Account For

`uedp_fcr.c` defines the action for each code. Several proposed expectations assume non-fatal behaviour that does not match the table.

| Code | Severity | Action | Consequence for tests |
| --- | --- | --- | --- |
| `UEDP_FCR_MSG_POOL_EXHAUSTED` | FATAL | SYS_PANIC | Calls `pal_sys_fatal()`. Does not return. Any test that exhausts a pool must stub `pal_sys_fatal()`. |
| `UEDP_FCR_MSG_ISR_FIFO_FULL` | FATAL | SYS_PANIC | Same as above. |
| `UEDP_FCR_MSG_POOL_MISCONFIG` | FATAL | SYS_PANIC | Same as above. |
| `UEDP_FCR_TASK_QUEUE_FULL` | ERROR | RESET_TASK | Logs only. The core does not reset TSM/FSM state. The caller must handle recovery. |
| `UEDP_FCR_GDP_NOT_FOUND` | WARN | LOG_ONLY | Logs at WARN level. Returns NULL or `STAT_ERROR` to the caller. |
| `UEDP_FCR_GDP_INVALID_PARAM` | WARN | LOG_ONLY | Same as above. |
| `UEDP_FCR_GDP_DUPLICATE_NAME` | ERROR | LOG_ONLY | Same as above. |
| `UEDP_FCR_GDP_TABLE_FULL` | ERROR | LOG_ONLY | Same as above. |
| Unknown code | FATAL | SYS_PANIC | `uedp_fcr_lookup()` falls back to `UEDP_FCR_UNKNOWN`, which panics. |

Test harness requirement: the test build must provide a `pal_sys_fatal()` stub that records the call and returns. Without it, every FATAL case halts the test process.

## 4. Review of the Proposed Cases

### 4.1 Positive Cases

TC_POS_01: Multicast to N subscribers. Verdict: REWRITE, BLOCKED by B1.

- Proposal expects `ref_count` = N on one shared message.
- Code: `uedp_msg_publish()` allocates one message per subscriber, each with `ref_count = 1` (`uedp_msg.c:146`). Pool usage is N slots, not 1, and `ref_count` is never N.
- Revised expectation: each of N tasks receives one message with `ref_count == 1`; the pool returns to its baseline count after all N tasks finish. Blocked until B1 is fixed.

TC_POS_02: Unicast, 1 subscriber. Verdict: BLOCKED by B1.

- Expectation is correct (`ref_count` = 1, released after processing). Cannot run until B1 is fixed.

TC_POS_03: Publish with 0 subscribers. Verdict: KEEP.

- With no subscribers, `g_tpm_matrix[sig]` is 0. The loop body never runs and the function returns `STAT_OK` without allocating. This matches the proposal.
- Note: this test does not depend on B1, because no subscriber is registered.

TC_POS_04: Dynamic unsubscribe. Verdict: REWRITE, BLOCKED by B1.

- `uedp_msg_clear_topic()` has the same defect as `register_topic()` (B1), so unsubscribe cannot be tested in its current form.
- The `ref_count` expectation does not apply, because counts are per message.
- Revised expectation: after `clear_topic()`, a publish on the topic delivers no message to the removed task, and delivers to the remaining tasks. Blocked until B1 is fixed.

TC_POS_05: Reference passing with `ptype: REF` through publish. Verdict: REWRITE.

- `uedp_msg_publish()` has no `ptype` argument and does not send pointers. REF access is provided by `uedp_gdp_get_ref()`, which the consumer calls directly.
- Revised expectation: `uedp_gdp_register()` a variable; `uedp_gdp_get_ref()` returns the same address; a write through the pointer is visible to a later read.

TC_POS_06: Value passing with `ptype: VAL` through publish. Verdict: REWRITE, BLOCKED by B2.

- The copy is wrong (B2): `memcpy` copies `sizeof(void*)` bytes of the pointer value, not the payload. The test would fail today, which is correct, but the expectation must describe the intended behaviour.
- Revised expectation: the subscriber receives a copy of the payload bytes; modifying the copy does not change the source.
- The current API cannot express a fixed-size payload separate from the pointer. The design must decide how the payload size is passed (see Section 6).

TC_POS_07: Zero-copy multicast. Verdict: DROP.

- Not implemented. Publish copies the payload into a new message per subscriber (`memcpy`), so there is no zero-copy path and no single shared header.
- Replace with: N tasks call `uedp_gdp_get_ref()` on the same name and receive the same address.

### 4.2 Negative Cases

TC_NEG_01: Out-of-range topic ID. Verdict: REWRITE, related to B3.

- The proposal names `PSE_INVALID_TOPIC`. No such FCR code exists in `uedp_fcr.h`.
- Current behaviour: `sig` is not bounds-checked, so `g_tpm_matrix[sig]` reads or writes out of bounds. No FCR is raised.
- Revised expectation depends on a design decision (Section 6). Recommended: add an FCR code (for example `UEDP_FCR_MSG_INVALID_TOPIC` under `UEDP_FCR_MOD_MSG`), check `sig < UEDP_TBM_MAX_SIZE` in publish, register and clear, and return `STAT_ERROR`. Until then the test fails.

TC_NEG_02: Pool exhaustion during publish. Verdict: REWRITE.

- The proposal expects "no crash, FCR raised, signal may be kept or dropped".
- Code: `UEDP_FCR_MSG_POOL_EXHAUSTED` is FATAL with SYS_PANIC. `pal_sys_fatal()` is called and does not return.
- Revised expectation: with `pal_sys_fatal()` stubbed, the stub is called with the description "MSG pool exhausted". The test must not expect a return value from `publish()`.

TC_NEG_03: Subscriber queue full. Verdict: REWRITE.

- Correct part: `UEDP_FCR_TASK_QUEUE_FULL` is raised when a task queue is full (`uedp_task.c:251` for normal post, `uedp_task.c:509` for head post).
- Incorrect part: the `ref_count` 3→2 adjustment does not happen. Each subscriber has its own message with `ref_count = 1`. When the post to C fails, `uedp_msg_publish()` calls `uedp_msg_free()` on C's message directly (`uedp_msg.c:697`) and continues the loop.
- Revised expectation: A and B receive their messages; C's message is freed (pool count returns to the value it would have been without C's message); `TASK_QUEUE_FULL` is logged at ERROR level; no panic.

TC_NEG_04: GDP name not found. Verdict: KEEP.

- `GDP_NOT_FOUND` is WARN/LOG_ONLY. `uedp_gdp_get_ref()` returns NULL; `uedp_gdp_get_val()` and `uedp_gdp_set_val()` return `STAT_ERROR`.
- Add: check that the log entry is at WARN level with the tag `ITNLOG_TAG_FCR`.

TC_NEG_05: Payload larger than the pool slot (VAL). Verdict: REWRITE.

- The proposal names `MSG_PAYLOAD_TOO_LARGE`. No such FCR code exists.
- Related real codes: `UEDP_FCR_GDP_INVALID_PARAM` with "out_buf too small" (`uedp_gdp_get_val`) and "size mismatch" (`uedp_gdp_set_val`).
- Because of B2, `publish()` never sees the real payload size, so the "too large" condition cannot be triggered through publish today.
- Revised expectation: `uedp_gdp_get_val()` with a buffer smaller than the registered size returns `STAT_ERROR`, raises `GDP_INVALID_PARAM`, and does not write to the buffer.

TC_NEG_06: Unauthorised publish (SSI). Verdict: DROP.

- No SSI module exists in `sources/core` (no matches for `SSI` or `ssi_`). The FCR code `SSI_UNAUTHORIZED_ACCESS` does not exist.
- Move to a future-scope list. Do not add to CI until the SSI module exists.

## 5. Additional Cases Recommended

These cover behaviour in the source that the proposal does not test.

| ID | Module | Case | Expected result |
| --- | --- | --- | --- |
| ADD-01 | PSE | `register_topic()` with valid task IDs `0xE0..0xEF` | `STAT_OK`. Fails today (B1). |
| ADD-02 | PSE | `publish()` with `sig >= 32` | Rejected with `STAT_ERROR`. Today: out-of-bounds (B3). |
| ADD-03 | PSE | `publish(sig, NULL)` | Rejected with `STAT_ERROR`. Today: undefined behaviour (B4). |
| ADD-04 | MSG | `uedp_msg_free(NULL)` and `uedp_msg_ref_dec(NULL)` | `MSG_INVALID_PTR` logged at ERROR; no crash. |
| ADD-05 | MSG | Double free of the same message | `MSG_INVALID_PTR` ("double free") logged; pool count is not corrupted. |
| ADD-06 | MSG | `ref_inc` on a message with `ref_count == 255` | Overflow prevented. Today: wraps to 0 (B5). |
| ADD-07 | MSG | ISR FIFO full (`internal_uedp_msg_enqueue_isr_sig`) | `MSG_ISR_FIFO_FULL` raised; `pal_sys_fatal()` stub called. |
| ADD-08 | GDP | Register 17 variables with `UEDP_GDP_QUEUE_SIZE = 16` | 17th returns `STAT_ERROR`; `GDP_TABLE_FULL` logged at ERROR. |
| ADD-09 | GDP | Register the same name twice | Second returns `STAT_ERROR`; `GDP_DUPLICATE_NAME` logged. |
| ADD-10 | GDP | Unregister, then `get_ref` on the same name | `get_ref` returns NULL; `GDP_NOT_FOUND` logged. The data memory is not freed. |
| ADD-11 | GDP | Register with a name in a temporary buffer, then look it up after the buffer is gone | Documents the lifetime rule (B7). Expected behaviour depends on the decision in Section 6. |
| ADD-12 | FCR | `uedp_fcr_lookup()` with an unknown code | Returns the `UEDP_FCR_UNKNOWN` entry (FATAL, SYS_PANIC). |
| ADD-13 | FCR | Each severity maps to the correct ITNLOG level | WARN to `ITNLOG_LEVEL_WARN`; ERROR to `ITNLOG_LEVEL_ERROR`; FATAL to `ITNLOG_LEVEL_FATAL`. |
| ADD-14 | FCR | `TASK_QUEUE_FULL` with RESET_TASK action | Logs only; TSM/FSM state of the task is unchanged. Documents current behaviour. |

## 6. Open Decisions Needed Before Writing Tests

1. Topic ID validation (B3, TC_NEG_01): Should an invalid topic return an error, raise a new FCR code, or panic? The current table has no code for it.
2. Payload size (B2, TC_POS_06): How does `publish()` learn the size of the payload? The current signature `publish(ui16 sig, void* data)` has no size argument.
3. Task ID mapping (B1): Topic bits are indexed by `task_id` in `0..31` (`uedp_msg.c:687`), but task IDs are in `0xE0..0xEF`. Which mapping is intended? The answer determines the fix for B1.
4. Publish failure policy (TC_NEG_02, TC_NEG_03): When one subscriber's queue is full, should delivery to the remaining subscribers continue? Today the payload branch frees the failed message and continues. The no-payload branch returns on the first failure. Because B2 makes the no-payload branch unreachable, only the continue behaviour is observable today.
5. GDP name ownership (B7): Should GDP copy the name, or document that the caller must keep it alive?
6. Pool exhaustion (TC_NEG_02): Is a FATAL panic the intended response to pool exhaustion during publish, or should publish return an error? The FCR table says panic.

## 7. Test Harness Requirements

- Stub `pal_sys_fatal()` to record the call and return, so FATAL cases can be checked.
- Capture ITNLOG output (tag `ITNLOG_TAG_FCR`) so that log level and description can be asserted.
- Provide a `pal_enter_critical()` / `pal_exit_critical()` stub that tracks nesting, to catch unbalanced sections.
- Reset `g_tpm_matrix`, `g_gdp_table` and the message pools before each test. These are file-static or global and carry state between cases.
- Each test should assert pool count before and after the test, to catch leaks.

## 8. Next Steps

1. Decide the open items in Section 6.
2. Fix B1 and B2 in the source, or mark TC_POS_01, 02, 04 and 06 as expected-fail until they are fixed.
3. Correct the expectations of the five REWRITE cases as described in Section 4.
4. Add the cases from Section 5 to the test set.
5. Only then add the cases to CI. Remove TC_POS_07 and TC_NEG_06 from the CI set.
