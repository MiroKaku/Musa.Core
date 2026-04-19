#pragma once

VEIL_DECLARE_STRUCT_ALIGN(HEAP_LOCK, 8)
{
    union
    {
        ERESOURCE    Resource;  // PagedPool
        EX_SPIN_LOCK SpinLock;  // NonPagedPool
    } Lock;
};

VEIL_DECLARE_STRUCT_ALIGN(HEAP_ENTRY, 8)
{
    LIST_ENTRY Entry;
    SIZE_T     Size;
    PVOID      UserData;
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

    PVOID (NTAPI*Allocate)(
        _In_ __drv_strictTypeMatch(__drv_typeExpr) POOL_TYPE PoolType,
        _In_ SIZE_T                                          NumberOfBytes,
        _In_ ULONG                                           Tag
    );

    VOID (NTAPI*Destroy)(
        _Pre_notnull_ __drv_freesMem(Mem) PVOID Buffer,
        _In_ ULONG                              Tag
    );
};

#define MUSA_HEAP_CREATE_VALID_MASK  (HEAP_NO_SERIALIZE           | \
                                HEAP_GROWABLE               | \
                                HEAP_GENERATE_EXCEPTIONS    | \
                                HEAP_ZERO_MEMORY            | \
                                HEAP_CREATE_SEGMENT_HEAP)

#define HEAP_LOCK_USER_ALLOCATED    0x80000000

typedef struct _HEAP* PHEAP;

EXTERN_C_START

VOID NTAPI MUSA_NAME_PRIVATE(RtlInitializeHeapLock)(_Inout_ PHEAP Heap);
VOID NTAPI MUSA_NAME_PRIVATE(RtlDestroyHeapLock)(_Inout_ PHEAP Heap);
KIRQL NTAPI MUSA_NAME_PRIVATE(RtlAcquireHeapLockExclusive)(_Inout_ PHEAP Heap);
VOID NTAPI MUSA_NAME_PRIVATE(RtlReleaseHeapLockExclusive)(_Inout_ PHEAP Heap, _In_ KIRQL Irql);
KIRQL NTAPI MUSA_NAME_PRIVATE(RtlAcquireHeapLockShared)(_Inout_ PHEAP Heap);
VOID NTAPI MUSA_NAME_PRIVATE(RtlReleaseHeapLockShared)(_Inout_ PHEAP Heap, _In_ KIRQL Irql);
NTSTATUS NTAPI MUSA_NAME_PRIVATE(RtlAppendHeap)(_In_ PHEAP Heap);
VOID NTAPI MUSA_NAME_PRIVATE(RtlRemoveHeap)(_In_ PHEAP Heap);

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
    );

_IRQL_requires_max_(APC_LEVEL)
PVOID NTAPI MUSA_NAME(RtlDestroyHeap)(
    _In_ _Post_invalid_ PVOID HeapHandle
    );

_IRQL_requires_max_(APC_LEVEL)
ULONG NTAPI MUSA_NAME(RtlGetProcessHeaps)(
    _In_ ULONG   NumberOfHeaps,
    _Out_ PVOID* ProcessHeaps
    );

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlEnumProcessHeaps)(
    _In_ PRTL_ENUM_HEAPS_ROUTINE EnumRoutine,
    _In_ PVOID                   Parameter
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
_Must_inspect_result_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID NTAPI MUSA_NAME(RtlAllocateHeap)(
    _In_ PVOID     HeapHandle,
    _In_opt_ ULONG Flags,
    _In_ SIZE_T    Size
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
_Must_inspect_result_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID NTAPI MUSA_NAME(RtlReAllocateHeap)(
    _In_ PVOID            HeapHandle,
    _In_ ULONG            Flags,
    _Frees_ptr_opt_ PVOID BaseAddress,
    _In_ SIZE_T           Size
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
_Success_(return != 0)
LOGICAL NTAPI MUSA_NAME(RtlFreeHeap)(
    _In_ PVOID            HeapHandle,
    _In_opt_ ULONG        Flags,
    _Frees_ptr_opt_ PVOID BaseAddress
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
SIZE_T NTAPI MUSA_NAME(RtlSizeHeap)(
    _In_ PVOID HeapHandle,
    _In_ ULONG Flags,
    _In_ PVOID BaseAddress
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlZeroHeap)(
    _In_ PVOID HeapHandle,
    _In_ ULONG Flags
    );

VOID NTAPI MUSA_NAME(RtlProtectHeap)(
    _In_ PVOID   HeapHandle,
    _In_ BOOLEAN MakeReadOnly
    );

BOOLEAN NTAPI MUSA_NAME(RtlLockHeap)(
    _In_ PVOID HeapHandle
    );

BOOLEAN NTAPI MUSA_NAME(RtlUnlockHeap)(
    _In_ PVOID HeapHandle
    );

NTSTATUS NTAPI MUSA_NAME(RtlExtendHeap)(
    _In_ PVOID  HeapHandle,
    _In_ ULONG  Flags,
    _In_ PVOID  Base,
    _In_ SIZE_T Size
    );

SIZE_T NTAPI MUSA_NAME(RtlCompactHeap)(
    _In_ PVOID HeapHandle,
    _In_ ULONG Flags
    );

BOOLEAN NTAPI MUSA_NAME(RtlValidateHeap)(
    _In_opt_ PVOID HeapHandle,
    _In_ ULONG     Flags,
    _In_opt_ PVOID BaseAddress
    );

BOOLEAN NTAPI MUSA_NAME(RtlValidateProcessHeaps)(
    VOID
    );

NTSTATUS NTAPI MUSA_NAME(RtlQueryHeapInformation)(
    _In_opt_ PVOID              HeapHandle,
    _In_ HEAP_INFORMATION_CLASS HeapInformationClass,
    _Out_opt_ PVOID             HeapInformation,
    _In_opt_ SIZE_T             HeapInformationLength,
    _Out_opt_ PSIZE_T           ReturnLength
    );

NTSTATUS NTAPI MUSA_NAME(RtlSetHeapInformation)(
    _In_ PVOID                  HeapHandle,
    _In_ HEAP_INFORMATION_CLASS HeapInformationClass,
    _In_opt_ PVOID              HeapInformation,
    _In_opt_ SIZE_T             HeapInformationLength
    );

NTSTATUS NTAPI MUSA_NAME(RtlWalkHeap)(
    _In_ PVOID                   HeapHandle,
    _Inout_ PRTL_HEAP_WALK_ENTRY Entry
    );

VOID NTAPI MUSA_NAME(RtlDetectHeapLeaks)(
    VOID
    );

VOID NTAPI MUSA_NAME(RtlFlushHeaps)(
    VOID
    );

EXTERN_C_END
