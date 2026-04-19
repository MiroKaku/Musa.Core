#include "KernelBase.Private.h"
#include "Internal/KernelBase.Synchronize.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(Sleep))
#pragma alloc_text(PAGE, MUSA_NAME(SleepEx))
#pragma alloc_text(PAGE, MUSA_NAME(WaitForSingleObject))
#pragma alloc_text(PAGE, MUSA_NAME(WaitForSingleObjectEx))
#pragma alloc_text(PAGE, MUSA_NAME(WaitForMultipleObjects))
#pragma alloc_text(PAGE, MUSA_NAME(WaitForMultipleObjectsEx))

#pragma alloc_text(PAGE, MUSA_NAME(InitOnceInitialize))
#pragma alloc_text(PAGE, MUSA_NAME(InitOnceExecuteOnce))
#pragma alloc_text(PAGE, MUSA_NAME(InitOnceBeginInitialize))
#pragma alloc_text(PAGE, MUSA_NAME(InitOnceComplete))

#pragma alloc_text(PAGE, MUSA_NAME(AcquireSRWLockExclusive))
#pragma alloc_text(PAGE, MUSA_NAME(AcquireSRWLockShared))
#pragma alloc_text(PAGE, MUSA_NAME(ReleaseSRWLockExclusive))
#pragma alloc_text(PAGE, MUSA_NAME(ReleaseSRWLockShared))
#pragma alloc_text(PAGE, MUSA_NAME(TryAcquireSRWLockExclusive))
#pragma alloc_text(PAGE, MUSA_NAME(TryAcquireSRWLockShared))

#pragma alloc_text(PAGE, MUSA_NAME(EnterCriticalSection))
#pragma alloc_text(PAGE, MUSA_NAME(LeaveCriticalSection))
#pragma alloc_text(PAGE, MUSA_NAME(TryEnterCriticalSection))

#pragma alloc_text(PAGE, MUSA_NAME(WakeConditionVariable))
#pragma alloc_text(PAGE, MUSA_NAME(WakeAllConditionVariable))
#pragma alloc_text(PAGE, MUSA_NAME(SleepConditionVariableCS))
#pragma alloc_text(PAGE, MUSA_NAME(SleepConditionVariableSRW))
#endif

EXTERN_C_START

//
// Wait
//

_IRQL_requires_max_(APC_LEVEL)
VOID WINAPI MUSA_NAME(Sleep)(
    _In_ DWORD Milliseconds
)
{
    PAGED_CODE();
    SleepEx(Milliseconds, FALSE);
}

MUSA_IAT_SYMBOL(Sleep, 4);

_IRQL_requires_max_(APC_LEVEL)
DWORD WINAPI MUSA_NAME(SleepEx)(
    _In_ DWORD Milliseconds,
    _In_ BOOL  Alertable
)
{
    PAGED_CODE();

    LARGE_INTEGER  Time;
    LARGE_INTEGER* Timeout = &Time;

    if (Milliseconds != INFINITE) {
        Time.QuadPart = Int32x32To64(Milliseconds, -10000);
    } else {
        Time.QuadPart = (LONGLONG)0x8000000000000000;
    }

    NTSTATUS Status = KeDelayExecutionThread(KernelMode, static_cast<BOOLEAN>(Alertable), Timeout);
    if (Alertable) {
        if (Status != STATUS_USER_APC) {
            Status = 0;
        }
    }

    return Status;
}

MUSA_IAT_SYMBOL(SleepEx, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(WaitForSingleObject)(
    _In_ HANDLE Handle,
    _In_ DWORD  Milliseconds
)
{
    PAGED_CODE();

    return WaitForSingleObjectEx(Handle, Milliseconds, FALSE);
}

MUSA_IAT_SYMBOL(WaitForSingleObject, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(WaitForSingleObjectEx)(
    _In_ HANDLE Handle,
    _In_ DWORD  Milliseconds,
    _In_ BOOL   Alertable
)
{
    PAGED_CODE();

    LARGE_INTEGER  Time;
    LARGE_INTEGER* Timeout = nullptr;

    if (Milliseconds != INFINITE) {
        Time.QuadPart = Int32x32To64(Milliseconds, -10000);
        Timeout       = &Time;
    }

    const auto Status = ZwWaitForSingleObject(Handle, static_cast<BOOLEAN>(Alertable), Timeout);
    if (NT_SUCCESS(Status)) {
        return static_cast<DWORD>(Status);
    }

    BaseSetLastNTError(Status);
    return WAIT_FAILED;
}

MUSA_IAT_SYMBOL(WaitForSingleObjectEx, 12);

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(WaitForMultipleObjects)(
    _In_ DWORD                      Count,
    _In_reads_(Count) CONST HANDLE* Handles,
    _In_ BOOL                       WaitAll,
    _In_ DWORD                      Milliseconds
)
{
    PAGED_CODE();

    return WaitForMultipleObjectsEx(Count, Handles, WaitAll, Milliseconds, FALSE);
}

MUSA_IAT_SYMBOL(WaitForMultipleObjects, 16);

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(WaitForMultipleObjectsEx)(
    _In_ DWORD                      Count,
    _In_reads_(Count) CONST HANDLE* Handles,
    _In_ BOOL                       WaitAll,
    _In_ DWORD                      Milliseconds,
    _In_ BOOL                       Alertable
)
{
    PAGED_CODE();

    LARGE_INTEGER  Time;
    LARGE_INTEGER* Timeout = nullptr;

    if (Milliseconds != INFINITE) {
        Time.QuadPart = Int32x32To64(Milliseconds, -10000);
        Timeout       = &Time;
    }

    const auto Status = ZwWaitForMultipleObjects(Count, const_cast<HANDLE*>(Handles),
        WaitAll ? WAIT_TYPE::WaitAll : WaitAny,
        static_cast<BOOLEAN>(Alertable), Timeout);
    if (NT_SUCCESS(Status)) {
        return static_cast<DWORD>(Status);
    }

    BaseSetLastNTError(Status);
    return WAIT_FAILED;
}

MUSA_IAT_SYMBOL(WaitForMultipleObjectsEx, 20);

//
// Run once
//

_IRQL_requires_max_(APC_LEVEL)
VOID WINAPI MUSA_NAME(InitOnceInitialize)(
    _Out_ PINIT_ONCE InitOnce
)
{
    PAGED_CODE();

    RtlRunOnceInitialize(InitOnce);
}

MUSA_IAT_SYMBOL(InitOnceInitialize, 4);

_IRQL_requires_max_(APC_LEVEL)
_Must_inspect_result_
BOOL WINAPI MUSA_NAME(InitOnceExecuteOnce)(
    _Inout_ PINIT_ONCE                    InitOnce,
    _In_ __callback PINIT_ONCE_FN         InitFn,
    _Inout_opt_ PVOID                     Parameter,
    _Outptr_opt_result_maybenull_ LPVOID* Context
)
{
    PAGED_CODE();

    NTSTATUS Status;

    __try {
        #pragma warning(suppress: 28023)
        Status = RtlRunOnceExecuteOnce(InitOnce, reinterpret_cast<PRTL_RUN_ONCE_INIT_FN>(InitFn),
            Parameter, Context);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(InitOnceExecuteOnce, 16);

_IRQL_requires_max_(APC_LEVEL)
_Must_inspect_result_
BOOL WINAPI MUSA_NAME(InitOnceBeginInitialize)(
    _Inout_ LPINIT_ONCE                   InitOnce,
    _In_ DWORD                            Flags,
    _Out_ PBOOL                           Pending,
    _Outptr_opt_result_maybenull_ LPVOID* Context
)
{
    PAGED_CODE();

    const auto Status = RtlRunOnceBeginInitialize(InitOnce, Flags, Context);
    if (NT_SUCCESS(Status)) {
        *Pending = Status == STATUS_PENDING;
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(InitOnceBeginInitialize, 16);

_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MUSA_NAME(InitOnceComplete)(
    _Inout_ LPINIT_ONCE InitOnce,
    _In_ DWORD          Flags,
    _In_opt_ LPVOID     Context
)
{
    PAGED_CODE();

    const auto Status = RtlRunOnceComplete(InitOnce, Flags, Context);
    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(InitOnceComplete, 12);

//
// R/W lock
//

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID WINAPI MUSA_NAME(InitializeSRWLock)(
    _Out_ PSRWLOCK SRWLock
)
{
    RtlInitializeSRWLock(SRWLock);
}

MUSA_IAT_SYMBOL(InitializeSRWLock, 4);

_IRQL_requires_max_(APC_LEVEL)
_Acquires_exclusive_lock_(*SRWLock)
VOID WINAPI MUSA_NAME(AcquireSRWLockExclusive)(
    _Inout_ PSRWLOCK SRWLock
)
{
    PAGED_CODE();
    RtlAcquireSRWLockExclusive(SRWLock);
}

MUSA_IAT_SYMBOL(AcquireSRWLockExclusive, 4);

_IRQL_requires_max_(APC_LEVEL)
_Acquires_shared_lock_(*SRWLock)
VOID WINAPI MUSA_NAME(AcquireSRWLockShared)(
    _Inout_ PSRWLOCK SRWLock
)
{
    PAGED_CODE();
    RtlAcquireSRWLockShared(SRWLock);
}

MUSA_IAT_SYMBOL(AcquireSRWLockShared, 4);

_IRQL_requires_max_(APC_LEVEL)
_Releases_exclusive_lock_(*SRWLock)
VOID WINAPI MUSA_NAME(ReleaseSRWLockExclusive)(
    _Inout_ PSRWLOCK SRWLock
)
{
    PAGED_CODE();
    RtlReleaseSRWLockExclusive(SRWLock);
}

MUSA_IAT_SYMBOL(ReleaseSRWLockExclusive, 4);

_IRQL_requires_max_(APC_LEVEL)
_Releases_shared_lock_(*SRWLock)
VOID WINAPI MUSA_NAME(ReleaseSRWLockShared)(
    _Inout_ PSRWLOCK SRWLock
)
{
    PAGED_CODE();
    RtlReleaseSRWLockShared(SRWLock);
}

MUSA_IAT_SYMBOL(ReleaseSRWLockShared, 4);

_IRQL_requires_max_(APC_LEVEL)
_When_(return != 0, _Acquires_exclusive_lock_(*SRWLock))
BOOLEAN WINAPI MUSA_NAME(TryAcquireSRWLockExclusive)(
    _Inout_ PSRWLOCK SRWLock
)
{
    PAGED_CODE();
    return RtlTryAcquireSRWLockExclusive(SRWLock);
}

MUSA_IAT_SYMBOL(TryAcquireSRWLockExclusive, 4);

_IRQL_requires_max_(APC_LEVEL)
_When_(return != 0, _Acquires_shared_lock_(*SRWLock))
BOOLEAN WINAPI MUSA_NAME(TryAcquireSRWLockShared)(
    _Inout_ PSRWLOCK SRWLock
)
{
    PAGED_CODE();
    return RtlTryAcquireSRWLockShared(SRWLock);
}

MUSA_IAT_SYMBOL(TryAcquireSRWLockShared, 4);

//
// Critical Section
//

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID WINAPI MUSA_NAME(InitializeCriticalSection)(
    _Out_ LPCRITICAL_SECTION CriticalSection
)
{
    (void)RtlInitializeCriticalSection(CriticalSection);
}

MUSA_IAT_SYMBOL(InitializeCriticalSection, 4);

_IRQL_requires_max_(DISPATCH_LEVEL)
_Must_inspect_result_
BOOL WINAPI MUSA_NAME(InitializeCriticalSectionAndSpinCount)(
    _Out_ LPCRITICAL_SECTION CriticalSection,
    _In_ DWORD               SpinCount
)
{
    const auto Status = RtlInitializeCriticalSectionAndSpinCount(CriticalSection, SpinCount);
    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(InitializeCriticalSectionAndSpinCount, 8);

_IRQL_requires_max_(DISPATCH_LEVEL)
_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MUSA_NAME(InitializeCriticalSectionEx)(
    _Out_ LPCRITICAL_SECTION CriticalSection,
    _In_ DWORD               SpinCount,
    _In_ DWORD               Flags
)
{
    const auto Status = RtlInitializeCriticalSectionEx(CriticalSection, SpinCount, Flags);
    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(InitializeCriticalSectionEx, 12);

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID WINAPI MUSA_NAME(DeleteCriticalSection)(
    _Inout_ LPCRITICAL_SECTION CriticalSection
)
{
    (void)RtlDeleteCriticalSection(CriticalSection);
}

MUSA_IAT_SYMBOL(DeleteCriticalSection, 4);

_IRQL_requires_max_(APC_LEVEL)
_Acquires_exclusive_lock_(*CriticalSection)
VOID WINAPI MUSA_NAME(EnterCriticalSection)(
    _Inout_ LPCRITICAL_SECTION CriticalSection
)
{
    PAGED_CODE();
    (void)RtlEnterCriticalSection(CriticalSection);
}

MUSA_IAT_SYMBOL(EnterCriticalSection, 4);

_IRQL_requires_max_(APC_LEVEL)
_Releases_exclusive_lock_(*CriticalSection)
VOID WINAPI MUSA_NAME(LeaveCriticalSection)(
    _Inout_ LPCRITICAL_SECTION CriticalSection
)
{
    PAGED_CODE();
    (void)RtlLeaveCriticalSection(CriticalSection);
}

MUSA_IAT_SYMBOL(LeaveCriticalSection, 4);

_IRQL_requires_max_(APC_LEVEL)
_When_(return != 0, _Acquires_exclusive_lock_(*CriticalSection))
BOOL WINAPI MUSA_NAME(TryEnterCriticalSection)(
    _Inout_ LPCRITICAL_SECTION CriticalSection
)
{
    PAGED_CODE();
    return !!RtlTryEnterCriticalSection(CriticalSection);
}

MUSA_IAT_SYMBOL(TryEnterCriticalSection, 4);

_IRQL_requires_max_(DISPATCH_LEVEL)
DWORD WINAPI MUSA_NAME(SetCriticalSectionSpinCount)(
    _Inout_ LPCRITICAL_SECTION CriticalSection,
    _In_ DWORD SpinCount
)
{
    return RtlSetCriticalSectionSpinCount(CriticalSection, SpinCount);
}

MUSA_IAT_SYMBOL(SetCriticalSectionSpinCount, 8);

//
// Condition variable
//

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID WINAPI MUSA_NAME(InitializeConditionVariable)(
    _Out_ PCONDITION_VARIABLE ConditionVariable
)
{
    RtlInitializeConditionVariable(ConditionVariable);
}

MUSA_IAT_SYMBOL(InitializeConditionVariable, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SleepConditionVariableCS)(
    _Inout_ PCONDITION_VARIABLE ConditionVariable,
    _Inout_ PCRITICAL_SECTION   CriticalSection,
    _In_ DWORD                  Milliseconds
)
{
    PAGED_CODE();

    LARGE_INTEGER  Time;
    LARGE_INTEGER* Timeout = nullptr;

    if (Milliseconds != INFINITE) {
        Time.QuadPart = Int32x32To64(Milliseconds, -10000);
        Timeout       = &Time;
    }

    const auto Status = RtlSleepConditionVariableCS(ConditionVariable, CriticalSection, Timeout);
    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(SleepConditionVariableCS, 12);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SleepConditionVariableSRW)(
    _Inout_ PCONDITION_VARIABLE ConditionVariable,
    _Inout_ PSRWLOCK            SRWLock,
    _In_ DWORD                  Milliseconds,
    _In_ ULONG                  Flags
)
{
    PAGED_CODE();

    LARGE_INTEGER  Time;
    LARGE_INTEGER* Timeout = nullptr;

    if (Milliseconds != INFINITE) {
        Time.QuadPart = Int32x32To64(Milliseconds, -10000);
        Timeout       = &Time;
    }

    const auto Status = RtlSleepConditionVariableSRW(ConditionVariable, SRWLock, Timeout, Flags);
    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(SleepConditionVariableSRW, 16);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MUSA_NAME(WakeConditionVariable)(
    _Inout_ PCONDITION_VARIABLE ConditionVariable
)
{
    PAGED_CODE();
    RtlWakeConditionVariable(ConditionVariable);
}

MUSA_IAT_SYMBOL(WakeConditionVariable, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MUSA_NAME(WakeAllConditionVariable)(
    _Inout_ PCONDITION_VARIABLE ConditionVariable
)
{
    PAGED_CODE();
    RtlWakeAllConditionVariable(ConditionVariable);
}

MUSA_IAT_SYMBOL(WakeAllConditionVariable, 4);

EXTERN_C_END
