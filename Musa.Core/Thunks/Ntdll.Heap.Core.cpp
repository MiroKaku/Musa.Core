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

_IRQL_requires_max_(DISPATCH_LEVEL)
PVOID MUSA_API MUSA_NAME(RtlProcessHeap)(VOID)
{
    const auto Peb = static_cast<Core::PKPEB>(MUSA_NAME_PRIVATE(RtlGetCurrentPeb)());
    if (Peb == nullptr) {
        return nullptr;
    }
    return Peb->DefaultHeap;
}

MUSA_IAT_SYMBOL(RtlProcessHeap, 0);

_IRQL_requires_max_(APC_LEVEL)
_Must_inspect_result_
PVOID NTAPI MUSA_NAME(RtlCreateHeap)(
    _In_ ULONG      Flags,
    _In_opt_ PVOID  HeapBase,
    _In_opt_ SIZE_T ReserveSize,
    _In_opt_ SIZE_T CommitSize,
    _In_opt_ PVOID  Lock,
    _When_((Flags & 0x100) != 0,
        _In_reads_bytes_opt_(sizeof(RTL_SEGMENT_HEAP_PARAMETERS)))
    _When_((Flags & 0x100) == 0,
        _In_reads_bytes_opt_(sizeof(RTL_HEAP_PARAMETERS)))
    PRTL_HEAP_PARAMETERS Parameters
)
{
    UNREFERENCED_PARAMETER(ReserveSize);
    UNREFERENCED_PARAMETER(CommitSize);

    PAGED_CODE();

    NTSTATUS Status;
    PHEAP    Heap       = nullptr;
    PVOID    HeapHandle = nullptr;

    do {
        if (HeapBase) {
            Status = STATUS_NOT_SUPPORTED;

            KdBreakPoint();
            break;
        }

        if (BooleanFlagOn(Flags, ~MUSA_HEAP_CREATE_VALID_MASK)) {
            Status = STATUS_NOT_SUPPORTED;

            KdBreakPoint();
            break;
        }

        if (Parameters != nullptr) {
            if (BooleanFlagOn(Flags, HEAP_CREATE_SEGMENT_HEAP) == FALSE) {
                Status = STATUS_NOT_SUPPORTED;

                KdBreakPoint();
                break;
            }
        }

        Flags &= (HEAP_NO_SERIALIZE |
            HEAP_GENERATE_EXCEPTIONS |
            HEAP_ZERO_MEMORY);

        auto SizeOfHeap = sizeof(HEAP);

        if (!BooleanFlagOn(Flags, HEAP_NO_SERIALIZE)) {
            if (Lock != nullptr) {
                SetFlag(Flags, HEAP_LOCK_USER_ALLOCATED);
            } else {
                SizeOfHeap += sizeof(HEAP_LOCK);
            }
        } else if (ARGUMENT_PRESENT(Lock)) {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        Heap = static_cast<PHEAP>(ExAllocatePoolZero(NonPagedPool, SizeOfHeap, MUSA_TAG));
        if (Heap == nullptr) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        #pragma warning( suppress : 4996 28751 )
        Heap->Allocate     = ExAllocatePoolWithTag;
        Heap->Destroy      = ExFreePoolWithTag;
        Heap->Tag          = MUSA_TAG;
        Heap->Type         = NonPagedPool;
        Heap->Priority     = NormalPoolPriority;
        Heap->Flags        = Flags;
        Heap->LockVariable = static_cast<PHEAP_LOCK>(Lock);

        InitializeListHead(&Heap->Blocks);

        if (Parameters) {
            const auto SegmentHeapParameters = reinterpret_cast<PRTL_SEGMENT_HEAP_PARAMETERS>(Parameters);
            if (!BooleanFlagOn(SegmentHeapParameters->MemorySource.MemoryTypeMask, MemoryTypeNonPaged)) {
                Heap->Type = PagedPool;
            }
            Heap->Tag = static_cast<ULONG>(SegmentHeapParameters->MemorySource.Reserved[1]);
        }

        if (!BooleanFlagOn(Heap->Flags, HEAP_LOCK_USER_ALLOCATED)) {
            Heap->LockVariable = static_cast<PHEAP_LOCK>(Add2Ptr(Heap, sizeof(HEAP)));
            MUSA_NAME_PRIVATE(RtlInitializeHeapLock)(Heap);
        }

        Status = MUSA_NAME_PRIVATE(RtlAppendHeap)(Heap);
        if (!NT_SUCCESS(Status)) {
            break;
        }

        HeapHandle = FastEncodePointer(Heap);
    } while (false);

    if (!NT_SUCCESS(Status)) {
        if (Heap) {
            if (!BooleanFlagOn(Heap->Flags, HEAP_LOCK_USER_ALLOCATED)) {
                MUSA_NAME_PRIVATE(RtlDestroyHeapLock)(Heap);
            }
            ExFreePoolWithTag(Heap, MUSA_TAG);
        }

        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
    }

    return HeapHandle;
}

MUSA_IAT_SYMBOL(RtlCreateHeap, 24);

_IRQL_requires_max_(APC_LEVEL)
PVOID NTAPI MUSA_NAME(RtlDestroyHeap)(
    _In_ _Post_invalid_ PVOID HeapHandle
)
{
    PAGED_CODE();

    do {
        if (HeapHandle == nullptr) {
            break;
        }

        const auto Heap = static_cast<PHEAP>(FastDecodePointer(HeapHandle));
        if (Heap == nullptr) {
            break;
        }

        KIRQL   LockIrql     = PASSIVE_LEVEL;
        BOOLEAN LockAcquired = FALSE;

        if (!BooleanFlagOn(Heap->Flags, HEAP_NO_SERIALIZE)) {
            LockIrql     = MUSA_NAME_PRIVATE(RtlAcquireHeapLockExclusive)(Heap);
            LockAcquired = TRUE;
        }
        __try {
            while (!IsListEmpty(&Heap->Blocks)) {
                const auto Entry = RemoveTailList(&Heap->Blocks);
                const auto Block = CONTAINING_RECORD(Entry, HEAP_ENTRY, Entry);

                if (Block->Size >= PAGE_SIZE) {
                    Heap->Destroy(Block->UserData, Heap->Tag);
                }
                Heap->Destroy(Block, Heap->Tag);
            }
        } __finally {
            if (LockAcquired) {
                MUSA_NAME_PRIVATE(RtlReleaseHeapLockExclusive)(Heap, LockIrql);
            }
        }

        MUSA_NAME_PRIVATE(RtlRemoveHeap)(Heap);

        if (!BooleanFlagOn(Heap->Flags, HEAP_NO_SERIALIZE)) {
            if (!BooleanFlagOn(Heap->Flags, HEAP_LOCK_USER_ALLOCATED)) {
                MUSA_NAME_PRIVATE(RtlDestroyHeapLock)(Heap);
            }
            Heap->LockVariable = nullptr;
        }

        ExFreePoolWithTag(Heap, MUSA_TAG);
    } while (false);

    return nullptr;
}

MUSA_IAT_SYMBOL(RtlDestroyHeap, 4);

_IRQL_requires_max_(APC_LEVEL)
ULONG NTAPI MUSA_NAME(RtlGetProcessHeaps)(
    _In_ ULONG   NumberOfHeaps,
    _Out_ PVOID* ProcessHeaps
)
{
    PAGED_CODE();

    ULONG TotalHeaps = 0;

    const auto Peb = (PKPEB)MUSA_NAME_PRIVATE(RtlGetCurrentPeb)();
    __try {
        MUSA_NAME_PRIVATE(RtlAcquirePebLockShared)();

        ULONG NumberOfHeapsToCopy;

        TotalHeaps = Peb->NumberOfHeaps;

        if (TotalHeaps > NumberOfHeaps) {
            NumberOfHeapsToCopy = NumberOfHeaps;
        } else {
            NumberOfHeapsToCopy = TotalHeaps;
        }

        RtlCopyMemory(ProcessHeaps, Peb->ProcessHeaps,
            NumberOfHeapsToCopy * sizeof(*ProcessHeaps));
    } __finally {
        MUSA_NAME_PRIVATE(RtlReleasePebLockShared)();
    }

    return TotalHeaps;
}

MUSA_IAT_SYMBOL(RtlGetProcessHeaps, 8);

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlEnumProcessHeaps)(
    _In_ PRTL_ENUM_HEAPS_ROUTINE EnumRoutine,
    _In_ PVOID                   Parameter
)
{
    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    const auto Peb = (PKPEB)MUSA_NAME_PRIVATE(RtlGetCurrentPeb)();
    __try {
        MUSA_NAME_PRIVATE(RtlAcquirePebLockShared)();

        for (auto Idx = 0ul; Idx < Peb->NumberOfHeaps; ++Idx) {
            Status = (*EnumRoutine)(Peb->ProcessHeaps[Idx], Parameter);
            if (!NT_SUCCESS(Status)) {
                break;
            }
        }
    } __finally {
        MUSA_NAME_PRIVATE(RtlReleasePebLockShared)();
    }

    return Status;
}

MUSA_IAT_SYMBOL(RtlEnumProcessHeaps, 8);

EXTERN_C_END
