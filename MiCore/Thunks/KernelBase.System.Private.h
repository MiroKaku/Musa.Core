#pragma once
#ifdef _KERNEL_MODE


EXTERN_C_START


_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(GetLogicalProcessorInformation)(
    _Out_writes_bytes_to_opt_(*ReturnedLength, *ReturnedLength) PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer,
    _Inout_ PDWORD ReturnedLength
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(GetLogicalProcessorInformationEx)(
    _In_ LOGICAL_PROCESSOR_RELATIONSHIP RelationshipType,
    _Out_writes_bytes_to_opt_(*ReturnedLength, *ReturnedLength) PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX Buffer,
    _Inout_ PDWORD ReturnedLength
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(GetSystemTimes)(
    _Out_opt_ PFILETIME IdleTime,
    _Out_opt_ PFILETIME KernelTime,
    _Out_opt_ PFILETIME UserTime
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MI_NAME(GetNativeSystemInfo)(
    _Out_ LPSYSTEM_INFO SystemInfo
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MI_NAME(GetSystemInfo)(
    _Out_ LPSYSTEM_INFO SystemInfo
    );


EXTERN_C_END


#endif
