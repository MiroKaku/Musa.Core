#include "Internal/Ntdll.Synchronize.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(RtlSleepConditionVariableCS))
#pragma alloc_text(PAGE, MUSA_NAME(RtlSleepConditionVariableSRW))
#pragma alloc_text(PAGE, MUSA_NAME(RtlWakeConditionVariable))
#pragma alloc_text(PAGE, MUSA_NAME(RtlWakeAllConditionVariable))
#endif

EXTERN_C_START

//
// Condition variable
//

_IRQL_requires_max_(DISPATCH_LEVEL)
VOID NTAPI MUSA_NAME(RtlInitializeConditionVariable)(
    _Out_ PRTL_CONDITION_VARIABLE ConditionVariable
)
{
    *ConditionVariable = RTL_CONDITION_VARIABLE_INIT;
}

MUSA_IAT_SYMBOL(RtlInitializeConditionVariable, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlSleepConditionVariableCS)(
    _Inout_ PRTL_CONDITION_VARIABLE ConditionVariable,
    _Inout_ PRTL_CRITICAL_SECTION   CriticalSection,
    _In_opt_ PLARGE_INTEGER         Timeout
)
{
    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    if (InterlockedIncrement(reinterpret_cast<volatile long*>(&ConditionVariable->Ptr)) > 0) {
        (void)RtlLeaveCriticalSection(CriticalSection);
        {
            Status = ZwWaitForKeyedEvent(nullptr, &ConditionVariable->Ptr, FALSE, Timeout);
        }
        (void)RtlEnterCriticalSection(CriticalSection);
    }

    return Status;
}

MUSA_IAT_SYMBOL(RtlSleepConditionVariableCS, 12);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlSleepConditionVariableSRW)(
    _Inout_ PRTL_CONDITION_VARIABLE ConditionVariable,
    _Inout_ PRTL_SRWLOCK            SRWLock,
    _In_opt_ PLARGE_INTEGER         Timeout,
    _In_ ULONG                      Flags
)
{
    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    if (InterlockedIncrement(reinterpret_cast<volatile long*>(&ConditionVariable->Ptr)) > 0) {
        if (Flags == RTL_CONDITION_VARIABLE_LOCKMODE_SHARED) {
            RtlReleaseSRWLockShared(SRWLock);
            {
                Status = ZwWaitForKeyedEvent(nullptr, &ConditionVariable->Ptr, FALSE, Timeout);
            }
            RtlAcquireSRWLockShared(SRWLock);
        } else {
            RtlReleaseSRWLockExclusive(SRWLock);
            {
                Status = ZwWaitForKeyedEvent(nullptr, &ConditionVariable->Ptr, FALSE, Timeout);
            }
            RtlAcquireSRWLockExclusive(SRWLock);
        }
    }

    return Status;
}

MUSA_IAT_SYMBOL(RtlSleepConditionVariableSRW, 16);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID NTAPI MUSA_NAME(RtlWakeConditionVariable)(
    _Inout_ PRTL_CONDITION_VARIABLE ConditionVariable
)
{
    PAGED_CODE();

    if (InterlockedCompareExchange(reinterpret_cast<volatile long*>(&ConditionVariable->Ptr), 0, 0) > 0) {
        const long Count = InterlockedDecrement(reinterpret_cast<volatile long*>(&ConditionVariable->Ptr));
        if (Count >= 0) {
            (void)ZwReleaseKeyedEvent(nullptr, &ConditionVariable->Ptr, 0, nullptr);
        }
    }
}

MUSA_IAT_SYMBOL(RtlWakeConditionVariable, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID NTAPI MUSA_NAME(RtlWakeAllConditionVariable)(
    _Inout_ PRTL_CONDITION_VARIABLE ConditionVariable
)
{
    PAGED_CODE();

    if (InterlockedCompareExchange(reinterpret_cast<volatile long*>(&ConditionVariable->Ptr), 0, 0) > 0) {
        long Count;

        do {
            Count = InterlockedDecrement(reinterpret_cast<volatile long*>(&ConditionVariable->Ptr));
            if (Count >= 0) {
                (void)ZwReleaseKeyedEvent(nullptr, &ConditionVariable->Ptr, 0, nullptr);
            }
        } while (Count);
    }
}

MUSA_IAT_SYMBOL(RtlWakeAllConditionVariable, 4);

EXTERN_C_END
