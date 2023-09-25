#pragma once


constexpr uint32_t MI_FLS_MAXIMUM_AVAILABLE = 256;

typedef struct _MI_FLS_DATA
{
    LIST_ENTRY  Entry;
    PVOID       Slots[MI_FLS_MAXIMUM_AVAILABLE];

} MI_FLS_DATA, * PMI_FLS_DATA;
typedef MI_FLS_DATA const* PCMI_FLS_DATA;

typedef struct _MI_KPEB
{
    FAST_MUTEX      FastPebLock;

    PDRIVER_OBJECT  DriverObject;
    UNICODE_STRING  RegistryPath;

    PVOID           ImageBaseAddress;
    SIZE_T          SizeOfImage;
    PVOID           ProcessHeap;

    PVOID           StandardInput;
    PVOID           StandardOutput;
    PVOID           StandardError;

    PVOID           Environment;
    UNICODE_STRING  ImagePathName;
    CURDIR          CurrentDirectory;

    PFLS_CALLBACK_FUNCTION FlsCallback[MI_FLS_MAXIMUM_AVAILABLE];
    LIST_ENTRY      FlsListHead;
    RTL_BITMAP      FlsBitmap;
    ULONG           FlsBitmapBits[MI_FLS_MAXIMUM_AVAILABLE / RTL_BITS_OF(ULONG)];
    ULONG           FlsHighIndex;

    ULONG           NumberOfHeaps;
    ULONG           MaximumNumberOfHeaps;
    PVOID*          ProcessHeaps;

    PVOID           WaitOnAddressHashTable[128];

} MI_KPEB, * PMI_KPEB;
typedef MI_KPEB const* PCMI_KPEB;
static_assert(ALIGN_DOWN(sizeof(MI_KPEB), PVOID) < PAGE_SIZE);

typedef struct _MI_KTEB
{
    UINT64          UniqueId;
    HANDLE          ThreadId;

    PMI_KPEB        ProcessEnvironmentBlock;
    PMI_FLS_DATA    FlsData;

    ULONG           HardErrorMode;
    NTSTATUS        ExceptionCode;

    ULONG           LastErrorValue;
    NTSTATUS        LastStatusValue;

} MI_KTEB, * PMI_KTEB;
typedef MI_KTEB const* PCMI_KTEB;


namespace Mi::Thunk
{
    //
    // Stub
    //

    NTSTATUS MICORE_API LoadStubs();
    NTSTATUS MICORE_API FreeStubs();
    PVOID    MICORE_API QueryStub(_In_ size_t NameHash);
    PVOID    MICORE_API QueryStub(_In_ const char* Name);

    //
    // KPEB & KTEB
    //

    NTSTATUS MICORE_API InitProcessEnvironmentBlock(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    );
    NTSTATUS MICORE_API FreeProcessEnvironmentBlock();

    PMI_KPEB MICORE_API GetCurrentPeb();
    PMI_KTEB MICORE_API GetCurrentTeb();

#define NtCurrentPeb() ::Mi::Thunk::GetCurrentPeb()
#define NtCurrentTeb() ::Mi::Thunk::GetCurrentTeb()
#define RtlAcquirePebLock() ExAcquireFastMutex(&NtCurrentPeb()->FastPebLock)
#define RtlReleasePebLock() ExReleaseFastMutex(&NtCurrentPeb()->FastPebLock)

}
