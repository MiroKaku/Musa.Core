#include "Internal/Ntdll.Synchronize.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(RtlEnterCriticalSection))
#pragma alloc_text(PAGE, MUSA_NAME(RtlLeaveCriticalSection))
#pragma alloc_text(PAGE, MUSA_NAME(RtlTryEnterCriticalSection))
#endif

EXTERN_C_START

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
    _In_ ULONG                    SpinCount
)
{
    return RtlInitializeCriticalSectionEx(CriticalSection, SpinCount, 0);
}

MUSA_IAT_SYMBOL(RtlInitializeCriticalSectionAndSpinCount, 8);

_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlInitializeCriticalSectionEx)(
    _Out_ PRTL_CRITICAL_SECTION CriticalSection,
    _In_ ULONG                  SpinCount,
    _In_ ULONG                  Flags
)
{
    UNREFERENCED_PARAMETER(SpinCount);
    UNREFERENCED_PARAMETER(Flags);

    NTSTATUS Status;

    *CriticalSection = {};
    do {
        CriticalSection->SpinCount     = SpinCount;
        CriticalSection->LockSemaphore = RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ERESOURCE));
        if (CriticalSection->LockSemaphore == nullptr) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        Status = ExInitializeResourceLite(static_cast<PERESOURCE>(CriticalSection->LockSemaphore));
        if (!NT_SUCCESS(Status)) {
            RtlFreeHeap(RtlProcessHeap(), 0, CriticalSection->LockSemaphore);
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
            RtlFreeHeap(RtlProcessHeap(), 0, CriticalSection->LockSemaphore);
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

    KeEnterCriticalRegion();
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
    KeLeaveCriticalRegion();

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

    KeEnterCriticalRegion();
    if (ExTryToAcquireResourceExclusiveLite(static_cast<PERESOURCE>(CriticalSection->LockSemaphore))) {
        return TRUE;
    }

    KeLeaveCriticalRegion();
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

_IRQL_requires_max_(DISPATCH_LEVEL)
ULONG NTAPI MUSA_NAME(RtlSetCriticalSectionSpinCount)(
    _Inout_ PRTL_CRITICAL_SECTION CriticalSection,
    _In_ ULONG SpinCount
)
{
    return (ULONG)InterlockedExchange((volatile long*)&CriticalSection->SpinCount, (long)SpinCount);
}

MUSA_IAT_SYMBOL(RtlSetCriticalSectionSpinCount, 8);

EXTERN_C_END
