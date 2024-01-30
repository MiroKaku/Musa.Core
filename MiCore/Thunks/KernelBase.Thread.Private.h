#pragma once
#ifdef _KERNEL_MODE


EXTERN_C_START


_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(SwitchToThread)();

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MI_NAME(CreateThread)(
    _In_opt_ LPSECURITY_ATTRIBUTES ThreadAttributes,
    _In_ SIZE_T StackSize,
    _In_ LPTHREAD_START_ROUTINE StartAddress,
    _In_opt_ __drv_aliasesMem LPVOID Parameter,
    _In_ DWORD CreationFlags,
    _Out_opt_ LPDWORD ThreadId
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MI_NAME(CreateRemoteThread)(
    _In_ HANDLE Process,
    _In_opt_ LPSECURITY_ATTRIBUTES ThreadAttributes,
    _In_ SIZE_T StackSize,
    _In_ LPTHREAD_START_ROUTINE StartAddress,
    _In_opt_ LPVOID Parameter,
    _In_ DWORD CreationFlags,
    _Out_opt_ LPDWORD ThreadId
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MI_NAME(ExitThread)(
    _In_ DWORD ExitCode
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MI_NAME(OpenThread)(
    _In_ DWORD DesiredAccess,
    _In_ BOOL  InheritHandle,
    _In_ DWORD ThreadId
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(SetThreadPriority)(
    _In_ HANDLE ThreadHandle,
    _In_ int    Priority
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
int WINAPI MI_NAME(GetThreadPriority)(
    _In_ HANDLE ThreadHandle
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(SetThreadPriorityBoost)(
    _In_ HANDLE ThreadHandle,
    _In_ BOOL   DisablePriorityBoost
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(GetThreadPriorityBoost)(
    _In_ HANDLE ThreadHandle,
    _Out_ PBOOL DisablePriorityBoost
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(TerminateThread)(
    _In_ HANDLE ThreadHandle,
    _In_ DWORD  ExitCode
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(GetExitCodeThread)(
    _In_  HANDLE  ThreadHandle,
    _Out_ LPDWORD ExitCode
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MI_NAME(SuspendThread)(
    _In_ HANDLE ThreadHandle
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MI_NAME(ResumeThread)(
    _In_ HANDLE ThreadHandle
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(SetThreadToken)(
    _In_opt_ PHANDLE ThreadHandle,
    _In_opt_ HANDLE  Token
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(OpenThreadToken)(
    _In_ HANDLE ThreadHandle,
    _In_ DWORD DesiredAccess,
    _In_ BOOL OpenAsSelf,
    _Outptr_ PHANDLE TokenHandle
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MI_NAME(GetThreadId)(
    _In_ HANDLE ThreadHandle
    );

_IRQL_requires_max_(APC_LEVEL)
VOID WINAPI MI_NAME(GetCurrentThreadStackLimits)(
    _Out_ PULONG_PTR LowLimit,
    _Out_ PULONG_PTR HighLimit
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(GetThreadContext)(
    _In_    HANDLE    ThreadHandle,
    _Inout_ LPCONTEXT Context
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(SetThreadContext)(
    _In_ HANDLE ThreadHandle,
    _In_ CONST CONTEXT* Context
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(GetThreadTimes)(
    _In_  HANDLE ThreadHandle,
    _Out_ LPFILETIME CreationTime,
    _Out_ LPFILETIME ExitTime,
    _Out_ LPFILETIME KernelTime,
    _Out_ LPFILETIME UserTime
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MI_NAME(SetThreadIdealProcessor)(
    _In_ HANDLE ThreadHandle,
    _In_ DWORD  IdealProcessor
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(SetThreadIdealProcessorEx)(
    _In_ HANDLE ThreadHandle,
    _In_ PPROCESSOR_NUMBER IdealProcessor,
    _Out_opt_ PPROCESSOR_NUMBER PreviousIdealProcessor
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(GetThreadIdealProcessorEx)(
    _In_ HANDLE ThreadHandle,
    _In_ PPROCESSOR_NUMBER IdealProcessor
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(GetThreadIOPendingFlag)(
    _In_ HANDLE ThreadHandle,
    _Out_ PBOOL IOIsPending
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(GetThreadInformation)(
    _In_ HANDLE ThreadHandle,
    _In_ THREAD_INFORMATION_CLASS ThreadInformationClass,
    _Out_writes_bytes_(ThreadInformationSize) LPVOID ThreadInformation,
    _In_ DWORD ThreadInformationSize
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(SetThreadInformation)(
    _In_ HANDLE ThreadHandle,
    _In_ THREAD_INFORMATION_CLASS ThreadInformationClass,
    _In_reads_bytes_(ThreadInformationSize) LPVOID ThreadInformation,
    _In_ DWORD ThreadInformationSize
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
HRESULT WINAPI MI_NAME(SetThreadDescription)(
    _In_ HANDLE ThreadHandle,
    _In_ PCWSTR ThreadDescription
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
HRESULT WINAPI MI_NAME(GetThreadDescription)(
    _In_ HANDLE ThreadHandle,
    _Outptr_result_z_ PWSTR* ThreadDescription
    );


EXTERN_C_END


#endif
