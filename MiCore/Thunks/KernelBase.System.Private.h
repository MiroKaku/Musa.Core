#pragma once
#ifdef _KERNEL_MODE


EXTERN_C_START


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
