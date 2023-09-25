#include <thunks/thunks.hpp>


typedef struct _MI_HEAP
{
    ULONG            Tag;
    POOL_TYPE        MemoryType;
    EX_POOL_PRIORITY Priority;
    ULONG            Flags;

    union
    {
        FAST_MUTEX   FastMutex; // PagedPool
        EX_SPIN_LOCK SpinLock;  // NonPagedPool
    };

    LIST_ENTRY Blocks;

    PVOID (NTAPI * Allocate)(POOL_TYPE MemoryType, SIZE_T Size, ULONG Tag);
    VOID  (NTAPI * Destroy )(PVOID Address, ULONG Tag);

} MI_HEAP, * PMI_HEAP;
typedef MI_HEAP const* PCMI_HEAP;

typedef struct _MI_HEAP_ENTRY
{
    LIST_ENTRY  Entry;
    SIZE_T      Size;
    PVOID       UserData;

    // This field:
    //     Normal: UserData -> Buffer
    // Large-Page: Empty

}MI_HEAP_ENTRY, * PMI_HEAP_ENTRY;
typedef MI_HEAP_ENTRY const* PCMI_HEAP_ENTRY;


EXTERN_C_START

PVOID NTAPI MI_NAME(RtlCreateHeap)(
    _In_     ULONG  Flags,
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
    PAGED_CODE()

    UNREFERENCED_PARAMETER(ReserveSize);
    UNREFERENCED_PARAMETER(CommitSize);
    UNREFERENCED_PARAMETER(Lock);

    PMI_HEAP Heap;

    do {
        NT_ASSERT(HeapBase == nullptr);
        NT_ASSERT(BooleanFlagOn(Flags, HEAP_SETTABLE_USER_VALUE));

        Heap = static_cast<PMI_HEAP>(ExAllocatePoolZero(NonPagedPool, sizeof(MI_HEAP), MI_TAG));
        if (Heap == nullptr) {
            break;
        }

        const auto SegmentHeapArg = reinterpret_cast<PRTL_SEGMENT_HEAP_PARAMETERS>(Parameters);

    #pragma warning(suppress: 4996 28751)
        Heap->Allocate   = ExAllocatePoolWithTag;
        Heap->Destroy    = ExFreePoolWithTag;
        Heap->MemoryType = BooleanFlagOn(SegmentHeapArg->MemorySource.MemoryTypeMask, MemoryTypeNonPaged) ? NonPagedPool : PagedPool;
        Heap->Priority   = static_cast<EX_POOL_PRIORITY>(SegmentHeapArg->MemorySource.Reserved[0]);
        Heap->Tag        = static_cast<ULONG>(SegmentHeapArg->MemorySource.Reserved[1]);
        Heap->Flags      = Flags;
        Heap->FastMutex  = {};

        InitializeListHead(&Heap->Blocks);

        const auto Peb = NtCurrentPeb();
        RtlAcquirePebLock();
        __try {
            if (Peb->NumberOfHeaps + 1 > Peb->MaximumNumberOfHeaps) {
                ExFreePoolWithTag(Heap, MI_TAG);
                Heap = nullptr;
            }
            else {
                Peb->ProcessHeaps[Peb->NumberOfHeaps++] = Heap;
            }
        }
        __finally {
            RtlReleasePebLock();
        }

    } while (false);

    return Heap;
}
MI_IAT_SYMBOL(RtlCreateHeap, 24);

PVOID NTAPI MI_NAME(RtlDestroyHeap)(
    _In_ _Post_invalid_ PVOID HeapHandle
    )
{
    PAGED_CODE()

    if (HeapHandle == nullptr) {
        return nullptr;
    }

    auto Removed = false;

    const auto Peb = NtCurrentPeb();
    RtlAcquirePebLock();
    __try {
        if (HeapHandle != Peb->ProcessHeap) {
            for (size_t Idx = 0ul; Idx < Peb->NumberOfHeaps; ++Idx) {
                if (HeapHandle == Peb->ProcessHeaps[Idx]) {
                    Peb->NumberOfHeaps    -= 1;
                    Peb->ProcessHeaps[Idx] = nullptr;

                    Removed = true;
                    break;
                }
            }
        }
        else {
            if (reinterpret_cast<size_t>(HeapHandle) & 1) {
                *static_cast<size_t*>(static_cast<void*>(&HeapHandle)) &= ~static_cast<size_t>(1);

                Removed = true; // Shutdown: FreeProcessEnvironmentBlock()
            }
        }
    }
    __finally {
        RtlReleasePebLock();
    }

    if (Removed) {
        const auto Heap = static_cast<PMI_HEAP>(HeapHandle);

        KIRQL Irql = PASSIVE_LEVEL;
        if (Heap->MemoryType == NonPagedPool) {
            Irql = ExAcquireSpinLockExclusive(&Heap->SpinLock);
        }
        else {
            ExAcquireFastMutex(&Heap->FastMutex);
        }
        __try {
            while (!IsListEmpty(&Heap->Blocks)) {
                const auto Entry = RemoveTailList(&Heap->Blocks);
                const auto Block = CONTAINING_RECORD(Entry, MI_HEAP_ENTRY, Entry);

                if (Block->Size >= PAGE_SIZE) {
                    Heap->Destroy(Block->UserData, Heap->Tag);
                }
                Heap->Destroy(Block, Heap->Tag);
            }
        }
        __finally {
            if (Heap->MemoryType == NonPagedPool) {
                ExReleaseSpinLockExclusive(&Heap->SpinLock, Irql);
            }
            else {
                ExReleaseFastMutex(&Heap->FastMutex);
            }
        }

        ExFreePoolWithTag(Heap, MI_TAG);
        HeapHandle = nullptr;
    }

    return HeapHandle;
}
MI_IAT_SYMBOL(RtlDestroyHeap, 4);

PVOID NTAPI MI_NAME(RtlAllocateHeap)(
    _In_ PVOID HeapHandle,
    _In_opt_ ULONG Flags,
    _In_ SIZE_T Size
    )
{
    auto  Status = STATUS_SUCCESS;
    PVOID Result = nullptr;

    do {
        if (HeapHandle == nullptr) {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        const auto Heap = static_cast<PMI_HEAP>(HeapHandle);

        Flags |= Heap->Flags;

        auto BlockSize = sizeof(MI_HEAP_ENTRY);
        if (Size < PAGE_SIZE) {
            BlockSize += Size;
        }

        const auto Block = static_cast<PMI_HEAP_ENTRY>(Heap->Allocate(Heap->MemoryType, BlockSize, Heap->Tag));
        if (Block == nullptr) {
            Status = STATUS_NO_MEMORY;
            break;
        }

        Block->Size = Size;

        if (Size >= PAGE_SIZE) {
            Block->UserData = Heap->Allocate(Heap->MemoryType, Size, Heap->Tag);
        }
        else {
            Block->UserData = Add2Ptr(Block, sizeof(MI_HEAP_ENTRY));
        }

        if (Block->UserData == nullptr) {
            Heap->Destroy(Block, Heap->Tag);

            Status = STATUS_NO_MEMORY;
            break;
        }

        if (Flags & HEAP_ZERO_MEMORY) {
            RtlSecureZeroMemory(Block->UserData, Size);
        }

        InitializeListHead(&Block->Entry);

        KIRQL Irql = PASSIVE_LEVEL;
        if (Heap->MemoryType == NonPagedPool) {
            Irql = ExAcquireSpinLockExclusive(&Heap->SpinLock);
        }
        else {
            ExAcquireFastMutex(&Heap->FastMutex);
        }
        __try {
            InsertHeadList(&Heap->Blocks, &Block->Entry);
        }
        __finally {
            if (Heap->MemoryType == NonPagedPool) {
                ExReleaseSpinLockExclusive(&Heap->SpinLock, Irql);
            }
            else {
                ExReleaseFastMutex(&Heap->FastMutex);
            }
        }

        Result = Block->UserData;

    } while (false);

    if (!NT_SUCCESS(Status)) {
        if (Flags & HEAP_GENERATE_EXCEPTIONS) {
            EXCEPTION_RECORD ExceptionRecord{};

            ExceptionRecord.ExceptionCode    = Status;
            ExceptionRecord.ExceptionRecord  = nullptr;
            ExceptionRecord.NumberParameters = 1;
            ExceptionRecord.ExceptionFlags   = 0;
            ExceptionRecord.ExceptionInformation[0] = Size;

            RtlRaiseException(&ExceptionRecord);
        }
    }

    return Result;
}
MI_IAT_SYMBOL(RtlAllocateHeap, 12);

PVOID NTAPI MI_NAME(RtlReAllocateHeap)(
    _In_ PVOID HeapHandle,
    _In_ ULONG Flags,
    _Frees_ptr_opt_ PVOID BaseAddress,
    _In_ SIZE_T Size
    )
{
    auto  Status = STATUS_SUCCESS;
    PVOID Result = nullptr;

    do {
        if (BaseAddress == nullptr) {
            break;
        }

        if (Flags & HEAP_REALLOC_IN_PLACE_ONLY) {
            break;
        }

        const auto Heap = static_cast<PCMI_HEAP>(HeapHandle);

        Flags |= Heap->Flags;

        const auto OldSize = RtlSizeHeap(HeapHandle, 0, BaseAddress);
        if (OldSize == static_cast<SIZE_T>(-1)) {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        Result = RtlAllocateHeap(HeapHandle, Flags, (Size ? Size : 1));
        if (Result == nullptr) {
            Status = STATUS_NO_MEMORY;
            break;
        }

        RtlMoveMemory(Result, BaseAddress, Size < OldSize ? Size : OldSize);

        RtlFreeHeap(HeapHandle, Flags, BaseAddress);

    } while (false);
    
    if (!NT_SUCCESS(Status)) {
        if (Flags & HEAP_GENERATE_EXCEPTIONS) {
            EXCEPTION_RECORD ExceptionRecord{};

            ExceptionRecord.ExceptionCode    = Status;
            ExceptionRecord.ExceptionRecord  = nullptr;
            ExceptionRecord.NumberParameters = 1;
            ExceptionRecord.ExceptionFlags   = 0;
            ExceptionRecord.ExceptionInformation[0] = Size;

            RtlRaiseException(&ExceptionRecord);
        }
    }

    return Result;
}
MI_IAT_SYMBOL(RtlReAllocateHeap, 16);

LOGICAL NTAPI MI_NAME(RtlFreeHeap)(
    _In_ PVOID HeapHandle,
    _In_opt_ ULONG Flags,
    _Frees_ptr_opt_ PVOID BaseAddress
    )
{
    UNREFERENCED_PARAMETER(Flags);

    auto  Status = STATUS_NOT_FOUND;
    KIRQL Irql   = PASSIVE_LEVEL;

    const auto Heap = static_cast<PMI_HEAP>(HeapHandle);
    if (Heap->MemoryType == NonPagedPool) {
        Irql = ExAcquireSpinLockExclusive(&Heap->SpinLock);
    }
    else {
        ExAcquireFastMutex(&Heap->FastMutex);
    }
    __try {
        for (auto Entry = Heap->Blocks.Flink; Entry != &Heap->Blocks; Entry = Entry->Flink){
            const auto Block = CONTAINING_RECORD(Entry, MI_HEAP_ENTRY, Entry);

            if (Block->UserData == BaseAddress) {
                RemoveEntryList(Entry);

                if (Block->Size >= PAGE_SIZE) {
                    Heap->Destroy(Block->UserData, Heap->Tag);
                }
                Heap->Destroy(Block, Heap->Tag);

                Status = STATUS_SUCCESS;
                break;
            }

            Entry = Entry->Flink;
        }
    }
    __finally {
        if (Heap->MemoryType == NonPagedPool) {
            ExReleaseSpinLockExclusive(&Heap->SpinLock, Irql);
        }
        else {
            ExReleaseFastMutex(&Heap->FastMutex);
        }
    }

    return NT_SUCCESS(Status);
}
MI_IAT_SYMBOL(RtlFreeHeap, 12);

SIZE_T NTAPI MI_NAME(RtlSizeHeap)(
    _In_ PVOID HeapHandle,
    _In_ ULONG Flags,
    _In_ PVOID BaseAddress
    )
{
    UNREFERENCED_PARAMETER(Flags);

    auto  Result = static_cast<SIZE_T>(-1);
    KIRQL Irql   = PASSIVE_LEVEL;

    const auto Heap = static_cast<PMI_HEAP>(HeapHandle);
    if (Heap->MemoryType == NonPagedPool) {
        Irql = ExAcquireSpinLockShared(&Heap->SpinLock);
    }
    else {
        ExAcquireFastMutex(&Heap->FastMutex);
    }
    __try {
        for (auto Entry = Heap->Blocks.Flink; Entry != &Heap->Blocks; Entry = Entry->Flink) {
            const auto Block = CONTAINING_RECORD(Entry, MI_HEAP_ENTRY, Entry);

            if (Block->UserData == BaseAddress) {
                Result = Block->Size;
                break;
            }

            Entry = Entry->Flink;
        }
    }
    __finally {
        if (Heap->MemoryType == NonPagedPool) {
            ExReleaseSpinLockShared(&Heap->SpinLock, Irql);
        }
        else {
            ExReleaseFastMutex(&Heap->FastMutex);
        }
    }

    return Result;
}
MI_IAT_SYMBOL(RtlSizeHeap, 12);

NTSTATUS NTAPI MI_NAME(RtlZeroHeap)(
    _In_ PVOID HeapHandle,
    _In_ ULONG Flags
    )
{
    UNREFERENCED_PARAMETER(HeapHandle);
    UNREFERENCED_PARAMETER(Flags);

    return STATUS_SUCCESS;
}
MI_IAT_SYMBOL(RtlZeroHeap, 12);


EXTERN_C_END
