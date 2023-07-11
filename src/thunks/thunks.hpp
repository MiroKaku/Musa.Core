#pragma once


EXTERN_C_START

constexpr uint32_t MI_FLS_MAXIMUM_AVAILABLE = 256;

typedef struct _MI_FLS_DATA
{
    LIST_ENTRY  Entry;
    PVOID       Slots[MI_FLS_MAXIMUM_AVAILABLE];
} MI_FLS_DATA, * PMI_FLS_DATA;
typedef MI_FLS_DATA const* PCMI_FLS_DATA;

typedef struct _MI_THREAD_BLOCK
{
    long long       UniqueId;
    void*           ThreadId;

    PMI_FLS_DATA    FlsData;

    NTSTATUS        LastNtStatus;
    ULONG           LastWin32Error;

} MI_THREAD_BLOCK, * PMI_THREAD_BLOCK;
typedef MI_THREAD_BLOCK const * PCMI_THREAD_BLOCK;

EXTERN_C_END



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
    // Threads
    //

    NTSTATUS MICORE_API InitThreadTable();
    NTSTATUS MICORE_API FreeThreadTable();
    PMI_THREAD_BLOCK MICORE_API GetThreadBlock();


}
