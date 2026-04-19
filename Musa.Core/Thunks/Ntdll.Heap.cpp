#include "Musa.Core/Musa.Core.SystemEnvironmentBlock.Process.h"
#include "Musa.Core/Musa.Utilities.h"
#include "Internal/Ntdll.Heap.h"

EXTERN_C_START

using namespace Musa;
using namespace Musa::Core;
using namespace Musa::Utils;

_IRQL_requires_max_(DISPATCH_LEVEL)
_Must_inspect_result_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID NTAPI MUSA_NAME(RtlAllocateHeap)(
    _In_ PVOID     HeapHandle,
    _In_opt_ ULONG Flags,
    _In_ SIZE_T    Size
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

        const auto Block = static_cast<PHEAP_ENTRY>(Heap->Allocate(static_cast<POOL_TYPE>(Heap->Type), BlockSize,
            Heap->Tag));
        if (Block == nullptr) {
            Status = STATUS_NO_MEMORY;
            break;
        }
        InitializeListHead(&Block->Entry);

        Block->Size = Size;

        if (Size < PAGE_SIZE) {
            Block->UserData = Add2Ptr(Block, sizeof(HEAP_ENTRY));
        } else {
            Block->UserData = Heap->Allocate(static_cast<POOL_TYPE>(Heap->Type), Size, Heap->Tag);
        }

        if (Block->UserData == nullptr) {
            Heap->Destroy(Block, Heap->Tag);

            Status = STATUS_NO_MEMORY;
            break;
        }

        BaseAddress = Block->UserData;

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
        } __finally {
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
    _In_ PVOID            HeapHandle,
    _In_ ULONG            Flags,
    _Frees_ptr_opt_ PVOID BaseAddress,
    _In_ SIZE_T           Size
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

        NewAddress = RtlAllocateHeap(HeapHandle, Flags & ~HEAP_ZERO_MEMORY, (Size ? Size : 1));
        if (NewAddress == nullptr) {
            Status = RtlGetLastNtStatus();
            break;
        }

        if (BaseAddress) {
            RtlCopyMemory(NewAddress, BaseAddress, Size < OldSize ? Size : OldSize);
        }

        if (Size > OldSize && (Flags & HEAP_ZERO_MEMORY)) {
            RtlZeroMemory(static_cast<PCHAR>(NewAddress) + OldSize, Size - OldSize);
        }

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
    _In_ PVOID            HeapHandle,
    _In_opt_ ULONG        Flags,
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
    } __finally {
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
    } __finally {
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

MUSA_IAT_SYMBOL(RtlZeroHeap, 8);

VOID NTAPI MUSA_NAME(RtlProtectHeap)(
    _In_ PVOID   HeapHandle,
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
    _In_ ULONG     Flags,
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
    _In_opt_ PVOID              HeapHandle,
    _In_ HEAP_INFORMATION_CLASS HeapInformationClass,
    _Out_opt_ PVOID             HeapInformation,
    _In_opt_ SIZE_T             HeapInformationLength,
    _Out_opt_ PSIZE_T           ReturnLength
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
            default: {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }
            case HeapCompatibilityInformation: {
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
    _In_ PVOID                  HeapHandle,
    _In_ HEAP_INFORMATION_CLASS HeapInformationClass,
    _In_opt_ PVOID              HeapInformation,
    _In_opt_ SIZE_T             HeapInformationLength
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
            default: {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }
            case HeapCompatibilityInformation: {
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
    _In_ PVOID                   HeapHandle,
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

EXTERN_C_END
