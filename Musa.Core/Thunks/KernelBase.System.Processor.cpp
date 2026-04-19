#include "KernelBase.Private.h"
#include "Internal/KernelBase.System.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(GetLogicalProcessorInformation))
#pragma alloc_text(PAGE, MUSA_NAME(GetLogicalProcessorInformationEx))
#endif

using namespace Musa;

EXTERN_C_START

//
// Processor
//

BOOL WINAPI MUSA_NAME(IsProcessorFeaturePresent)(
    _In_ DWORD ProcessorFeature
)
{
    return RtlIsProcessorFeaturePresent(ProcessorFeature);
}

MUSA_IAT_SYMBOL(IsProcessorFeaturePresent, 4);

DWORD WINAPI MUSA_NAME(GetCurrentProcessorNumber)(
    VOID
)
{
    return RtlGetCurrentProcessorNumber();
}

MUSA_IAT_SYMBOL(GetCurrentProcessorNumber, 0);

VOID WINAPI MUSA_NAME(GetCurrentProcessorNumberEx)(
    _Out_ PPROCESSOR_NUMBER ProcNumber
)
{
    return RtlGetCurrentProcessorNumberEx(ProcNumber);
}

MUSA_IAT_SYMBOL(GetCurrentProcessorNumberEx, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetLogicalProcessorInformation)(
    _Out_writes_bytes_to_opt_(*ReturnedLength, *ReturnedLength) PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer,
    _Inout_ PDWORD ReturnedLength
)
{
    PAGED_CODE();

    if (ReturnedLength == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    NTSTATUS Status = ZwQuerySystemInformation(SystemLogicalProcessorInformation,
        Buffer, *ReturnedLength, ReturnedLength);
    if (Status == STATUS_INFO_LENGTH_MISMATCH) {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    } else {
        return TRUE;
    }
}

MUSA_IAT_SYMBOL(GetLogicalProcessorInformation, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetLogicalProcessorInformationEx)(
    _In_ LOGICAL_PROCESSOR_RELATIONSHIP RelationshipType,
    _Out_writes_bytes_to_opt_(*ReturnedLength, *ReturnedLength) PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX Buffer,
    _Inout_ PDWORD ReturnedLength
)
{
    PAGED_CODE();

    if (ReturnedLength == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    NTSTATUS Status = ZwQuerySystemInformationEx(SystemLogicalProcessorAndGroupInformation,
        &RelationshipType, sizeof(RelationshipType),
        Buffer, *ReturnedLength, ReturnedLength);
    if (Status == STATUS_INFO_LENGTH_MISMATCH) {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    } else {
        return TRUE;
    }
}

MUSA_IAT_SYMBOL(GetLogicalProcessorInformationEx, 12);

WORD WINAPI MUSA_NAME(GetActiveProcessorGroupCount)(VOID)
{
    return KeQueryActiveGroupCount();
}

MUSA_IAT_SYMBOL(GetActiveProcessorGroupCount, 0);

WORD WINAPI MUSA_NAME(GetMaximumProcessorGroupCount)(VOID)
{
    return KeQueryMaximumGroupCount();
}

MUSA_IAT_SYMBOL(GetMaximumProcessorGroupCount, 0);

DWORD WINAPI MUSA_NAME(GetActiveProcessorCount)(
    _In_ WORD GroupNumber
)
{
    return KeQueryActiveProcessorCountEx(GroupNumber);
}

MUSA_IAT_SYMBOL(GetActiveProcessorCount, 4);

DWORD WINAPI MUSA_NAME(GetMaximumProcessorCount)(
    _In_ WORD GroupNumber
)
{
    return KeQueryMaximumProcessorCountEx(GroupNumber);
}

MUSA_IAT_SYMBOL(GetMaximumProcessorCount, 4);

EXTERN_C_END
