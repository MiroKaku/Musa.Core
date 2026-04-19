#pragma once
#ifndef _MUSA_CORE_
#define _MUSA_CORE_

#if !defined(_AMD64_) && !defined(_X86_) && !defined(_ARM64_)
#error Unsupported architecture
#endif

//
// Public
//

#define MUSA_API __stdcall

//
// MusaCore
//

EXTERN_C_START

NTSTATUS MUSA_API MusaCoreStartup(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath,
    _In_ BOOL TLSWithThreadNotifyCallback
);

NTSTATUS MUSA_API MusaCoreShutdown();

EXTERN_C_END

#endif // _MUSA_CORE_
