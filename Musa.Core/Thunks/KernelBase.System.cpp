#include "KernelBase.Private.h"
#include "Internal/KernelBase.System.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(GetNativeSystemInfo))
#pragma alloc_text(PAGE, MUSA_NAME(GetSystemInfo))
#pragma alloc_text(PAGE, MUSA_NAME(VerifyVersionInfoW))
#endif

using namespace Musa;


EXTERN_C_START

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
