#pragma once

namespace Musa::Core
{
    //
    // Kernel Thread  Environment Block
    //

    VEIL_DECLARE_STRUCT_ALIGN(KTEB, 8)
    {
        HANDLE          ThreadId;
        HANDLE          ProcessId;
        struct _KPEB*   ProcessEnvironmentBlock;

        ULONG           HardErrorMode;
        NTSTATUS        ExceptionCode;

        ULONG           LastErrorValue;
        NTSTATUS        LastStatusValue;

        struct _RTL_FLS_DATA* FlsData;
    };
}


EXTERN_C_START

_Must_inspect_result_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS MUSA_API MUSA_NAME_PRIVATE(ThreadEnvironmentBlockSetup)(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

_Must_inspect_result_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS MUSA_API MUSA_NAME_PRIVATE(ThreadEnvironmentBlockTeardown)();

_IRQL_requires_max_(DISPATCH_LEVEL)
PVOID MUSA_API MUSA_NAME_PRIVATE(RtlGetCurrentTeb)();

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
