# AGENTS.md — Musa.Core/Thunks

> Parent: `../AGENTS.md` — core headers, init, PE parser. Root: `../../AGENTS.md` — naming, build.

## ROLE

Win32 API shim implementations. Reimplements Win32 APIs via ntoskrnl.

## FILE NAMING

`{DLL}.{Category}.cpp` — DLL source × functional group:
- `KernelBase.Process.cpp` — process creation, info, startup
- `KernelBase.Thread.cpp` — thread creation, context, naming
- `KernelBase.Heap.cpp` — HeapCreate/Alloc/Free wrappers
- `Ntdll.Heap.cpp` — RtlCreateHeap/RtlAllocateHeap (lowest level)
- `KernelBase.LibraryLoader.cpp` — LoadLibrary, GetModuleHandle, GetProcAddress
- `KernelBase.Synchronize.cpp` — critical sections, SRW locks, condition vars
- `KernelBase.FileVersion.cpp` — version resource APIs
- `KernelBase.NLS.cpp` — locale/codepage/Unicode conversion

Private shared header: `KernelBase.Private.h` (in this directory, not `Internal/`).

## THUNK IMPLEMENTATION PATTERN

Every thunk follows this exact structure:

```cpp
#include "KernelBase.Private.h"  // or Ntdll equivalent

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(FunctionName))
#endif

DWORD MUSA_API MUSA_NAME(FunctionName)(params...)
{
    // kernel implementation using Nt*/Rtl* APIs
}

MUSA_IAT_SYMBOL(FunctionName, stack_size)
```

### Critical steps (miss any → broken):
1. `MUSA_NAME(Fn)` wraps the function name → `_Musa_Fn`
2. `MUSA_API` = `__stdcall` calling convention
3. `#pragma alloc_text` — kernel builds REQUIRE section placement
4. `MUSA_IAT_SYMBOL(Fn, N)` at bottom — registers for IAT hooking. N = stack bytes (x86 only, 0 for x64/ARM64)

## Internal/ SUBDIRECTORY

17 private headers + 1 .cpp (`{DLL}.{Category}.h`) providing shared helpers for thunks.
Examples: `KernelBase.Fiber.h`, `KernelBase.Locale.h`, `KernelBase.Heap.h`, `Ntdll.Image.h`.
NOT public — never include from outside `Thunks/`.

### Ntdll.Image.h notable macros
- `LDR_RESOURCE_ID_NAME_MASK` — mask clearing lower 16 bits (identifies name-type resource IDs)
- `LDR_RESOURCE_ID_NAME_MINVAL` — minimum value for name-type resource IDs (17th bit set)
- `RtlFindResource` uses `ULONG_PTR[3]` array (Type, Name, Language) instead of `LDR_RESOURCE_INFO` struct, with `_countof(IdPath)` as the level parameter to `LdrFindResource_U`

### KernelBase.NLS.Table.cpp
Locale/NLS table lookup implementation — shared by `KernelBase.NLS.cpp` thunks.

## ADDING A NEW THUNK

1. Find or create `{DLL}.{Category}.cpp` matching the API's origin DLL
2. Follow the THUNK IMPLEMENTATION PATTERN above exactly
3. Update `.vcxproj` if a new .cpp file was created
4. Test in `Musa.Core.TestForDriver` (kernel-mode)

## COMMON PATTERNS

### Status conversion
`BaseSetLastNTError(Status)` converts NTSTATUS → Win32 last-error. Return `NT_SUCCESS(Status)`.

### Heap
`RtlCreateHeap`/`RtlAllocateHeap` with custom pool. Pool tag: `'-iM-'`.

### Module enumeration
`GetModuleHandle`/`GetProcAddress`: walk PEB loader data manually.

### Thunk file splitting
Large categories split into sub-files: `KernelBase.Synchronize.cpp` includes `Synchronize.Event.cpp`, `Synchronize.Mutex.cpp`, `Synchronize.Semaphore.cpp`. Similarly `KernelBase.Thread.cpp` splits into `Thread.Create.cpp`, `Thread.ProcAttr.cpp`.

### #pragma warning suppressions
7 instances across 6 thunk files — all for ReSharper/SAL false positives (6387, 28023, 6385, 6001, 4996, 28751). Not real bugs.

## KNOWN INCOMPLETE

See root AGENTS.md KNOWN ISSUES.
