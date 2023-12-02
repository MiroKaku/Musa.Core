#pragma once


#ifdef _KERNEL_MODE
namespace Mi
{
    constexpr uint32_t MI_FLS_MAXIMUM_AVAILABLE = 256;

    // Fiber Local storage
    VEIL_DECLARE_STRUCT(FLS_DATA)
    {
        LIST_ENTRY  Entry;
        PVOID       Slots[MI_FLS_MAXIMUM_AVAILABLE];

    };

    // Kernel Process Environment Block
    VEIL_DECLARE_STRUCT_ALIGN(KPEB, 8)
    {
        FAST_MUTEX      Lock;
        EX_RUNDOWN_REF  RundownProtect;

        PDRIVER_OBJECT  DriverObject;
        UNICODE_STRING  RegistryPath;

        SIZE_T          SizeOfImage;
        PVOID           ImageBaseAddress;
        UNICODE_STRING  ImagePathName;
        UNICODE_STRING  ImageBaseName;

        PVOID           StandardInput;
        PVOID           StandardOutput;
        PVOID           StandardError;

        PVOID           Environment;
        CURDIR          CurrentDirectory;

        RTL_BITMAP      FlsBitmap;
        ULONG           FlsBitmapBits[MI_FLS_MAXIMUM_AVAILABLE / RTL_BITS_OF(ULONG)];

        ULONG           FlsHighIndex;
        LIST_ENTRY      FlsListHead;
        PFLS_CALLBACK_FUNCTION FlsCallback[MI_FLS_MAXIMUM_AVAILABLE];

        ULONG           NumberOfHeaps;
        ULONG           MaximumNumberOfHeaps;
        PVOID           DefaultHeap;
        PVOID*          ProcessHeaps;

        PVOID           WaitOnAddressHashTable[128];

        // Heaps[1] ...

    };
    static_assert(ALIGN_DOWN(sizeof(KPEB), 8) < PAGE_SIZE);

    // Kernel Thread  Environment Block
    VEIL_DECLARE_STRUCT_ALIGN(KTEB, 8)
    {
        HANDLE      ThreadId;
        HANDLE      ProcessId;
        PKPEB       ProcessEnvironmentBlock;

        ULONG       HardErrorMode;
        NTSTATUS    ExceptionCode;

        ULONG       LastErrorValue;
        NTSTATUS    LastStatusValue;

        PFLS_DATA   FlsData;

    };

}



EXTERN_C_START


Mi::PKPEB MICORE_API MI_NAME_PRIVATE(RtlGetCurrentPeb)();

_IRQL_raises_(APC_LEVEL)
VOID MICORE_API MI_NAME_PRIVATE(RtlAcquirePebLock)();

_IRQL_requires_(APC_LEVEL)
VOID MICORE_API MI_NAME_PRIVATE(RtlReleasePebLock)();

_Must_inspect_result_
_Success_(return != FALSE)
_IRQL_raises_(APC_LEVEL)
BOOLEAN MICORE_API MI_NAME_PRIVATE(RtlTryAcquirePebLock)();


EXTERN_C_END


#endif
