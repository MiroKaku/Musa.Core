#pragma once
#ifdef _KERNEL_MODE


EXTERN_C_START

//
// R/W lock
//

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID WINAPI MI_NAME(RtlInitializeSRWLock)(
    _Out_ PRTL_SRWLOCK SRWLock
    );

_IRQL_requires_max_(APC_LEVEL)
_Acquires_exclusive_lock_(*SRWLock)
VOID NTAPI MI_NAME(RtlAcquireSRWLockExclusive)(
    _Inout_ PRTL_SRWLOCK SRWLock
    );

_IRQL_requires_max_(APC_LEVEL)
_Acquires_shared_lock_(*SRWLock)
VOID NTAPI MI_NAME(RtlAcquireSRWLockShared)(
    _Inout_ PRTL_SRWLOCK SRWLock
    );

_IRQL_requires_max_(APC_LEVEL)
_Releases_exclusive_lock_(*SRWLock)
VOID NTAPI MI_NAME(RtlReleaseSRWLockExclusive)(
    _Inout_ PRTL_SRWLOCK SRWLock
    );

_IRQL_requires_max_(APC_LEVEL)
_Releases_shared_lock_(*SRWLock)
VOID NTAPI MI_NAME(RtlReleaseSRWLockShared)(
    _Inout_ PRTL_SRWLOCK SRWLock
    );

_IRQL_requires_max_(APC_LEVEL)
_When_(return != 0, _Acquires_exclusive_lock_(*SRWLock))
BOOLEAN NTAPI MI_NAME(RtlTryAcquireSRWLockExclusive)(
    _Inout_ PRTL_SRWLOCK SRWLock
    );

_IRQL_requires_max_(APC_LEVEL)
_When_(return != 0, _Acquires_shared_lock_(*SRWLock))
BOOLEAN NTAPI MI_NAME(RtlTryAcquireSRWLockShared)(
    _Inout_ PRTL_SRWLOCK SRWLock
    );


//
// Critical Section
//

STATIC_ASSERT(sizeof(RTL_CRITICAL_SECTION) >= sizeof(EX_PUSH_LOCK));

_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS NTAPI MI_NAME(RtlInitializeCriticalSection)(
    _Out_ PRTL_CRITICAL_SECTION CriticalSection
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
_Must_inspect_result_
NTSTATUS NTAPI MI_NAME(RtlInitializeCriticalSectionAndSpinCount)(
    _Out_ PRTL_CRITICAL_SECTION CriticalSection,
    _In_ ULONG SpinCount
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS NTAPI MI_NAME(RtlInitializeCriticalSectionEx)(
    _Out_ PRTL_CRITICAL_SECTION CriticalSection,
    _In_ ULONG SpinCount,
    _In_ ULONG Flags
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS NTAPI MI_NAME(RtlDeleteCriticalSection)(
    _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    );

_IRQL_requires_max_(APC_LEVEL)
_Acquires_exclusive_lock_(*CriticalSection)
NTSTATUS NTAPI MI_NAME(RtlEnterCriticalSection)(
    _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    );

_IRQL_requires_max_(APC_LEVEL)
_Releases_exclusive_lock_(*CriticalSection)
NTSTATUS NTAPI MI_NAME(RtlLeaveCriticalSection)(
    _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    );

_IRQL_requires_max_(APC_LEVEL)
_When_(return != 0, _Acquires_exclusive_lock_(*CriticalSection))
LOGICAL NTAPI MI_NAME(RtlTryEnterCriticalSection)(
    _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    );

_IRQL_requires_max_(APC_LEVEL)
LOGICAL NTAPI MI_NAME(RtlIsCriticalSectionLocked)(
    _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    );


//
// Condition variable
//

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID NTAPI MI_NAME(RtlInitializeConditionVariable)(
    _Out_ PRTL_CONDITION_VARIABLE ConditionVariable
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MI_NAME(RtlSleepConditionVariableCS)(
    _Inout_ PRTL_CONDITION_VARIABLE ConditionVariable,
    _Inout_ PRTL_CRITICAL_SECTION CriticalSection,
    _In_opt_ PLARGE_INTEGER Timeout
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MI_NAME(RtlSleepConditionVariableSRW)(
    _Inout_ PRTL_CONDITION_VARIABLE ConditionVariable,
    _Inout_ PRTL_SRWLOCK SRWLock,
    _In_opt_ PLARGE_INTEGER Timeout,
    _In_ ULONG Flags
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID NTAPI MI_NAME(RtlWakeConditionVariable)(
    _Inout_ PRTL_CONDITION_VARIABLE ConditionVariable
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID NTAPI MI_NAME(RtlWakeAllConditionVariable)(
    _Inout_ PRTL_CONDITION_VARIABLE ConditionVariable
    );



EXTERN_C_END


#endif
