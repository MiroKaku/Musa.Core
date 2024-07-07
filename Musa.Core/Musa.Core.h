#pragma once
#ifndef _MUSA_CORE_
#define _MUSA_CORE_

#if !defined(_AMD64_) && !defined(_X86_) && !defined(_ARM64_)
#error Unsupported architecture
#endif

//
// Configs
//

//   - User   Mode
#define MUSA_CORE_SECURE_CORE 0

//   - Kernel Mode
#define MUSA_CORE_USE_THREAD_NOTIFY_CALLBACK 1


//
// Public
//

#define MUSA_API __stdcall


EXTERN_C_START


#ifdef _KERNEL_MODE
NTSTATUS MUSA_API MusaCoreStartup(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);
#else
NTSTATUS MUSA_API MusaCoreStartup();
#endif

NTSTATUS MUSA_API MusaCoreShutdown();

PVOID    MUSA_API MusaCoreGetSystemRoutine(
    _In_z_ const char* Name
);

PVOID    MUSA_API MusaCoreGetSystemRoutineByNameHash(
    _In_ size_t NameHash
);


EXTERN_C_END


#endif
