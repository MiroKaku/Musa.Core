#pragma once
#ifdef _KERNEL_MODE


EXTERN_C_START


_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(CloseHandle)(
    _In_ _Post_ptr_invalid_ HANDLE Handle
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(DuplicateHandle)(
    _In_ HANDLE SourceProcessHandle,
    _In_ HANDLE SourceHandle,
    _In_ HANDLE TargetProcessHandle,
    _Outptr_ LPHANDLE TargetHandle,
    _In_ DWORD  DesiredAccess,
    _In_ BOOL   InheritHandle,
    _In_ DWORD  Options
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(CompareObjectHandles)(
    _In_ HANDLE FirstHandle,
    _In_ HANDLE SecondHandle
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(GetHandleInformation)(
    _In_  HANDLE  Handle,
    _Out_ LPDWORD Flags
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(SetHandleInformation)(
    _In_ HANDLE Handle,
    _In_ DWORD  Mask,
    _In_ DWORD  Flags
    );


EXTERN_C_END


#endif
