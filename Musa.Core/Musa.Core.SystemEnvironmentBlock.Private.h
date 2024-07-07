#pragma once
#ifdef _KERNEL_MODE

#include "Thunks/Ntdll.FiberLocalStorage.Private.h"


namespace Musa
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

        PVOID           WaitOnAddressHashTable[128];

        // Heaps[1] ...
    };
    STATIC_ASSERT(ALIGN_DOWN(sizeof(KPEB), 8) < PAGE_SIZE);

    // Kernel Thread  Environment Block
    VEIL_DECLARE_STRUCT_ALIGN(KTEB, 8)
    {
        HANDLE          ThreadId;
        HANDLE          ProcessId;
        PKPEB           ProcessEnvironmentBlock;

        ULONG           HardErrorMode;
        NTSTATUS        ExceptionCode;

        ULONG           LastErrorValue;
        NTSTATUS        LastStatusValue;

        PRTL_FLS_DATA   FlsData;
    };

}



EXTERN_C_START


_IRQL_requires_max_(DISPATCH_LEVEL)
Musa::PKPEB MUSA_API MUSA_NAME_PRIVATE(RtlGetCurrentPeb)();

_IRQL_raises_(APC_LEVEL)
VOID MUSA_API MUSA_NAME_PRIVATE(RtlAcquirePebLockExclusive)();

_IRQL_requires_(APC_LEVEL)
VOID  MUSA_API MUSA_NAME_PRIVATE(RtlReleasePebLockExclusive)();

_IRQL_raises_(APC_LEVEL)
VOID MUSA_API MUSA_NAME_PRIVATE(RtlAcquirePebLockShared)();

_IRQL_requires_(APC_LEVEL)
VOID  MUSA_API MUSA_NAME_PRIVATE(RtlReleasePebLockShared)();


_IRQL_requires_max_(DISPATCH_LEVEL)
Musa::PKTEB MUSA_API MUSA_NAME_PRIVATE(RtlGetCurrentTeb)();

_IRQL_saves_
_IRQL_raises_(DISPATCH_LEVEL)
KIRQL MUSA_API MUSA_NAME_PRIVATE(RtlAcquireTebLockExclusive)();

_IRQL_requires_(DISPATCH_LEVEL)
VOID  MUSA_API MUSA_NAME_PRIVATE(RtlReleaseTebLockExclusive)(_In_ _IRQL_restores_ KIRQL Irql);

_IRQL_saves_
_IRQL_raises_(DISPATCH_LEVEL)
KIRQL MUSA_API MUSA_NAME_PRIVATE(RtlAcquireTebLockShared)();

_IRQL_requires_(DISPATCH_LEVEL)
VOID  MUSA_API MUSA_NAME_PRIVATE(RtlReleaseTebLockShared)(_In_ _IRQL_restores_ KIRQL Irql);


EXTERN_C_END


#endif
