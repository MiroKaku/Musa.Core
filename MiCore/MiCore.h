#pragma once
#ifndef _MICORE_
#define _MICORE_

#if !defined(_AMD64_) && !defined(_X86_) && !defined(_ARM64_)
#error Unsupported architecture
#endif

//
// Configs
//

//   - User   Mode
//#define MICORE_SECURE_CORE 1

//   - Kernel Mode
//#define MICORE_FLS_USE_THREAD_NOTIFY_CALLBACK 1


//
// Public
//

#define MICORE_API __stdcall


EXTERN_C_START


#ifdef _KERNEL_MODE
NTSTATUS MICORE_API MiCoreStartup(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);
#else
NTSTATUS MICORE_API MiCoreStartup();
#endif

NTSTATUS MICORE_API MiCoreShutdown();

PVOID    MICORE_API MiCoreGetSystemRoutine(
    _In_z_ const char* Name
);

PVOID    MICORE_API MiCoreGetSystemRoutineByNameHash(
    _In_ size_t NameHash
);


EXTERN_C_END


#endif
