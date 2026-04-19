#include "KernelBase.Private.h"
#include "Internal/KernelBase.Thread.h"

EXTERN_C_START

_Success_(return != FALSE)
BOOL WINAPI MUSA_NAME(InitializeProcThreadAttributeList)(
    _Out_writes_bytes_to_opt_(*Size, *Size) LPPROC_THREAD_ATTRIBUTE_LIST                      AttributeList,
    _In_ DWORD                                                                                AttributeCount,
    _Reserved_ DWORD                                                                          Flags,
    _When_(AttributeList == nullptr, _Out_) _When_(AttributeList != nullptr, _Inout_) PSIZE_T Size
)
{
    if (Flags) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER_3);
        return FALSE;
    }

    if (AttributeCount > 30 /* _countof(PROC_THREAD_ATTRIBUTE_NUM) */) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER_2);
        return FALSE;
    }

    BOOL Result;

    const auto TotalSize = sizeof(PROC_THREAD_ATTRIBUTE_LIST) + sizeof(PROC_THREAD_ATTRIBUTE) * AttributeCount;
    if (AttributeList && *Size >= TotalSize) {
        *AttributeList = { 0, AttributeCount };
        Result         = TRUE;
    } else {
        BaseSetLastNTError(STATUS_BUFFER_TOO_SMALL);
        Result = FALSE;
    }

    *Size = TotalSize;

    return Result;
}

MUSA_IAT_SYMBOL(InitializeProcThreadAttributeList, 16);

VOID WINAPI MUSA_NAME(DeleteProcThreadAttributeList)(
    _Inout_ LPPROC_THREAD_ATTRIBUTE_LIST AttributeList
)
{
    UNREFERENCED_PARAMETER(AttributeList);
    return;
}

MUSA_IAT_SYMBOL(DeleteProcThreadAttributeList, 4);

EXTERN_C_END
