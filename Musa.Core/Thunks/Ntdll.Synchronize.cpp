#include "Ntdll.Synchronize.Private.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(RtlAcquireSRWLockExclusive))
#pragma alloc_text(PAGE, MUSA_NAME(RtlAcquireSRWLockShared))
#pragma alloc_text(PAGE, MUSA_NAME(RtlReleaseSRWLockExclusive))
#pragma alloc_text(PAGE, MUSA_NAME(RtlReleaseSRWLockShared))
#pragma alloc_text(PAGE, MUSA_NAME(RtlTryAcquireSRWLockExclusive))
#pragma alloc_text(PAGE, MUSA_NAME(RtlTryAcquireSRWLockShared))

#pragma alloc_text(PAGE, MUSA_NAME(RtlEnterCriticalSection))
#pragma alloc_text(PAGE, MUSA_NAME(RtlLeaveCriticalSection))
#pragma alloc_text(PAGE, MUSA_NAME(RtlTryEnterCriticalSection))

#pragma alloc_text(PAGE, MUSA_NAME(RtlSleepConditionVariableCS))
#pragma alloc_text(PAGE, MUSA_NAME(RtlSleepConditionVariableSRW))
#pragma alloc_text(PAGE, MUSA_NAME(RtlWakeConditionVariable))
#pragma alloc_text(PAGE, MUSA_NAME(RtlWakeAllConditionVariable))
#endif


EXTERN_C_START
namespace Musa
{

#if defined _KERNEL_MODE

    PVOID NTAPI RtlGetDefaultHeap();

    //
    // R/W lock
    //

    STATIC_ASSERT(sizeof(RTL_SRWLOCK) >= sizeof(EX_PUSH_LOCK));

    _IRQL_requires_max_(DISPATCH_LEVEL)
    VOID WINAPI MUSA_NAME(RtlInitializeSRWLock)(
        _Out_ PRTL_SRWLOCK SRWLock
        )
    {
        ExInitializePushLock(reinterpret_cast<PEX_PUSH_LOCK>(SRWLock));
    }
    MUSA_IAT_SYMBOL(RtlInitializeSRWLock, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Acquires_exclusive_lock_(*SRWLock)
    VOID NTAPI MUSA_NAME(RtlAcquireSRWLockExclusive)(
        _Inout_ PRTL_SRWLOCK SRWLock
    )
    {
        PAGED_CODE();

        KeEnterGuardedRegion();
        ExAcquirePushLockExclusiveEx(reinterpret_cast<PEX_PUSH_LOCK>(SRWLock), EX_DEFAULT_PUSH_LOCK_FLAGS);
    }
    MUSA_IAT_SYMBOL(RtlAcquireSRWLockExclusive, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Acquires_shared_lock_(*SRWLock)
    VOID NTAPI MUSA_NAME(RtlAcquireSRWLockShared)(
        _Inout_ PRTL_SRWLOCK SRWLock
    )
    {
        PAGED_CODE();

        KeEnterGuardedRegion();
        ExAcquirePushLockSharedEx(reinterpret_cast<PEX_PUSH_LOCK>(SRWLock), EX_DEFAULT_PUSH_LOCK_FLAGS);
    }
    MUSA_IAT_SYMBOL(RtlAcquireSRWLockShared, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Releases_exclusive_lock_(*SRWLock)
    VOID NTAPI MUSA_NAME(RtlReleaseSRWLockExclusive)(
        _Inout_ PRTL_SRWLOCK SRWLock
    )
    {
        PAGED_CODE();

        ExReleasePushLockExclusiveEx(reinterpret_cast<PEX_PUSH_LOCK>(SRWLock), EX_DEFAULT_PUSH_LOCK_FLAGS);
        KeLeaveGuardedRegion();
    }
    MUSA_IAT_SYMBOL(RtlReleaseSRWLockExclusive, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Releases_shared_lock_(*SRWLock)
    VOID NTAPI MUSA_NAME(RtlReleaseSRWLockShared)(
        _Inout_ PRTL_SRWLOCK SRWLock
    )
    {
        PAGED_CODE();

        ExReleasePushLockSharedEx(reinterpret_cast<PEX_PUSH_LOCK>(SRWLock), EX_DEFAULT_PUSH_LOCK_FLAGS);
        KeLeaveGuardedRegion();
    }
    MUSA_IAT_SYMBOL(RtlReleaseSRWLockShared, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _When_(return != 0, _Acquires_exclusive_lock_(*SRWLock))
    BOOLEAN NTAPI MUSA_NAME(RtlTryAcquireSRWLockExclusive)(
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
    MUSA_IAT_SYMBOL(RtlTryAcquireSRWLockExclusive, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _When_(return != 0, _Acquires_shared_lock_(*SRWLock))
    BOOLEAN NTAPI MUSA_NAME(RtlTryAcquireSRWLockShared)(
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
    MUSA_IAT_SYMBOL(RtlTryAcquireSRWLockShared, 4);


    //
    // Critical Section
    //

    STATIC_ASSERT(sizeof(RTL_CRITICAL_SECTION) >= sizeof(EX_PUSH_LOCK));

    _IRQL_requires_max_(DISPATCH_LEVEL)
    NTSTATUS NTAPI MUSA_NAME(RtlInitializeCriticalSection)(
        _Out_ PRTL_CRITICAL_SECTION CriticalSection
    )
    {
        return RtlInitializeCriticalSectionEx(CriticalSection, 0, 0);
    }
    MUSA_IAT_SYMBOL(RtlInitializeCriticalSection, 4);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Must_inspect_result_
    NTSTATUS NTAPI MUSA_NAME(RtlInitializeCriticalSectionAndSpinCount)(
        _Inout_ PRTL_CRITICAL_SECTION CriticalSection,
        _In_ ULONG SpinCount
    )
    {
        return RtlInitializeCriticalSectionEx(CriticalSection, SpinCount, 0);
    }
    MUSA_IAT_SYMBOL(RtlInitializeCriticalSectionAndSpinCount, 8);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    NTSTATUS NTAPI MUSA_NAME(RtlInitializeCriticalSectionEx)(
        _Out_ PRTL_CRITICAL_SECTION CriticalSection,
        _In_ ULONG SpinCount,
        _In_ ULONG Flags
    )
    {
        UNREFERENCED_PARAMETER(SpinCount);
        UNREFERENCED_PARAMETER(Flags);

        NTSTATUS Status;

        *CriticalSection = {};
        do {
            CriticalSection->SpinCount = SpinCount;
            CriticalSection->LockSemaphore = RtlAllocateHeap(RtlGetDefaultHeap(), HEAP_ZERO_MEMORY, sizeof(ERESOURCE));
            if (CriticalSection->LockSemaphore == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            Status = ExInitializeResourceLite(static_cast<PERESOURCE>(CriticalSection->LockSemaphore));
            if (!NT_SUCCESS(Status)) {
                RtlFreeHeap(RtlGetDefaultHeap(), 0, CriticalSection->LockSemaphore);
                break;
            }

        } while (false);

        return Status;

    }
    MUSA_IAT_SYMBOL(RtlInitializeCriticalSectionEx, 12);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    NTSTATUS NTAPI MUSA_NAME(RtlDeleteCriticalSection)(
        _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    )
    {
        UNREFERENCED_PARAMETER(CriticalSection);

        NTSTATUS Status = STATUS_SUCCESS;

        if (CriticalSection->LockSemaphore) {

            Status = ExDeleteResourceLite(static_cast<PERESOURCE>(CriticalSection->LockSemaphore));
            if (NT_SUCCESS(Status)) {
                RtlFreeHeap(RtlGetDefaultHeap(), 0, CriticalSection->LockSemaphore);
            }
        }

        return Status;
    }
    MUSA_IAT_SYMBOL(RtlDeleteCriticalSection, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Acquires_exclusive_lock_(*CriticalSection)
    NTSTATUS NTAPI MUSA_NAME(RtlEnterCriticalSection)(
        _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    )
    {
        PAGED_CODE();

        KeEnterGuardedRegion();
        return ExAcquireResourceExclusiveLite(static_cast<PERESOURCE>(CriticalSection->LockSemaphore), TRUE)
            ? STATUS_SUCCESS
            : STATUS_UNSUCCESSFUL;
    }
    MUSA_IAT_SYMBOL(RtlEnterCriticalSection, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Releases_exclusive_lock_(*CriticalSection)
    NTSTATUS NTAPI MUSA_NAME(RtlLeaveCriticalSection)(
        _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    )
    {
        PAGED_CODE();

        ExReleaseResourceLite(static_cast<PERESOURCE>(CriticalSection->LockSemaphore));
        KeLeaveGuardedRegion();

        return STATUS_SUCCESS;
    }
    MUSA_IAT_SYMBOL(RtlLeaveCriticalSection, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _When_(return != 0, _Acquires_exclusive_lock_(*CriticalSection))
    LOGICAL NTAPI MUSA_NAME(RtlTryEnterCriticalSection)(
        _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    )
    {
        PAGED_CODE();

        KeEnterGuardedRegion();
        if (ExTryToAcquireResourceExclusiveLite(static_cast<PERESOURCE>(CriticalSection->LockSemaphore))) {
            return TRUE;
        }

        KeLeaveGuardedRegion();
        return FALSE;
    }
    MUSA_IAT_SYMBOL(RtlTryEnterCriticalSection, 4);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    LOGICAL NTAPI MUSA_NAME(RtlIsCriticalSectionLocked)(
        _Inout_ PRTL_CRITICAL_SECTION CriticalSection
    )
    {
        return ExIsResourceAcquiredExclusiveLite(static_cast<PERESOURCE>(CriticalSection->LockSemaphore));
    }
    MUSA_IAT_SYMBOL(RtlIsCriticalSectionLocked, 4);


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
    MUSA_IAT_SYMBOL(RtlSleepConditionVariableCS, 12);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    NTSTATUS NTAPI MUSA_NAME(RtlSleepConditionVariableSRW)(
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


#endif

}
EXTERN_C_END
