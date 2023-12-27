#include "Ntdll.Heap.Private.h"


EXTERN_C_START
namespace Mi
{
    _IRQL_requires_max_(DISPATCH_LEVEL)
    HANDLE WINAPI MI_NAME(GetProcessHeap)(
        VOID
        )
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
        _In_ HANDLE HeapHandle
        )
    {
        return (RtlDestroyHeap(HeapHandle) == nullptr);
    }
    MI_IAT_SYMBOL(HeapDestroy, 4);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Ret_maybenull_
    _Post_writable_byte_size_(Bytes)
    DECLSPEC_ALLOCATOR
    LPVOID WINAPI MI_NAME(HeapAlloc)(
        _In_ HANDLE HeapHandle,
        _In_ DWORD  Flags,
        _In_ SIZE_T Bytes
        )
    {
        return RtlAllocateHeap(HeapHandle, Flags, Bytes);
    }
    MI_IAT_SYMBOL(HeapAlloc, 12);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Success_(return != 0)
    _Ret_maybenull_
    _Post_writable_byte_size_(Bytes)
    DECLSPEC_ALLOCATOR
    LPVOID WINAPI MI_NAME(HeapReAlloc)(
        _Inout_ HANDLE HeapHandle,
        _In_    DWORD  Flags,
        _Frees_ptr_opt_ LPVOID Mem,
        _In_    SIZE_T Bytes
        )
    {
        return RtlReAllocateHeap(HeapHandle, Flags, Mem, Bytes);
    }
    MI_IAT_SYMBOL(HeapReAlloc, 16);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Success_(return != FALSE)
    BOOL WINAPI MI_NAME(HeapFree)(
        _Inout_ HANDLE HeapHandle,
        _In_    DWORD  Flags,
        __drv_freesMem(Mem) _Frees_ptr_opt_ LPVOID Mem
        )
    {
        return RtlFreeHeap(HeapHandle, Flags, Mem);
    }
    MI_IAT_SYMBOL(HeapFree, 12);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    SIZE_T WINAPI MI_NAME(HeapSize)(
        _In_ HANDLE  HeapHandle,
        _In_ DWORD   Flags,
        _In_ LPCVOID Mem
        )
    {
        return RtlSizeHeap(HeapHandle, Flags, const_cast<PVOID>(Mem));
    }
    MI_IAT_SYMBOL(HeapSize, 12);

    SIZE_T WINAPI MI_NAME(HeapCompact)(
        _In_ HANDLE HeapHandle,
        _In_ DWORD  Flags
        )
    {
        return RtlCompactHeap(HeapHandle, Flags);
    }
    MI_IAT_SYMBOL(HeapCompact, 8);

    BOOL WINAPI MI_NAME(HeapLock)(
        _In_ HANDLE HeapHandle
        )
    {
        return RtlLockHeap(HeapHandle);
    }
    MI_IAT_SYMBOL(HeapLock, 4);

    BOOL WINAPI MI_NAME(HeapUnlock)(
        _In_ HANDLE HeapHandle
        )
    {
        return RtlUnlockHeap(HeapHandle);
    }
    MI_IAT_SYMBOL(HeapUnlock, 4);

    BOOL WINAPI MI_NAME(HeapValidate)(
        _In_ HANDLE HeapHandle,
        _In_ DWORD  Flags,
        _In_opt_ LPCVOID Mem
        )
    {
        return RtlValidateHeap(HeapHandle, Flags, Mem);
    }
    MI_IAT_SYMBOL(HeapValidate, 12);

    static
    _Function_class_(RTL_HEAP_EXTENDED_ENUMERATION_ROUTINE)
    NTSTATUS NTAPI HeapSummaryWorkerRoutine(
        _In_ PHEAP_INFORMATION_ITEM Information,
        _In_opt_ PVOID Context
    )
    {
        const auto Summary = static_cast<LPHEAP_SUMMARY>(Context);
        if (Summary == nullptr) {
            return STATUS_SUCCESS;
        }

        switch (Information->Level) {
            default:
            {
                break;
            }
            case HeapExtendedHeapRegionInformationLevel:
            {
                Summary->cbReserved   += Information->HeapRegionInformation.ReserveSize;
                Summary->cbMaxReserve += Information->HeapRegionInformation.ReserveSize;
                break;
            }
            case HeapExtendedHeapRangeInformationLevel:
            {
                if (Information->HeapRangeInformation.Type == 1) {
                    Summary->cbCommitted += Information->HeapRangeInformation.Size;
                }
                break;
            }
            case HeapExtendedHeapBlockInformationLevel:
            {
                Summary->cbAllocated += Information->HeapBlockInformation.DataSize;
                break;
            }
        }

        return STATUS_SUCCESS;
    }

    BOOL WINAPI MI_NAME(HeapSummary)(
        _In_ HANDLE HeapHandle,
        _In_ DWORD  Flags,
        _Inout_ LPHEAP_SUMMARY Summary
        )
    {
        UNREFERENCED_PARAMETER(Flags);

        if (Summary->cb != sizeof(*Summary)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(STATUS_INVALID_PARAMETER);
            return FALSE;
        }

        Summary->cbAllocated  = 0;
        Summary->cbCommitted  = 0;
        Summary->cbReserved   = 0;
        Summary->cbMaxReserve = 0;

        HEAP_EXTENDED_INFORMATION SummaryInformation{};
        SummaryInformation.ProcessHandle    = ZwCurrentProcess();
        SummaryInformation.HeapHandle       = HeapHandle;
        SummaryInformation.Level            = HeapExtendedHeapBlockInformationLevel;
        SummaryInformation.CallbackRoutine  = HeapSummaryWorkerRoutine;
        SummaryInformation.CallbackContext  = Summary;

        const auto Status = RtlQueryHeapInformation(HeapHandle, HeapExtendedInformation,
            &SummaryInformation, sizeof(SummaryInformation), nullptr);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        return FALSE;
    }
    MI_IAT_SYMBOL(HeapSummary, 12);

    BOOL WINAPI MI_NAME(HeapWalk)(
        _In_ HANDLE HeapHandle,
        _Inout_ LPPROCESS_HEAP_ENTRY Entry
    )
    {
        const auto Status = RtlWalkHeap(HeapHandle,
            reinterpret_cast<PRTL_HEAP_WALK_ENTRY>(Entry));
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
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
        const auto Status = RtlSetHeapInformation(HeapHandle,
            HeapInformationClass, HeapInformation, HeapInformationLength);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
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
        const auto Status = RtlQueryHeapInformation(HeapHandle,
            HeapInformationClass, HeapInformation, HeapInformationLength, ReturnLength);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        return FALSE;
    }
    MI_IAT_SYMBOL(HeapQueryInformation, 20);

}
EXTERN_C_END
