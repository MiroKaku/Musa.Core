# AGENTS.md — Musa.Core

## OVERVIEW

Win32 API reimplementation (Kernel32, Advapi32) on top of ntoskrnl.
Kernel-mode driver only.
Author: MiroKaku/MeeSong. Beta. MIT license.

## STRUCTURE

```
Musa.Core/                  Core source — headers, init, PE parser, thunks
  Thunks/                   Win32 API shim implementations (31 .cpp files)
  Thunks/Internal/          Private headers for thunk internals (17 .h/.cpp files)
Musa.Core.StaticLibraryForDriver/  Kernel-mode static library (universal.h + vcxproj)
Musa.Core.TestForDriver/    Kernel-mode test (KMDF driver, DriverEntry, boot-start reinit)
Musa.Core.NuGet/            NuGet packaging (.nuspec, .props, .targets)
Publish/                    Staged output consumed by dependents + NuGet
  Include/                  Published headers (copy of Musa.Core.h)
  Library/{Config}/{Platform}/  Built .lib files
  Config/                   .props/.targets injected into consuming projects
Output/                     Build artifacts (Binaries/, Objects/, .binlog)
```

## KEY DEPENDENCIES

- `Mile.Project.Windows` MSBuild SDK via `Mile.Project.Configurations` v1.0.1917
- `Musa.CoreLite` NuGet v1.1.1 — lightweight core subset (exposes `MusaCoreLiteGetNtdllBase()` function)
- `Musa.Veil` — header-only NT internals (Veil.h), git submodule or NuGet

## BUILD

- `BuildAllTargets.cmd` → sources VS env → `MSBuild BuildAllTargets.proj`
- Builds 6 configurations: {Debug,Release} x {x86,x64,ARM64}
- CI: GitHub Actions `windows-latest`, triggers on push to `main` + `v*` tags
- NuGet publish: tag push `v*` → pack → `dotnet nuget push` to nuget.org
- Kernel driver: MUST link with `/INTEGRITYCHECK`
- Solution: `.slnx` format (VS2022 17.10+ XML solution, not traditional `.sln`)
- WDK: CI auto-installs matching WDK via `winget` if kernel headers missing
- PCH: `universal.h` forced-include via empty `Musa.Core.Nothing.cpp`

## CONVENTIONS

### Naming
- Public symbols: `MUSA_NAME(name)` → `_Musa_name`
- Private symbols: `MUSA_NAME_PRIVATE(name)` → `_Musa_Private_name`
- IAT exports: `MUSA_IAT_SYMBOL(name, stack)` — x86 uses decorated `_name@stack`
- Thunk files: `{DLL}.{Category}.cpp` (e.g., `KernelBase.Process.cpp`)
- Pool tags: `'-iM-'` (kernel-mode)

### Code Style (.editorconfig enforced)
- C/C++: UTF-8 BOM, CRLF, 4-space indent, doxygen `/** */` comments
- `.rc`/`.inf`: UTF-16LE
- `.bat`: UTF-16LE
- Calling convention: `__stdcall` (`MUSA_API` macro)

### Section Placement (kernel-mode)
- `#pragma alloc_text(PAGE, ...)` for startup/init and pageable functions (all startup functions moved from INIT to PAGE to support boot-start driver reinitialization)
- Every thunk file uses this for kernel builds

### Debug Logging
- `MusaLOG(fmt, ...)` → `DbgPrintEx` (DEBUG builds), no-op in Release

## WHERE TO LOOK

| Task | Location |
|---|---|
| Add new Win32 API shim | `Musa.Core/Thunks/` — match existing `{DLL}.{Category}.cpp` pattern |
| Modify init/startup | `Musa.Core/Musa.Core.cpp` (MusaCoreStartup/MusaCoreShutdown) |
| Change exported symbols | `Musa.Core/Musa.Core.def` (~490 ZwRoutine exports) |
| PE parsing utilities | `Musa.Core/Musa.Utilities.PEParser.h` |
| System environment block | `Musa.Core/Musa.Core.SystemEnvironmentBlock.{h,cpp}` |
| Build configuration | `Directory.Build.props`, `Directory.Packages.Cpp.props` |
| NuGet package layout | `Musa.Core.NuGet/Musa.Core.nuspec` |
| Kernel-mode macros | `Musa.Core.StaticLibraryForDriver/universal.h` |

## ANTI-PATTERNS

- Do NOT use `as`-style casts — project uses C-style casts and `reinterpret_cast`
- Do NOT add new .cpp files without updating the corresponding `.vcxproj`
- Do NOT forget `MUSA_IAT_SYMBOL` after implementing a thunk — the API won't be exported
- Do NOT omit `#pragma alloc_text` in kernel-mode thunks — causes non-paged pool bloat
- Do NOT assume heap APIs work identically in kernel — kernel uses `RtlAllocateHeap` with special pool setup
- Do NOT test kernel-mode code without `/INTEGRITYCHECK` linker flag

## KNOWN ISSUES

- `GetStartupInfoW` kernel-mode: incomplete, TODO for PEB access
- `GetProcessInformation(ProcessAppMemoryInfo)`: returns `STATUS_NOT_IMPLEMENTED`
- `RtlFindAndFormatMessage`: now uses `MusaCoreLiteGetNtdllBase()` with null check; returns `STATUS_DLL_NOT_FOUND` if ntdll base unavailable
- `ThreadNotifyCallback`: added null guard for `MusaCoreThreadNotifyCallbackObject` before `ExNotifyCallback`
- Several `#pragma warning(suppress: ...)` for ReSharper false positives
- `RtlWalkHeap`: stub returning `STATUS_NOT_IMPLEMENTED`
- `LocalAlloc`/`LocalReAlloc`: `LMEM_MOVEABLE` flag not supported
- `Ntdll.LibraryLoader.ProcAddr.cpp`: callback field assignment is TODO placeholder
- `Ntdll.LibraryLoader.cpp`: module unload logic incomplete (TODO: if LdrEntry->LoadCount == 1 then Unload)

## CHILD AGENTS

- `Musa.Core/AGENTS.md` — core source, headers, init logic
- `Musa.Core/Thunks/AGENTS.md` — thunk implementation patterns and conventions
