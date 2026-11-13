# libmg Review — Safe Heap Allocation (SHA) for μE-OS v1.4.0

<!-- STATUS - REVIEW: technical review of an existing vendored component against the SHA roadmap requirements -->

- Date: 2026-09-23
- Scope: `libmg/mg/{include,source}/{heap,memalloc,membit,mempool,mg_alloc}.*`, `libmg/lib/{include,source}/{rbtree,link_list,macro,math,compare}.*`, `libmg/docs/中文/使用手册.md`, `libmg/README.md`
- Related: [`docs/ueos-roadmap.md`](../ueos-roadmap.md) (v1.4.0 — "The Safe Heap Allocation"), [`docs/to-do.md`](../to-do.md), [`sources/pal/service/memrp/pal_memrp.h`](../../sources/pal/service/memrp/pal_memrp.h) (existing memory-reporting service), [`sources/pal/pal_core.h`](../../sources/pal/pal_core.h) (PAL type/macro conventions)
- Purpose: `libmg` was recently vendored into the repository as a candidate memory-allocation component. This document reviews its five allocator modules in detail against the four checklist items of v1.4.0, identifies concrete gaps against "double free hoặc memory leak" protection, and proposes which module(s) μEDP should adopt and how to wire them into the existing PAL/core conventions.

## 1. Summary / Recommendation

`libmg` already provides working implementations of both algorithms named explicitly in the roadmap — First-fit (`heap.c`) and Best-fit (`memalloc.c`) — and both already implement bidirectional coalescing on free. This satisfies the *algorithmic* core of the second checklist item out of the box. However, neither module implements double-free protection, and neither implements always-on memory-leak detection (only `heap.c` has an optional, compile-time, off-by-default leak tracker). These two gaps map directly to the "double free hoặc memory leak" protection clause in the roadmap and are the primary engineering work remaining before v1.4.0 can claim the feature complete.

Recommendation: adopt `heap.c` (First-fit) as the primary SHA backend, harden it with a magic-tagged, double-free-safe block header and always-on leak accounting, expose it through a new PAL-level API (`pal_sha_*`, following `pal_core.h` conventions), and report usage through the existing `pal_memrp` service. Keep `memalloc.c` (Best-fit) available as a compile-time alternative for workloads with high size variance, behind the same PAL API, but do not attempt to run both simultaneously against the same backing memory (see §4.4). `membit.c`/`mempool.c`/`mg_alloc.c` are fixed-size/pool allocators — valuable for other purposes, but out of scope for "Safe Heap Allocation" since the roadmap explicitly asks for a general-purpose First-fit/Best-fit allocator with coalescing, which only `heap.c`/`memalloc.c` provide.

## 2. Module-by-module review

### 2.1 `heap.c` / `heap.h` — First-fit, singly-linked free list

- Algorithm: SLOB-style singly-linked free list, kept in address order. Allocation walks the list and returns the first free block large enough (`heap_malloc`) — this is First-fit, matching the roadmap's first named option verbatim.
- Coalescing: on `heap_free`, the freed block is re-inserted at its address-ordered position and merged with its immediate predecessor and/or successor if they are physically adjacent and both free. This is full bidirectional coalescing, already covering the "cơ chế Coalescing để giảm fragmentation" requirement.
- Backing storage: a single static array `all_heap[CONFIG_HEAP]` (12 KB by default) — no `sbrk`/OS dependency, appropriate for a bare-metal MCU target.
- Leak tracking: optional, compile-time (`HEAP_TRACKING`), off by default. When enabled, a fixed 128-entry static table records `(ptr, size, file, line)` per live allocation, populated via `__FILE__`/`__LINE__` at the call site (implies `heap_malloc` is normally invoked through a tracking macro, not called directly, when tracking is enabled).
- Double-free protection: none. `real_heap_free()` does not check whether the block being freed is already on the free list; freeing the same pointer twice will corrupt the address-ordered free list (e.g., inserting the same node twice, or merging a block with itself) with no detection.
- Concurrency: no locking. Single-core, non-preemptive-scheduler assumption, consistent with μEDP's existing scheduler design (see `docs/review/dmp-gda.md`).
- Alignment: 8-byte, via a locally-defined constant rather than the shared `libmg/lib/include/macro.h` `alignment_byte` macro.

### 2.2 `memalloc.c` / `memalloc.h` — Best-fit, rbtree + doubly-linked list

- Algorithm: free blocks are indexed twice — once in an intrusive rbtree keyed by block size (`rb_first_greater()` finds the smallest free block ≥ requested size in O(log n), giving true Best-fit, matching the roadmap's second named option), and once in a doubly-linked list in physical address order (used to find adjacent neighbors for coalescing in O(1) instead of a linear scan).
- Coalescing: on free, the block's physical neighbors are found via the doubly-linked list; if either neighbor is free it is removed from the rbtree, merged, and the merged block is re-inserted into the rbtree at its new size. Also fully bidirectional.
- Backing storage: its own static array (10 KB by default), completely independent of `heap.c` — see §4.4 for the implication of this.
- Leak tracking: none.
- Double-free protection: none. `mem_free()` does not verify the block is currently allocated before touching the rbtree/list, so a double-free will attempt to remove/merge a node that may already have been unlinked, which is a more severe failure mode than in `heap.c` because rbtree corruption from a double-remove is harder to reason about than singly-linked-list corruption.
- Concurrency: no locking, same assumption as `heap.c`.
- Dependency: self-contained — does not call into `heap.c`. This is the only one of the five modules with no dependency on another `mg`-family module.

### 2.3 `membit.c` / `membit.h` — fixed-block bitmask pool

- Fixed block size, up to 32 blocks per pool, tracked with a 32-bit bitmask and `__builtin_ctz` for O(1) find-first-free.
- Backing storage for each pool is obtained via `heap_malloc()` — i.e. depends on `heap.c` being linked and initialized even if the application never calls `heap_malloc()`/`heap_free()` directly.
- No coalescing concept applies (fixed block size). No double-free guard.
- Not applicable to the roadmap's First-fit/Best-fit-with-Coalescing requirement — this is a pool allocator, not a general heap. Relevant only as a secondary/opt-in mechanism (e.g., for fixed-size, high-churn allocations) if the SHA design later wants a two-tier allocator.

### 2.4 `mempool.c` / `mempool.h` — fixed-object intrusive free list

- Similar to `membit.c` but uses an intrusive singly-linked free list instead of a bitmask, so it is not capped at 32 blocks. Backing storage obtained via a single `heap_malloc()` call — same `heap.c` dependency as `membit.c`.
- This is the only module among the five with a double-free guard: `mem_pool_free()` checks a per-block `used` flag and returns early (`if (!blk->used) return;`) instead of touching the free list when the flag is already clear. This is a useful precedent/pattern to replicate in `heap.c`/`memalloc.c` (see §3.1) even though `mempool.c` itself is not the SHA target.
- Not applicable to the general-heap requirement for the same reason as `membit.c`.

### 2.5 `mg_alloc.c` / `mg_alloc.h` (`mg_region`) — region/arena allocator

- Two modes: `MG_REGION_POOL` (auto-creates `membit_pool_t` instances per power-of-two size class on demand — depends transitively on `membit.c` → `heap.c`) and `MG_REGION_BUMP` (linear bump allocator, no per-object free, only `mg_region_reset()`/`mg_region_destroy()`).
- `MG_REGION_POOL` mode has no per-block free function exposed in the header — objects are freed only implicitly via `mg_region_reset()` (keeps the pools, clears bitmasks) or `mg_region_destroy()` (full teardown). This makes it unsuitable as a general "malloc/free" replacement for arbitrary task-lifetime objects.
- Interesting as a future *complementary* mechanism (e.g., per-task or per-message-batch scratch arenas that never leak because the whole region is torn down at once), but does not fulfill the roadmap's explicit First-fit/Best-fit-with-Coalescing ask, and is out of scope for the v1.4.0 checklist as currently worded.

## 3. Gap analysis against the four SHA checklist items

### 3.1 Item 2 — "thuật toán ... First-fit hoặc Best-fit với cơ chế Coalescing ... bảo vệ double free hoặc memory leak"

| Sub-requirement | `heap.c` | `memalloc.c` | Gap to close for v1.4.0 |
| --- | --- | --- | --- |
| First-fit or Best-fit | ✅ First-fit | ✅ Best-fit | None — pick one or expose both |
| Coalescing | ✅ bidirectional | ✅ bidirectional | None |
| Double-free protection | ❌ | ❌ | Must add. Proposed approach below. |
| Memory-leak protection | ⚠️ optional, compile-time, off by default | ❌ | Must make always-on (at least accounting), independent of a debug build flag |

Proposed double-free protection (applies to whichever of `heap.c`/`memalloc.c` is adopted):
Add a small in-band header per allocation (or repurpose the existing block-header field) carrying: (a) a magic value written on allocation and cleared/poisoned on free, and (b) an explicit `state: {FREE, USED}` flag, following the exact pattern already proven in `mempool.c`'s `used` flag. `real_heap_free()`/`mem_free()` should check this flag first and call `pal_sys_fatal()`/`UEDP_PANIC()` (per `pal_core.h` conventions) on a double-free instead of silently corrupting the free-list/rbtree. This is a small, local change — it does not require restructuring either allocator's core algorithm.

Proposed leak protection: promote `heap.c`'s existing tracking table design (currently `HEAP_TRACKING`-gated) to an always-compiled, lightweight live-allocation counter (`ui32 sha_live_count`, `ui32 sha_live_bytes`) updated on every malloc/free, exposed through `pal_memrp` (see §4.3) so leaks are visible via the existing reporting path without requiring a debug build. The full per-allocation file/line table can remain an optional `HEAP_TRACKING`-style build flag for deep debugging, since a 128-entry static table has a real RAM cost that is not appropriate to force on for all targets (e.g., STM32F103).

### 3.2 Items 1, 3 — design/algorithm documentation

Items 1 and 3 ask for design-algorithm documentation for SHA and specifically for the First-fit/Best-fit-with-Coalescing algorithm. `libmg/docs/中文/使用手册.md` already documents each module's API and usage in Chinese, but does not document the algorithm in the context of μEDP's own safety requirements (single-core assumption, PAL integration, double-free/leak hardening proposed in §3.1). This English review, together with a follow-up dedicated algorithm document (see §5, open item), is intended to close items 1 and 3 once the design decisions here are confirmed.

### 3.3 Item 4 — release v1.4.0

Out of scope for this review; depends on items 1–3 being closed first.

## 4. Integration considerations for μEDP

### 4.1 μEDP core currently has no dynamic allocation

A repository-wide search (`sources/`) for `malloc|heap_malloc|mem_malloc` returns no matches. All existing μEDP subsystems (task table, GDA/GDP, message pools per `sources/core/inc/uedp_msg.h`) use fixed-size static arrays exclusively. Introducing a libmg-backed heap is therefore a new capability, not a refactor of existing code — it carries no migration risk to existing subsystems, but also means there is no existing call site to learn integration conventions from; the API proposed in §4.2 is a fresh design.

### 4.2 Proposed PAL-level API shape

Following `sources/pal/pal_core.h` conventions (`ui8/ui16/ui32` typedefs, `RETR_STAT` return enum, `UEDP_ATTR_WEAK`/`PACKED`/`ALIGNED`/`UNUSED` attributes, `pal_sys_fatal()`/`UEDP_PANIC()` for fatal conditions), a new service under `sources/pal/service/sha/` mirroring the existing `sources/pal/service/memrp/` layout is proposed:

```c
RETR_STAT pal_sha_init(void);
void      *pal_sha_malloc(ui32 size);
void       pal_sha_free(void *ptr);
RETR_STAT  pal_sha_get_info(pal_sha_info_t *out);   /* live_count, live_bytes, total, algo */
```

`pal_sha_malloc`/`pal_sha_free` wrap the chosen libmg backend (`heap_malloc`/`heap_free` for First-fit, or `mem_malloc`/`mem_free` for Best-fit, selected at compile time), and internally invoke the hardened double-free check and leak counters from §3.1. A double-free or corruption detected inside the wrapper should call `pal_sys_fatal()`/`UEDP_PANIC()`, consistent with how other PAL fatal conditions are already handled, rather than returning an error code silently.

### 4.3 Reuse of `pal_memrp`

`sources/pal/service/memrp/pal_memrp.c` currently only forwards to `internal_uedp_msg_pool_get_info()` — there is no existing hook for a general heap. `pal_memrp_report()` should be extended with a second source: `pal_sha_get_info()`, so heap usage (current/peak used, live allocation count) surfaces through the same reporting path operators already use for the message pool, rather than inventing a parallel reporting mechanism.

### 4.4 Do not mix `heap.c` and `memalloc.c` on the same backing memory

`heap.c` and `memalloc.c` each own a separate, disjoint static array (`all_heap[CONFIG_HEAP]`, 12 KB vs 10 KB by default). Using both simultaneously does not produce one unified heap — it produces two independently-fragmenting heaps whose combined static RAM footprint is the sum of both, which is wasteful on the target MCUs in this repo (STM32F103/H723, ESP32-S3). The v1.4.0 design should pick one algorithm as the default SHA backend (recommendation: `heap.c`/First-fit, for its lower per-operation overhead — O(n) list walk with no rbtree bookkeeping cost — appropriate for the small heap sizes typical of the current targets) and keep the other available only as an explicit compile-time alternative, not as a simultaneously-linked second heap.

### 4.5 Dependency chain if fixed-size pools are used later

If a future iteration wants to layer `membit.c`/`mempool.c`/`mg_alloc.c` (`MG_REGION_POOL`) on top of the SHA heap for fixed-size, high-churn allocations, note that all three transitively call `heap_malloc()` for their own backing storage — so `heap.c` must be initialized regardless of which general-purpose algorithm (`heap.c` vs `memalloc.c`) is chosen as the "primary" SHA allocator. This is not a blocker, just a wiring detail: `pal_sha_init()` should always initialize `heap.c` first even when `memalloc.c` is selected as the primary allocator, so these secondary modules remain usable.

### 4.6 Minor consistency note

`heap.c`, `memalloc.c`, `mempool.c`, and `membit.c` each redefine their own local 8-byte alignment constant instead of reusing `libmg/lib/include/macro.h`'s `alignment_byte` macro. Not a functional issue, but worth unifying while touching these files for the double-free/leak hardening in §3.1, to avoid the constants silently drifting apart in the future.

## 5. Open questions for the roadmap owner

1. Default algorithm choice: confirm First-fit (`heap.c`) as the v1.4.0 default per §4.4, with Best-fit (`memalloc.c`) as a compile-time alternative — or is Best-fit preferred as the default (e.g., if fragmentation matters more than per-call overhead for the target workloads)?
2. Leak-detection granularity: is an always-on live-count/live-bytes counter (§3.1) sufficient for v1.4.0, or is the full per-allocation file/line tracking table (currently `HEAP_TRACKING`-gated in `heap.c`) required to be always-on as well, accepting its static RAM cost on all targets including STM32F103?
3. Fatal vs. recoverable double-free: confirm that a detected double-free should trigger `pal_sys_fatal()`/`UEDP_PANIC()` (hard stop) rather than a recoverable `RETR_STAT` error — this affects whether callers of `pal_sha_free()` need to check a return value at all.
4. `mg_region`/`membit`/`mempool` scope: confirm these fixed-size/pool/arena modules are explicitly out of scope for the v1.4.0 "Safe Heap Allocation" checklist items (as argued in §2.3–2.5) and belong to a later or separate roadmap item, rather than being folded into this feature.

## 6. Conclusion

`libmg` already supplies working, coalescing-capable First-fit and Best-fit allocators that directly match the algorithms named in the μE-OS v1.4.0 roadmap. The remaining work to satisfy the "double free hoặc memory leak" protection clause is small and localized (a per-block state flag modeled on `mempool.c`'s existing guard, plus an always-on live-allocation counter) rather than requiring a new allocator to be written from scratch. Once the open questions in §5 are resolved, the next step is a dedicated First-fit/Best-fit-with-Coalescing algorithm document (roadmap item 3) and the `pal_sha_*` PAL service implementation (roadmap item 2) proposed in §4.2–4.3.
