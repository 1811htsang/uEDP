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
- `sources/common/kconfigspec/`: functions that generate `decl.kconfig` (task norm, task poll, signal, hardware API) based on the quantities the user enters (`usrinp.py`, `tsknrmdcl.py`, `tskpoldcl.py`, `sigdcl.py`, `hwapidcl.py`).
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
- **`sources/common/pycdscriptor/`** (an earlier Jinja2-based prototype) already existed but was only a **draft never wired into the real flow**: the original `appcfgpgen.py` just `print(output)`ed to the screen with a hardcoded `current_date` of `'16 May 2025'` — it didn't read the real `.config` and didn't write a file.
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
├── kconfigspec/                 # Generates decl.kconfig (replaces the old sources/common/kconfigspec)
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
│   └── archc.txt
└── pycdscriptor/
    ├── attribarse/            # Reads .config (and, in future, YAML) into a structured context
    │   ├── dotcfg.py
    │   ├── glbda.py       # Draft for the μE-LS direction (see section 3.5)
    │   └── test.yaml
    └── generators/           # Each file is responsible for one output artifact
        ├── appcfgpgen.py
        ├── corecfgpgen.py
        ├── palcfgpgen.py
        ├── appdeclpgen.py
        ├── archdirpgen.py
        ├── archhpgen.py
        ├── archcpgen.py
        └── fpregen.py          # Orchestrator, calls all 7 generators above in sequence
```

Compared with `sources/common/kconfiglib/` (kept unchanged, not moved, since it is a third-party library rather than in-house code), the entirety of KwDI's **in-house** portion (`kconfigspec`, `pycdscriptor`) is consolidated into a single location, `pltf/`, separate from `sources/common/` — reflecting the true meaning of "Portable": `pltf/` does not depend on the `sources/` structure and could be reused for a different μEDP project simply by pointing it at the correct output path.

### 3.1 `uedp.py` after the refactor

`uedp.py` now has exactly one responsibility: generate `decl.kconfig` and run the interactive `menuconfig`.

```python
from pltf.kconfigspec.usrinp import user_input
from pltf.kconfigspec.tnorm import task_norm_declaration
from pltf.kconfigspec.tpoll import task_poll_declaration
from pltf.kconfigspec.sig import signal_declaration
from pltf.kconfigspec.hwapi import hardware_api_declaration

def main():
  os.environ["KCONFIG_CONFIG"] = ".config"
  os.environ["MENUCONFIG_STYLE"] = "aquatic"
  (n_norm, n_poll, n_sig, use_fsm, use_tsm, n_tsm_st, n_fsm_st, n_hw_api) = user_input(DEFAULT_VALS)
  open("sources/app/kconfig/decl.kconfig", "w").close()
  task_norm_declaration(n_norm, n_tsm_st, n_fsm_st, use_tsm, use_fsm)
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

### 3.2 `pltf/kconfigspec/` — generating Kconfig declarations

The logic is nearly identical to the old `sources/common/kconfigspec/` (files renamed with a `_pspec` suffix for consistency, e.g. `tsknrmdcl.py` → `tnorm.py`); it still generates `sources/app/kconfig/decl.kconfig` using raw Kconfig syntax (`menu`, `config ... string`, `default`, `depends on`). The difference is that these modules now live inside `pltf/`, imported by `uedp.py` via `pltf.kconfigspec.*` instead of `sources.common.kconfigspec.*`.

### 3.3 `pltf/pycdscriptor/attribarse/dotcfg.py` — the heart of the code-generation pipeline

This is the direct replacement for the logic that used to walk `kconf.unique_defined_syms`, scattered across `corecfg_gen`/`palcfg_gen` inside the old `uedp.py`. `dotcfg.cfp_parse_dotcfg(config_path)` reads the `.config` file (in `CONFIG_KEY=value` text form) directly — `kconfiglib` is **no longer needed** at this step — and returns a structured `context` dict:

- `core_configs`, `pal_configs`: lists of `#define` strings for `CORE_*` / `PAL_*`.
- `tasknorm_defs`, `taskpoll_defs`, `sig_defs`: automatically assigned increasing hex IDs, starting at `0xE6` (task norm), `0xD4` (task poll), `0x01` (signal) — matching the `[HES] Heximal Encoding Signals` ranges already described in `arch-design.md` (`TASK_NORM` in the `0xEx` range, `TASK_POLL` in the `0xDx` range).
- `msgq_defs`, `normhler_lists`, `pollhler_lists`: lists of queue/handler names used to generate the task table.
- `appcfg_tsm_*`, `appcfg_fsm_*`, `tsmio_lists`, `fsmio_lists`: data specific to TSM/FSM (objects, state-transition tables, state lists).
- `arch_name`, `arch_apis`: the PAL architecture name and the list of Hardware APIs to generate.
- `task_tsm`, `task_fsm`: a task → state-list map, reserved for a future μE-LS integration (see section 3.5).

Each generator (`*_tsgen.py`) calls `dotcfg.cfp_parse_dotcfg()` independently — this is a point of duplication worth noting; see section 5.

### 3.4 `pltf/templates/` + `pltf/pycdscriptor/generators/` — generating files with Jinja2

Unlike KwDI's "patch a string between 2 markers" mechanism, each generator in PLTF **renders the entire file content from a Jinja2 template and then fully overwrites** the target file:

```python
# pltf/pycdscriptor/generators/corecfgpgen.py
context = dotcfg.cfp_parse_dotcfg(config_dir)
env = Environment(loader=FileSystemLoader('./pltf/templates'))
template = env.get_template('corecfgh.txt')
output = template.render(current_date=context["current_date"], core_configs=context['core_configs'])
with open("sources/app/config/core_cfg.h", "w", encoding="utf-8") as f:
  f.write(output)
```

There are 7 generators corresponding to 7 output artifacts:

| Generator | File generated | Notes |
| --- | --- | --- |
| `corecfgpgen.py` | `sources/app/config/core_cfg.h` | Replaces the old `corecfg_gen()` |
| `palcfgpgen.py` | `sources/app/config/pal_cfg.h` | Replaces the old `palcfg_gen()` |
| `appcfgpgen.py` | `sources/app/config/app_cfg.h` | Replaces the old `app_cfg_gen()` |
| `appdeclpgen.py` | `sources/app/declaration/app_decl.h` | Replaces the old `app_decl_gen()` |
| `archdirpgen.py` | directory `sources/pal/arch/<arch_name>/` | Creates the directory before the 2 generators below write files into it |
| `archhpgen.py` | `sources/pal/arch/<arch_name>/<arch_name>_arch.h` | Replaces the old `pal_arch_gen()` (the `.h` part) |
| `archcpgen.py` | `sources/pal/arch/<arch_name>/<arch_name>_arch.c` | Replaces the old `pal_arch_gen()` (the `.c` part) |

`tsgen.py` is the orchestrator, running all 7 generators in sequence and printing progress logs:

```python
import appcfgpgen, corecfgpgen, palcfgpgen, appdeclpgen
import archdirpgen, archhpgen, archcpgen

if __name__ == "__main__":
  appcfgpgen.main(); corecfgpgen.main(); palcfgpgen.main()
  appdeclpgen.main(); archdirpgen.main(); archhpgen.main(); archcpgen.main()
```

Because it uses whole-file template rendering instead of patching, PLTF **no longer depends on the target file already existing with fixed markers** — this is a direct improvement on the "string-patching code generation" limitation noted in section 2.3.

### 3.5 Extension direction: μE-LS / YAML test spec (`attribarse/glbda.py`, `test.yaml`)

`pltf/pycdscriptor/attribarse/glbda.py` and `test.yaml` are a **draft/PoC**, currently **not yet called by `tsgen.py`** — they only run standalone for parser debugging. This is a preparatory infrastructure step toward μE-LS (Logical Syntax-izer), described in detail in `docs/uels-syntax.md`: a YAML-based declaration syntax for Task/TSM/FSM/Signal/Action, part of the PLD (Parse-able Logical Descriptor) feature set, planned as the foundation for PLTF and TLC (Test Level Coverager) in this same version 1.2.0. `test.yaml` already illustrates a 3-task scenario (`KID_TASK_USR` using TSM, `KID_TASK_A` using TSM, `KID_TASK_B` using FSM) with `post_msg`/`log` actions — exactly the data model that `dotcfg.py` has already prepared the `task_tsm`/`task_fsm` fields to receive.

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
chown $USER_ID:$GROUP_ID /uedp-libs /uedp-test
export HOME=/home/uedp_user
echo "source $IDF_PATH/export.sh > /dev/null 2>&1" >> /home/uedp_user/.bashrc
# [ENTRY 1] KwDI stage — input collection + menuconfig
python uedp.py menuconfig
# [ENTRY 2] PLTF stage — generating code from .config
python pltf/pycdscriptor/generators/tsgen.py
exec gosu uedp_user bash
```

Three design points worth noting:

- **Handling UID/GID via the `MY_UID`/`MY_GID` environment variables** (default `1000`): directly solves KwDI's "files created end up owned by `root` on the host" problem, since the `.:/uedp-libs` volume is a two-way mount.
- **Combining both the KwDI and PLTF stages into a single container run**: `entrypoint.sh` calls `uedp.py menuconfig` (the KwDI stage, unchanged) and then immediately calls `pltf/pycdscriptor/generators/tsgen.py` (the new PLTF stage) — from the user's perspective, the experience is still "one command, one run," but internally these are now 2 separate pipelines that can also be invoked independently.
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
| Location of in-house code | `sources/common/{kconfiglib,kconfigspec}` | `pltf/{kconfigspec,templates,pycdscriptor}` (`kconfiglib` — a 3rd-party library — remains in `sources/common/kconfiglib`) |
| Number of stages | 1 (merged together in `uedp.py`) | 2 (declaration+menuconfig in `uedp.py`, code generation in `pltf/pycdscriptor`) |
| Code-generation mechanism | Patching a string between 2 markers into an existing `.h` file | Rendering an entirely new file via a Jinja2 template |
| Input to the code-generation step | The `kconf` object directly (`kconfiglib`) | The `.config` file already written to disk (`dotcfg.py` re-parses it) |
| Docker image | `python:3.13-slim` + `kconfiglib` | + `gcc/cmake/gdb`, + ESP-IDF v5.1, + `jinja2/pytest/pyserial`, + `gosu` |
| Container startup | `CMD` calling `uedp.py menuconfig` directly | `entrypoint.sh` (creates a user, handles UID/GID, runs the KwDI-stage → PLTF-stage in sequence, then drops into a shell) |
| Orchestration | None (manual `docker run`) | `docker-compose.yaml` (`uedp_udc` service) |
| Workspace | Everything mixed into one directory | Separated into `/uedp-libs` (core lib) and `/uedp-test` (PLTF workspace) |
| Advanced test spec | None | `attribarse/glbda.py` + `test.yaml` (draft, heading toward μE-LS/PLD — see `docs/uels-syntax.md`) |

## 6. Remaining work / risks to note going forward

- **`glbda.py` is not yet wired into `tsgen.py`**: currently it's only an independent debug script (`python pltf/pycdscriptor/attribarse/glbda.py`) — no generator yet consumes data from `test.yaml`. This is the main remaining piece of work to complete the μE-LS direction.
- **Duplicated `.config` parsing code**: all 6 generators (`appcfg`, `corecfg`, `palcfg`, `appdecl`, `arch_h`, `arch_c`) each call `dotcfg.cfp_parse_dotcfg(config_dir)` separately instead of parsing once and sharing a common `context` — this could be consolidated inside `tsgen.py` to avoid reading the `.config` file multiple times.
- **`entrypoint.sh` always runs `uedp.py menuconfig` on every container startup**: fine for an interactive session on a dev machine, but there's no non-interactive branch yet (e.g., reading an existing `.config` directly and skipping menuconfig) for use in CI/CD.
- **No automated tests for `pltf/` itself yet**: the testing framework itself (parser, generator, template) currently has no dedicated tests to guard against regressions when editing a template or a parser.
- **`archdirpgen.py`** uses Python 3.12+-style nested double-quote f-strings (`f"{context["arch_name"]}"`) — worth confirming compatibility, since it matches the `python:3.13-slim` base image currently used, but is worth watching if the base image is ever downgraded to an older Python version.

## 7. Conclusion

PLTF does not replace KwDI's Kconfig or Docker usage — instead, it **decouples the layers**, separating code generation from configuration collection, while also containerizing the environment more fully (build toolchain + ESP-IDF + user-permission handling) so the container can be used not just to run `menuconfig` once, but as a full development and testing environment for μEDP throughout. The biggest remaining piece before PLTF can be considered complete, as described in `docs/to-do.md`, is integrating `glbda.py`/`test.yaml` (the μE-LS direction) into the `tsgen.py` pipeline, which is currently still an independent draft.
