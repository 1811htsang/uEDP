# μEDP Core Architecture Design Analysis

## Original Design Architecture from AKEDP

The original AKEDP version is the predecessor of μEDP; this original design contributed many core ideas (static message pool management, bitmask-based scheduler, lightweight TSM/FSM, and an ISR handling mechanism) but also had a number of practical limitations when applied to platforms and stricter real-time requirements.

Key characteristics of the original AKEDP design:

- Use of multiple static message pools: "pure" messages (header-only), "common" messages (header + fixed data), and "dynamic" messages (header + heap-allocated payload).
- Reference (ref-count) management to allow messages to be shared between multiple consumers.
- A bitmask-based ready-mask scheduler (O(1)) to find the highest-priority task.
- TSM (table-driven) for clearly structured transitions; FSM (function-pointer) for lightweight state, swapping function pointers to change behavior.
- ISR mechanism: it is recommended that an ISR only create/fill a message interface and then forward/post it to a task; a critical-section wrapper is provided for safe operations inside interrupts.
- A supporting container ecosystem (FIFO, ring buffer, log queue) to manage queues and data in interrupt contexts.

Limitations of the original AKEDP design (why improvement was needed in μEDP):

- Heap allocation for "dynamic" messages causes fragmentation and is non-deterministic in timing — a problem for real-time systems.
- Ref-counting makes code and debugging more complex; it can easily lead to leaks or double-frees if mismanaged along ISR/forward paths.
- Pool sizes and configuration are mostly fixed at compile time, lacking the flexibility to adapt to application needs.
- The critical section can be coarse-grained, increasing interrupt latency and reducing responsiveness in systems with many ISRs.
- Some ports assume certain architectural properties (e.g., alignment/sizeof(void*) and ordering), leading to portability issues when moving between 32-bit/64-bit or between MCU and Linux.
- Lacking a sufficiently robust ISR-to-core FIFO mechanism to fully isolate the core from ISRs under all loads — there is a risk of overflow if an ISR generates too many signals in a short time without a way for the core to serialize and process them safely.
- Lacking centralized diagnostic and error-reporting tools (e.g., pool exhaustion statistics, ref-count mismatch), making operation and debugging difficult on real hardware.

The lessons learned from these limitations directly led to design decisions in μEDP (e.g., a clearer separation between PAL/Core, an ISR-safe FIFO inside the core, words-aligned pool sizing, and reduced dependency on the heap wherever possible).

## μEDP Design Architecture

μEDP is divided into 3 clearly defined functional layers to achieve the goal of "Zero-Touch Porting":

### Application Layer

Contains user-defined business tasks and the application's FSM. This layer interacts with the Core only through the standard API set such as `uedp_post_msg()` or `uedp_timer_set()`. This layer contains no code related to hardware or microcontroller registers, ensuring independence and ease of migration across different platforms.

Note: In the test design under the `test/` directory, declarations for task handlers, FSM handlers, task tables, etc. are all kept self-contained within each test case's implementation to keep testing independent and easy to manage. However, in actual use in a user application, these declarations should be placed in the `app/` directory to keep them clearly separated.

### μEDP Core (Core Layer — Immutable)

Contains the pure logic of the event-driven programming model, including:

- Scheduler: An O(1) bitmask-based priority multitasking scheduler.
- Message Manager: Manages the static, fragmentation-resistant memory pool.
- Timer Service: Manages the linked list of software timers.
- FSM/TSM Engines: The state machine execution engines.
- Itnlog: A logging system that captures all data at the point of the call, which can then be pushed to logdp and rprintf to be output to multiple destinations.

In this layer, the core is designed to be completely independent of hardware to ensure portability and easy integration into any embedded platform. The core only interacts with hardware through the abstract functions provided by the PAL layer.

#### Message Manager

The μEDP Message Manager currently uses a static pool model combined with a free list to avoid fragmentation and avoid depending on the heap in the main processing path. The source currently has 3 main pools:

- `BLANK`: messages with no payload, used for simple signals.
- `ALLOC`: messages with a small or medium payload, with the data buffer kept separate from the header.
- `EXTAL`: messages coming from an external interface to the core, also using a separate data area.

`UEDP_MSG_TYPE_NORM` is still defined in the enum as a reserve for future pool variants, but in the current implementation it has not yet been built into a separate pool in `uedp_msg_pool_init()`.

There is also a separate queue for ISR signals. This queue does not go through the normal message pool but is initialized with its own FIFO to reduce risk when capturing signals from an interrupt context.

A message in the core carries the following main fields: `src_task_id`, `des_task_id`, `sig`, `type`, `ref_count`, `data`, `interface` metadata, and a `timestamp` when the debug flag is enabled. On allocation, `uedp_msg_alloc()` selects the appropriate pool based on payload size; on release, `uedp_msg_free()` returns the message to its corresponding pool.

#### Task Manager

The Task Manager is the layer that coordinates message-driven and poll-driven tasks.

- A message-driven task is declared with `task_norm_t`, consisting of `id`, `pri`, a handler, an internal FIFO, and a FIFO buffer.
- A poll-driven task is declared with `task_poll_t`, consisting of `id`, `ability`, and a handler.
- When `uedp_task_norm_create()` runs, the core counts the task list up to the `UEDP_TASK_NORM_EOT_ID` element and then automatically initializes an internal FIFO for each task with `UEDP_TASK_MSG_QUEUE_SIZE` pointer slots.
- The current scheduler is bitmask-priority based: it takes the highest-priority task that is ready, processes exactly one message, then loops back on the next pass.

The current task's processing path is stored temporarily in internal variables to serve the API for retrieving the current context and other modules such as itnlog or the timer.

#### Timer Service

The Timer Service uses a fixed pool of `UEDP_TIMER_MAX_NODES` nodes and a linked list for the free-list/active-list. Each node stores `des_task_id`, `sig`, `type`, `period`, `counter`, `is_active`.

- `uedp_timer_init()` only needs to rebuild the free list.
- `uedp_timer_set()` creates or updates a timer for a given task ID + signal pair.
- `uedp_timer_remove()` removes a timer from the active list and returns the node to the free list.
- `uedp_timer_tick()` is called in the tick interrupt context to decrement the counter; when it expires, it generates an ISR message to the destination task.

The timer currently supports 2 types: one-shot and periodic. The `period` value is converted from milliseconds to ticks according to `UEDP_TIMER_TICK`.

#### ISR Bridge

μEDP separates the path from ISR into the core using an internal FIFO for ISR signals. Instead of creating a message directly inside the interrupt, the ISR only registers a task ID and signal pair into the FIFO, and `uedp_task_scheduler()` calls `uedp_msg_drain_isr_pool()` at the start of each cycle to pull these signals into the normal processing flow.

This approach keeps the path inside the ISR shorter, reduces the risk of contention, and helps the core remain independent of specific hardware.

### PAL - Platform Abstraction Layer

This layer acts as the bridge between the Core and specific hardware. The PAL provides system services such as interrupt management, bit operations, and other utility functions that the Core requires to operate. Each platform has its own PAL implementation, but all of them conform to the same common interface to ensure consistency.

In this layer, `pal_core.h` contains the common declarations for the entire PAL, including system service functions such as `pal_enter_critical()`, `pal_exit_critical()`, and `pal_get_highest_priority()`. These services are implemented differently depending on the platform (for example, on STM32 interrupts are used to manage the critical section, while on Linux a mutex is used). This allows the Core to be completely unconcerned with hardware details, thereby achieving the "Zero-Touch Porting" goal.

#### Log output chain: xprintf, rprintf, and logdp

μEDP splits log output into three layers to keep the Core from directly depending on `printf` while also allowing a single log entry to be distributed to multiple backends:

- `xprintf`: a character-level formatter layer, providing functions such as `xprintf()`, `xfprintf()`, and `xsprintf()` to build strings with a stable format across platforms.
- `rprintf`: a print-redirect layer at the PAL level, which takes a `uedp_itnlog_entry_t`, formats the entry into a display string, and pushes that string to a backend via `write` or `putc`.
- `logdp`: the PAL's dispatch layer, which holds a callback table of type `void (*)(uedp_itnlog_entry_t*)` to fan out the same log entry to multiple destinations such as UART, console, file, or trace buffer.

The standard data flow in the current architecture is:

1. The Core or the application creates a `uedp_itnlog_entry_t`.
2. `pal_logdp_dispatch()` broadcasts that entry to all callbacks registered via `pal_logdp_register()`.
3. Each callback typically holds its own `pal_rprintf_service_t`, copies the entry into `service->entry`, then calls `pal_rprintf_flush_entry()`.
4. `pal_rprintf_flush_entry()` calls `xfprintf()` to format the log string and output it to the previously initialized backend.

This design has two main benefits:

- Separation of responsibilities: `xprintf` handles formatting, `rprintf` handles converting an entry into an output string, and `logdp` handles distribution.
- Backend extensibility: the same log entry can be output simultaneously to UART, a debug screen, and a log file without modifying the Core's logic.

Integration notes:

- `pal_logdp_dispatch()` only passes the entry pointer, so a callback should not hold on to that pointer if the entry has a short lifetime; it is best to copy the content into an internal service before flushing.
- `pal_rprintf_service_t` allows `init` to be `NULL` if the backend has already been initialized by the BSP or the application.
- `pal_rprintf_flush_entry()` only actually outputs data when `is_ready()` returns `true`.
- The default format of `rprintf` is a single line containing a timestamp, task ID, signal ID, and message, built using `xfprintf()` rather than manual string concatenation.

## Detailed Design Logic

### [DMP] Deterministic Memory Pooling — Message memory management with architecture-independent static allocation

The memory management system uses static memory to prevent fragmentation. The Core automatically coordinates memory allocation based on the architecture, as reflected by the `sizeof(void*)` of each message.

For example:

- On a 64-bit Linux architecture, `sizeof(void*)` returns 8.
- On a 32-bit STM32 architecture, `sizeof(void*)` returns 4.

Based on this architecture, when a user wants to declare a custom message pool size, the size must follow the rule `sizeof(void*) * 2^n` to ensure efficient memory management and avoid wasting storage space. The Core automatically coordinates this through the PAL configuration, helping to optimize performance and use memory efficiently.

The Core is designed with the following 4 pool types:

- `BLANK`: 8 units by default, each unit sized according to `sizeof(uedp_msg_t)`, used to allocate messages with no payload, suitable for simple signals.
- `ALLOC`: 16 units by default, each unit sized according to `sizeof(void*) * 2u`, used to allocate messages with a payload, allowing flexible pass-by-value or pass-by-reference.
- `EXTAL`: 16 units by default, each unit sized according to `sizeof(void*) * 4u`, used to allocate messages coming from outside the core, allowing resources to be isolated so the Core can process them before passing them into the system and to the tasks registered to receive these messages.
- `ISR`: 16 units by default, each unit sized according to `sizeof(uedp_msg_isr_t)`, used for an ISR to pass signals into the system via a FIFO, helping to isolate signals coming from an ISR and ensure safety when passing them into the system.

Beyond these 4 pool types (each allocating/freeing memory tied to a message's lifecycle), the Core also provides **[GDP] Global Data Pool** (internal codename `GAXES`) - a mechanism of a fundamentally different nature: it does not allocate memory at all, only registers a name-to-pointer lookup for global variables (static/global storage duration) that already exist. GDP exists to serve the `glbda:` block of PLD/μE-LS when passing `ptype: REF`/`ptype: VAL`, and is described in detail in [D2MP] below.

### [SII] Safe ISR Injection — A safe mechanism for passing signals from an ISR into the system

To ensure safety when passing signals from an ISR into the system, μEDP adds an internal FIFO inside the Core to store signals coming from ISRs. When an interrupt occurs (e.g., UART, Timer), the PAL pushes the signal into this FIFO. The Core "drains" this FIFO into the Task Queues at the start of each Scheduler cycle. This mechanism completely removes the need for the Core to know anything about ISRs, while ensuring safety and efficiency when passing signals from an ISR into the system.

To ensure safety and avoid resource contention, draining this FIFO is performed inside a critical section, ensuring that this process is not interrupted by other tasks or other ISRs. This helps maintain data consistency and ensures that signals from an ISR are processed safely and efficiently within the μEDP system.

When a timer expires, `uedp_timer_tick()` also goes through this same mechanism by calling `uedp_task_norm_post_isr()` to deliver the timer's signal to the destination task through the same processing flow.

### [D2MP] Data-to-Message Passing — A mechanism for safely and efficiently passing data through a message

Building on the message memory management design, and to ensure that the full content of one message can be safely and efficiently passed into the payload of another message, μEDP uses a Data-to-Message passing mechanism. This mechanism allows the user to pass data directly into a message's payload without needing to worry about memory management or fragmentation.

Specifically, if the size of the data is smaller than the pool's declared size, the Core provides the `uedp_msg_set_data_val` API to pass data directly into the message's payload. If the size of the data is larger than the pool's declared size, the user can use the `uedp_msg_set_data_ref` API to pass the data's address into the message's payload.

It should therefore be noted that when passing a reference to a **local variable** (limited lifetime within a single function call), the user must ensure that memory remains valid at the time the message is processed (typically by declaring it `static`), to avoid a memory access error when the message is processed after the local variable has gone out of scope (dangling pointer).

For the specific case of passing a reference to a **genuinely global variable** (static/global storage duration, serving the `glbda:` block of PLD/μE-LS), the Core additionally provides the **[GDP] Global Data Pool** mechanism to manage this explicitly, instead of leaving the user to manage a raw pointer as above. GDP is a static registration table (`uedp_gdp_slot_t`, `UEDP_GDP_MAX_SLOTS = 16` slots by default) mapping name to pointer, with 5 APIs: `uedp_gdp_init()` (initialize the table), `uedp_gdp_register()`/`uedp_gdp_unregister()` (register/unregister a global variable by name), `uedp_gdp_get_ref()` (get a direct reference pointer, used for `ptype: REF`), and `uedp_gdp_get_val()`/`uedp_gdp_set_val()` (copy a value out of/into a buffer, used for `ptype: VAL`).

The core difference from the `BLANK`/`ALLOC`/`EXTAL`/`ISR` pools: GDP does **not allocate** the `data` memory (it only stores a pointer to memory that already exists, declared by the user or by PLTF) and has **no concept of freeing/lifecycle** - a global variable lives for the entire program lifetime, so there is no "free" operation for a registered slot. This differs from `ALLOC`, which is tightly bound to the `uedp_msg_alloc()`/`uedp_msg_free()` lifecycle and is not suitable for reuse to store global variables (doing so would require bolting on an ad-hoc "never-free" mechanism).

`uedp_gdp_get_ref()` currently does **not** wrap `pal_enter_critical()`/`pal_exit_critical()`: since the current scheduler is single-core, non-preemptive (each scheduling cycle dispatches exactly one task) and an ISR is not allowed to call `actv`/`act`, there is no real contention path in the current version. Wrapping a critical section should be reconsidered if μEDP later grows to a multi-core environment (AMP/SMP/HELF). The full design discussion, including rejected alternatives (e.g. reusing `ALLOC`, or adding a dedicated global reference FIFO), is recorded in detail in `docs/review/dmp-gda.md`.

Generating the actual static memory for a `glbda:` block (calling `uedp_gdp_register()`) is the responsibility of a separate PLTF generator (planned as `gda_tsgen.py`, within the scope of PLD/μE-LS) - the core only provides the management API, it does not generate variable-declaration code itself.

When retrieving data passed by reference, the user can refer to the declaration style used in `test02`, as follows:

```c
static const char* data_a_to_b = "Hello from Task A!";
uintptr_t received_addr = (uintptr_t)(*(char**)(msg->data));
char* final_str = *(char**)received_addr;
```

Here, `uintptr_t` allows retrieving the address without regard to data type, helping to ensure flexibility and safety when passing references inside a message, independent of architecture or specific data type.

Here, the document uses an example of passing a reference to a string of characters from Task A to Task B through a message. Because `data_a_to_b` itself is a second-level pointer, simply using `char* received_str = *(char**)(msg->data)` would only retrieve the address information of the `data_a_to_b` pointer, not the actual content of the string.

Therefore, an additional step is needed to obtain the actual content of the string through a double dereference, as shown in the example above. In practical use, users will apply the appropriate dereferencing approach depending on the specific data type in order to retrieve the actual content from a message's payload when using this pass-by-reference mechanism.

### [HSMC] Hybrid State Machine Control — A combined state-machine management mechanism between TSM and FSM

In μEDP, each Task (Active Object) is not just a handler function but an entity with "memory". To manage this memory, the system provides two levels of state machine:

- TSM (Macro-level): manages large-scale Operational Modes.
- FSM (Micro-level): manages detailed Functional Logic.

#### TSM - Task State Machine

The TSM is designed using a Table-Driven model to ensure transparency and determinism.

##### Design logic

The TSM completely separates Configuration Data (residing in Flash) from Runtime Data (residing in RAM):

- `tsm_state_desc_t` (State descriptor): Contains an ID, an on_entry function, an on_exit function, and an array of transitions.
- `tsm_trans_t` (State transition table): Defines: "If currently in state X, receiving signal Y -> execute function Z -> jump to state K".
- `uedp_tsm_t` (Management object): Stores the current state (cur_state) and the previous state (prev_state).

##### Operating mechanism

- Automated Entry/Exit: When `tsm_trans` is executed, the Core automatically calls the exit function of the old state and the entry function of the new state. This ensures resources (such as a Timer) are always cleaned up properly.
- "Stay" & "Back" mechanisms:
  - STAY: Execute logic without changing state (avoiding unnecessary repeated Entry/Exit).
  - BACK: Automatically returns to the previous state using the prev_state variable, solving the "State Explosion" problem.
- O(1) lookup: Using a 16-bit ID helps state-transition speed reach the maximum the hardware can offer.

You can refer to the sample program design in `test01` to clearly see how the TSM is used in μEDP, where the TSM is used to manage a Task's operational modes efficiently and flexibly.

#### FSM - Finite State Machine

The FSM is designed using a Pointer-Swapping model to achieve maximum flexibility.

##### Data structure

- state_handler: A function pointer taking a `uedp_msg_t*` parameter.
- uedp_fsm_t: Contains only a single variable — a pointer to the current state function.

##### Operating characteristics

- Agility: Allows the processing logic to be changed immediately with just a single pointer assignment.
- Direct dispatch: The Scheduler calls fsm_dispatch, and the Core immediately executes the function the pointer currently points to.
- Suited to Transient Logic: Used for short-lived action sequences such as protocol decoding (UART parsing) or interface menus.

You can refer to the sample program design in `test03` to clearly see how the FSM is used in μEDP, where the FSM is used to manage UART protocol-decoding logic in a flexible and efficient way.

In `test03`, the FSM is designed so that each state_handler function represents one state. Every state has 3 signals — `UEDP_FSM_SIG_INIT`, `UEDP_FSM_SIG_ENTRY`, `UEDP_FSM_SIG_EXIT` — to manage the state's lifecycle, before any other business-logic signals. When a state-transition event occurs, it proceeds in the order `EXIT` -> `ENTRY` to ensure resources are cleaned up before entering the new state.

Note that in the `test03` design, the FSM of the tasks is always initialized into `state_idle`; only `state_idle` contains the `UEDP_FSM_SIG_INIT` signal to perform FSM initialization operations, after which — depending on a start signal from the user — it transitions to `state_active` to perform the main functions of the test case. This ensures that the FSM is always initialized correctly and can operate efficiently as soon as it receives a start signal from the user.

In addition, for the case of a state looping on itself, this can be handled through call isolation — simply ignoring a state that is not being called. For example, in `test03`:

```c
void usr_state_active(uedp_msg_t* msg) {
  switch (msg->sig) {
    case UEDP_FSM_SIG_EXIT:
      printf("[USR] Exiting ACTIVE state...\n");
      break;
    case UEDP_FSM_SIG_ENTRY:
      printf("[USR] Entering ACTIVE state. System is now active.\n");
      // Send SIG_USR_START to Task A to trigger the action sequence
      uedp_msg_t* msg_to_a = uedp_msg_alloc(TASK_NORM_A_ID, SIG_USR_START, 0);
      uedp_task_norm_post_msg(TASK_NORM_A_ID, msg_to_a);
      printf("[USR] Sent START signal to Task A. Waiting for further signals...\n");
      break;
    case SIG_USR_STOP:
      printf("[USR] Received STOP signal. Transitioning to IDLE state...\n");
      uedp_fsm_go_next(&fsm_usr, usr_state_idle); 
      /**
       * @brief uedp_fsm_go_back(&fsm_usr) could be used to return to the
       *        previous state, but in this context go_next is more
       *        intuitive for clearly showing the transition from
       *        ACTIVE to IDLE when the STOP signal is received.
       */
      break;
    default:
      printf("[USR] Encountered unexpected signal in ACTIVE state: %x\n", msg->sig);
      break;
  }
}
```

While in `state_active`, when a signal is passed through Task A, the FSM of TASK_USR effectively becomes a self-loop because it is not called further. This allows the FSM of TASK_USR to keep the `state_active` state and continue receiving and processing other signals without being interrupted by a state transition, while also ensuring resources are managed efficiently throughout the state's operation.

Another point worth noting in `test03` is that when Task A receives `SIG_TSK_B_TO_A`, it calls `uedp_fsm_go_next(&fsm_a, task_a_state_idle)` to transition Task A's state back to `state_idle`. Here the user could equally well use `uedp_fsm_go_back(&fsm_a)` to return to the previous state. However, in this context `go_next` is more intuitive for clearly showing the transition from `state_active` to `state_idle` upon receiving the `SIG_TSK_B_TO_A` signal, which makes the code easier to read and understand, while still ensuring Task A's FSM is managed effectively and can operate flexibly throughout signal processing.

#### Coordination between TSM and FSM

μEDP encourages users to use a nested model to optimize code, in which:

- The TSM protective layer (Shell):
  - Determines which mode a Task is currently in (e.g., MODE_NORMAL, MODE_CONFIG, MODE_ALARM).
  - If an incoming message causes a mode change, the TSM performs the transition and manages system services (such as enabling/disabling related Polling Tasks).
- The FSM execution layer (Core):
  - Sits inside the TSM's handler functions.
  - Performs computations, processes data from messages, and interacts with hardware.

Through rounds of discussion and design/debugging on Linux, the coordination design between TSM and FSM has also added the following safety measures:

- Lock Splitting: Inside the `tsm_trans` function, interrupt-locking instructions (`pal_enter_critical`) only wrap the pointer change itself. Logic functions (`on_entry/exit`) are called outside the critical region to avoid deadlocks when the user subsequently calls other APIs (such as `timer_set`).
- Atomic Pointer Swap: The state change is guaranteed to be atomic and cannot be interrupted by other flows.
- RTC (Run-to-Completion): Ensures that one state event is fully processed before a Task receives the next event, eliminating race conditions at the logic level.

### [HES] Heximal Encoding Signals — Hexadecimal signal encoding

μEDP uses a Signal Management system to ensure that signals are handled safely and efficiently within the system. This system includes:

- `TASK_NORM` occupies the `0xEx` range with 16 units, in which 6 internal TASK_NORMs are predefined — `TIM`, `IF`, `SYS`, `DBG`, `USR`, `IDLE` — to serve system and user functions. An additional `EOT` (End Of Table) task is added with ID `0xEF` to mark the end of the TASK_NORM signal range, helping the Core easily determine the scope of these signals.
- `TASK_POLL` occupies the `0xDx` range with 8 units, in which 4 internal TASK_POLLs are predefined — `WDG`, `SYSLF`, `MEMRP`, `IDLE` — to serve system and user functions. An additional `EOT` (End Of Poll) task is added with ID `0xDF` to mark the end of the TASK_POLL signal range, helping the Core easily determine the scope of these signals.
- `TASK_PRI` occupies the `0xCx` range with 16 units to define task priority levels, with 16 predefined priority levels ranging from `UEDP_TASK_PRI_LEVEL_0` (lowest) to `UEDP_TASK_PRI_LEVEL_15` (highest) to support system scheduling. Note that within each TASK_NORM, the Core recommends using distinct priority levels; if all TASK_NORMs use the same priority level, the Core will run into signal-processing errors — a check for this error condition may be considered for addition in the future to ensure system stability.
- `FSM_SIG` occupies the `0xBx` range with 16 units to define signals dedicated to state management within an FSM, with 4 signals defined — `ENTRY`, `EXIT`, `INIT`, `LOOP` — to support the lifecycle management of states within an FSM.
- `TSM_SIG` occupies the `0xAx` range with 16 units to define signals dedicated to state management within a TSM, with 4 signals defined — `ENTRY`, `EXIT`, `INIT` — to support the lifecycle management of states within a TSM.
- `TSM_STATE` occupies the `0xAFx` range with 16 units to define states dedicated to state management within a TSM, with 4 states defined — `BACK`, `STAY`, `RESET` — to support the lifecycle management of states within a TSM.

Note that each signal range is guaranteed to have an offset declaration so that, when retrieving a signal to process by pool index or handling any issue related to signal management, the Core can easily determine the signal type and process it correctly.

When declaring additional new signals, users do not need to reconfigure the offset themselves, since this offset only affects signal management internal to the Core; for the user, it is enough to follow the already-defined signal range so that signals continue to be managed accurately and efficiently within the system.

### [PPLP] Plug-N-Play Logging Pipeline — A flexible and extensible logging mechanism

Itnlog is μEDP's internal logging layer, designed to replace scattered `printf`-based debugging throughout the processing flow. The goal of itnlog is to provide a consistent logging path that can be filtered by level and tag, while making it easy to change the log output destination per platform without making the Core directly dependent on stdio.

In terms of design, itnlog acts as a lightweight event-recording layer that is separate from the main processing flow:

- The Core only writes logs into an internal buffer; it does not call `printf` directly.
- The log output destination is exposed externally through the `uedp_itnlog_set_output()` callback, so the same Core can output logs to a console, UART, file, or another debug backend.
- For local debugging on Linux, the callback can wrap `printf` or `fputs` followed by `fflush(stdout)` to ensure the log is output immediately.
- On an embedded platform, the callback can switch to UART, semihosting, a log file, or a dedicated trace mechanism without needing to modify the Core.

This design helps avoid scattering `printf` calls throughout business logic, reduces the Core's dependence on the standard library, and keeps the real-time path more stable when logging is needed at high frequency.

By design, itnlog does not support flushing when `uedp_itnlog_clear()` is called, because the purpose of this function is to clear logs from the internal buffer and reset statistics state, not to output logs externally. Flushing logs should be done in `uedp_itnlog_dump()` when logs are pulled out and sent to the callback, ensuring that only logs that have already been processed and filtered are output externally, which helps optimize performance and avoid unnecessary log output.

Therefore, `uedp_itnlog_dump()` should ideally be placed outside the scheduler or inside a separate polling task, to ensure that log output does not affect the real-time path of the main tasks, while still ensuring that logs are output efficiently and can be controlled through the filters that have been set up.

This design is called Out-Context Execution (OCE), which helps guarantee the following 3 principles:

1. Protecting the RTC (Run-to-Completion) principle: The main tasks are not interrupted by log output, avoiding increased processing latency.
2. Utilizing CPU idle time: Logs are output during periods when the CPU is not busy, helping to optimize performance.
3. Interrupt-nesting safety (ISR Safety): Log output does not occur within an ISR context, avoiding the risk of resource contention or deadlock.

#### Operating model

Itnlog operates through the following sequence:

1. `uedp_itnlog_init()` initializes the internal ring buffer used to store log entries.
2. `uedp_itnlog_log()` writes each entry into the internal buffer based on the currently running context.
3. `uedp_itnlog_dump()` pulls out all entries, applies the level/tag filters, then outputs them to the callback provided by the user.

In terms of storage, an entry is currently a small structure containing `level`, `tag`, `task_id`, `msg_sig`, `msg`, `tmstmp`, and `hash`. The buffer uses a fixed-size ring buffer of `UEDP_ITNLOG_MAX_LOG_ENTRIES`, so when the number of written entries exceeds the threshold, the logger automatically dumps before continuing to write.

When dumping, the log line is assembled using the template:

`[ITNLOG] tmstmp task_id msg_id msg`

Here, `task_id` and `msg_id` are output in hex format, while `tmstmp` is a plain integer value with no unit suffix. `msg_id` is the `msg_sig` of the current message at the moment the log was written.

#### Filters and tag semantics

Itnlog supports filtering by log level and by tag:

- `itnlog_filter_level` determines which entries are eligible for output.
- `itnlog_filter_tag` allows filtering by module, e.g., `TSK`, `MSG`, `FSM`, `TSM`, `TIM`.
- If the tag filter is `NULL`, itnlog treats this as no tag filtering and accepts every valid entry.

This design makes it clearer to enable or disable logging by functional group during debugging, especially in integration tests such as test 04. Because the output callback only ever receives a single pre-formatted string, the log-formatting logic is fixed inside `uedp_itnlog_dump()` rather than left for the user to assemble the string themselves inside the callback.

#### Log output destination

Itnlog does not bind the log output destination to a single fixed backend. The Core only calls the `uedp_itnlog_set_output()` callback to pass a pre-formatted log line up to the application layer or the PAL layer. This approach allows the same logging logic to output to a console, UART, file, or a platform-specific debug interface, instead of depending on `printf` calls scattered throughout each handler.

When used on Linux, the callback should be a wrapper that takes a `const char*`, prints that string to the terminal, and flushes immediately after output to avoid the log being held in `stdout`'s buffer until the program ends.

#### Internal behavior worth noting

- `uedp_itnlog_log()` takes the `task_id` from the currently running task and the `msg_sig` from the current message, so it is only safe to call while the scheduler is processing a valid message.
- `uedp_itnlog_dump()` does not just print the log — it also clears the log from the ring buffer and resets internal statistics state.
- `uedp_itnlog_set_tag(NULL)` is equivalent to turning off tag filtering.

#### Logdp - Log Dispatcher

`logdp` is the PAL's dispatch layer. It holds a callback table of type `void (*)(uedp_itnlog_entry_t*)` and allows the same log entry to be broadcast to multiple destinations. This is the layer to use when a user wants a single entry to go out to UART, console, and a log file or trace buffer all at once.

`logdp` implements this mechanism through registering callbacks via `pal_logdp_register()`; afterward, when the Core calls `pal_logdp_dispatch()` with an entry, `logdp` broadcasts that entry to all registered callbacks. Each callback typically holds its own `pal_rprintf_service_t`, copies the entry into `service->entry`, then calls `pal_rprintf_flush_entry()` to perform the actual log output.

`pal_logdp_dispatch()` is recommended as itnlog's output function when dumping logs, helping to ensure the same entry can be output to multiple backends simultaneously without modifying the Core's logic. For example, on Linux, the callback can wrap `printf` or `fputs` following a defined contract, so `logdp` can select the appropriate output destination based on the entry's tag.

#### Rprintf - Redirect Printf

`rprintf` is the print-redirect layer. It takes a `pal_rprintf_service_t` containing the entry to output and backend callbacks such as `init`, `putc`, `write`, `is_ready`. When `pal_rprintf_flush_entry()` is called, it checks the backend, formats the entry using `xfprintf()`, then outputs the resulting string via `write` if available, or character-by-character via `putc` if `write` is not available.

`rprintf` is where the contract lives for assigning concrete log-output functions to each backend. For example, on Linux, functions such as `printf` or `fputs` followed by `fflush(stdout)` are used, while on an embedded platform a UART wrapper or semihosting might be used instead. This design completely separates formatting the log (handled by `xfprintf`) from where the log is output (handled by `rprintf`), while allowing the same entry to be output to multiple different backends through `logdp`.

#### Xprintf - Extended Printf

`xprintf` is the character-level formatter layer. It provides APIs such as `xprintf()`, `xfprintf()`, and `xsprintf()` to format strings according to the same set of rules across multiple platforms.

`xprintf` is a third-party library integrated into μEDP to ensure consistent and extensible log formatting. This design avoids having to rewrite log-formatting logic for each platform, while ensuring logs are formatted accurately and efficiently before being output through `rprintf` and `logdp`.

### [APE] Atomic Priority Escalation — Temporary atomic priority increase

In the original design, μEDP had a total of only 16 priority levels, divided evenly among all TASK_NORMs. Each TASK_NORM was constrained to have a unique priority level to avoid signal-processing errors. However, in the case where a TASK_NORM needs a temporary priority boost to handle an important signal, the existing design had no mechanism to do this safely and efficiently.

To solve this problem, a Priority Escalation mechanism can be added to the Core. This mechanism allows a TASK_NORM to be temporarily raised to a higher priority level for a short period of time in order to handle an important signal, then automatically lowered back to its original priority level once processing is complete.

#### New design additions

- The total number of priority levels is increased to 24, i.e., the original 16 priority levels plus 8 temporary escalation levels. Note that if the number of TASK_NORMs in use is fewer than 16, the surplus priority levels can be regarded as temporary escalation levels, which helps ensure TASK_NORMs can be temporarily escalated flexibly without being limited by the number of TASK_NORMs currently in use.
- Each TASK_NORM's metadata gains an additional `base_pri` field to store the TASK_NORM's original priority level, allowing the Core to easily lower it back to the original priority once an important signal has been handled.
- A new API, `uedp_task_norm_set_urgent(task_id, new_pri)`, is added to allow temporarily raising a TASK_NORM's priority, where `new_pri` must fall within the range from `UEDP_TASK_PRI_LEVEL_16` to `UEDP_TASK_PRI_LEVEL_23` to ensure the temporary priority increase does not exceed the defined limit.
- A new API, `internal_task_norm_reset_pri(task_id)`, is added to allow lowering a TASK_NORM back to its original priority level after handling an important signal, ensuring that TASK_NORMs can return to their normal state after completing an important task. This means a task is only allowed to hold an elevated priority temporarily for one scheduling round in order to handle an important signal, after which it is automatically lowered back to its original priority level, ensuring other TASK_NORMs also get a fair and effective opportunity to have their own signals processed within the system.
- Logic for handling pending escalation is added to the scheduler to ensure that if all temporary escalation levels are already in use, a TASK_NORM that needs to be temporarily escalated will be marked as pending; after one scheduling round, it will be assigned the newest free priority level to process the important signal, helping ensure that important signals are handled promptly as soon as a temporary escalation slot becomes available.

#### Issues to address

For distributing priority levels when multiple TASK_NORMs are being temporarily escalated at the same time, the author proposes a mechanism that searches for the current highest temporary priority level in the TASK_NORM table, then, for each increasing priority level, if a TASK_NORM has already been escalated to that level, continues searching the next level until a free priority level is found to assign to the TASK_NORM that needs temporary escalation. If all temporary escalation levels are already in use, the API keeps the TASK_NORM's priority unchanged and does not perform the temporary escalation, while returning an error so the user can handle the situation appropriately.

#### Processing algorithm

- The priority level to assign, `target_pri`, is computed using the formula: `target_pri = current_max + step`.
- The Core looks up the `g_task_norm_ready` bitmap within the 16-23 partition to find the highest priority level currently ready, then increases `target_pri` step by step from `UEDP_TASK_PRI_LEVEL_16` to `UEDP_TASK_PRI_LEVEL_23` to find a free priority level.
- If a free priority level is found, `target_pri` is assigned to the TASK_NORM that needs temporary escalation, and this TASK_NORM's `base_pri` is saved so it can be lowered back after the important signal has been processed. If a given priority level is already assigned to another TASK_NORM, the search continues to the next level until either a free level is found or all temporary escalation levels have been checked. The step increment each time is 1 unit rather than 2 units.
- After the TASK_NORM finishes processing the important signal, the Core automatically lowers it back to its original priority level by reassigning `base_pri` to this TASK_NORM, while also updating the `g_task_norm_ready` bitmap to reflect the priority change.
- In the case where all temporary priority levels are already in use, the API stores a special value in `target_pri` to mark that the TASK_NORM is pending; after one scheduling round, it is guaranteed that this TASK_NORM will be assigned the newest free priority level to handle the important signal. While pending, this TASK_NORM keeps its original priority level but is marked so the scheduler prioritizes assigning it a temporary priority level once a slot becomes free, which helps ensure important signals are handled promptly as soon as a temporary escalation slot becomes available.

#### Conflict-handling rules

To ensure the consistency of the Ready Bitmap, when a TASK_NORM performs an Escalation, the Core performs the following:

- Temporarily clears the Ready bit at the original priority level (base_pri).
- Finds a free slot in the escalation range and sets the Ready bit there.
- After the urgent message has been processed, the Core performs the reverse process to return the Task to its static position, ensuring the Unique Priority property is always preserved at every point in time in the system.

#### Hypothetical operational analysis

When a TASK_NORM's handler function is called for execution, this means the TASK_NORM has been selected by the scheduler for execution and has a message allocated from that TASK_NORM's task queue. If the user wants the TASK_NORM to be executed with a higher priority in the next scheduling round, they can call the `uedp_task_norm_set_urgent(task_id, new_pri)` API to temporarily raise that TASK_NORM's priority; this requires that, after calling set_urgent, the TASK_NORM must send a message to itself so that the scheduler selects it for execution in the next scheduling round at the higher priority. After the TASK_NORM finishes processing the urgent message, the Core automatically lowers it back to its original priority level by reassigning `base_pri` to this TASK_NORM, while also updating the `g_task_norm_ready` bitmap to reflect the priority change.

> So, given that there may be multiple messages in the task queue, should the self-sent message be prioritized for processing, or should the messages already in the task queue be processed in order first?

The answer is that the self-sent message should be processed. This is because processing sequentially would result in a virtual priority-inversion error — that is, the TASK_NORM has been temporarily escalated but must still wait for old messages in the task queue to be processed first, which defeats the purpose of the temporary priority increase. Therefore, when a TASK_NORM sends a message to itself, the Core ensures this message is processed immediately in the next scheduling round at the higher priority, helping to ensure important signals are handled promptly and effectively.

The supplementary mechanism for PE will be Safe LIFO-nested FIFO (S-LnF), to ensure urgent messages are processed immediately without having to wait for old messages in the task queue, while still ensuring urgent messages are processed in a fair and efficient priority order within the system.

As of version 1.1.0, the PE mechanism was officially implemented with the `uedp_task_norm_set_urgent()` and `internal_task_norm_reset_pri()` APIs to raise and lower a TASK_NORM's temporary priority, along with pending-escalation handling logic added to the scheduler to ensure TASK_NORMs can be temporarily escalated flexibly and effectively within the system. However, the absence of the S-LnF mechanism will be addressed in later versions to ensure urgent messages are processed safely and efficiently, while still ensuring TASK_NORMs can operate flexibly and effectively within the system.

#### Feature-naming update and standardization

In version 1.1.1, the PE mechanism was updated and standardized to the name Atomic Priority Escalation (APE), to better reflect the atomic and safe nature of the temporary priority increase within the system. At the same time, the supplementary Safe LIFO-nested FIFO (S-LnF) mechanism was also implemented, to ensure urgent messages are processed safely and efficiently while still ensuring TASK_NORMs can operate flexibly and effectively within the system.

This mechanism, as it existed in version 1.1.0, has been retroactively redefined as non=S-LnF APE (non-supported Safe LIFO-nested FIFO Atomic Priority Escalation), to better reflect that this mechanism is only guaranteed to work correctly when a TASK_NORM's message queue is empty. If a TASK_NORM has multiple messages in its task queue, virtual priority inversion occurs — that is, the TASK_NORM has been temporarily escalated but must still wait for old messages in the task queue to be processed, which defeats the purpose of the temporary priority increase. The S-LnF mechanism was therefore added to ensure urgent messages are processed immediately without having to wait for old messages in the task queue, while still ensuring urgent messages are processed in a fair and efficient priority order within the system.

From version 1.1.1 onward, the APE mechanism was updated and standardized to the name Safe LIFO-nested FIFO Atomic Priority Escalation (S-LnF APE), to better reflect the safe and effective nature of the temporary priority increase within the system, while also ensuring urgent messages are processed fairly and efficiently within the system.

> So why were the non=S-LnF APE and S-LnF APE mechanisms renamed with the new "Atomic" property?

This stems from the process of testing and migrating from non=S-LnF APE to S-LnF APE. The developer discovered a phenomenon where, after PE had been executed, the task that had previously been granted PE would not be selected for execution by the next scheduling round, even though it still had a higher priority than every other TASK_NORM. This phenomenon occurred because the PE implementation had inadvertently also cleared the TASK_NORM's ready bit before performing the temporary priority increase, which resulted in the TASK_NORM having been escalated but not being selected for execution in the next scheduling round. Because of this, the S-LnF APE mechanism was implemented to ensure urgent messages are processed immediately without having to wait for old messages in the task queue, while still ensuring urgent messages are processed in a fair and efficient priority order within the system.

### [SLNF] Safe LIFO-nested FIFO — A safe mechanism for handling urgent messages

#### Revisiting the old design

In version 1.1.0, the PE mechanism (non=S-LnF PE) was implemented with a complete set of APIs. However, one fatal weak point was that this function was only guaranteed to work correctly when a TASK_NORM had exactly 1 message in its task queue. If a TASK_NORM had multiple messages in its task queue, virtual priority inversion would occur — that is, the TASK_NORM had been temporarily escalated but still had to wait for old messages in the task queue to be processed, defeating the purpose of the temporary priority increase.

To solve this problem, a Safe LIFO-nested FIFO (S-LnF) mechanism can be added to the Core. This mechanism ensures urgent messages are processed immediately without having to wait for old messages in the task queue, while still ensuring urgent messages are processed in a fair and efficient priority order within the system.

#### Completing the design implementation

##### The FIFO First-Insertion problem

In a FIFO, we perform insertion at the end of the list and removal from the front of the list. If insertion were instead performed at the front of the list, this approach would always run in O(n). This wastes time on sequential copying and does not guarantee fairness in handling urgent messages.

> So is there a way to guarantee both FIFO ordering and O(1) for both insert and remove?

##### The list-reversal solution

The solution is to reverse the data direction: insertion now means pushing to the end of the list, but in practice this is actually inserting at the front of the list. Removal is done by popping from the front of the list, but in practice this is actually removing from the end of the list. This approach guarantees FIFO ordering while still guaranteeing O(1) for both insert and remove. Because the S-LnF mechanism only needs to guarantee correct handling for the non-S-LnF PE version, there is no need to add an API for reversed-list removal — only an API for reversed-list insertion is needed to guarantee FIFO ordering.

#### Implementation logic

In the original FIFO API, we update `head = (head + 1) % capacity` to perform removal from the front of the list, and update `tail = (tail + 1) % capacity` to perform insertion at the end of the list. In the new S-LnF API, we update `head = (head - 1 + capacity) % capacity` to perform insertion at the front of the list, and update `tail = (tail - 1 + capacity) % capacity` to perform removal from the end of the list. This approach guarantees FIFO ordering while still guaranteeing O(1) for both insert and remove.

> So after performing a reversed insertion and completing the operation, what will the values of head and tail be?

The value of `head` will be the position of the first element in the list, and the value of `tail` will be the position of the last element in the list. This still preserves the original FIFO execution order without requiring any additional logic, while still guaranteeing O(1) for both insert and remove.

#### New API

A new function, `fifo_put_head()`, is added to perform a reversed insertion at the front of the list, in which `head` is updated using the formula `head = (head - 1 + capacity) % capacity` to guarantee FIFO ordering while still guaranteeing O(1) for both insert and remove. This API is used within the S-LnF mechanism to ensure that urgent messages are processed immediately without having to wait for old messages in the task queue, while still ensuring urgent messages are processed in a fair and efficient priority order within the system.

### [OCE] Out-Context Execution

Out-Context Execution (OCE) is μEDP's background service layer, used for work that should not run in the main path of Norm Tasks and Polling Tasks — for example, flushing logs, synchronizing data, or background I/O tasks. The goal of this layer is to make use of spare CPU time without affecting the timing of the main scheduler.

In the current implementation, each OCE service is modeled with `ocesvc_t` and managed through an internal singly linked list. This list has a sentinel `head` node to represent the empty state and act as a traversal anchor, where `head.id` is kept at `UINT8_MAX` so as not to collide with any valid service ID.

Key characteristics of the current design:

- `ocesvc_t` acts as a minimal SCB for a single OCE service.
- `ocesvc_ctrl_t` holds the `head` pointer and the `fill_size` of the actual services, not counting the sentinel.
- `ocesvc_register()` only commits `id`, `state`, `next`, and `fill_size` after successfully appending to the list.
- `ocesvc_unregister()` only removes a registered service and does not allow removing the sentinel `head`.
- `ocesvc_scheduler()` executes on an FCFS basis and runs at most one READY service per scheduler round.

The proposed operating flow is:

1. The main scheduler finishes processing Norm Tasks and Polling Tasks.
2. If the system still has spare time, the OCE scheduler traverses from `head->next` and finds the first READY service.
3. That service transitions to `RUNNING`, its handler is called, then it transitions to `COMPLETED` once processing is finished.
4. If no service is READY, OCE does not consume any additional CPU time.

This design keeps OCE separate from the Core's real-time path, while remaining simple enough to implement across multiple platforms. At a later stage, μE-OS may extend this model into AOCE with additional fields such as priority, quantum, timeout, and an error callback, but the current foundation should still be understood as FCFS service dispatch on a linked list.

#### Distinguishing TASK_POLL from ocesvc

- **Task Polling (Application Domain):** Part of application logic but not message-based. It still remains within the Scheduler's managed list.
  - *Example:* Key scanning (debounce), checking sensor thresholds.
  - *Characteristics:* Has a "priority" (though lower than Norm Tasks) and needs to run regularly.
- **Out-Context Execution (System Domain):** "Background" work that supports the system's continued operation.
  - *Example:* Flushing itnlog, VFS data migration (writing to SD Card), hardware watchdog feeding.
  - *Characteristics:* Runs only when the system is **completely idle** (`STAT_NRDY`).

Conclusion:

- **Use a Polling Task when:** You need to handle repeating application logic that is "lightweight" and needs to be guaranteed to run after messages have been processed (e.g., LED blinking).
- **Use OCE when:** Writing System Services whose nature is to "clean up" data from the Core out to peripherals (Logging, Storage, Cloud Sync).

#### Benefits of Separation

- **Jitter Isolation:**
  - If you put log printing (which is time-consuming) into a Polling Task, it will delay other Polling Tasks (such as key scanning).
  - When separated into OCE, the entire App logic (Norm + Polling) has already finished. Whether logging runs slowly or quickly no longer affects the responsiveness of a button press.
- **Resource management via an "Idle Window":**
  - OCE only executes when the Scheduler confirms: *"I have nothing urgent left to do."*
  - This design allows implementing smart algorithms such as: *"If there's 10ms of idle time left, write to the SD Card. If there's only 1ms free, just feed the Watchdog."* A Polling Task has no way to sense the system's overall idle rhythm like this.
- **Power Awareness:**
  - OCE is the final "stopping point" before the CPU goes to sleep. If it were merged into Polling, the CPU would have to stay awake longer to scan through the Polling Task table even when it might not be necessary. Separating them lets μE-OS decide on a `Sleep` command more accurately.

#### Drawbacks and Challenges

- **Increased main-loop complexity:** An additional execution layer outside `ciedpc_task_scheduler()` must be managed.
- **Risk of delaying the next cycle:** If OCE performs a task that is too heavy (such as writing a large file to the SD Card without breaking it into smaller chunks), it will slow down the start of the next scheduling cycle. This is addressed via the SCB in AOCE, which breaks work into smaller quanta and includes a timeout mechanism to prevent OCE from occupying too much time.

#### Current implementation design

- After finishing all Norm Tasks and Polling Tasks, the scheduler switches to OCE when the system still has idle time.
- The OCE scheduler traverses the internal linked list, finds the first READY service, and executes at most one service per scheduler round.
- This implementation keeps OCE at a simple FCFS level, without needing a separate queue beyond the existing linked list.

This design is sufficient to serve lightweight background services and can gradually be extended to AOCE once μE-OS needs additional priority, quantum, timeout, and error-callback support.

In μE-OS, this will be upgraded to AOCE (Advanced OCE) with an SCB (Service Control Block) to manage OCE services more flexibly and handle time-based prioritization, along with an expected-execution-time mechanism, quantum, and error callback.

### [FCR] Fatal Code Return — Identifying and handling critical errors

Before FCR existed, critical errors inside the Core (pool exhaustion, invalid pointers, wrong task IDs, non-existent transitions, etc.) were handled **silently and inconsistently** across modules: some places did `return NULL`, others did `return STAT_ERROR`, and others simply left a comment like `// could log the error here` without actually doing anything. As a result, when a critical error occurred on real hardware, no trace was recorded and no consistent handling action was taken.

FCR (Fatal Code Return) solves this with a **centralized error-code table**: every critical error in the Core is assigned a fixed code, which is looked up to determine its severity and corresponding handling action, and is always logged through `itnlog` before that action is executed.

#### Error-code design

The FCR error code (`uedp_fcr_code_t`, of type `ui16`) uses the same encoding principle as `[HES]`: the high byte is the **MODULE** code where the error originated, and the low byte is the specific **SUB-CODE** within that module, combined using the `UEDP_FCR_CODE(mod, sub)` macro. The `0x9x` range was chosen for FCR because the `0xAx` → `0xFx` ranges are already taken by `TASK_NORM`/`TASK_POLL`/`TASK_PRI`/`FSM_SIG`/`TSM_SIG`/`TSM_STATE` (see `[HES]`).

| Module | Code | Meaning |
| --- | --- | --- |
| `UEDP_FCR_MOD_MSG` | `0x90` | Message management (`uedp_msg`) |
| `UEDP_FCR_MOD_TASK` | `0x91` | Task management (`uedp_task`) |
| `UEDP_FCR_MOD_TIMER` | `0x92` | Timer management (`uedp_timer`) |
| `UEDP_FCR_MOD_SM` | `0x93` | State machines (`uedp_fsm`/`uedp_tsm`) |
| `UEDP_FCR_MOD_ITNLOG` | `0x94` | Internal logger (`uedp_itnlog`) |
| `UEDP_FCR_MOD_OCE` | `0x95` | Out-Context Execution (`uedp_ocesvc`) |
| `UEDP_FCR_MOD_PAL` | `0x96` | PAL / hardware services (logdp, rprintf, memrp, arch...) |
| `0x97` → `0x9D` | *(unused)* | Reserved for core modules introduced later |
| `UEDP_FCR_MOD_APP` | `0x9E` | Reserved for the application layer to declare its own error codes |
| `UEDP_FCR_MOD_UNK` | `0x9F` | Fallback when a lookup fails to find the error code |

Each error code is attached to a `uedp_fcr_entry_t` consisting of a short description (`desc`), a severity level (`severity`: `WARN`/`ERROR`/`FATAL`), and a handling action (`action`):

- `UEDP_FCR_ACT_LOG_ONLY`: only logs the error, without interfering with the execution flow.
- `UEDP_FCR_ACT_RESET_TASK`: flags the error so a higher layer can recover the affected task on its own (FCR does not itself reset another task's TSM/FSM).
- `UEDP_FCR_ACT_SYS_RESET`: calls `pal_sys_reset()` to restart the entire system.
- `UEDP_FCR_ACT_SYS_PANIC`: calls `pal_sys_fatal()` to halt the system immediately.

#### Raise flow

```c
void uedp_fcr_raise(uedp_fcr_code_t code, const char* file, ui32 line, const char* extra_msg) {
  const uedp_fcr_entry_t* entry = uedp_fcr_lookup(code);

  // 1. Always log first, even when the following action is SYS_PANIC/SYS_RESET
  uedp_itnlog_log(pal_sys_get_tick(), internal_uedp_fcr_sev_to_level(entry->severity),
                  ITNLOG_TAG_FCR, (extra_msg != NULL) ? extra_msg : entry->desc);

  // 2. Execute the corresponding action
  switch (entry->action) { /* LOG_ONLY / RESET_TASK / SYS_RESET / SYS_PANIC */ }
}
```

The two macros `UEDP_FCR_RAISE(code)` and `UEDP_FCR_RAISE_MSG(code, extra)` automatically fill in `__FILE__`/`__LINE__`, where `RAISE_MSG` allows passing an additional, more specific context description (e.g., a function name or an invalid parameter value) in place of the table's default `desc`.

`uedp_fcr_lookup()` performs a linear scan of the `g_fcr_table[]` table; if the error code is not found, it returns the `UEDP_FCR_UNKNOWN` entry (default `SEV_FATAL` + `ACT_SYS_PANIC`) — the most severe action is deliberately chosen for the "unknown error" case to avoid missing anything.

#### Integration with itnlog

FCR does not log directly on its own; instead it goes through `uedp_itnlog_log()` with a dedicated `ITNLOG_TAG_FCR` tag, mapping `severity` to the corresponding log level (`WARN`/`ERROR` → `ITNLOG_LEVEL_WARN`/`ERROR`, `FATAL` → `ITNLOG_LEVEL_FATAL`). This reuses the entire existing `[PPLP]` mechanism (ring buffer, filtering by tag/level, dispatch to multiple backends via `logdp`) instead of building a separate logging path for critical errors.

#### Points where FCR has been integrated into the Core

FCR only has value once it is actually "stitched" into the places that were genuinely silent failures before, rather than existing as a standalone module on its own. As of this version, FCR has been raised at more than 40 points across 7 core files:

| File | Some representative error codes |
| --- | --- |
| `uedp_msg.c` | `MSG_POOL_EXHAUSTED`, `MSG_INVALID_PTR`, `MSG_ISR_FIFO_FULL`, `MSG_POOL_MISCONFIG` |
| `uedp_task.c` | `TASK_INVALID_ID`, `TASK_QUEUE_FULL`, `TASK_INVALID_PRI`, `TASK_PRI_EXHAUSTED` |
| `uedp_timer.c` | `TIMER_POOL_EXHAUSTED`, `TIMER_INVALID_PARAM`, `TIMER_CORRUPTED` |
| `uedp_tsm.c` | `SM_INVALID_TRANS`, `SM_NULL_HANDLER` |
| `uedp_fsm.c` / `uedp_fsm.h` | `SM_NULL_HANDLER` (in both `go_next`/`go_back` and `uedp_fsm_dispatch()`) |
| `uedp_ocesvc.c` | `OCE_REGISTRY_FULL`, `OCE_INVALID_SVC`, `OCE_APPEND_FAILED`, `OCE_NOT_INIT` |
| `pal_logdp.c` | `PAL_LOGDP_TABLE_FULL` (fully replacing the old direct call to `pal_sys_fatal()`) |

The principle for choosing where to raise: **only raise on branches that are genuinely abnormal**, not on valid branches that occur regularly during normal operation — for example `TSM_STATE_STAY` (staying in the current state), `g_task_norm_ready == 0` (the scheduler is idle, which happens on every loop when idle), or calling `uedp_timer_remove()` on a timer that was never set. Raising indiscriminately on normal branches would turn FCR into a source of log noise rather than a meaningful warning signal.

> **A lesson learned during integration**: as FCR began to be raised from more points in the Core, a latent bug already present in `uedp_itnlog_log()` was exposed: this function dereferences `uedp_task_norm_get_current_msg()->sig` without a NULL check. Previously, no one called `itnlog_log()` from outside the context of a dispatching task, so this bug never occurred; but FCR can be raised from many places, including from `main()` during setup (before any task has run) — leaving `g_current_msg` still `NULL` and causing a crash on the very first raise. This was fixed with a single defensive NULL check in `itnlog_log()`. This is concrete evidence of why FCR needs to be written and tested carefully: adding an error-reporting mechanism can itself unintentionally open up a new crash path if the modules it depends on (here, `itnlog`) are not yet defensive enough.

#### Limitations / remaining work

- In version 0.1, `UEDP_FCR_ACT_RESET_TASK` **does not yet automatically recover** the affected task — it currently only goes as far as logging an `ERROR`; resetting the TSM/FSM to a safe state still has to be handled by a higher layer (a supervisor task or OCE) on its own.
- `UEDP_FCR_ITNLOG_BUF_CORRUPT` already has a code in the table, but there is **no actual hash-checking logic yet** on the read side (`uedp_itnlog_dump()`) — currently `itnlog` only computes the hash on write and does not yet re-compare it on read to detect corruption.
- Users at the application layer can declare their own error codes via `UEDP_FCR_CODE(UEDP_FCR_MOD_APP, x)`, but there is currently no mechanism allowing the application layer to **register additional entries** into `g_fcr_table[]` at runtime — the table is currently `static const`, so adding a new entry requires directly editing `uedp_fcr.c`.

## Development Tools

### [KwDI] Kconfig with Docker Integration

Kconfig is a popular configuration tool used in Linux kernel projects and other embedded projects. It allows users to easily configure software features through a command-line or graphical interface. In μEDP, Kconfig is integrated with Docker to provide a consistent development environment that is easy to deploy across different platforms.

With Docker, users can easily create containers that hold all the tools needed to build and run μEDP, while ensuring the development environment is consistent across all machines by using the developer's original Linux development environment. This helps minimize issues related to differences in development environments between different machines, while making it easy for users to deploy and test new μEDP features without having to worry about installing and configuring development tools on their own machine.
