# Design Document: Migrating features from [KwDI] to [PLTF]

## 1. Background

`docs/to-do.md`, under **Version 1.2.0: The Infrastructure Preparation for μE-OS**, states:

- `[ ] Add a PLTF (Portable Local Test Framework) design document to provide multi-scale automated testing capability.`
- `[ ] Implement the PLTF design.`

Meanwhile, `arch-design.md`'s **[KwDI] Kconfig with Docker Integration** section (version 1.1.2) only describes integrating Kconfig with Docker to configure the μEDP core through a command-line interface — it does not yet cover automated code generation with accompanying testing. This document fills that gap: it describes the original KwDI architecture, its limitations, and the PLTF design that has replaced/extended KwDI in the current code.

## 2. Original design architecture from KwDI

### 2.1 Components

KwDI consists of 3 main parts, all located neatly at the repo root and in `sources/common/`:

- `Kconfig` (root) + `sources/app/kconfig/{core,pal,decl}.kconfig`: define the configuration tree.
- `sources/common/kconfiglib/`: the `kconfiglib` + `menuconfig` library (third-party) used to read the Kconfig tree and display an interactive `menuconfig` interface on the terminal.
- `sources/common/pyspec/`: functions that generate `decl.kconfig` (task norm, task poll, signal, hardware API) based on the quantities the user enters (`usrinp_pspec.py`, `tsknrmdcl.py`, `tskpoldcl.py`, `sigdcl.py`, `hwapidcl.py`).
- `uedp.py` (root): the single script that orchestrates the entire flow — it collects input, calls `menuconfig`, and also **generates code itself** (`corecfg_gen`, `palcfg_gen`, `app_cfg_gen`, `app_decl_gen`, `pal_arch_gen`) by inserting `#define` strings directly between 2 markers (`// KCONFIG_CORECFG_START` / `// KCONFIG_CORECFG_END`) inside existing header files under `sources/app/config/`.
- `Dockerfile` (original version): a `python:3.13-slim` image, installing only `kconfiglib`, `CMD ["python", "uedp.py", "menuconfig"]`.

### 2.2 Original operating flow

1. The user runs `docker build` then `docker run` (there is no `docker-compose.yaml`, no `entrypoint.sh`).
2. The container starts and runs `python uedp.py menuconfig` directly.
3. `uedp.py` asks for input (number of tasks, number of signals, whether to use FSM/TSM...) → writes to `sources/app/kconfig/decl.kconfig`.
4. `kconfiglib.Kconfig("Kconfig")` loads the whole tree, opens `menuconfig` for the user to adjust values → writes out `.config`.
5. Right inside `main()` of `uedp.py`, it calls `corecfg_gen()`, `palcfg_gen()`, `app_cfg_gen()`, `app_decl_gen()`, `pal_arch_gen()` in sequence — each function walks `kconf.unique_defined_syms` itself, formats the `#define` strings itself, then **patches them directly** into existing `.h` files via the marker pair.

### 2.3 Limitations of KwDI (why PLTF was needed)

- **No stage separation**: input collection, interactive configuration (menuconfig), and code generation all live inside a single `main()` function of `uedp.py`. To regenerate code from an existing `.config` (e.g., in CI), the whole interactive `menuconfig` still has to be run again.
- **"String-patching" code generation (marker-based patch)**: `corecfg_gen`/`palcfg_gen` require the target `.h` file to **already exist** with the correct marker pair before it can be patched — a brand-new file cannot be generated from scratch, and it's fragile if someone accidentally deletes a marker.
- **`sources/common/testspec/`** (an earlier Jinja2-based prototype, later renamed to `pycdscriptor`) already existed but was only a **draft never wired into the real flow**: the original `appcfgpgen.py` just `print(output)`ed to the screen with a hardcoded `current_date` of `'16 May 2025'` — it didn't read the real `.config` and didn't write a file.
- **A minimal Docker image**, with only `kconfiglib` installed: no `gcc/cmake/gdb`, no ESP-IDF, unable to build or run tests inside the container — users still had to leave the container to build by hand.
- **No `entrypoint.sh`/`docker-compose.yaml`**: the container ran the `CMD` directly as root, with no UID/GID handling → files created (via the mounted volume) ended up owned by `root` on the host, which was inconvenient when editing them from outside the container.
- **No workspace separation**: there was no concept of separate directories for "core source code" versus "testing workspace" — everything was mixed together in the repo.

## 3. PLTF design architecture

The core principle of PLTF is to **clearly separate the 2 stages** that were merged together in KwDI:

- **Stage 1 — Declaration & Interactive Config** (still handled by `uedp.py`, but now trimmed down).
- **Stage 2 — Test/Config Generation** (moved entirely to `pltf/pycdscriptor/`, using Jinja2 templates instead of string-patching).

New directory structure:

```text
pltf/
├── kconfigspec/                 # Generates decl.kconfig (replaces the old sources/common/pyspec)
│   ├── usrinp.py
│   ├── tnorm.py
│   ├── tpoll.py
│   ├── sig.py
│   └── hwapi.py
├── templates/               # Jinja2 templates — generate NEW files, no more string-patching
│   ├── appcfgh.txt
│   ├── appdeclh.txt
│   ├── corecfgh.txt
│   ├── palcfgh.txt
│   ├── archh.txt
│   ├── archc.txt
│   └── appc.txt             # Template for app.c, used by jnerator/postgen (μE-LS)
└── pycdscriptor/
    ├── attribarse/            # Reads .config into a structured context
    │   ├── dotcfg.py
    │   └── glbda.py           # Bridges context["task_tsm"/"task_fsm"] with the μE-LS glbda: block
    ├── lstaxer/               # Parses + validates the μE-LS YAML (see section 3.5)
    │   ├── symresolv.py, nullremov.py, strucjec.py
    │   ├── lukupmodel.py, vlid.py, kre8.py
    │   └── pydantic_model/    # logic.py, resrc.py, misc.py
    ├── ustab/                 # Unified Symbol Table (see section 3.5)
    │   ├── gnnerate.py, cvert.py, xportstax.py
    │   └── custab.py          # Orchestrator
    └── jnerator/
        ├── pregen/            # Generates Kconfig-based declarations (one artifact per file)
        │   ├── cfpcall.py     # Parses .config once, returns a shared context
        │   ├── appcfgpgen.py, corecfgpgen.py, palcfgpgen.py, appdeclpgen.py
        │   ├── archdirpgen.py, archhpgen.py, archcpgen.py
        │   └── fpregen.py     # Orchestrator, calls all 7 generators above in sequence
        └── postgen/           # Generates implementation logic from validated μE-LS YAML
            ├── cgen.py         # Entry point: yaml -> app.c
            └── modalcvert.py   # Renders app.c using the appc.txt template
```

Compared with `sources/common/kconfiglib/` (kept unchanged, not moved, since it is a third-party library rather than in-house code), the entirety of KwDI's **in-house** portion (`kconfigspec`, `pycdscriptor`) is consolidated into a single location, `pltf/`, separate from `sources/common/` — reflecting the true meaning of "Portable": `pltf/` does not depend on the `sources/` structure and could be reused for a different μEDP project simply by pointing it at the correct output path.

### 3.1 `uedp.py` after the refactor

`uedp.py` now has exactly one responsibility: generate `decl.kconfig` and run the interactive `menuconfig`.

```python
from pltf.kconfigspec import user_input, task_norm_declaration, task_poll_declaration, signal_declaration, hardware_api_declaration

def main():
  os.environ["KCONFIG_CONFIG"] = ".config"
  os.environ["MENUCONFIG_STYLE"] = "aquatic"
  # fsm_flags/tsm_flags/n_tsm_st_list/n_fsm_st_list are now lists, each element
  # corresponding to one task norm (each task can declare its own FSM/TSM and
  # state count, instead of sharing a single flag + count as before 1.1.6).
  (n_norm, n_poll, n_sig, fsm_flags, tsm_flags, n_tsm_st_list, n_fsm_st_list, n_hw_api) = user_input(DEFAULT_VALS)
  open("sources/app/kconfig/decl.kconfig", "w").close()
  task_norm_declaration(n_norm, n_tsm_st_list, n_fsm_st_list, tsm_flags, fsm_flags)
  task_poll_declaration(n_poll)
  signal_declaration(n_sig)
  hardware_api_declaration(n_hw_api)
  kconf = kconfiglib.Kconfig("Kconfig")
  if os.path.exists(".config"):
    kconf.load_config(".config")
  menuconfig.menuconfig(kconf)
  kconf.write_config(".config")
```

All 5 functions `corecfg_gen`, `palcfg_gen`, `app_cfg_gen`, `app_decl_gen`, `pal_arch_gen` **have been removed from `uedp.py`** — there is no more code-generation logic here at all. `uedp.py` now stops exactly at the step of writing out `.config`; code generation has been handed over entirely to `pltf/pycdscriptor/`. This is the most important change compared with KwDI: **separating "collecting configuration" from "generating code,"** which allows the code-generation step to be re-run multiple times from the same `.config` without repeating `menuconfig`.

Difference from versions prior to 1.1.6: `user_input()` now returns 4 lists instead of 4 single values — `fsm_flags`, `tsm_flags`, `n_tsm_st_list`, `n_fsm_st_list`, each element corresponding to one task norm in declaration order. `task_norm_declaration()` uses these 4 lists to generate `APPCFG_TSM_TASK_{i}`/`APPCFG_FSM_TASK_{i}` with an independent state count per task #i (see how the prompt/generation works in detail in `docs/uels-syntax.md`, section "Đồng bộ với `kconfigspec.usrinp` / `kconfigspec.tnorm`"), instead of the single shared flag + count used across all tasks in the original KwDI/PLTF design.

### 3.2 `pltf/kconfigspec/` — generating Kconfig declarations

The logic is nearly identical to the old `sources/common/pyspec/` from KwDI (back then the module was still called `pyspec`, and files still carried the `_pspec` suffix, e.g. `tsknrmdcl.py`). After moving into `pltf/`, the module went through 2 separate renames: first dropping the `_pspec` suffix for brevity (`usrinp_pspec.py` → `usrinp.py`, `tnorm_pspec.py` → `tnorm.py`, similarly for `tpoll`, `sig`, `hwapi`), then renaming the package itself from `pyspec` to `kconfigspec` to better reflect its role of "generating Kconfig declarations" rather than keeping a name that is only historically meaningful from KwDI. It still generates `sources/app/kconfig/decl.kconfig` using raw Kconfig syntax (`menu`, `config ... string`, `default`, `depends on`). The difference from KwDI is that these modules now live inside `pltf/kconfigspec/`, imported concisely by `uedp.py` via the `pltf.kconfigspec` package (see section 3.1) instead of `sources.common.pyspec.*` as in the original KwDI version.

### 3.3 `pltf/pycdscriptor/attribarse/dotcfg.py` — the heart of the code-generation pipeline

This is the direct replacement for the logic that used to walk `kconf.unique_defined_syms`, scattered across `corecfg_gen`/`palcfg_gen` inside the old `uedp.py`. `dotcfg.cfp_parse_dotcfg(config_path)` reads the `.config` file (in `CONFIG_KEY=value` text form) directly — `kconfiglib` is **no longer needed** at this step — and returns a structured `context` dict:

- `core_configs`, `pal_configs`: lists of `#define` strings for `CORE_*` / `PAL_*`.
- `tasknorm_defs`, `taskpoll_defs`, `sig_defs`: automatically assigned increasing hex IDs, starting at `0xE6` (task norm), `0xD4` (task poll), `0x01` (signal) — matching the `[HES] Heximal Encoding Signals` ranges already described in `arch-design.md` (`TASK_NORM` in the `0xEx` range, `TASK_POLL` in the `0xDx` range).
- `msgq_defs`, `normhler_lists`, `pollhler_lists`: lists of queue/handler names used to generate the task table.
- `appcfg_tsm_*`, `appcfg_fsm_*`, `tsmio_lists`, `fsmio_lists`: data specific to TSM/FSM (objects, state-transition tables, state lists).
- `arch_name`, `arch_apis`: the PAL architecture name and the list of Hardware APIs to generate.
- `task_tsm`, `task_fsm`: a task → state-list map, originally reserved only for μE-LS, now actually consumed by `pltf/pycdscriptor/attribarse/glbda.py` and the `lstaxer` pipeline (see section 3.5).

Since `pltf/pycdscriptor/jnerator/pregen/cfpcall.py` was introduced, each generator calling `dotcfg.cfp_parse_dotcfg()` independently has been eliminated: `cfpcall.main()` parses `.config` exactly once and returns a shared `context`, which the `fpregen.py` orchestrator then passes into all 7 generators (see section 3.4). This is a point that has already been fixed compared with the original PLTF design.

### 3.4 `pltf/templates/` + `pltf/pycdscriptor/jnerator/pregen/` — generating files with Jinja2

Unlike KwDI's "patch a string between 2 markers" mechanism, each generator in PLTF **renders the entire file content from a Jinja2 template and then fully overwrites** the target file. Since `cfpcall.py` (section 3.3), each generator receives a ready-made `context` as a parameter instead of parsing `.config` itself:

```python
# pltf/pycdscriptor/jnerator/pregen/corecfgpgen.py
def main(context):
  env = Environment(loader=FileSystemLoader('./pltf/templates'))
  template = env.get_template('corecfgh.txt')
  output = template.render(current_date=context["current_date"], core_configs=context['core_configs'])
  with open("sources/app/config/core_cfg.h", "w", encoding="utf-8") as f:
    f.write(output)
```

There are 7 generators corresponding to 7 output artifacts (all located inside `pltf/pycdscriptor/jnerator/pregen/`):

| Generator | File generated | Notes |
| --- | --- | --- |
| `corecfgpgen.py` | `sources/app/config/core_cfg.h` | Replaces the old `corecfg_gen()` |
| `palcfgpgen.py` | `sources/app/config/pal_cfg.h` | Replaces the old `palcfg_gen()` |
| `appcfgpgen.py` | `sources/app/config/app_cfg.h` | Replaces the old `app_cfg_gen()` |
| `appdeclpgen.py` | `sources/app/declaration/app_decl.h` | Replaces the old `app_decl_gen()` |
| `archdirpgen.py` | directory `sources/pal/arch/<arch_name>/` | Creates the directory before the 2 generators below write files into it |
| `archhpgen.py` | `sources/pal/arch/<arch_name>/<arch_name>_arch.h` | Replaces the old `pal_arch_gen()` (the `.h` part) |
| `archcpgen.py` | `sources/pal/arch/<arch_name>/<arch_name>_arch.c` | Replaces the old `pal_arch_gen()` (the `.c` part) |

`cfpcall.py` (parses `.config` once, returns `context`) and `fpregen.py` (orchestrator, referred to as "tsgen" in earlier task-list notes) both live inside `pltf/pycdscriptor/jnerator/pregen/`, and run all 7 generators in sequence with a shared `context`:

```python
# pltf/pycdscriptor/jnerator/pregen/fpregen.py
from . import appcfgpgen, corecfgpgen, palcfgpgen, appdeclpgen
from . import archdirpgen, archhpgen, archcpgen, cfpcall

if __name__ == "__main__":
  context = cfpcall.main()
  appcfgpgen.main(context); corecfgpgen.main(context); palcfgpgen.main(context)
  appdeclpgen.main(context); archdirpgen.main(context); archhpgen.main(context); archcpgen.main(context)
```

Because it uses whole-file template rendering instead of patching, PLTF **no longer depends on the target file already existing with fixed markers** — this is a direct improvement on the "string-patching code generation" limitation noted in section 2.3.

### 3.5 μE-LS / PLD: the `lstaxer` + `ustab` pipeline (now implemented)

Unlike PLTF's early stage (when `attribarse/glbda.py` and `test.yaml` were still an independent draft/PoC, not yet called by any orchestrator), as of 1.1.6 the μE-LS pipeline (Logical Syntax-izer, part of the PLD - Parse-able Logical Descriptor feature) has been fully implemented and integrated into `entrypoint.sh` (see section 4.2). The full YAML syntax is described in detail in `docs/uels-syntax.md`; this section only summarizes the modules directly relevant to the code-generation pipeline:

- `pltf/pycdscriptor/lstaxer/`: the parser + validator for the μE-LS YAML file (e.g. `sources/app/lstaxizer.yaml`) — including `symresolv.py` (builds a Symbol Resolution Map to resolve anchors/aliases), `nullremov.py` (cleans up redundant `NULL` declarations the user doesn't need to fill in), `strucjec.py` (normalizes the `actvobj`/`fsm`/`tsm` structure of `tlist`), `lukupmodel.py` (feeds post-validated data into `pydantic_model`), `vlid.py` (5 validation strategies), and `kre8.py` (assembles everything into the `context` for the code-generation step, via `build_generator_context()`).
- `pltf/pycdscriptor/attribarse/glbda.py`: the bridge between `dotcfg.py` (the `task_tsm`/`task_fsm` field, section 3.3) and the `glbda:` block in the μE-LS syntax.
- `pltf/pycdscriptor/jnerator/postgen/`: the code-generation stage **after** the logic has been validated (`cgen.py` calls `lstaxer.kre8.build_generator_context()` then `modalcvert.generate_appc()` to render `sources/app/app.c` directly from the YAML — this is the biggest difference from `jnerator/pregen/`, which only generates declarations/definitions rather than implementation logic).
- `pltf/pycdscriptor/ustab/`: the Unified Symbol Table — `gnnerate.py` builds a symbol table from `.config`/Kconfig, `xportstax.py` exports the entire config to YAML for cross-checking against μE-LS, and `custab.py` is the orchestrator calling both.

In other words, the pipeline now has 3 sequential stages: **pre-logicdef** (`jnerator/pregen`, generates Kconfig-based declarations) → **post-logicdef** (`jnerator/postgen`, generates implementation logic from validated YAML) → **ustab** (cross-checking, symbol-table export). `docs/uels-syntax.md` describes the YAML semantics in detail; this document only focuses on how the PLTF pipeline calls those modules.

## 4. New Docker & orchestration

### 4.1 `Dockerfile`

Compared with the original KwDI version (just `python:3.13-slim` + `kconfiglib`), PLTF's Dockerfile expands considerably to properly serve its role as a "Local Test Framework":

- Additional build toolchain installed: `git wget cmake binutils gcc make g++ gdb`.
- **ESP-IDF v5.1** installed into `/opt/esp-idf` (the `IDF_PATH` variable) — ready in advance to build/test on the ESP32 architecture directly inside the container, without needing to step outside as with KwDI.
- `jinja2`, `pytest`, `pyserial` installed alongside `kconfiglib` — serving the template-rendering pipeline and (in the future) running automated tests.
- 2 separate working directories pre-created: `/uedp-libs` (the μEDP core source, mounted from the repo) and `/uedp-test` (a dedicated PLTF workspace) — directly addressing the "no workspace separation" limitation noted for KwDI.
- `gosu` installed to drop privileges from `root` down to a regular user before entering the interactive shell.
- `ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]` instead of a `CMD` that calls `python uedp.py menuconfig` directly as in the original version.

### 4.2 `entrypoint.sh`

```bash
#!/bin/bash
set -e
USER_ID=${MY_UID:-1000}
GROUP_ID=${MY_GID:-1000}
# Create a "uedp_user" matching the host user's UID/GID
if ! id -u uedp_user >/dev/null 2>&1; then
  groupadd -g $GROUP_ID uedp_group 2>/dev/null || true
  useradd --shell /bin/bash -u $USER_ID -g $GROUP_ID -o -c "" -m uedp_user
fi
chown $USER_ID:$GROUP_ID /uedp-libs
chown $USER_ID:$GROUP_ID /uedp-test
export HOME=/home/uedp_user
echo "source $IDF_PATH/export.sh > /dev/null 2>&1" >> /home/uedp_user/.bashrc
# [ENTRY 1] Input collection + menuconfig (unchanged from KwDI)
python uedp.py menuconfig
# [ENTRY 2] Generate pre-logicdef (Kconfig-based) declarations from .config
python -m pltf.pycdscriptor.jnerator.pregen.fpregen
# [ENTRY 3] Generate implementation logic (app.c) from the post-logicdef YAML (μE-LS)
python -m pltf.pycdscriptor.jnerator.postgen.cgen \
  --yaml sources/app/lstaxizer.yaml \
  --output sources/app/app.c
# [ENTRY 4] Generate/cross-check the Unified Symbol Table
python -m pltf.pycdscriptor.ustab.custab
chown -R $USER_ID:$GROUP_ID /uedp-libs/*
exec gosu uedp_user bash
```

Four design points worth noting:

- **Handling UID/GID via the `MY_UID`/`MY_GID` environment variables** (default `1000`): directly solves KwDI's "files created end up owned by `root` on the host" problem, since the `.:/uedp-libs` volume is a two-way mount.
- **4 sequential stages in a single container run**: `menuconfig` (KwDI, unchanged) → `fpregen` (pre-logicdef, generates Kconfig-based declarations, section 3.4) → `cgen` (post-logicdef, generates `app.c` from validated μE-LS YAML, section 3.5) → `ustab.custab` (cross-checking, symbol-table export). From the user's perspective, the experience is still "one command, one run," but internally there are now 4 separate pipelines, each of which can also be invoked independently via `python -m pltf.pycdscriptor....`.
- Compared with the original PLTF version (only 2 ENTRY stages: menuconfig + a single code-generation script), splitting out ENTRY 3/4 reflects the fact that the μE-LS pipeline (section 3.5) is now genuinely wired into orchestration, no longer an independent debug script.
- **`exec gosu uedp_user bash`** at the end: after code generation finishes, the container does not exit immediately but drops into a shell as a regular user, allowing further work (`cd /uedp-test` to develop PLTF further, or `exit` to simply take the code that was just generated).

### 4.3 `docker-compose.yaml`

```yaml
services:
  uedp_udc:
    image: uedp-p:latest
    container_name: uedp_udc
    stdin_open: true
    tty: true
    volumes:
      - .:/uedp-libs
    working_dir: /uedp-libs
    hostname: container_env
```

Instead of typing `docker run` by hand with all its parameters (which did not exist for KwDI), `docker-compose.yaml` fixes all the parameters needed for an interactive working session (`stdin_open`/`tty` so `menuconfig` works properly), shrinking the startup command down to `docker compose run uedp_udc` (or `up`), keeping it consistent across different development machines — in keeping with PLTF's "Portable" spirit.

### 4.4 `.dockerignore`

```text
.git
__pycache__
*.pyc
sources/test/
.vscode/
*.o
*.a
docs/
```

Unchanged from KwDI — `docs/` (which holds large reference PDFs and instructional videos under `docs/references/`, `docs/videos/`) is still excluded from the build context to reduce `docker build` time.

## 5. Comparison: KwDI vs PLTF

| Aspect | KwDI (original) | PLTF (current) |
| --- | --- | --- |
| Location of in-house code | `sources/common/{kconfiglib,pyspec}` | `pltf/{kconfigspec,templates,pycdscriptor}` (`kconfiglib` — a 3rd-party library — remains in `sources/common/kconfiglib`) |
| Number of stages | 1 (merged together in `uedp.py`) | 4 (declaration+menuconfig in `uedp.py` → `jnerator/pregen` generates declarations → `jnerator/postgen` generates logic from μE-LS → `ustab` cross-checks symbols) |
| Code-generation mechanism | Patching a string between 2 markers into an existing `.h` file | Rendering an entirely new file via a Jinja2 template (`pregen`) or from validated YAML (`postgen`) |
| Input to the code-generation step | The `kconf` object directly (`kconfiglib`) | The `.config` file, parsed once via `cfpcall.py` (`pregen`), and the μE-LS YAML file after it passes `lstaxer.vlid` (`postgen`) |
| Docker image | `python:3.13-slim` + `kconfiglib` | + `gcc/cmake/gdb`, + ESP-IDF v5.1, + `jinja2/pytest/pyserial`, + `gosu` |
| Container startup | `CMD` calling `uedp.py menuconfig` directly | `entrypoint.sh` (creates a user, handles UID/GID, runs 4 ENTRY stages in sequence, then drops into a shell) |
| Orchestration | None (manual `docker run`) | `docker-compose.yaml` (`uedp_udc` service) |
| Workspace | Everything mixed into one directory | Separated into `/uedp-libs` (core lib) and `/uedp-test` (PLTF workspace) |
| Advanced test spec (μE-LS/PLD) | None | Fully implemented: `lstaxer/` (parse + validate) + `attribarse/glbda.py` (bridge to `dotcfg`) + `jnerator/postgen` (generates `app.c`) + `ustab/` (symbol cross-checking) — see `docs/uels-syntax.md` and section 3.5 |

## 6. Remaining work / risks to note going forward

- **A leftover `sys.path` entry from before the move into `pltf/`**: `uedp.py` still inserts `sources/common/kconfigspec` into `sys.path` even though that directory no longer exists (the real module has fully moved to `pltf/kconfigspec/` and is imported via the `pltf.kconfigspec` package) — this insertion is now dead code, harmless but confusing to read. This cleanup is listed as a separate item in `docs/to-do.md` ("chore filename to unify PLTF's separate modules"), not yet done as of this writing.
- **The position of `ustab.custab` in `entrypoint.sh` needs review**: it currently runs at the very last ENTRY stage, after `cgen` has already generated `app.c` — worth confirming whether this ordering matches the intended design (`docs/to-do.md` still has a separate, unresolved item: "adjust the position of ustab.custab in the entrypoint.sh pipeline").
- **`entrypoint.sh` always runs `uedp.py menuconfig` on every container startup**: fine for an interactive session on a dev machine, but there's no non-interactive branch yet (e.g., reading an existing `.config` directly and skipping menuconfig) for use in CI/CD.
- **No automated tests for `pltf/` itself yet**: the testing framework itself (parser, generator, template, and now also the `lstaxer`/`ustab` pipeline) currently has no dedicated tests to guard against regressions when editing a template or a parser.
- **`lstaxer.nullremov` is flagged as possibly redundant**: `docs/to-do.md` records a consideration to drop this module from the shared pipeline, since it may add parsing complexity without a matching benefit - the final decision should be tracked and section 3.5 updated if the module is removed.
- **No BST (Basic Software Test) on real hardware yet for the PLD/μE-LS pipeline**: work so far has stopped at code generation and design review; there is not yet a real test cycle confirming that `app.c` generated from μE-LS actually runs correctly on hardware.

## 7. Conclusion

PLTF does not replace KwDI's Kconfig or Docker usage — instead, it **decouples the layers**, separating code generation from configuration collection, while also containerizing the environment more fully (build toolchain + ESP-IDF + user-permission handling) so the container can be used not just to run `menuconfig` once, but as a full development and testing environment for μEDP throughout. Unlike the early stage (when the μE-LS/PLD extension direction was still an independent `glbda.py`/`test.yaml` draft), as of 1.1.6 this pipeline has been fully implemented and genuinely integrated into `entrypoint.sh` through 3 stages, `pregen` → `postgen` → `ustab` (sections 3.4, 3.5, 4.2). The largest remaining piece of work is no longer "integrating μE-LS" — it has shifted to technical cleanup (the leftover `sys.path` entry, the position of `ustab.custab` in the pipeline), adding automated tests for `pltf/`, and running BST on real hardware — as listed in section 6.
