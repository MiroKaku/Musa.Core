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

//
// R/W lock
//

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID WINAPI MI_NAME(InitializeSRWLock)(
    _Out_ PSRWLOCK SRWLock
    );

_IRQL_requires_max_(APC_LEVEL)
_Acquires_exclusive_lock_(*SRWLock)
VOID WINAPI MI_NAME(AcquireSRWLockExclusive)(
    _Inout_ PSRWLOCK SRWLock
    );

_IRQL_requires_max_(APC_LEVEL)
_Acquires_shared_lock_(*SRWLock)
VOID WINAPI MI_NAME(AcquireSRWLockShared)(
    _Inout_ PSRWLOCK SRWLock
    );

_IRQL_requires_max_(APC_LEVEL)
_Releases_exclusive_lock_(*SRWLock)
VOID WINAPI MI_NAME(ReleaseSRWLockExclusive)(
    _Inout_ PSRWLOCK SRWLock
    );

_IRQL_requires_max_(APC_LEVEL)
_Releases_shared_lock_(*SRWLock)
VOID WINAPI MI_NAME(ReleaseSRWLockShared)(
    _Inout_ PSRWLOCK SRWLock
    );

_IRQL_requires_max_(APC_LEVEL)
_When_(return != 0, _Acquires_exclusive_lock_(*SRWLock))
BOOLEAN WINAPI MI_NAME(TryAcquireSRWLockExclusive)(
    _Inout_ PSRWLOCK SRWLock
    );

_IRQL_requires_max_(APC_LEVEL)
_When_(return != 0, _Acquires_shared_lock_(*SRWLock))
BOOLEAN WINAPI MI_NAME(TryAcquireSRWLockShared)(
    _Inout_ PSRWLOCK SRWLock
    );

//
// Critical Section
//

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID WINAPI MI_NAME(InitializeCriticalSection)(
    _Out_ LPCRITICAL_SECTION CriticalSection
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
_Must_inspect_result_
BOOL WINAPI MI_NAME(InitializeCriticalSectionAndSpinCount)(
    _Out_ LPCRITICAL_SECTION CriticalSection,
    _In_ DWORD SpinCount
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
_Must_inspect_result_
BOOL WINAPI MI_NAME(InitializeCriticalSectionEx)(
    _Out_ LPCRITICAL_SECTION CriticalSection,
    _In_ DWORD SpinCount,
    _In_ DWORD Flags
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID WINAPI MI_NAME(DeleteCriticalSection)(
    _Inout_ LPCRITICAL_SECTION CriticalSection
    );

_IRQL_requires_max_(APC_LEVEL)
_Acquires_exclusive_lock_(*CriticalSection)
VOID WINAPI MI_NAME(EnterCriticalSection)(
    _Inout_ LPCRITICAL_SECTION CriticalSection
    );

_IRQL_requires_max_(APC_LEVEL)
_Releases_exclusive_lock_(*CriticalSection)
VOID WINAPI MI_NAME(LeaveCriticalSection)(
    _Inout_ LPCRITICAL_SECTION CriticalSection
    );

_IRQL_requires_max_(APC_LEVEL)
_When_(return != 0, _Acquires_exclusive_lock_(*CriticalSection))
BOOL WINAPI MI_NAME(TryEnterCriticalSection)(
    _Inout_ LPCRITICAL_SECTION CriticalSection
    );

_IRQL_requires_max_(APC_LEVEL)
DWORD WINAPI MI_NAME(SetCriticalSectionSpinCount)(
    _Inout_ LPCRITICAL_SECTION CriticalSection,
    _In_ DWORD SpinCount
    );


//
// Condition variable
//

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID WINAPI MI_NAME(InitializeConditionVariable)(
    _Out_ PCONDITION_VARIABLE ConditionVariable
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(SleepConditionVariableCS)(
    _Inout_ PCONDITION_VARIABLE ConditionVariable,
    _Inout_ PCRITICAL_SECTION CriticalSection,
    _In_ DWORD Milliseconds
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MI_NAME(SleepConditionVariableSRW)(
    _Inout_ PCONDITION_VARIABLE ConditionVariable,
    _Inout_ PSRWLOCK SRWLock,
    _In_ DWORD Milliseconds,
    _In_ ULONG Flags
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MI_NAME(WakeConditionVariable)(
    _Inout_ PCONDITION_VARIABLE ConditionVariable
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MI_NAME(WakeAllConditionVariable)(
    _Inout_ PCONDITION_VARIABLE ConditionVariable
    );



EXTERN_C_END


#endif
