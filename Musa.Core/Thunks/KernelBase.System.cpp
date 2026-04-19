#include "KernelBase.Private.h"
#include "Internal/KernelBase.System.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(GetLogicalProcessorInformation))
#pragma alloc_text(PAGE, MUSA_NAME(GetLogicalProcessorInformationEx))
#pragma alloc_text(PAGE, MUSA_NAME(GetSystemTimes))
#pragma alloc_text(PAGE, MUSA_NAME(GetNativeSystemInfo))
#pragma alloc_text(PAGE, MUSA_NAME(GetSystemInfo))
#pragma alloc_text(PAGE, MUSA_NAME(VerifyVersionInfoW))
#endif

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

//
// System Times
//

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetSystemTimes)(
    _Out_opt_ PFILETIME IdleTime,
    _Out_opt_ PFILETIME KernelTime,
    _Out_opt_ PFILETIME UserTime
)
{
    PAGED_CODE();

    NTSTATUS                                  Status;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION ProcessorTimes = nullptr;

    do {
        SYSTEM_BASIC_INFORMATION BaseInformation{};
        Status = ZwQuerySystemInformation(SystemBasicInformation,
            &BaseInformation, sizeof(BaseInformation), nullptr);
        if (!NT_SUCCESS(Status)) {
            break;
        }

        const auto NumberOfProcessors   = BaseInformation.NumberOfProcessors;
        const auto SizeOfProcessorTimes = NumberOfProcessors * sizeof(*ProcessorTimes);

        ProcessorTimes = static_cast<PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION>(
            LocalAlloc(LPTR, SizeOfProcessorTimes));
        if (ProcessorTimes == nullptr) {
            Status = STATUS_NO_MEMORY;
            break;
        }

        ULONG ReturnLength = 0;
        Status = ZwQuerySystemInformation(SystemProcessorPerformanceInformation,
            ProcessorTimes, static_cast<ULONG>(SizeOfProcessorTimes), &ReturnLength);
        if (!NT_SUCCESS(Status)) {
            break;
        }

        if (ReturnLength != SizeOfProcessorTimes) {
            Status = STATUS_INTERNAL_ERROR;
            break;
        }

        LONG           Lupe;
        ULARGE_INTEGER Sum;

        #pragma warning(suppress: 6385)
        #define BASEP_GST_SUM(DST, SRC)                                \
        if ( DST ) {                                                   \
            Sum.QuadPart = 0;                                          \
            for (Lupe = 0;                                             \
                 Lupe < NumberOfProcessors;                            \
                 Lupe++) {                                             \
                Sum.QuadPart += ProcessorTimes[Lupe]. SRC .QuadPart ;  \
            }                                                          \
            (DST)->dwLowDateTime  = Sum.LowPart;                       \
            (DST)->dwHighDateTime = Sum.HighPart;                      \
        }

        BASEP_GST_SUM(IdleTime, IdleTime)
        BASEP_GST_SUM(KernelTime, KernelTime)
        BASEP_GST_SUM(UserTime, UserTime)
        #undef BASEP_GST_SUM

        Status = STATUS_SUCCESS;
    } while (false);

    if (ProcessorTimes) {
        LocalFree(ProcessorTimes);
    }

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

MUSA_IAT_SYMBOL(GetSystemTimes, 12);

//
// System Information
//

static VOID GetSystemInfoInternal(
    _In_ PSYSTEM_BASIC_INFORMATION     BasicInfo,
    _In_ PSYSTEM_PROCESSOR_INFORMATION ProcessorInfo,
    _Out_ LPSYSTEM_INFO                lpSystemInfo
)
{
    RtlZeroMemory(lpSystemInfo, sizeof(*lpSystemInfo));

    lpSystemInfo->wProcessorArchitecture      = ProcessorInfo->ProcessorArchitecture;
    lpSystemInfo->wReserved                   = 0;
    lpSystemInfo->dwPageSize                  = BasicInfo->PageSize;
    lpSystemInfo->lpMinimumApplicationAddress = (LPVOID)BasicInfo->MinimumUserModeAddress;
    lpSystemInfo->lpMaximumApplicationAddress = (LPVOID)BasicInfo->MaximumUserModeAddress;
    lpSystemInfo->dwActiveProcessorMask       = BasicInfo->ActiveProcessorsAffinityMask;
    lpSystemInfo->dwNumberOfProcessors        = BasicInfo->NumberOfProcessors;
    lpSystemInfo->wProcessorLevel             = ProcessorInfo->ProcessorLevel;
    lpSystemInfo->wProcessorRevision          = ProcessorInfo->ProcessorRevision;

    if (ProcessorInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
        if (ProcessorInfo->ProcessorLevel == 3) {
            lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_386;
        } else if (ProcessorInfo->ProcessorLevel == 4) {
            lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_486;
        } else {
            lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_PENTIUM;
        }
    } else if (ProcessorInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) {
        lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_IA64;
    } else if (ProcessorInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
        lpSystemInfo->dwProcessorType = PROCESSOR_AMD_X8664;
    } else {
        lpSystemInfo->dwProcessorType = 0;
    }

    lpSystemInfo->dwAllocationGranularity = BasicInfo->AllocationGranularity;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MUSA_NAME(GetNativeSystemInfo)(
    _Out_ LPSYSTEM_INFO SystemInfo
)
{
    PAGED_CODE();

    SYSTEM_BASIC_INFORMATION BasicInfo{};
    NTSTATUS                 Status = ZwQuerySystemInformation(
        SystemBasicInformation,
        &BasicInfo,
        sizeof(BasicInfo),
        nullptr);
    if (!NT_SUCCESS(Status)) {
        return;
    }

    SYSTEM_PROCESSOR_INFORMATION ProcessorInfo{};
    Status = ZwQuerySystemInformation(
        SystemProcessorInformation,
        &ProcessorInfo,
        sizeof(ProcessorInfo),
        nullptr);
    if (!NT_SUCCESS(Status)) {
        return;
    }

    GetSystemInfoInternal(
        &BasicInfo,
        &ProcessorInfo,
        SystemInfo);
}

MUSA_IAT_SYMBOL(GetNativeSystemInfo, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MUSA_NAME(GetSystemInfo)(
    _Out_ LPSYSTEM_INFO SystemInfo
)
{
    PAGED_CODE();

    return GetNativeSystemInfo(SystemInfo);
}

MUSA_IAT_SYMBOL(GetSystemInfo, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(VerifyVersionInfoW)(
    _Inout_ LPOSVERSIONINFOEXW lpVersionInformation,
    _In_    DWORD dwTypeMask,
    _In_    DWORDLONG dwlConditionMask
)
{
    PAGED_CODE();

    return RtlVerifyVersionInfo(lpVersionInformation, dwTypeMask, dwlConditionMask) == STATUS_SUCCESS;
}

MUSA_IAT_SYMBOL(VerifyVersionInfoW, 12);

EXTERN_C_END
