#include "Ntdll.Heap.Private.h"


EXTERN_C_START
namespace Mi
{
    _IRQL_requires_max_(DISPATCH_LEVEL)
    HANDLE WINAPI MI_NAME(GetProcessHeap)()
    {
        return RtlGetDefaultHeap();
    }
    MI_IAT_SYMBOL(GetProcessHeap, 0);

    _IRQL_requires_max_(APC_LEVEL)
    DWORD WINAPI MI_NAME(GetProcessHeaps)(
        _In_ DWORD NumberOfHeaps,
        _Out_writes_to_(NumberOfHeaps, return) PHANDLE ProcessHeaps
    )
    {
        return RtlGetProcessHeaps(NumberOfHeaps, ProcessHeaps);
    }
    MI_IAT_SYMBOL(GetProcessHeaps, 8);

    _IRQL_requires_max_(APC_LEVEL)
    _Ret_maybenull_
    HANDLE WINAPI MI_NAME(HeapCreate)(
        _In_ DWORD  Options,
        _In_ SIZE_T InitialSize,
        _In_ SIZE_T MaximumSize
    )
    {
        return RtlCreateHeap(Options, nullptr,
            MaximumSize, InitialSize, nullptr, nullptr);
    }
    MI_IAT_SYMBOL(HeapCreate, 12);

    _IRQL_requires_max_(APC_LEVEL)
    BOOL WINAPI MI_NAME(HeapDestroy)(
        _In_ HANDLE Heap
    )
    {
        return (RtlDestroyHeap(Heap) == nullptr);
    }
    MI_IAT_SYMBOL(HeapDestroy, 4);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Ret_maybenull_
    _Post_writable_byte_size_(Bytes)
    DECLSPEC_ALLOCATOR
    LPVOID WINAPI MI_NAME(HeapAlloc)(
        _In_ HANDLE Heap,
        _In_ DWORD  Flags,
        _In_ SIZE_T Bytes
    )
    {
        return RtlAllocateHeap(Heap, Flags, Bytes);
    }
    MI_IAT_SYMBOL(HeapAlloc, 12);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Success_(return != 0)
    _Ret_maybenull_
    _Post_writable_byte_size_(Bytes)
    DECLSPEC_ALLOCATOR
    LPVOID WINAPI MI_NAME(HeapReAlloc)(
        _Inout_ HANDLE Heap,
        _In_    DWORD  Flags,
        _Frees_ptr_opt_ LPVOID Mem,
        _In_    SIZE_T Bytes
    )
    {
        return RtlReAllocateHeap(Heap, Flags, Mem, Bytes);
    }
    MI_IAT_SYMBOL(HeapReAlloc, 16);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Success_(return != FALSE)
    BOOL WINAPI MI_NAME(HeapFree)(
        _Inout_ HANDLE Heap,
        _In_    DWORD  Flags,
        __drv_freesMem(Mem) _Frees_ptr_opt_ LPVOID Mem
    )
    {
        return RtlFreeHeap(Heap, Flags, Mem);
    }
    MI_IAT_SYMBOL(HeapFree, 12);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    SIZE_T WINAPI MI_NAME(HeapSize)(
        _In_ HANDLE  Heap,
        _In_ DWORD   Flags,
        _In_ LPCVOID Mem
    )
    {
        return RtlSizeHeap(Heap, Flags, const_cast<PVOID>(Mem));
    }
    MI_IAT_SYMBOL(HeapSize, 12);

    SIZE_T WINAPI MI_NAME(HeapCompact)(
        _In_ HANDLE Heap,
        _In_ DWORD  Flags
    )
    {
        UNREFERENCED_PARAMETER(Heap);
        UNREFERENCED_PARAMETER(Flags);

        DbgBreakPoint();

        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_NOT_SUPPORTED);
        return 0;
    }
    MI_IAT_SYMBOL(HeapCompact, 8);

    BOOL WINAPI MI_NAME(HeapLock)(
        _In_ HANDLE Heap
    )
    {
        UNREFERENCED_PARAMETER(Heap);

        DbgBreakPoint();

        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_NOT_SUPPORTED);
        return FALSE;
    }
    MI_IAT_SYMBOL(HeapLock, 4);

    BOOL WINAPI MI_NAME(HeapUnlock)(
        _In_ HANDLE Heap
    )
    {
        UNREFERENCED_PARAMETER(Heap);

        DbgBreakPoint();

        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_NOT_SUPPORTED);
        return FALSE;
    }
    MI_IAT_SYMBOL(HeapUnlock, 4);

    BOOL WINAPI MI_NAME(HeapValidate)(
        _In_ HANDLE Heap,
        _In_ DWORD  Flags,
        _In_opt_ LPCVOID Mem
    )
    {
        UNREFERENCED_PARAMETER(Heap);
        UNREFERENCED_PARAMETER(Flags);
        UNREFERENCED_PARAMETER(Mem);

        DbgBreakPoint();

        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_NOT_SUPPORTED);
        return FALSE;
    }
    MI_IAT_SYMBOL(HeapValidate, 12);

    BOOL WINAPI MI_NAME(HeapSummary)(
        _In_ HANDLE Heap,
        _In_ DWORD  Flags,
        _Out_ LPHEAP_SUMMARY Summary
    )
    {
        UNREFERENCED_PARAMETER(Heap);
        UNREFERENCED_PARAMETER(Flags);
        UNREFERENCED_PARAMETER(Summary);

        DbgBreakPoint();

        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_NOT_SUPPORTED);
        return FALSE;
    }
    MI_IAT_SYMBOL(HeapSummary, 12);

    BOOL WINAPI MI_NAME(HeapWalk)(
        _In_ HANDLE Heap,
        _Inout_ LPPROCESS_HEAP_ENTRY Entry
    )
    {
        UNREFERENCED_PARAMETER(Heap);
        UNREFERENCED_PARAMETER(Entry);

        DbgBreakPoint();

        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_NOT_SUPPORTED);
        return FALSE;
    }
    MI_IAT_SYMBOL(HeapWalk, 8);

    BOOL WINAPI MI_NAME(HeapSetInformation)(
        _In_opt_ HANDLE HeapHandle,
        _In_ HEAP_INFORMATION_CLASS HeapInformationClass,
        _In_reads_bytes_opt_(HeapInformationLength) PVOID HeapInformation,
        _In_ SIZE_T HeapInformationLength
    )
    {
        UNREFERENCED_PARAMETER(HeapHandle);
        UNREFERENCED_PARAMETER(HeapInformationClass);
        UNREFERENCED_PARAMETER(HeapInformation);
        UNREFERENCED_PARAMETER(HeapInformationLength);

        DbgBreakPoint();

        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_NOT_SUPPORTED);
        return FALSE;
    }
    MI_IAT_SYMBOL(HeapSetInformation, 16);

    BOOL WINAPI MI_NAME(HeapQueryInformation)(
        _In_opt_ HANDLE HeapHandle,
        _In_ HEAP_INFORMATION_CLASS HeapInformationClass,
        _Out_writes_bytes_to_opt_(HeapInformationLength, *ReturnLength) PVOID HeapInformation,
        _In_ SIZE_T HeapInformationLength,
        _Out_opt_ PSIZE_T ReturnLength
    )
    {
        UNREFERENCED_PARAMETER(HeapHandle);
        UNREFERENCED_PARAMETER(HeapInformationClass);
        UNREFERENCED_PARAMETER(HeapInformation);
        UNREFERENCED_PARAMETER(HeapInformationLength);
        UNREFERENCED_PARAMETER(ReturnLength);

        DbgBreakPoint();

        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_NOT_SUPPORTED);
        return FALSE;
    }
    MI_IAT_SYMBOL(HeapQueryInformation, 20);


}
EXTERN_C_END
