#pragma once
#ifdef _KERNEL_MODE


EXTERN_C_START


//
// Mutant
//

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MI_NAME(CreateMutexW)(
    _In_opt_ LPSECURITY_ATTRIBUTES MutexAttributes,
    _In_     BOOL    InitialOwner,
    _In_opt_ LPCWSTR Name
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MI_NAME(CreateMutexExW)(
    _In_opt_ LPSECURITY_ATTRIBUTES MutexAttributes,
    _In_opt_ LPCWSTR Name,
    _In_ DWORD Flags,
    _In_ DWORD DesiredAccess
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MI_NAME(OpenMutexW)(
    _In_ DWORD   DesiredAccess,
    _In_ BOOL    InheritHandle,
    _In_ LPCWSTR Name
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(ReleaseMutex)(
    _In_ HANDLE Mutex
    );

//
// Event
//

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MI_NAME(CreateEventW)(
    _In_opt_ LPSECURITY_ATTRIBUTES EventAttributes,
    _In_ BOOL ManualReset,
    _In_ BOOL InitialState,
    _In_opt_ LPCWSTR Name
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MI_NAME(CreateEventExW)(
    _In_opt_ LPSECURITY_ATTRIBUTES EventAttributes,
    _In_opt_ LPCWSTR Name,
    _In_ DWORD Flags,
    _In_ DWORD DesiredAccess
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MI_NAME(OpenEventW)(
    _In_ DWORD   DesiredAccess,
    _In_ BOOL    InheritHandle,
    _In_ LPCWSTR Name
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(SetEvent)(
    _In_ HANDLE Event
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(ResetEvent)(
    _In_ HANDLE Event
    );

//
// Semaphore
//

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MI_NAME(CreateSemaphoreW)(
    _In_opt_ LPSECURITY_ATTRIBUTES SemaphoreAttributes,
    _In_ LONG InitialCount,
    _In_ LONG MaximumCount,
    _In_opt_ LPCWSTR Name
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MI_NAME(CreateSemaphoreExW)(
    _In_opt_    LPSECURITY_ATTRIBUTES SemaphoreAttributes,
    _In_        LONG InitialCount,
    _In_        LONG MaximumCount,
    _In_opt_    LPCWSTR Name,
    _Reserved_  DWORD Flags,
    _In_        DWORD DesiredAccess
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MI_NAME(OpenSemaphoreW)(
    _In_ DWORD   DesiredAccess,
    _In_ BOOL    InheritHandle,
    _In_ LPCWSTR Name
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(ReleaseSemaphore)(
    _In_ HANDLE Semaphore,
    _In_ LONG   ReleaseCount,
    _Out_opt_ LPLONG PreviousCount
    );

//
// Wait
//

_IRQL_requires_max_(APC_LEVEL)
VOID WINAPI MI_NAME(Sleep)(
    _In_ DWORD  Milliseconds
    );

#if defined _KERNEL_MODE
_IRQL_requires_max_(APC_LEVEL)
DWORD WINAPI MI_NAME(SleepEx)(
    _In_ DWORD Milliseconds,
    _In_ BOOL  Alertable
    );
#endif

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MI_NAME(WaitForSingleObject)(
    _In_ HANDLE Handle,
    _In_ DWORD  Milliseconds
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MI_NAME(WaitForSingleObjectEx)(
    _In_ HANDLE Handle,
    _In_ DWORD  Milliseconds,
    _In_ BOOL   Alertable
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MI_NAME(WaitForMultipleObjects)(
    _In_ DWORD Count,
    _In_reads_(Count) CONST HANDLE* Handles,
    _In_ BOOL  WaitAll,
    _In_ DWORD Milliseconds
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MI_NAME(WaitForMultipleObjectsEx)(
    _In_ DWORD Count,
    _In_reads_(Count) CONST HANDLE* Handles,
    _In_ BOOL  WaitAll,
    _In_ DWORD Milliseconds,
    _In_ BOOL  Alertable
    );

//
// Run once
//

_IRQL_requires_max_(APC_LEVEL)
VOID WINAPI MI_NAME(InitOnceInitialize)(
    _Out_ PINIT_ONCE InitOnce
    );

_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MI_NAME(InitOnceExecuteOnce)(
    _Inout_ PINIT_ONCE InitOnce,
    _In_ __callback PINIT_ONCE_FN InitFn,
    _Inout_opt_ PVOID Parameter,
    _Outptr_opt_result_maybenull_ LPVOID* Context
    );

_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MI_NAME(InitOnceBeginInitialize)(
    _Inout_ LPINIT_ONCE InitOnce,
    _In_ DWORD Flags,
    _Out_ PBOOL Pending,
    _Outptr_opt_result_maybenull_ LPVOID* Context
    );

_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MI_NAME(InitOnceComplete)(
    _Inout_ LPINIT_ONCE InitOnce,
    _In_ DWORD Flags,
    _In_opt_ LPVOID Context
    );












EXTERN_C_END


#endif
