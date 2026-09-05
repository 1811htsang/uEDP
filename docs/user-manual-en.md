# μEDP User Manual

Author: Shang Huang - Huỳnh Thanh Sang

## I. Overview

μEDP - micro-EDP is a core module designed to support the event-driven programming model on embedded platforms. The goal of μEDP is to provide a flexible, easy-to-use, and extensible solution for developing embedded applications without depending on specific hardware.

## II. Directory Structure

<!-- CRITICAL
Review commit c3f4c9866ec1ea548f2e108f059328fe0dd68183 to revert the changes related to the project's directory structure.

This note applies to both the VN and EN versions.
-->

```text
μEDP/
├── core/                        # Definition and implementation of μEDP's core logic
│   ├── inc/                     # uedp_msg.h, uedp_task.h, uedp_timer.h, uedp_fsm.h, uedp_tsm.h
│   │   └── uedp_core.h          # Definitions of μEDP's core signals, constants, and data structures
│   └── src/                     # Implementation of the scheduler, timer engine, and message manager
├── pal/                         # BACKEND (Abstraction layer)
│   ├── pal_core.h               # Common declarations shared by the whole PAL and its services
│   ├── services/                # Hardware Services (Hardware mapping)
│   │   ├── logdp/               # pal_logdp.h holds the log API declarations for dispatching logs to multiple backends
│   │   ├── memrp/               # pal_memrp.h holds the memory profiling API declarations to implement per platform
│   │   └── rprintf/             # pal_rprintf.h holds the rprintf API declarations to implement per platform
│   └── arch/                    # Implementation (Chip-specific source code)
│       └── .../                 # Each platform has its own dedicated implementation folder
├── app/                         # Definition of application logic, including user-defined tasks and FSMs
│   ├── config/                  # Application, Core, and PAL configuration
│   ├── declaration/             # Design declarations for business logic
│   ├── interface/                # Implementations for forwarding external signals into the Core
│   ├── kconfig/                 # Application configuration via Kconfig
│   └── app.c                    # Main implementation of the user application's behavior
├── common/                      # Common utilities and data structures shared across the whole project
│   ├── container/               # Data structures such as FIFO, Ring Buffer, Linked List, implemented in plain C
│   ├── kconfiglib/              # Configuration for running the Kconfig terminal
│   ├── kconfigspec/              # Python configuration used to generate code from the Kconfig terminal
│   └── xprintf/                 # xprintf library used to format log strings and output them to various backends
└── test/                        # Sample test cases used to verify μEDP's features
    ├── test01/                  # Basic test with ISR tasks and TSM
    ├── test02/                  # Test with features such as message pooling and memrp
    ├── test03/                  # Test with features such as message pooling and memrp
    └── test04/                  # Test with the itnlog feature
```

## III. Usage Guide

### Kconfig

Kconfig is a configuration tool used to manage the options and settings of a μEDP project. It lets the user easily enable or disable features, adjust parameters, and produce different configurations for the application.

Command to run Kconfig:

```bash
python uedp.py menuconfig
```

#### Prerequisites for using Kconfig

- A Linux environment (Docker can be used to map the project directory into a Linux environment and install the required tools).
- Python 3.x installed on the system.

#### Configurations supported by μEDP

- Number of norm tasks used in the application.
- Number of poll tasks used in the application.
- Number of signals defined in the application.
- Size of the BLANK, ALLOC, EXTAL, and ISR pools.
- Message queue size for each norm task.
- Maximum number of timers used in the application.
- Maximum number of log entries stored in itnlog's internal buffer.
- Warning threshold for when the log buffer is close to full.
- Number of backends registered with logdp.
- Configuration of the norm task name, message queue name, and handler name for each norm task.
- Configuration of the poll task name and handler name for each poll task.
- Configuration of signal names.
- Configuration of the managing object, state descriptor table, and transition descriptor table for each TSM. Starting from 1.1.6, each norm task is asked **individually** whether it uses TSM and, if so, how many states it needs, instead of a single choice shared by every task.
- Configuration of the managing object and state names for each FSM. Similarly to TSM, starting from 1.1.6 each norm task is also asked individually whether it uses FSM and how many states it needs.

#### Configurations supporting automatic code generation from Kconfig

- Signal value for each poll task. (Starting from `0xE6u`)
- Priority level for each norm task. (Starting from level `0`)
- Signal value for each signal. (Starting from `0x01u`)
- Handler name for each TSM and FSM. (Based on the state name and the managing object's name)

#### Notes on using Kconfig

Kconfig only supports generating code for defined values, handler names, and state names. The processing logic inside handlers, and the transition logic of TSM and FSM, must still be implemented by the user in the application's implementation under `app.c`.

Starting from version 1.1.6, when running `menuconfig`, for each declared norm task (`Number of norm tasks used in the application`), the tool asks in turn: "Does task #i use TSM?" — and if so, how many states; then "Does task #i use FSM?" — and if so, how many states accordingly. A task may use both TSM and FSM at the same time, only one of the two, or neither, and the number of states per task is not required to be the same across tasks. This is a difference from versions prior to 1.1.6, when the tool only asked once for the whole application and applied the same number of states to every norm task that used TSM/FSM.

### Message Pool

The Message Pool is the component that manages memory for messages used in μEDP. It provides an efficient allocation and reclamation mechanism for messages, helping to optimize memory usage and ensure that messages are processed correctly by the system.

#### Message pool categories

- `UEDP_MSG_TYPE_BLANK`: a message with no payload.
- `UEDP_MSG_TYPE_ALLOC`: a message with a small or medium payload, backed by its own data area.
- `UEDP_MSG_TYPE_EXTAL`: a message coming from an interface outside the Core.
- `UEDP_MSG_TYPE_ISR`: a signal originating from interrupt context.

When initialized via `uedp_msg_pool_init()`, the Core builds static pools for `BLANK`, `ALLOC`, `EXTAL`, and a dedicated FIFO for `ISR`. For `ALLOC` and `EXTAL`, the data area is laid out as a 2D array `[queue_size][data_size]` so that each message has its own data cell.

#### How to use

1. Call `uedp_msg_pool_init()` after `uedp_core_init()`.
2. Call `uedp_msg_alloc(des_task_id, sig, size)` to obtain a message from the appropriate pool.
3. Use `uedp_msg_set_data_val()` if you want to copy a value into the payload.
4. Use `uedp_msg_set_data_ref()` if you want to pass a reference to data whose lifetime is long enough.

#### Points to note

- `uedp_msg_alloc()` automatically selects the pool based on the payload size, so the `size` value must accurately reflect the actual data need.
- If passing by reference, the referenced buffer must remain alive past the point where the message is processed.
- `uedp_msg_drain_isr_pool()` is the dedicated path for ISR signals; do not push ISR data directly into a regular task queue.
- For `ALLOC` and `EXTAL`, `data` is a pointer to a dedicated memory area for each message, not an inline payload sitting directly in the header.
- There is no need to call `uedp_msg_free()` for a message coming from the `ISR` pool, since the Core automatically frees it after the handler finishes running in each scheduling round.

#### Source/destination task identification for messages

Starting from version 1.1.5, `uedp_msg_t` supports independently assigning the ID of the source task (the sender) and the destination task (the receiver) for each message, instead of only having the `des_task_id` parameter passed in at `uedp_msg_alloc()` time:

- `uedp_msg_set_src_task_id(msg, src_task_id)`: assigns the ID of the source task sending the message.
- `uedp_msg_set_des_task_id(msg, des_task_id)`: assigns/changes the ID of the destination task receiving the message.

Example: task A wants to send a message to task B, but task B still needs to know this message came from task A (for example, so it can reply back to the correct sender):

```c
uedp_msg_t* msg = uedp_msg_alloc(UEDP_TASK_NORM_B_ID, SIG_REQUEST, 0);
uedp_msg_set_src_task_id(msg, UEDP_TASK_NORM_A_ID);
uedp_task_norm_send_msg(msg);
```

In task B's handler, it can read back `msg->src_task_id` to know who sent the message, and use `uedp_msg_set_des_task_id()` on a new reply message to send it back to task A. This is a simple mechanism where the user chooses an appropriate ID themselves - the Core does not enforce or validate this source/destination pairing on its own.

> **Note**: starting from 1.1.5, the `internal_uedp_msg_pool_panic` API has been removed from the Core. Fatal errors related to the message pool (pool exhaustion, invalid pointer, ISR FIFO full, ...) are now reported through the FCR mechanism (see the "FCR (Fatal Code Return)" section below) instead of calling panic directly as before.

### FCR (Fatal Code Return) - Handling fatal errors

Starting from version 1.1.5, μEDP adds FCR to identify and handle fatal errors inside the Core (pool exhaustion, invalid pointer, wrong task ID, non-existent transition, ...) in a consistent way, instead of each module silently handling them on its own as before.

Each fatal error is assigned a fixed code (`uedp_fcr_code_t`), which is looked up to determine its severity (`severity`: `WARN`/`ERROR`/`FATAL`) and a corresponding handling action, and is always logged through `itnlog` (tag `ITNLOG_TAG_FCR`) before that action is executed:

- `UEDP_FCR_ACT_LOG_ONLY`: only logs the event, without interfering with program flow.
- `UEDP_FCR_ACT_RESET_TASK`: marks the related task so that a higher layer (a supervisor task or OCE) can recover it.
- `UEDP_FCR_ACT_SYS_RESET`: calls `pal_sys_reset()` to restart the whole system.
- `UEDP_FCR_ACT_SYS_PANIC`: calls `pal_sys_fatal()` to halt the system immediately.

#### Declaring application-level error codes

An application can declare its own error codes using the `UEDP_FCR_MOD_APP` module and raise them itself when it detects an abnormal condition in its own logic:

```c
#define APP_FCR_SENSOR_TIMEOUT UEDP_FCR_CODE(UEDP_FCR_MOD_APP, 0x01)

// When a sensor fails to respond within the allowed time:
UEDP_FCR_RAISE_MSG(APP_FCR_SENSOR_TIMEOUT, "sensor #2 timeout after 500ms");
```

`UEDP_FCR_RAISE(code)` and `UEDP_FCR_RAISE_MSG(code, extra)` automatically fill in `__FILE__`/`__LINE__` into the log entry; `RAISE_MSG` allows passing an additional, more specific context description (e.g. a function name or an invalid parameter value) instead of the default description in the error code table, which makes debugging easier.

#### Points to note about FCR

- Since a lookup miss in the error code table defaults to `UEDP_FCR_UNKNOWN` (`SEV_FATAL` + `ACT_SYS_PANIC`), users should always register the exact error code they intend to use rather than passing an arbitrary code.
- `UEDP_FCR_ACT_SYS_RESET`/`UEDP_FCR_ACT_SYS_PANIC` will halt or restart the whole system as soon as they are raised - carefully consider before raising error codes in these groups from application logic.
- Currently `g_fcr_table[]` is a `static const` table; the application layer cannot yet register new entries at runtime - adding a new error code and its handling action requires declaring it directly in the Core's source code.
- `UEDP_FCR_ACT_RESET_TASK` does not yet automatically recover the related task - it currently only logs at the `ERROR` level; recovery still needs to be handled by a higher layer.

### Dpool GDA (Global Data Pool) - Managing global data

Starting from version 1.1.6, μEDP adds GDP (Global Data Pool) to explicitly manage global variables shared across multiple tasks, serving the `glbda:` block of the PLD/μE-LS design (see `docs/uels-syntax.md` and `docs/review/dmp-gda.md`). Unlike the `Message Pool`, GDP does not allocate memory and has no concept of "freeing" a registered slot - it only registers a name that points to a `static`/`global` memory area that already exists, declared by the user, since a global variable is considered to live for the entire lifetime of the program.

#### How to use Dpool GDA

1. Call `uedp_gdp_init()` once after `uedp_core_init()`, before using any other GDP API.
2. Declare a global (`static`/`global`) variable in the application, then register it with GDP using `uedp_gdp_register(name, data_ptr, size)`.
3. Use `uedp_gdp_get_ref(name)` when you need a direct reference pointer to the data (corresponding to `ptype: REF` in μE-LS).
4. Use `uedp_gdp_get_val(name, out_buf, buf_size)` / `uedp_gdp_set_val(name, in_buf, buf_size)` when you need to read/write a value by copy instead of holding a direct reference (corresponding to `ptype: VAL` in μE-LS).
5. Call `uedp_gdp_unregister(name)` if that variable no longer needs to be looked up through GDP (the real memory area is still not freed).

Example declaring and using a global status variable shared between 2 tasks:

```c
static ui32 g_system_status = 0;

void app_init(void) {
  uedp_gdp_init();
  uedp_gdp_register("GDA_SYSTEM_STATUS", &g_system_status, sizeof(g_system_status));
}

// Task A updates the status by writing a new value (ptype: VAL)
void task_a_update_status(ui32 new_status) {
  uedp_gdp_set_val("GDA_SYSTEM_STATUS", &new_status, sizeof(new_status));
}

// Task B reads directly through a reference pointer (ptype: REF)
void task_b_check_status(void) {
  ui32* status_ref = (ui32*)uedp_gdp_get_ref("GDA_SYSTEM_STATUS");
  if (status_ref != NULL && *status_ref != 0) {
    // handle the case where the system status is non-zero
  }
}
```

#### Points to note about Dpool GDA

- GDP does not own the `data` memory - registering a local variable instead of a `static`/`global` one will cause the pointer to reference memory that is no longer valid after the declaring function returns.
- The maximum number of slots GDP can manage at once defaults to `UEDP_GDP_MAX_SLOTS` (16); this can be overridden by defining the macro before including the header if more slots are needed.
- `uedp_gdp_get_val()`/`uedp_gdp_set_val()` will raise FCR if `buf_size` does not match/is insufficient compared to the registered size, instead of silently reading/writing to the wrong memory area.
- `uedp_gdp_get_ref()` returns a pointer directly into the real memory area, with no critical section wrapper - this fits μEDP's current single-core, non-preemptive scheduler; it should be reconsidered if μEDP is ported to a multi-core environment.
- The registered `name` in GDP should match the `name` declared in the `glbda:` block of μE-LS to keep the PLD/μE-LS design documentation consistent with the actual implementation.

### Task

Tasks in μEDP come in two kinds: message-driven and poll-driven.

#### Message-driven task

A message-driven task is declared using `task_norm_t`, with 5 members:

- `id`: task ID.
- `base_pri`: base priority level.
- `cur_pri`: current priority level (can change at runtime).
- `urgent_pending`: a flag indicating the task has a pending urgent signal.
- `task_norm`: the main handler.
- `msg_queue`: internal FIFO.
- `msg_queue_buffer`: pointer buffer for the FIFO.

When `uedp_task_norm_create()` runs, the Core automatically initializes the FIFO for each task up to the `UEDP_TASK_NORM_EOT_ID` element. The default queue size is taken from `UEDP_TASK_MSG_QUEUE_SIZE`.

`uedp_task_scheduler()` currently selects the highest-priority ready task, pops exactly one message from that task's queue, dispatches the handler, then frees the message once the handler has finished running.

#### Poll-driven task

A poll-driven task is declared using `task_poll_t`, with `id`, `ability`, and `task_poll`.

- `uedp_task_poll_create()` only counts the list up to `UEDP_TASK_POLL_EOT_ID`.
- `uedp_task_poll_set_ability()` enables/disables a poll task by ID.
- When no message-driven task is ready, the scheduler runs the enabled poll tasks.

#### Task context API

While a task is running, its current context can be retrieved using:

- `uedp_task_norm_get_current_id()`
- `uedp_task_norm_get_current_msg()`

These APIs are especially useful for itnlog, since the logger obtains `task_id` and `msg_sig` from the current context.

#### Temporarily raising priority

After finishing a message, or when called by an ISR to raise its priority, a task can temporarily raise its priority using `uedp_task_norm_set_urgent(task_id_t tid)` and `uedp_task_norm_post_urgent(task_id_t tid, uedp_msg_t* msg)`. The scheduler uses this priority level for the next scheduling round. Once the task has no more messages left in its queue, the priority automatically resets back to `base_pri`.

### ISR

An ISR in μEDP should not directly handle complex logic. The standard path is:

1. The ISR calls the Core's signal-registration API.
2. The Core places the task ID + signal pair into an internal ISR FIFO.
3. At the start of the scheduler loop, the Core drains this FIFO and turns it into normal processing flow.

This path is shared between signals coming from the timer tick and other interrupts.

In the current code, `uedp_task_norm_post_isr()` is the API dedicated to ISR use, and `uedp_timer_tick()` also uses this API when a timer expires to deliver the signal to the destination task.

### Timer

μEDP's Timer Service uses a fixed pool of `UEDP_TIMER_MAX_NODES` nodes, with no heap allocation.

#### How to declare and use

1. Call `uedp_timer_init()` after initializing the Core.
2. Call `uedp_timer_set(task_id, sig, ms, type)` to create a new timer or update an existing one.
3. Call `uedp_timer_remove(task_id, sig)` to remove a timer.
4. Call `uedp_timer_tick()` from the platform's periodic tick context.

#### Actual behavior

- `type` currently supports `UEDP_TIMER_ONE_SHOT` and `UEDP_TIMER_PERIODIC`.
- `ms` is converted to a number of ticks using `UEDP_TIMER_TICK`.
- When a timer expires, the Core raises a signal to the destination task through the ISR-safe path.
- For periodic timers, the counter is reloaded after each expiration; for one-shot timers, the node is returned to the free list.

#### Resources

- The maximum number of timers active at the same time is `UEDP_TIMER_MAX_NODES`.
- `uedp_timer_get_stats()` allows checking the number of active timers and the maximum capacity.

#### Intended use

A timer can be used as a tool to create a delay or a timeout inside tasks. For example, in a task that needs to use a blocking API such as UART, I2C, or other communication protocols, a timer can be used to create a timeout for those APIs, avoiding the task hanging indefinitely if something goes wrong. Timers can also be used to create periodic events, for example reading a sensor at fixed intervals or sending a heartbeat to indicate that the system is still running.

### Itnlog

Itnlog is μEDP's internal logging mechanism, used to replace scattered `printf`-style debugging throughout the processing flow. This keeps the Core independent of stdio directly, while allowing the log output destination to be changed per platform without modifying the processing logic. When the same entry needs to be sent to multiple backends, `logdp` and `rprintf` should be combined instead of using a single string callback alone.

#### Basic usage

1. Call `uedp_itnlog_init()` after initializing the Core and before starting the scheduler.
2. Register a log output wrapper using `uedp_itnlog_set_output()`.
3. Call `uedp_itnlog_log()` wherever an event needs to be recorded.
4. Call `uedp_itnlog_dump()` when you want to print all the log currently held in the buffer.

Example configuration on Linux:

```c
static void itnlog_stdout_output(const char* text) {
  printf("%s", text);
  fflush(stdout);
}

int main(void) {
  uedp_core_init();
  uedp_msg_pool_init();
  uedp_timer_init();

  uedp_itnlog_init();
  uedp_itnlog_set_output(itnlog_stdout_output);
  uedp_itnlog_set_filter(ITNLOG_LEVEL_DEBUG, ITNLOG_TAG_TSK);

  while (1) {
    uedp_task_scheduler();
    usleep(100);
    uedp_itnlog_dump();
  }
}
```

When writing application code, instead of inserting `printf` directly in a handler, call `uedp_itnlog_log()` with an appropriate tag such as `TSK`, `MSG`, `FSM`, `TSM`, or `TIM`. Then call `uedp_itnlog_dump()` whenever you want to flush the entire log buffer to the configured output.

Important note: `uedp_itnlog_set_output()` accepts a function with the signature `void (*)(const char*)`. Therefore, do not pass `printf` directly into this API; instead, wrap `printf` or `fputs` in a wrapper as shown in the example above.

#### Log line format

When dumping, each entry is assembled into a line following a template, or according to the output callback's own design, but by default the format is:

```text
[ITNLOG] tmstmp task_id msg_id msg
```

Here, `task_id` and `msg_id` are printed in hex form to make it easy to map to the Core's internal IDs. `msg_id` is the `msg_sig` of the current message at the time the log was written. Example:

```text
[ITNLOG] 0 0xE4 0x01 System is alive and running...
```

#### Filtering by tag

To filter logs by module, use `uedp_itnlog_set_tag("TSK")`, `uedp_itnlog_set_tag("MSG")`, `uedp_itnlog_set_tag("FSM")`, `uedp_itnlog_set_tag("TSM")`, or `uedp_itnlog_set_tag("TIM")`.

Notes:

- `NULL` means no filtering by tag.
- When outputting to a terminal, use a dedicated wrapper instead of passing `printf` directly as the callback, to avoid mismatched function signatures and to actively flush `stdout`.
- `uedp_itnlog_log()` reads `task_id` from the current task and `msg_sig` from the current message, so it should only be called while inside the scheduler context processing a valid message.

#### Debugging notes

- If logs are not appearing immediately on the terminal, check whether the output callback flushes `stdout`.
- If you want to print logs at a specific point in time, you can call `uedp_itnlog_dump()` inside a polling task or right before a test case ends.
- `uedp_itnlog_log()` only writes into the internal buffer; actual display depends on the output callback and when the dump happens.
- If the log buffer is full, `uedp_itnlog_log()` will make the logger auto-dump before writing the next entry, per the current design in the source.

### Logdp, Rprintf, and Xprintf

`xprintf` is the character-level formatter layer. It provides APIs such as `xprintf()`, `xfprintf()`, and `xsprintf()` to format strings following the same set of rules across multiple platforms. In the current rprintf flow, `pal_rprintf_flush_entry()` uses `xfprintf()` to build the log string into an intermediate buffer before pushing it to the backend.

`logdp` is the PAL's dispatch layer. It holds a table of callbacks of type `void (*)(uedp_itnlog_entry_t*)` and allows the same log entry to be sent to multiple destinations. This is the right layer when the user wants an entry to go out over UART, console, log file, and trace buffer all at once.

`rprintf` is the redirect-print layer. It receives a `pal_rprintf_service_t` containing the entry to output and the backend callbacks such as `init`, `putc`, `write`, `is_ready`. When `pal_rprintf_flush_entry()` is called, it checks the backend, formats the entry using `xfprintf()`, then outputs the resulting string through `write` if available, or scatters it character by character through `putc` if `write` is not available.

#### How to wire the layers together

1. Initialize the specific backend, such as UART, console, or file.
2. Create a `pal_rprintf_service_t` for that backend.
3. Write an adapter callback with the signature `void (*)(uedp_itnlog_entry_t*)`.
4. Inside the adapter, copy `*entry` into `service.entry` then call `pal_rprintf_flush_entry(&service)`.
5. Register the adapter using `pal_logdp_register()`.
6. When a log entry needs to be emitted, call `pal_logdp_dispatch(&entry)`.

Minimal example on Linux:

```c
static void linux_putc(unsigned char c) {
  putchar(c);
}

static void linux_write(const uint8_t* data, uint16_t len) {
  fwrite(data, 1, len, stdout);
  fflush(stdout);
}

static bool linux_is_ready(void) {
  return true;
}

static pal_rprintf_service_t linux_rprintf = {
  .entry = {0},
  .init = NULL,
  .putc = linux_putc,
  .write = linux_write,
  .is_ready = linux_is_ready,
};

static void linux_log_output(uedp_itnlog_entry_t* entry) {
  linux_rprintf.entry = *entry;
  pal_rprintf_flush_entry(&linux_rprintf);
}

int main(void) {
  uedp_core_init();
  uedp_msg_pool_init();
  uedp_timer_init();
  uedp_itnlog_init();
  uedp_itnlog_set_output(pal_logdp_dispatch);
  pal_logdp_register(linux_log_output);

  while (1) {
    uedp_task_scheduler();
    uedp_itnlog_dump();
  }
}
```

Points to remember:

- If only a single log destination is needed, `pal_rprintf_flush_entry()` can be called directly without going through `logdp`.
- If multiple log destinations are needed, register several different callbacks with `logdp`; each callback should own its own `pal_rprintf_service_t`.
- `xprintf` usually does not need to be called directly in the application once `rprintf` is in use, since `rprintf` already uses `xfprintf()` to format the output string.
- `pal_rprintf_service_t` allows `init = NULL` if the backend has already been initialized by the BSP or the application.

#### Naming an rprintf backend

Starting from version 1.1.5, `pal_rprintf_service_t` has an additional `name` field (a string, e.g. `"UART"`, `"FILE"`, `"CONSOLE"`) to give the backend a logical label. This field does not affect the Core's dispatch logic - it only serves debug-trace purposes and maps 1-to-1 to the `contract` field in the `pplp.rprintf[]` block of the μE-LS syntax, keeping the PLD/μE-LS design documentation and the actual implementation code named consistently with each other:

```c
static pal_rprintf_service_t linux_rprintf = {
  .name = "CONSOLE",
  .entry = {0},
  .init = NULL,
  .putc = linux_putc,
  .write = linux_write,
  .is_ready = linux_is_ready,
};
```

If the application has multiple rprintf backends, give each a different `name` matching the corresponding `contract` declared in μE-LS, to make it easier to look things up while debugging.

### Declaring TASK_NORM, TASK_POLL, SIG, and STATE values

Based on the signal ranges, here is the reference convention used in the test cases:

- TASK_NORM is declared from `0xE6` to `0xEE` (avoid using `0xEF`, which is already defined as EOT).
- TASK_POLL is declared from `0xD4` to `0xDE` (avoid using `0xDF`, which is already defined as EOT).
- SIG is declared from `0x01` to `0xFF` (avoid values already defined in special signal ranges such as FSM_SIG, TSM_SIG, TSM_STATE).

### Declaring message queues, global buffers, FSM, and TSM

The user should declare the message queues and global buffers for each task within the implementation of each test case, to keep things independent and easy to manage.

Example:

```c
static uedp_msg_t* usr_q_mem[8];
static uedp_msg_t* a_q_mem[8];
static uedp_msg_t* b_q_mem[8];

static const char* data_a_to_b = "Hello from Task A!";
static const char* data_b_to_a = "Hello from Task B!";

static uedp_tsm_t blinker_tsm;

static uedp_fsm_t fsm_usr;
static uedp_fsm_t fsm_a;
static uedp_fsm_t fsm_b;
```

Note that these global buffers are used to hold messages whose size is too large compared to the declared pool size; in that case, the user relies on the pass-by-reference mechanism to carry the data's address in the message payload, so these buffers must have global scope to avoid memory-access errors when the message is processed after a local variable has gone out of scope.

Also, follow the declaration order of message queue, global buffer, TSM, and FSM to keep things consistent and easy to manage during application development.

### Declaring handlers for Task, TSM, and FSM

Handlers for Task, TSM, and FSM should be declared within the implementation of each test case, to keep things independent and easy to manage.

Example:

```c
static void fn_on_active_exit(uedp_msg_t* msg);
static void fn_on_active_entry(uedp_msg_t* msg);

static void fn_on_idle_entry(uedp_msg_t* msg);

static void fn_active_logic(uedp_msg_t* msg);

static void usr_state_idle(uedp_msg_t* msg);
static void usr_state_active(uedp_msg_t* msg);

static void task_a_state_idle(uedp_msg_t* msg);
static void task_a_state_active(uedp_msg_t* msg);

static void task_b_state_idle(uedp_msg_t* msg);
static void task_b_state_active(uedp_msg_t* msg);

static void task_usr_handler(uedp_msg_t* msg);
static void task_a_handler(uedp_msg_t* msg);
static void task_b_handler(uedp_msg_t* msg);
```

Note: follow the declaration order of TSM handlers, FSM handlers, and finally Task handlers, to keep things consistent and easy to manage during application development.

### Initializing TSM

#### Initializing the TSM table

In a TSM, each state has a transition-descriptor table `tsm_trans_t` that defines:

- The transition signal.
- The next state to transition to for that signal.
- The logic function to execute during the transition.

Example:

```c
const tsm_trans_t blink_idle_trans[] = {
  { SIG_USR_START, STATE_BLINK_ACTIVE, NULL },
  { SIG_USR_STOP,  UEDP_TSM_STATE_STAY, NULL } 
};

const tsm_trans_t blink_active_trans[] = {
  { SIG_INTERNAL_TICK, UEDP_TSM_STATE_STAY, fn_active_logic },
  { SIG_USR_STOP,      STATE_BLINK_IDLE,      NULL },
  { SIG_USR_START,     UEDP_TSM_STATE_STAY, NULL }
};
```

After all transition tables have been fully declared, the next step is to declare the state-descriptor table `tsm_state_desc_t` to define the states the TSM can have, linking each state to its `on_entry` function, `on_exit` function, and its corresponding transition table.

Example:

```c
const tsm_state_desc_t blinker_tsm_table[] = {
  { STATE_BLINK_IDLE,   fn_on_idle_entry,   NULL,              blink_idle_trans,   1 },
  { STATE_BLINK_ACTIVE, fn_on_active_entry, fn_on_active_exit, blink_active_trans, 2 }
};
```

Note that a state does not necessarily need an `on_entry` and `on_exit` function; if not needed, they can be left as `NULL`. However, the transition table and the number of transitions are required to define the TSM's transition logic.

#### Initializing the Task table

Each task is defined in the task table `task_norm_t` with the following information:

- Task ID.
- Task priority level.
- Task handler.
- Memory used for the task's message queue.

Example:

```c
task_norm_t app_task_table[] = {
  { UEDP_TASK_NORM_USR_ID,  UEDP_TASK_PRI_LEVEL_8, task_norm_usr_handler, {0}, usr_q_mem  },
  { TASK_NORM_A_ID,           UEDP_TASK_PRI_LEVEL_7, task_norm_a_handler,   {0}, a_q_mem    },
  { TASK_NORM_B_ID,           UEDP_TASK_PRI_LEVEL_6, task_norm_b_handler,   {0}, b_q_mem    },
  { UEDP_TASK_NORM_EOT_ID,  UEDP_TASK_PRI_LEVEL_0, NULL,                  {0}, NULL       }
};
```

Here, the 4th parameter is the task's internal FIFO, which the Core automatically initializes based on the 5th parameter. So the 4th parameter is left as `{0}` so the Core can automatically initialize the FIFO based on the memory declared in the 5th parameter.

Note that each task should have a different priority level to ensure the Core can process signals correctly; if all tasks have the same priority level, the Core will run into signal-processing errors, so care must be taken when assigning priority levels to tasks in the system.

Also, `UEDP_TASK_NORM_USR_ID` is the default task the user uses to deliver the starting signal to the Core. So if the user wants to use a different task to deliver the starting signal to the Core, that task's ID must be changed to `UEDP_TASK_NORM_USR_ID` to ensure the Core can receive the starting signal and that it has the highest priority so it is processed before other tasks in the system.

### Initializing FSM

FSM is initialized similarly to TSM, where each state is a single handler function that processes the logic for that state. When the FSM receives a signal and the task handler dispatches it to the FSM, the Core calls the handler function corresponding to the FSM's current state to process the logic and decide the next state based on the signal received.

### Initializing the Tick handler

This initialization depends on the platform and the implementation approach.

Example:

- On Linux, a dedicated thread performs ticking at a fixed delay, where this thread calls the Core's `uedp_timer_tick()` API to update the time and process software timers.
- On STM32, `SysTick_Handler()` is called directly to perform ticking, where this function calls the Core's `uedp_timer_tick()` API to update the time and process software timers.
- On other platforms, a hardware timer can be used to generate a periodic interrupt, where the interrupt handler calls the Core's `uedp_timer_tick()` API to update the time and process software timers.

### Initializing the application

After completing the handler declarations and initializing the TSM table and Task table, the application is initialized following this sequence:

- Initialize the environment with `uedp_core_init()`, which performs environment configuration depending on the platform.
- Initialize the message pool with `uedp_msg_pool_init()`, which initializes the static memory pools based on the configuration declared in the PAL.
- Initialize the timer with `uedp_timer_init()`, which initializes the software timers and sets up the tick handler depending on the platform.
- Initialize the task table with `uedp_task_norm_create()`, which initializes the tasks based on the declared task table, and also sets up the internal FIFO for each task based on the declared memory.
- Initialize TSM and FSM with `uedp_tsm_init()` and `uedp_fsm_init()`, which initialize the TSMs and FSMs based on the declared state-descriptor tables, and also set the initial state for each TSM and FSM.
- Deliver the starting signal to `UEDP_TASK_NORM_USR_ID` with `uedp_post_msg()`, which delivers the starting signal to the user's default task to activate the system and begin processing subsequent signals.
- The main loop runs `uedp_task_scheduler()` to start the system's signal-processing loop, in which the Core continuously checks and processes signals from tasks based on the configured priority levels, while also managing software timers and executing TSM/FSM logic when a corresponding signal arrives.
- After the main loop, `ocesvc` performs resource cleanup and release, calling the corresponding APIs to free memory, destroy tasks and TSM/FSM, and ensure all signals have been processed before the program ends. This design is handled in later versions of the Core, which will provide APIs to safely and efficiently clean up and release resources.

### Recommended initialization sequence

To match the current source, the initialization order should be:

1. `uedp_core_init()`
2. `uedp_msg_pool_init()`
3. `uedp_gdp_init()` if using global variables through GDP (Dpool GDA)
4. `uedp_timer_init()`
5. `uedp_tsm_init()` and `uedp_fsm_init()` if TSM and FSM are used
6. `uedp_itnlog_init()` if using the logger.
7. Initialize the log output backend if using `rprintf`.
8. Register the callback with `pal_logdp_register()` if fanning out logs to multiple destinations.
9. `uedp_itnlog_set_output()` and other log configuration APIs if using a string-based log output path.
10. `uedp_task_norm_create()`
11. `uedp_task_poll_create()` if there are poll tasks
12. Send the starting message to `UEDP_TASK_NORM_USR_ID`
13. The `uedp_task_scheduler()` loop

## IV. Important notes

- Assigning priority levels to tasks is very important to ensure the Core can process signals correctly. If all tasks have the same priority level, the Core will run into signal-processing errors, so care must be taken when assigning priority levels to tasks in the system.
- When using the pass-by-reference mechanism to carry a data address in a message's payload, make sure the buffers holding this data have global scope, to avoid memory-access errors when the message is processed after a local variable has gone out of scope.
- In the TSM design, using the "Stay" and "Back" mechanisms helps optimize performance and avoid unnecessarily repeating the `on_entry` and `on_exit` functions; however, this mechanism must be used carefully to ensure the transition logic is still maintained correctly and does not introduce logic errors into the system.
- When designing an FSM, using the Pointer-Swapping model achieves maximum flexibility; however, note that changing the processing logic instantly with a single pointer assignment can lead to bugs if not managed carefully, so states and processing logic must be designed clearly and understandably to avoid confusion and logic errors in the system.
- Starting from 1.1.5, fatal errors in the Core are reported through FCR; note that some error codes have `UEDP_FCR_ACT_SYS_RESET`/`UEDP_FCR_ACT_SYS_PANIC` as their handling action, which will restart or halt the system as soon as they are raised, even when raised from an application-declared error code.
- Starting from 1.1.6, when using Dpool GDA, only register variables with `static`/`global` scope into GDP; registering a local variable will cause the pointer obtained via `uedp_gdp_get_ref()` to reference memory that is no longer valid after the declaring function returns.
- During application development, follow the guidelines and structure laid out in this document to keep the system consistent and easy to manage, and regularly test and debug to ensure the system operates stably and effectively.
