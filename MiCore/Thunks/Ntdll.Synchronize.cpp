#include "Ntdll.Synchronize.Private.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MI_NAME(RtlAcquireSRWLockExclusive))
#pragma alloc_text(PAGE, MI_NAME(RtlAcquireSRWLockShared))
#pragma alloc_text(PAGE, MI_NAME(RtlReleaseSRWLockExclusive))
#pragma alloc_text(PAGE, MI_NAME(RtlReleaseSRWLockShared))
#pragma alloc_text(PAGE, MI_NAME(RtlTryAcquireSRWLockExclusive))
#pragma alloc_text(PAGE, MI_NAME(RtlTryAcquireSRWLockShared))

#pragma alloc_text(PAGE, MI_NAME(RtlEnterCriticalSection))
#pragma alloc_text(PAGE, MI_NAME(RtlLeaveCriticalSection))
#pragma alloc_text(PAGE, MI_NAME(RtlTryEnterCriticalSection))
#pragma alloc_text(PAGE, MI_NAME(RtlIsCriticalSectionLocked))

#pragma alloc_text(PAGE, MI_NAME(RtlSleepConditionVariableCS))
#pragma alloc_text(PAGE, MI_NAME(RtlSleepConditionVariableSRW))
#pragma alloc_text(PAGE, MI_NAME(RtlWakeConditionVariable))
#pragma alloc_text(PAGE, MI_NAME(RtlWakeAllConditionVariable))
#endif


EXTERN_C_START
namespace Mi
{

#if defined _KERNEL_MODE

    //
    // R/W lock
    //

    STATIC_ASSERT(sizeof(RTL_SRWLOCK) >= sizeof(EX_PUSH_LOCK));

    _IRQL_requires_max_(DISPATCH_LEVEL)
    VOID WINAPI MI_NAME(RtlInitializeSRWLock)(
        _Out_ PRTL_SRWLOCK SRWLock
        )
    {
        ExInitializePushLock(reinterpret_cast<PEX_PUSH_LOCK>(SRWLock));
    }
    MI_IAT_SYMBOL(RtlInitializeSRWLock, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Acquires_exclusive_lock_(*SRWLock)
    VOID NTAPI MI_NAME(RtlAcquireSRWLockExclusive)(
        _Inout_ PRTL_SRWLOCK SRWLock
    )
    {
        PAGED_CODE();

        KeEnterGuardedRegion();
        ExAcquirePushLockExclusiveEx(reinterpret_cast<PEX_PUSH_LOCK>(SRWLock), EX_DEFAULT_PUSH_LOCK_FLAGS);
    }
    MI_IAT_SYMBOL(RtlAcquireSRWLockExclusive, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Acquires_shared_lock_(*SRWLock)
    VOID NTAPI MI_NAME(RtlAcquireSRWLockShared)(
        _Inout_ PRTL_SRWLOCK SRWLock
    )
    {
        PAGED_CODE();

        KeEnterGuardedRegion();
        ExAcquirePushLockSharedEx(reinterpret_cast<PEX_PUSH_LOCK>(SRWLock), EX_DEFAULT_PUSH_LOCK_FLAGS);
    }
    MI_IAT_SYMBOL(RtlAcquireSRWLockShared, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Releases_exclusive_lock_(*SRWLock)
    VOID NTAPI MI_NAME(RtlReleaseSRWLockExclusive)(
        _Inout_ PRTL_SRWLOCK SRWLock
    )
    {
        PAGED_CODE();

        ExReleasePushLockExclusiveEx(reinterpret_cast<PEX_PUSH_LOCK>(SRWLock), EX_DEFAULT_PUSH_LOCK_FLAGS);
        KeLeaveGuardedRegion();
    }
    MI_IAT_SYMBOL(RtlReleaseSRWLockExclusive, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Releases_shared_lock_(*SRWLock)
    VOID NTAPI MI_NAME(RtlReleaseSRWLockShared)(
        _Inout_ PRTL_SRWLOCK SRWLock
    )
    {
        PAGED_CODE();

        ExReleasePushLockSharedEx(reinterpret_cast<PEX_PUSH_LOCK>(SRWLock), EX_DEFAULT_PUSH_LOCK_FLAGS);
        KeLeaveGuardedRegion();
    }
    MI_IAT_SYMBOL(RtlReleaseSRWLockShared, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _When_(return != 0, _Acquires_exclusive_lock_(*SRWLock))
    BOOLEAN NTAPI MI_NAME(RtlTryAcquireSRWLockExclusive)(
        _Inout_ PRTL_SRWLOCK SRWLock
    )
    {
        PAGED_CODE();

        KeEnterGuardedRegion();
        if (ExTryAcquirePushLockExclusiveEx(reinterpret_cast<PEX_PUSH_LOCK>(SRWLock), EX_DEFAULT_PUSH_LOCK_FLAGS)) {
            return TRUE;
        }

        KeLeaveGuardedRegion();
        return FALSE;
    }
    MI_IAT_SYMBOL(RtlTryAcquireSRWLockExclusive, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _When_(return != 0, _Acquires_shared_lock_(*SRWLock))
    BOOLEAN NTAPI MI_NAME(RtlTryAcquireSRWLockShared)(
        _Inout_ PRTL_SRWLOCK SRWLock
    )
    {
        PAGED_CODE();

        KeEnterGuardedRegion();
        if (ExTryAcquirePushLockSharedEx(reinterpret_cast<PEX_PUSH_LOCK>(SRWLock), EX_DEFAULT_PUSH_LOCK_FLAGS)) {
            return TRUE;
        }

        KeLeaveGuardedRegion();
        return FALSE;
    }
    MI_IAT_SYMBOL(RtlTryAcquireSRWLockShared, 4);


    //
    // Critical Section
    //

    STATIC_ASSERT(sizeof(RTL_CRITICAL_SECTION) >= sizeof(EX_PUSH_LOCK));

    _IRQL_requires_max_(DISPATCH_LEVEL)
    NTSTATUS NTAPI MI_NAME(RtlInitializeCriticalSection)(
        _Out_ PRTL_CRITICAL_SECTION CriticalSection
    )
    {
        return RtlInitializeCriticalSectionEx(CriticalSection, 0, 0);
    }
    MI_IAT_SYMBOL(RtlInitializeCriticalSection, 4);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Must_inspect_result_
    NTSTATUS NTAPI MI_NAME(RtlInitializeCriticalSectionAndSpinCount)(
        _Inout_ PRTL_CRITICAL_SECTION CriticalSection,
        _In_ ULONG SpinCount
    )
    {
        return RtlInitializeCriticalSectionEx(CriticalSection, SpinCount, 0);
    }
    MI_IAT_SYMBOL(RtlInitializeCriticalSectionAndSpinCount, 8);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    NTSTATUS NTAPI MI_NAME(RtlInitializeCriticalSectionEx)(
        _Out_ PRTL_CRITICAL_SECTION CriticalSection,
        _In_ ULONG SpinCount,
        _In_ ULONG Flags
    )
    {
        UNREFERENCED_PARAMETER(SpinCount);
        UNREFERENCED_PARAMETER(Flags);

        ExInitializePushLock(reinterpret_cast<PEX_PUSH_LOCK>(CriticalSection));
        return STATUS_SUCCESS;
    }
    MI_IAT_SYMBOL(RtlInitializeCriticalSectionEx, 12);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    NTSTATUS NTAPI MI_NAME(RtlDeleteCriticalSection)(
        _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    )
    {
        UNREFERENCED_PARAMETER(CriticalSection);

        return STATUS_SUCCESS;
    }
    MI_IAT_SYMBOL(RtlDeleteCriticalSection, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Acquires_exclusive_lock_(*CriticalSection)
    NTSTATUS NTAPI MI_NAME(RtlEnterCriticalSection)(
        _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    )
    {
        PAGED_CODE();

        KeEnterGuardedRegion();
        ExAcquirePushLockExclusiveEx(reinterpret_cast<PEX_PUSH_LOCK>(CriticalSection), EX_DEFAULT_PUSH_LOCK_FLAGS);

        return STATUS_SUCCESS;
    }
    MI_IAT_SYMBOL(RtlEnterCriticalSection, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Releases_exclusive_lock_(*CriticalSection)
    NTSTATUS NTAPI MI_NAME(RtlLeaveCriticalSection)(
        _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    )
    {
        PAGED_CODE();

        ExReleasePushLockExclusiveEx(reinterpret_cast<PEX_PUSH_LOCK>(CriticalSection), EX_DEFAULT_PUSH_LOCK_FLAGS);
        KeLeaveGuardedRegion();

        return STATUS_SUCCESS;
    }
    MI_IAT_SYMBOL(RtlLeaveCriticalSection, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _When_(return != 0, _Acquires_exclusive_lock_(*CriticalSection))
    LOGICAL NTAPI MI_NAME(RtlTryEnterCriticalSection)(
        _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    )
    {
        PAGED_CODE();

        KeEnterGuardedRegion();
        if (ExTryAcquirePushLockExclusiveEx(reinterpret_cast<PEX_PUSH_LOCK>(CriticalSection), EX_DEFAULT_PUSH_LOCK_FLAGS)) {
            return TRUE;
        }

        KeLeaveGuardedRegion();
        return FALSE;
    }
    MI_IAT_SYMBOL(RtlTryEnterCriticalSection, 4);
    
    _IRQL_requires_max_(APC_LEVEL)
    LOGICAL NTAPI MI_NAME(RtlIsCriticalSectionLocked)(
        _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    )
    {
        PAGED_CODE();

        if (!RtlTryEnterCriticalSection(CriticalSection)) {
            return TRUE;

        }

        (void)RtlLeaveCriticalSection(CriticalSection);
        return FALSE;
    }
    MI_IAT_SYMBOL(RtlIsCriticalSectionLocked, 4);


    //
    // Condition variable
    //

    _IRQL_requires_max_(DISPATCH_LEVEL)
    VOID NTAPI MI_NAME(RtlInitializeConditionVariable)(
        _Out_ PRTL_CONDITION_VARIABLE ConditionVariable
    )
    {
        *ConditionVariable = RTL_CONDITION_VARIABLE_INIT;
    }
    MI_IAT_SYMBOL(RtlInitializeConditionVariable, 4);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    NTSTATUS NTAPI MI_NAME(RtlSleepConditionVariableCS)(
        _Inout_ PRTL_CONDITION_VARIABLE ConditionVariable,
        _Inout_ PRTL_CRITICAL_SECTION CriticalSection,
        _In_opt_ PLARGE_INTEGER Timeout
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
    MI_IAT_SYMBOL(RtlSleepConditionVariableCS, 12);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    NTSTATUS NTAPI MI_NAME(RtlSleepConditionVariableSRW)(
        _Inout_ PRTL_CONDITION_VARIABLE ConditionVariable,
        _Inout_ PRTL_SRWLOCK SRWLock,
        _In_opt_ PLARGE_INTEGER Timeout,
        _In_ ULONG Flags
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
            }
            else {
                RtlReleaseSRWLockExclusive(SRWLock);
                {
                    Status = ZwWaitForKeyedEvent(nullptr, &ConditionVariable->Ptr, FALSE, Timeout);
                }
                RtlAcquireSRWLockExclusive(SRWLock);
            }
        }

        return Status;
    }
    MI_IAT_SYMBOL(RtlSleepConditionVariableSRW, 16);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    VOID NTAPI MI_NAME(RtlWakeConditionVariable)(
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
    MI_IAT_SYMBOL(RtlWakeConditionVariable, 4);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    VOID NTAPI MI_NAME(RtlWakeAllConditionVariable)(
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
    MI_IAT_SYMBOL(RtlWakeAllConditionVariable, 4);


#endif

}
EXTERN_C_END
