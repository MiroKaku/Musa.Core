#pragma once
#ifdef _KERNEL_MODE

EXTERN_C_START

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID NTAPI MUSA_NAME(RtlExitUserThread)(
    _In_ NTSTATUS ExitStatus
    );

EXTERN_C_END

#endif
