#pragma once

EXTERN_C_START


PVOID NTAPI MUSA_NAME(RtlGetDefaultHeap)();

PVOID NTAPI RtlGetDefaultHeap();

#ifdef _X86_
_VEIL_DECLARE_ALTERNATE_NAME(RtlGetDefaultHeap@0, _Musa_RtlGetDefaultHeap@0);
#else
_VEIL_DECLARE_ALTERNATE_NAME(RtlGetDefaultHeap, _Musa_RtlGetDefaultHeap);
#endif

#ifdef _KERNEL_MODE

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
    );

_IRQL_requires_max_(APC_LEVEL)
PVOID NTAPI MUSA_NAME(RtlDestroyHeap)(
    _In_ _Post_invalid_ PVOID HeapHandle
    );

_IRQL_requires_max_(APC_LEVEL)
ULONG NTAPI MUSA_NAME(RtlGetProcessHeaps)(
    _In_  ULONG  NumberOfHeaps,
    _Out_ PVOID* ProcessHeaps
    );

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlEnumProcessHeaps)(
    _In_ PRTL_ENUM_HEAPS_ROUTINE EnumRoutine,
    _In_ PVOID Parameter
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
_Must_inspect_result_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID NTAPI MUSA_NAME(RtlAllocateHeap)(
    _In_ PVOID HeapHandle,
    _In_opt_ ULONG Flags,
    _In_ SIZE_T Size
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
_Must_inspect_result_
_Ret_maybenull_
_Post_writable_byte_size_(Size)
PVOID NTAPI MUSA_NAME(RtlReAllocateHeap)(
    _In_ PVOID HeapHandle,
    _In_ ULONG Flags,
    _Frees_ptr_opt_ PVOID BaseAddress,
    _In_ SIZE_T Size
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
_Success_(return != 0)
LOGICAL NTAPI MUSA_NAME(RtlFreeHeap)(
    _In_ PVOID HeapHandle,
    _In_opt_ ULONG Flags,
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

#endif // _KERNEL_MODE


EXTERN_C_END
