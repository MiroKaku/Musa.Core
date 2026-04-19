#include "Internal/Ntdll.Synchronize.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(RtlAcquireSRWLockExclusive))
#pragma alloc_text(PAGE, MUSA_NAME(RtlAcquireSRWLockShared))
#pragma alloc_text(PAGE, MUSA_NAME(RtlReleaseSRWLockExclusive))
#pragma alloc_text(PAGE, MUSA_NAME(RtlReleaseSRWLockShared))
#pragma alloc_text(PAGE, MUSA_NAME(RtlTryAcquireSRWLockExclusive))
#pragma alloc_text(PAGE, MUSA_NAME(RtlTryAcquireSRWLockShared))
#endif

EXTERN_C_START

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

    KeEnterCriticalRegion();
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

    KeEnterCriticalRegion();
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
    KeLeaveCriticalRegion();
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
    KeLeaveCriticalRegion();
}

MUSA_IAT_SYMBOL(RtlReleaseSRWLockShared, 4);

_IRQL_requires_max_(APC_LEVEL)
_When_(return != 0, _Acquires_exclusive_lock_(*SRWLock))
BOOLEAN NTAPI MUSA_NAME(RtlTryAcquireSRWLockExclusive)(
    _Inout_ PRTL_SRWLOCK SRWLock
)
{
    PAGED_CODE();

    KeEnterCriticalRegion();
    if (ExTryAcquirePushLockExclusiveEx(reinterpret_cast<PEX_PUSH_LOCK>(SRWLock), EX_DEFAULT_PUSH_LOCK_FLAGS)) {
        return TRUE;
    }

    KeLeaveCriticalRegion();
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

    KeEnterCriticalRegion();
    if (ExTryAcquirePushLockSharedEx(reinterpret_cast<PEX_PUSH_LOCK>(SRWLock), EX_DEFAULT_PUSH_LOCK_FLAGS)) {
        return TRUE;
    }

    KeLeaveCriticalRegion();
    return FALSE;
}

MUSA_IAT_SYMBOL(RtlTryAcquireSRWLockShared, 4);

EXTERN_C_END
