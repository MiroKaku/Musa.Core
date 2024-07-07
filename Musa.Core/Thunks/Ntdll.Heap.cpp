#include "Ntdll.Heap.Private.h"
#include "Musa.Core/Musa.Core.SystemEnvironmentBlock.Private.h"
#include "Musa.Core/Musa.Core.Utility.h"


#ifdef _KERNEL_MODE

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(RtlCreateHeap))
#pragma alloc_text(PAGE, MUSA_NAME(RtlDestroyHeap))
#pragma alloc_text(PAGE, MUSA_NAME(RtlGetProcessHeaps))
#pragma alloc_text(PAGE, MUSA_NAME(RtlEnumProcessHeaps))
#endif


EXTERN_C_START
namespace Musa
{
    VEIL_DECLARE_STRUCT_ALIGN(HEAP_LOCK, 8)
    {
        union {
            ERESOURCE    Resource;  // PagedPool
            EX_SPIN_LOCK SpinLock;  // NonPagedPool
        } Lock;
    };

    VEIL_DECLARE_STRUCT_ALIGN(HEAP_ENTRY, 8)
    {
        LIST_ENTRY  Entry;
        SIZE_T      Size;
        PVOID       UserData;

        // This field:
        //     Normal: UserData -> Buffer
        // Large-Page: Empty
    };

    VEIL_DECLARE_STRUCT_ALIGN(HEAP, 8)
    {
        ULONG Index;

        ULONG Tag;
        ULONG Type;
        ULONG Priority;
        ULONG Flags;

        LIST_ENTRY Blocks;
        HEAP_LOCK* LockVariable;

        PVOID(NTAPI* Allocate)(
            _In_ __drv_strictTypeMatch(__drv_typeExpr) POOL_TYPE PoolType,
            _In_ SIZE_T NumberOfBytes,
            _In_ ULONG  Tag);

        VOID (NTAPI* Destroy)(
            _Pre_notnull_ __drv_freesMem(Mem) PVOID Buffer,
            _In_ ULONG Tag);

        // HEAP_LOCK Lock
    };


#define MUSA_HEAP_CREATE_VALID_MASK  (HEAP_NO_SERIALIZE           | \
                                    HEAP_GROWABLE               | \
                                    HEAP_GENERATE_EXCEPTIONS    | \
                                    HEAP_ZERO_MEMORY            | \
                                    HEAP_CREATE_SEGMENT_HEAP)

#define HEAP_LOCK_USER_ALLOCATED    0x80000000


    // Private functions

    static VOID NTAPI MUSA_NAME_PRIVATE(RtlInitializeHeapLock)(_Inout_ PHEAP Heap)
    {
        if (Heap->Type == PagedPool) {
            (VOID)ExInitializeResourceLite(&Heap->LockVariable->Lock.Resource);
        }
        else {
            Heap->LockVariable->Lock.SpinLock = 0;
        }
    }

    static VOID NTAPI MUSA_NAME_PRIVATE(RtlDestroyHeapLock)(_Inout_ PHEAP Heap)
    {
        if (Heap->Type == PagedPool) {
            (void)ExDeleteResourceLite(&Heap->LockVariable->Lock.Resource);
        }
    }

    static KIRQL NTAPI MUSA_NAME_PRIVATE(RtlAcquireHeapLockExclusive)(_Inout_ PHEAP Heap)
    {
        if (Heap->Type == PagedPool) {
            ExEnterCriticalRegionAndAcquireResourceExclusive(&Heap->LockVariable->Lock.Resource);
            return KeGetCurrentIrql();
        }
        else {
            return ExAcquireSpinLockExclusive(&Heap->LockVariable->Lock.SpinLock);
        }
    }

    static VOID NTAPI MUSA_NAME_PRIVATE(RtlReleaseHeapLockExclusive)(_Inout_ PHEAP Heap, _In_ KIRQL Irql)
    {
        if (Heap->Type == PagedPool) {
            ExReleaseResourceAndLeaveCriticalRegion(&Heap->LockVariable->Lock.Resource);
        }
        else {
            ExReleaseSpinLockExclusive(&Heap->LockVariable->Lock.SpinLock, Irql);
        }
    }

    static KIRQL NTAPI MUSA_NAME_PRIVATE(RtlAcquireHeapLockShared)(_Inout_ PHEAP Heap)
    {
        if (Heap->Type == PagedPool) {
            ExEnterCriticalRegionAndAcquireResourceShared(&Heap->LockVariable->Lock.Resource);
            return KeGetCurrentIrql();
        }
        else {
            return ExAcquireSpinLockShared(&Heap->LockVariable->Lock.SpinLock);
        }
    }

    static VOID NTAPI MUSA_NAME_PRIVATE(RtlReleaseHeapLockShared)(_Inout_ PHEAP Heap, _In_ KIRQL Irql)
    {
        if (Heap->Type == PagedPool) {
            ExReleaseResourceAndLeaveCriticalRegion(&Heap->LockVariable->Lock.Resource);
        }
        else {
            ExReleaseSpinLockShared(&Heap->LockVariable->Lock.SpinLock, Irql);
        }
    }

    static NTSTATUS NTAPI MUSA_NAME_PRIVATE(RtlAppendHeap)(_In_ PHEAP Heap)
    {
        NTSTATUS Status = STATUS_SUCCESS;

        const auto Peb = MUSA_NAME_PRIVATE(RtlGetCurrentPeb)();
        __try {
            MUSA_NAME_PRIVATE(RtlAcquirePebLockExclusive)();

            //
            //  If the processes heap list is already full then we'll
            //  double the size of the heap list for the process
            //

            if (Peb->NumberOfHeaps == Peb->MaximumNumberOfHeaps) {
                Peb->MaximumNumberOfHeaps *= 2;

                //
                //  Copy over the old buffer to the new buffer
                //

                auto NewList = static_cast<PHEAP*>(RtlAllocateHeap(RtlGetDefaultHeap(), 0,
                    Peb->MaximumNumberOfHeaps * sizeof(PHEAP)));
                if (NewList == nullptr) {
                    Peb->MaximumNumberOfHeaps = Peb->NumberOfHeaps;

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    __leave;
                }

                RtlCopyMemory(NewList,
                    Peb->ProcessHeaps,
                    Peb->NumberOfHeaps * sizeof(PHEAP));

                //
                //  Check if we should free the previous heap list buffer
                //

                if (Peb->ProcessHeaps != Add2Ptr(Peb, sizeof(KPEB))) {
                    RtlFreeHeap(RtlGetDefaultHeap(), 0, Peb->ProcessHeaps);
                }

                //
                //  Set the new list
                //

                Peb->ProcessHeaps = reinterpret_cast<PVOID*>(NewList);
            }

            //
            //  Add the input heap to the next free heap list slot, and note that
            //  the processes heap list index is really one beyond the actualy
            //  index used to get the processes heap
            //

            Peb->ProcessHeaps[Peb->NumberOfHeaps++] = FastEncodePointer(Heap);
            Heap->Index = Peb->NumberOfHeaps;
        }
        __finally {
            MUSA_NAME_PRIVATE(RtlReleasePebLockExclusive)();
        }

        return Status;
    }

    static VOID NTAPI MUSA_NAME_PRIVATE(RtlRemoveHeap)(_In_ PHEAP Heap)
    {
        const auto Peb = MUSA_NAME_PRIVATE(RtlGetCurrentPeb)();
        __try {
            MUSA_NAME_PRIVATE(RtlAcquirePebLockExclusive)();

            //
            //  We only want to the the work if the current process actually has some
            //  heaps, the index stored in the heap is within the range for active
            //  heaps.
            //

            if ((Peb->NumberOfHeaps != 0) &&
                (Heap->Index != 0) &&
                (Heap->Index <= Peb->NumberOfHeaps)) {

                //
                //  Establish a pointer into the array of process heaps at the
                //  current heap location and one beyond
                //

                auto ThisLocation = reinterpret_cast<PHEAP*>(&Peb->ProcessHeaps[Heap->Index - 1]);
                auto NextLocation = ThisLocation + 1;

                //
                //  Calculate the number of heaps that exist beyond the current
                //  heap in the array including the current heap location
                //

                auto Leftover = Peb->NumberOfHeaps - (Heap->Index - 1);

                //
                //  For every heap beyond the current one that we are removing
                //  we'll move that heap down to the previous index.
                //

                while (--Leftover) {

                    //
                    //  Copy the heap process array entry of the next entry to
                    //  the current entry, and move NextLocation to the next next entry
                    //

                    *ThisLocation = *NextLocation++;

                    //
                    //  Assign the moved heap its new heap index
                    //

                    (*ThisLocation)->Index -= 1;

                    //
                    //  Move on to the next heap entry
                    //

                    ThisLocation += 1;
                }

                //
                //  Zero out the last process heap pointer, update the count, and
                //  make the heap we just removed realize it has been removed by
                //  zeroing out its process heap list index
                //

                Heap->Index = 0;
                Peb->ProcessHeaps[--Peb->NumberOfHeaps] = nullptr;
            }
        }
        __finally {
            MUSA_NAME_PRIVATE(RtlReleasePebLockExclusive)();
        }
    }


    // Public  functions

    _IRQL_requires_max_(APC_LEVEL)
    _Must_inspect_result_
    PVOID NTAPI MUSA_NAME(RtlCreateHeap)(
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

            Flags &= (HEAP_NO_SERIALIZE         | \
                      HEAP_GENERATE_EXCEPTIONS  | \
                      HEAP_ZERO_MEMORY);

            auto SizeOfHeap = sizeof(HEAP);

            if (!BooleanFlagOn(Flags, HEAP_NO_SERIALIZE)) {
                if (Lock != nullptr) {
                    SetFlag(Flags, HEAP_LOCK_USER_ALLOCATED);
                }
                else {
                    SizeOfHeap += sizeof(HEAP_LOCK);
                }
            }
            else if (ARGUMENT_PRESENT(Lock)) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            Heap = static_cast<PHEAP>(ExAllocatePoolZero(NonPagedPool, SizeOfHeap, MUSA_TAG));
            if (Heap == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

        #pragma warning( suppress : 4996 28751 )
            Heap->Allocate  = ExAllocatePoolWithTag;
            Heap->Destroy   = ExFreePoolWithTag;
            Heap->Tag       = MUSA_TAG;
            Heap->Type      = NonPagedPool;
            Heap->Priority  = NormalPoolPriority;
            Heap->Flags     = Flags;
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

            //
            //  For every allocation we remove it from the list and free the vm
            //

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
            }
            __finally {
                if (LockAcquired) {
                    MUSA_NAME_PRIVATE(RtlReleaseHeapLockExclusive)(Heap, LockIrql);
                }
            }

            //
            //  We need to remove this heap from the process heap list
            //

            MUSA_NAME_PRIVATE(RtlRemoveHeap)(Heap);

            //
            //  If the heap is serialized, delete the lock created
            //  by RtlCreateHeap.
            //
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
        _In_  ULONG  NumberOfHeaps,
        _Out_ PVOID* ProcessHeaps
    )
    {
        PAGED_CODE();

        ULONG TotalHeaps = 0;

        const auto Peb = MUSA_NAME_PRIVATE(RtlGetCurrentPeb)();
        __try {
            MUSA_NAME_PRIVATE(RtlAcquirePebLockShared)();

            ULONG NumberOfHeapsToCopy;

            //
            //  Return no more than the number of heaps currently in use
            //

            TotalHeaps = Peb->NumberOfHeaps;

            if (TotalHeaps > NumberOfHeaps) {
                NumberOfHeapsToCopy = NumberOfHeaps;
            }
            else {
                NumberOfHeapsToCopy = TotalHeaps;
            }

            //
            //  Return the heap pointers to the caller
            //

            RtlCopyMemory(ProcessHeaps, Peb->ProcessHeaps,
                NumberOfHeapsToCopy * sizeof(*ProcessHeaps));
        }
        __finally {
            MUSA_NAME_PRIVATE(RtlReleasePebLockShared)();
        }

        return TotalHeaps;
    }
    MUSA_IAT_SYMBOL(RtlGetProcessHeaps, 8);

    _IRQL_requires_max_(APC_LEVEL)
    NTSTATUS NTAPI MUSA_NAME(RtlEnumProcessHeaps)(
        _In_ PRTL_ENUM_HEAPS_ROUTINE EnumRoutine,
        _In_ PVOID Parameter
    )
    {
        PAGED_CODE();

        NTSTATUS Status = STATUS_SUCCESS;

        const auto Peb = MUSA_NAME_PRIVATE(RtlGetCurrentPeb)();
        __try {
            MUSA_NAME_PRIVATE(RtlAcquirePebLockShared)();

            //
            //  For each heap in the process invoke the callback routine
            //  and if the callback returns anything other than success
            //  then break out and return immediately to our caller
            //

            for (auto Idx = 0ul; Idx < Peb->NumberOfHeaps; ++Idx) {
                Status = (*EnumRoutine)(Peb->ProcessHeaps[Idx], Parameter);
                if (!NT_SUCCESS(Status)) {
                    break;
                }
            }
        }
        __finally {
            MUSA_NAME_PRIVATE(RtlReleasePebLockShared)();
        }

        return Status;
    }
    MUSA_IAT_SYMBOL(RtlEnumProcessHeaps, 8);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Must_inspect_result_
    _Ret_maybenull_
    _Post_writable_byte_size_(Size)
    PVOID NTAPI MUSA_NAME(RtlAllocateHeap)(
        _In_ PVOID HeapHandle,
        _In_opt_ ULONG Flags,
        _In_ SIZE_T Size
        )
    {
        NTSTATUS Status      = STATUS_SUCCESS;
        PVOID    BaseAddress = nullptr;

        auto BlockSize = sizeof(HEAP_ENTRY);
        if (Size < PAGE_SIZE) {
            BlockSize += (Size ? Size : 1);
        }

        do {
            if (HeapHandle == nullptr) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            const auto Heap = static_cast<PHEAP>(FastDecodePointer(HeapHandle));
            if (Heap == nullptr) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            SetFlag(Flags, Heap->Flags);

            if (Size > MAXINT_PTR) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            const auto Block = static_cast<PHEAP_ENTRY>(Heap->Allocate(static_cast<POOL_TYPE>(Heap->Type), BlockSize, Heap->Tag));
            if (Block == nullptr) {
                Status = STATUS_NO_MEMORY;
                break;
            }
            InitializeListHead(&Block->Entry);

            Block->Size = Size;

            if (Size < PAGE_SIZE) {
                Block->UserData = Add2Ptr(Block, sizeof(HEAP_ENTRY));
            }
            else {
                Block->UserData = Heap->Allocate(static_cast<POOL_TYPE>(Heap->Type), Size, Heap->Tag);
            }

            if (Block->UserData == nullptr) {
                Heap->Destroy(Block, Heap->Tag);

                Status = STATUS_NO_MEMORY;
                break;
            }

            BaseAddress = Block->UserData;

            //
            //  If the flags indicate that we should zero memory then do it now
            //

            if (Flags & HEAP_ZERO_MEMORY) {
                RtlZeroMemory(BaseAddress, Size);
            }

            KIRQL   LockIrql     = PASSIVE_LEVEL;
            BOOLEAN LockAcquired = FALSE;

            if (!BooleanFlagOn(Flags, HEAP_NO_SERIALIZE)) {
                LockIrql     = MUSA_NAME_PRIVATE(RtlAcquireHeapLockExclusive)(Heap);
                LockAcquired = TRUE;
            }
            __try {
                InsertHeadList(&Heap->Blocks, &Block->Entry);
            }
            __finally {
                if (LockAcquired) {
                    MUSA_NAME_PRIVATE(RtlReleaseHeapLockExclusive)(Heap, LockIrql);
                }
            }

        } while (false);

        if (Status == STATUS_NO_MEMORY) {
            if (Flags & HEAP_GENERATE_EXCEPTIONS) {
                EXCEPTION_RECORD ExceptionRecord{};

                ExceptionRecord.ExceptionCode           = STATUS_NO_MEMORY;
                ExceptionRecord.ExceptionAddress        = _ReturnAddress();
                ExceptionRecord.NumberParameters        = 2;
                ExceptionRecord.ExceptionFlags          = 0;
                ExceptionRecord.ExceptionInformation[0] = Size;
                ExceptionRecord.ExceptionInformation[1] = BlockSize;

                RtlRaiseException(&ExceptionRecord);
            }
        }

        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        }

        return BaseAddress;
    }
    MUSA_IAT_SYMBOL(RtlAllocateHeap, 12);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Must_inspect_result_
    _Ret_maybenull_
    _Post_writable_byte_size_(Size)
    PVOID NTAPI MUSA_NAME(RtlReAllocateHeap)(
        _In_ PVOID HeapHandle,
        _In_ ULONG Flags,
        _Frees_ptr_opt_ PVOID BaseAddress,
        _In_ SIZE_T Size
        )
    {
        NTSTATUS Status     = STATUS_SUCCESS;
        PVOID    NewAddress = nullptr;

        do {
            if (HeapHandle == nullptr) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            const auto Heap = static_cast<PHEAP>(FastDecodePointer(HeapHandle));
            if (Heap == nullptr) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            SetFlag(Flags, Heap->Flags);

            if (BooleanFlagOn(Flags, HEAP_REALLOC_IN_PLACE_ONLY)) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            if (Size > MAXINT_PTR) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            SIZE_T OldSize = 0;

            if (BaseAddress) {
            #pragma warning(suppress: 6001)
                OldSize = RtlSizeHeap(HeapHandle, 0, BaseAddress);
            }

            //
            //  Allocate from the heap space for the reallocation
            //

            NewAddress = RtlAllocateHeap(HeapHandle, Flags & ~HEAP_ZERO_MEMORY, (Size ? Size : 1));
            if (NewAddress == nullptr) {
                Status = RtlGetLastNtStatus();
                break;
            }

            //
            //  Copy over the user's data area to the new block
            //

            if (BaseAddress) {
                RtlCopyMemory(NewAddress, BaseAddress, Size < OldSize ? Size : OldSize);
            }

            //
            //  Check if we grew the block and we should zero
            //  the remaining part.
            //

            if (Size > OldSize && (Flags & HEAP_ZERO_MEMORY)) {
                RtlZeroMemory(static_cast<PCHAR>(NewAddress) + OldSize, Size - OldSize);
            }

            //
            //  Release the old block
            //

            RtlFreeHeap(HeapHandle, Flags, BaseAddress);

        } while (false);

        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        }

        return NewAddress;
    }
    MUSA_IAT_SYMBOL(RtlReAllocateHeap, 16);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Success_(return != 0)
    LOGICAL NTAPI MUSA_NAME(RtlFreeHeap)(
        _In_ PVOID HeapHandle,
        _In_opt_ ULONG Flags,
        _Frees_ptr_opt_ PVOID BaseAddress
        )
    {
        LOGICAL Result = FALSE;

        if (BaseAddress == nullptr) {
            return TRUE;
        }

        if (HeapHandle == nullptr) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_INVALID_PARAMETER);
            return Result;
        }

        const auto Heap = static_cast<PHEAP>(FastDecodePointer(HeapHandle));
        if (Heap == nullptr) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_INVALID_PARAMETER);
            return Result;
        }

        SetFlag(Flags, Heap->Flags);

        KIRQL   LockIrql     = PASSIVE_LEVEL;
        BOOLEAN LockAcquired = FALSE;

        if (!BooleanFlagOn(Flags, HEAP_NO_SERIALIZE)) {
            LockIrql     = MUSA_NAME_PRIVATE(RtlAcquireHeapLockExclusive)(Heap);
            LockAcquired = TRUE;
        }
        __try {
            for (auto Entry = Heap->Blocks.Flink; Entry != &Heap->Blocks; Entry = Entry->Flink) {
                const auto Block = CONTAINING_RECORD(Entry, HEAP_ENTRY, Entry);
                if (Block->UserData == BaseAddress) {
                    RemoveEntryList(Entry);

                    if (Block->Size >= PAGE_SIZE) {
                        Heap->Destroy(Block->UserData, Heap->Tag);
                    }
                    Heap->Destroy(Block, Heap->Tag);

                    Result = TRUE;
                    break;
                }
            }
        }
        __finally {
            if (LockAcquired) {
                MUSA_NAME_PRIVATE(RtlReleaseHeapLockExclusive)(Heap, LockIrql);
            }
        }

        if (!Result) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_INVALID_PARAMETER);
        }

        return Result;
    }
    MUSA_IAT_SYMBOL(RtlFreeHeap, 12);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    SIZE_T NTAPI MUSA_NAME(RtlSizeHeap)(
        _In_ PVOID HeapHandle,
        _In_ ULONG Flags,
        _In_ PVOID BaseAddress
        )
    {
        SIZE_T Result = static_cast<SIZE_T>(-1);

        if (BaseAddress == nullptr) {
            return 0;
        }

        if (HeapHandle == nullptr) {
            return Result;
        }

        const auto Heap = static_cast<PHEAP>(FastDecodePointer(HeapHandle));
        if (Heap == nullptr) {
            return Result;
        }

        SetFlag(Flags, Heap->Flags);

        KIRQL   LockIrql     = PASSIVE_LEVEL;
        BOOLEAN LockAcquired = FALSE;

        if (!BooleanFlagOn(Flags, HEAP_NO_SERIALIZE)) {
            LockIrql     = MUSA_NAME_PRIVATE(RtlAcquireHeapLockShared)(Heap);
            LockAcquired = TRUE;
        }
        __try {
            for (auto Entry = Heap->Blocks.Flink; Entry != &Heap->Blocks; Entry = Entry->Flink) {
                const auto Block = CONTAINING_RECORD(Entry, HEAP_ENTRY, Entry);
                if (Block->UserData == BaseAddress) {
                    Result = Block->Size;
                    break;
                }
            }
        }
        __finally {
            if (LockAcquired) {
                MUSA_NAME_PRIVATE(RtlReleaseHeapLockShared)(Heap, LockIrql);
            }
        }

        return Result;
    }
    MUSA_IAT_SYMBOL(RtlSizeHeap, 12);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    NTSTATUS NTAPI MUSA_NAME(RtlZeroHeap)(
        _In_ PVOID HeapHandle,
        _In_ ULONG Flags
        )
    {
        UNREFERENCED_PARAMETER(HeapHandle);
        UNREFERENCED_PARAMETER(Flags);

        return STATUS_SUCCESS;
    }
    MUSA_IAT_SYMBOL(RtlZeroHeap, 12);

    VOID NTAPI MUSA_NAME(RtlProtectHeap)(
        _In_ PVOID HeapHandle,
        _In_ BOOLEAN MakeReadOnly
        )
    {
        UNREFERENCED_PARAMETER(HeapHandle);
        UNREFERENCED_PARAMETER(MakeReadOnly);

        return;
    }
    MUSA_IAT_SYMBOL(RtlProtectHeap, 8);

    BOOLEAN NTAPI MUSA_NAME(RtlLockHeap)(
        _In_ PVOID HeapHandle
        )
    {
        UNREFERENCED_PARAMETER(HeapHandle);

        return TRUE;
    }
    MUSA_IAT_SYMBOL(RtlLockHeap, 4);

    BOOLEAN NTAPI MUSA_NAME(RtlUnlockHeap)(
        _In_ PVOID HeapHandle
        )
    {
        UNREFERENCED_PARAMETER(HeapHandle);

        return TRUE;
    }
    MUSA_IAT_SYMBOL(RtlUnlockHeap, 4);

    NTSTATUS NTAPI MUSA_NAME(RtlExtendHeap)(
        _In_ PVOID  HeapHandle,
        _In_ ULONG  Flags,
        _In_ PVOID  Base,
        _In_ SIZE_T Size
        )
    {
        UNREFERENCED_PARAMETER(HeapHandle);
        UNREFERENCED_PARAMETER(Flags);
        UNREFERENCED_PARAMETER(Base);
        UNREFERENCED_PARAMETER(Size);

        return STATUS_SUCCESS;
    }
    MUSA_IAT_SYMBOL(RtlExtendHeap, 16);

    SIZE_T NTAPI MUSA_NAME(RtlCompactHeap)(
        _In_ PVOID HeapHandle,
        _In_ ULONG Flags
        )
    {
        UNREFERENCED_PARAMETER(HeapHandle);
        UNREFERENCED_PARAMETER(Flags);

        return 0;
    }
    MUSA_IAT_SYMBOL(RtlCompactHeap, 8);

    BOOLEAN NTAPI MUSA_NAME(RtlValidateHeap)(
        _In_opt_ PVOID HeapHandle,
        _In_ ULONG Flags,
        _In_opt_ PVOID BaseAddress
        )
    {
        UNREFERENCED_PARAMETER(HeapHandle);
        UNREFERENCED_PARAMETER(Flags);
        UNREFERENCED_PARAMETER(BaseAddress);

        return TRUE;
    }
    MUSA_IAT_SYMBOL(RtlValidateHeap, 12);

    BOOLEAN NTAPI MUSA_NAME(RtlValidateProcessHeaps)(
        VOID
        )
    {
        return TRUE;
    }
    MUSA_IAT_SYMBOL(RtlValidateProcessHeaps, 0);

    NTSTATUS NTAPI MUSA_NAME(RtlQueryHeapInformation)(
        _In_opt_ PVOID HeapHandle,
        _In_ HEAP_INFORMATION_CLASS HeapInformationClass,
        _Out_opt_ PVOID HeapInformation,
        _In_opt_ SIZE_T HeapInformationLength,
        _Out_opt_ PSIZE_T ReturnLength
        )
    {
        NTSTATUS Status;

        do {
            if (HeapHandle == nullptr) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            const auto Heap = static_cast<PHEAP>(FastDecodePointer(HeapHandle));
            if (Heap == nullptr) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            switch (HeapInformationClass) {
                default:
                {
                    Status = STATUS_UNSUCCESSFUL;
                    break;
                }
                case HeapCompatibilityInformation:
                {
                    if (ReturnLength) {
                        *ReturnLength = sizeof(ULONG);
                    }

                    if (HeapInformationLength < sizeof(ULONG)) {
                        Status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    if (HeapInformation) {
                        *static_cast<PULONG>(HeapInformation) = 1;
                    }

                    Status = STATUS_SUCCESS;
                    break;
                }
            }

        } while (false);

        return Status;
    }
    MUSA_IAT_SYMBOL(RtlQueryHeapInformation, 20);

    NTSTATUS NTAPI MUSA_NAME(RtlSetHeapInformation)(
        _In_ PVOID HeapHandle,
        _In_ HEAP_INFORMATION_CLASS HeapInformationClass,
        _In_opt_ PVOID HeapInformation,
        _In_opt_ SIZE_T HeapInformationLength
        )
    {
        NTSTATUS Status;

        do {
            if (HeapHandle == nullptr) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            const auto Heap = static_cast<PHEAP>(FastDecodePointer(HeapHandle));
            if (Heap == nullptr) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            switch (HeapInformationClass) {
                default:
                {
                    Status = STATUS_UNSUCCESSFUL;
                    break;
                }
                case HeapCompatibilityInformation:
                {
                    if (HeapInformationLength < sizeof(ULONG)) {
                        Status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    UNREFERENCED_PARAMETER(HeapInformation);

                    Status = STATUS_UNSUCCESSFUL;
                    break;
                }
            }

        } while (false);

        return Status;
    }
    MUSA_IAT_SYMBOL(RtlSetHeapInformation, 16);

    NTSTATUS NTAPI MUSA_NAME(RtlWalkHeap)(
        _In_ PVOID HeapHandle,
        _Inout_ PRTL_HEAP_WALK_ENTRY Entry
        )
    {
        UNREFERENCED_PARAMETER(HeapHandle);
        UNREFERENCED_PARAMETER(Entry);

        return STATUS_NOT_IMPLEMENTED;
    }
    MUSA_IAT_SYMBOL(RtlWalkHeap, 8);

    VOID NTAPI MUSA_NAME(RtlDetectHeapLeaks)(
        VOID
        )
    {
        return;
    }
    MUSA_IAT_SYMBOL(RtlDetectHeapLeaks, 0);

    VOID NTAPI MUSA_NAME(RtlFlushHeaps)(
        VOID
        )
    {
        return;
    }
    MUSA_IAT_SYMBOL(RtlFlushHeaps, 0);

}
EXTERN_C_END

#endif // _KERNEL_MODE


EXTERN_C_START
namespace Musa
{
    // Public  functions

    PVOID NTAPI MUSA_NAME(RtlGetDefaultHeap)()
    {
    #ifdef _KERNEL_MODE
        return MUSA_NAME_PRIVATE(RtlGetCurrentPeb)()->DefaultHeap;
    #else
        return ZwCurrentPeb()->ProcessHeap;
    #endif
    }
    MUSA_IAT_SYMBOL(RtlGetDefaultHeap, 0);

}
EXTERN_C_END
