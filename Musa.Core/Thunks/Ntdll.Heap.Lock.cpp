#include "Musa.Core/Musa.Core.SystemEnvironmentBlock.Process.h"
#include "Musa.Core/Musa.Utilities.h"
#include "Internal/Ntdll.Heap.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(RtlCreateHeap))
#pragma alloc_text(PAGE, MUSA_NAME(RtlDestroyHeap))
#pragma alloc_text(PAGE, MUSA_NAME(RtlGetProcessHeaps))
#pragma alloc_text(PAGE, MUSA_NAME(RtlEnumProcessHeaps))
#endif

EXTERN_C_START

using namespace Musa;
using namespace Musa::Core;
using namespace Musa::Utils;

VOID NTAPI MUSA_NAME_PRIVATE(RtlInitializeHeapLock)(_Inout_ PHEAP Heap)
{
    if (Heap->Type == PagedPool) {
        (VOID)ExInitializeResourceLite(&Heap->LockVariable->Lock.Resource);
    } else {
        Heap->LockVariable->Lock.SpinLock = 0;
    }
}

VOID NTAPI MUSA_NAME_PRIVATE(RtlDestroyHeapLock)(_Inout_ PHEAP Heap)
{
    if (Heap->Type == PagedPool) {
        (void)ExDeleteResourceLite(&Heap->LockVariable->Lock.Resource);
    }
}

KIRQL NTAPI MUSA_NAME_PRIVATE(RtlAcquireHeapLockExclusive)(_Inout_ PHEAP Heap)
{
    if (Heap->Type == PagedPool) {
        ExEnterCriticalRegionAndAcquireResourceExclusive(&Heap->LockVariable->Lock.Resource);
        return KeGetCurrentIrql();
    } else {
        return ExAcquireSpinLockExclusive(&Heap->LockVariable->Lock.SpinLock);
    }
}

VOID NTAPI MUSA_NAME_PRIVATE(RtlReleaseHeapLockExclusive)(_Inout_ PHEAP Heap, _In_ KIRQL Irql)
{
    if (Heap->Type == PagedPool) {
        ExReleaseResourceAndLeaveCriticalRegion(&Heap->LockVariable->Lock.Resource);
    } else {
        ExReleaseSpinLockExclusive(&Heap->LockVariable->Lock.SpinLock, Irql);
    }
}

KIRQL NTAPI MUSA_NAME_PRIVATE(RtlAcquireHeapLockShared)(_Inout_ PHEAP Heap)
{
    if (Heap->Type == PagedPool) {
        ExEnterCriticalRegionAndAcquireResourceShared(&Heap->LockVariable->Lock.Resource);
        return KeGetCurrentIrql();
    } else {
        return ExAcquireSpinLockShared(&Heap->LockVariable->Lock.SpinLock);
    }
}

VOID NTAPI MUSA_NAME_PRIVATE(RtlReleaseHeapLockShared)(_Inout_ PHEAP Heap, _In_ KIRQL Irql)
{
    if (Heap->Type == PagedPool) {
        ExReleaseResourceAndLeaveCriticalRegion(&Heap->LockVariable->Lock.Resource);
    } else {
        ExReleaseSpinLockShared(&Heap->LockVariable->Lock.SpinLock, Irql);
    }
}

NTSTATUS NTAPI MUSA_NAME_PRIVATE(RtlAppendHeap)(_In_ PHEAP Heap)
{
    NTSTATUS Status = STATUS_SUCCESS;

    const auto Peb = (PKPEB)MUSA_NAME_PRIVATE(RtlGetCurrentPeb)();
    __try {
        MUSA_NAME_PRIVATE(RtlAcquirePebLockExclusive)();

        if (Peb->NumberOfHeaps == Peb->MaximumNumberOfHeaps) {
            Peb->MaximumNumberOfHeaps *= 2;

            auto NewList = static_cast<PHEAP*>(RtlAllocateHeap(RtlProcessHeap(), 0,
                Peb->MaximumNumberOfHeaps * sizeof(PHEAP)));
            if (NewList == nullptr) {
                Peb->MaximumNumberOfHeaps = Peb->NumberOfHeaps;

                Status = STATUS_INSUFFICIENT_RESOURCES;
                __leave;
            }

            RtlCopyMemory(NewList,
                Peb->ProcessHeaps,
                Peb->NumberOfHeaps * sizeof(PHEAP));

            if (Peb->ProcessHeaps != Add2Ptr(Peb, sizeof(KPEB))) {
                RtlFreeHeap(RtlProcessHeap(), 0, Peb->ProcessHeaps);
            }

            Peb->ProcessHeaps = reinterpret_cast<PVOID*>(NewList);
        }

        Peb->ProcessHeaps[Peb->NumberOfHeaps++] = FastEncodePointer(Heap);
        Heap->Index                             = Peb->NumberOfHeaps;
    } __finally {
        MUSA_NAME_PRIVATE(RtlReleasePebLockExclusive)();
    }

    return Status;
}

VOID NTAPI MUSA_NAME_PRIVATE(RtlRemoveHeap)(_In_ PHEAP Heap)
{
    const auto Peb = (PKPEB)MUSA_NAME_PRIVATE(RtlGetCurrentPeb)();
    __try {
        MUSA_NAME_PRIVATE(RtlAcquirePebLockExclusive)();

        if ((Peb->NumberOfHeaps != 0) &&
            (Heap->Index != 0) &&
            (Heap->Index <= Peb->NumberOfHeaps)) {

            auto ThisLocation = reinterpret_cast<PHEAP*>(&Peb->ProcessHeaps[Heap->Index - 1]);
            auto NextLocation = ThisLocation + 1;

            auto Leftover = Peb->NumberOfHeaps - (Heap->Index - 1);

            while (--Leftover) {
                *ThisLocation = *NextLocation++;
                (*ThisLocation)->Index -= 1;
                ThisLocation += 1;
            }

            Heap->Index                             = 0;
            Peb->ProcessHeaps[--Peb->NumberOfHeaps] = nullptr;
        }
    } __finally {
        MUSA_NAME_PRIVATE(RtlReleasePebLockExclusive)();
    }
}

EXTERN_C_END
