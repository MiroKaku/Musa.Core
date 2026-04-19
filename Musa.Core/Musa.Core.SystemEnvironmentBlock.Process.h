#pragma once

namespace Musa::Core
{
    // Kernel Process Environment Block
    VEIL_DECLARE_STRUCT_ALIGN(KPEB, 8)
    {
        ERESOURCE       Lock;
        EX_RUNDOWN_REF  RundownProtect;

        PDRIVER_OBJECT  DriverObject;
        UNICODE_STRING  RegistryPath;

        SIZE_T          SizeOfImage;
        PVOID           ImageBaseAddress;
        UNICODE_STRING  ImagePathName;
        UNICODE_STRING  ImageBaseName;

        ULONG           NumberOfHeaps;
        ULONG           MaximumNumberOfHeaps;
        PVOID           DefaultHeap;
        PVOID*          ProcessHeaps;

        ULONG           HardErrorMode;

        HANDLE          StandardInput;
        HANDLE          StandardOutput;
        HANDLE          StandardError;

        HANDLE          DefaultStandardInput;
        HANDLE          DefaultStandardOutput;
        HANDLE          DefaultStandardError;

        PVOID           WaitOnAddressHashTable[128];

        // Heaps[1] ...
    };
    STATIC_ASSERT(ALIGN_DOWN(sizeof(KPEB), 8) < PAGE_SIZE);
}


EXTERN_C_START

_Must_inspect_result_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS MUSA_API MUSA_NAME_PRIVATE(ProcessEnvironmentBlockSetup)(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

_Must_inspect_result_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS MUSA_API MUSA_NAME_PRIVATE(ProcessEnvironmentBlockTeardown)();

_IRQL_requires_max_(DISPATCH_LEVEL)
PVOID MUSA_API MUSA_NAME_PRIVATE(RtlGetCurrentPeb)();

_IRQL_raises_(APC_LEVEL)
VOID MUSA_API MUSA_NAME_PRIVATE(RtlAcquirePebLockExclusive)();

_IRQL_requires_(APC_LEVEL)
VOID  MUSA_API MUSA_NAME_PRIVATE(RtlReleasePebLockExclusive)();

_IRQL_raises_(APC_LEVEL)
VOID MUSA_API MUSA_NAME_PRIVATE(RtlAcquirePebLockShared)();

_IRQL_requires_(APC_LEVEL)
VOID  MUSA_API MUSA_NAME_PRIVATE(RtlReleasePebLockShared)();

EXTERN_C_END
