# AGENTS.md — Musa.Core (core source)

> Parent: `../AGENTS.md` covers project overview, build, naming, dual-mode pattern.

## ROLE

Central headers, initialization, PE utilities, system environment block.
Everything here is shared infrastructure consumed by `Thunks/`.

## KEY FILES

| File | Purpose |
|---|---|
| `Musa.Core.h` | Master header — arch detection, includes Veil.h, declares `MusaCoreStartup`/`MusaCoreShutdown` |
| `Musa.Core.cpp` | Init sequence: `MusaCoreStartup` → `MusaCoreLiteStartup` → `EnvironmentBlockSetup` → thunk logging |
| `Musa.Core.def` | Module definition — ~490 `_Mi_Zw*` exports mapping ntdll ZwRoutines |
| `Musa.Core.SystemEnvironmentBlock.h/.cpp` | Kernel-mode PEB/TEB/SEB setup via `EnvironmentBlockSetup`/`EnvironmentBlockTeardown`; `ThreadNotifyCallback` null-guards `MusaCoreThreadNotifyCallbackObject` before `ExNotifyCallback` |
| `Musa.Utilities.h` | Security cookie init (`__security_init_cookie`), stack alignment helpers |
| `Musa.Utilities.PEParser.h` | PE image parsing: section/export/import/delay-import enumeration |
| `Musa.Utilities.Overlay.cpp` | IAT overlay — `MakeImageOverlay` patches import table at init |
| `Musa.Core.Nothing.cpp` | Empty — exists to satisfy build system |

## INIT SEQUENCE

1. `MusaCoreStartup()` — entry point for both modes
2. Calls `MusaCoreLiteStartup()` (from Musa.CoreLite v1.1.0 dependency)
3. Calls `EnvironmentBlockSetup()` — kernel-mode only: creates fake PEB/TEB
4. Logs result via `MusaLOG`
5. Shutdown reverses: `EnvironmentBlockTeardown()` → `MusaCoreLiteShutdown()`

**Note:** Musa.CoreLite v1.1.0 exposes `MusaCoreLiteGetNtdllBase()` function instead of the previous `MusaCoreLiteNtdllBase` global variable. All call sites must use the function form.

## SYSTEM ENVIRONMENT BLOCK (kernel-mode)

- Allocates `RTL_SYSTEM_ENVIRONMENT_BLOCK` at IRQL <= APC_LEVEL
- Creates process/thread environment blocks so Win32-style APIs work in kernel
- Pool tag: `'-iM-'`
- Teardown frees all allocated blocks

## PE PARSER

Header-only in `Musa.Utilities.PEParser.h`. Template-based enumeration:
- `EnumerateImageSection(base, callback)` — iterate PE sections
- `EnumerateImageExport(base, callback)` — iterate export directory
- `EnumerateImageImport(base, callback)` — iterate import descriptors
- `EnumerateImageDelayImport(base, callback)` — iterate delay-load imports
- `GetImageEntryPoint(base)` — resolve entry point RVA

## IAT OVERLAY

`MakeImageOverlay` in `Musa.Utilities.Overlay.cpp`:
- Walks all loaded modules' import tables
- Redirects imports matching `Musa.Core.def` exports to internal implementations
- Called during init to hook Win32 APIs transparently

## EDITING GUIDELINES

- `Musa.Core.def`: one `_Mi_Zw*` symbol per line, maintain alphabetical order within groups
- `Musa.Core.cpp`: init functions use `#pragma alloc_text(INIT, ...)` in kernel mode
- `SystemEnvironmentBlock`: all allocations MUST use pool tag `'-iM-'`, free on teardown
- PE parser: header-only, no .cpp — keep it that way
