#pragma once

#if !defined(_AMD64_) && !defined(_X86_) && !defined(_ARM64_)
#error Unsupported architecture
#endif

#define MICORE_API __stdcall

EXTERN_C_START

NTSTATUS MICORE_API MiCoreStartup (
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);

NTSTATUS MICORE_API MiCoreShutdown();

PVOID    MICORE_API MiGetZwRoutineAddress(
    _In_ size_t NameHash
);

EXTERN_C_END
