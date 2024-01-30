#include "KernelBase.Private.h"
#include "KernelBase.System.Private.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MI_NAME(GetSystemTimes))
#pragma alloc_text(PAGE, MI_NAME(GetNativeSystemInfo))
#pragma alloc_text(PAGE, MI_NAME(GetSystemInfo))
#endif

EXTERN_C_START
namespace Mi
{
    BOOL WINAPI MI_NAME(IsProcessorFeaturePresent)(
        _In_ DWORD ProcessorFeature
        )
    {
        return RtlIsProcessorFeaturePresent(ProcessorFeature);
    }
    MI_IAT_SYMBOL(IsProcessorFeaturePresent, 4);

    DWORD WINAPI MI_NAME(GetCurrentProcessorNumber)(
        VOID
        )
    {
        return RtlGetCurrentProcessorNumber();
    }
    MI_IAT_SYMBOL(GetCurrentProcessorNumber, 0);

    VOID WINAPI MI_NAME(GetCurrentProcessorNumberEx)(
        _Out_ PPROCESSOR_NUMBER ProcNumber
        )
    {
        return RtlGetCurrentProcessorNumberEx(ProcNumber);
    }
    MI_IAT_SYMBOL(GetCurrentProcessorNumberEx, 4);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(GetSystemTimes)(
        _Out_opt_ PFILETIME IdleTime,
        _Out_opt_ PFILETIME KernelTime,
        _Out_opt_ PFILETIME UserTime
        )
    {
        PAGED_CODE();

        NTSTATUS Status;
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

            ProcessorTimes = static_cast<PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION>(RtlAllocateHeap(
                GetProcessHeap(), HEAP_ZERO_MEMORY, SizeOfProcessorTimes));
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

            LONG Lupe;
            ULARGE_INTEGER Sum;

        #pragma warning(suppress: 6385)
        #define BASEP_GST_SUM(DST, SRC)                                    \
            if ( DST ) {                                                   \
                Sum.QuadPart = 0;                                          \
                for (Lupe = 0;                                             \
                     Lupe < NumberOfProcessors;                            \
                     Lupe++) {                                             \
                    Sum.QuadPart += ProcessorTimes[Lupe]. SRC .QuadPart ;  \
                }                                                          \
                DST->dwLowDateTime = Sum.LowPart;                          \
                DST->dwHighDateTime = Sum.HighPart;                        \
            }

            BASEP_GST_SUM(IdleTime,   IdleTime);
            BASEP_GST_SUM(KernelTime, KernelTime);
            BASEP_GST_SUM(UserTime,   UserTime);
        #undef BASEP_GST_SUM

            Status = STATUS_SUCCESS;

        } while (false);

        if (ProcessorTimes) {
            RtlFreeHeap(GetProcessHeap(), 0, ProcessorTimes);
        }

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(GetSystemTimes, 12);

    VOID WINAPI MI_NAME(GetSystemTimePreciseAsFileTime)(
        _Out_ LPFILETIME SystemTimeAsFileTime
        )
    {
        const auto SystemTimePrecise = RtlGetSystemTimePrecise();

        SystemTimeAsFileTime->dwHighDateTime = SystemTimePrecise.HighPart;
        SystemTimeAsFileTime->dwLowDateTime  = SystemTimePrecise.LowPart;
    }
    MI_IAT_SYMBOL(GetSystemTimePreciseAsFileTime, 4);

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
            }
            else if (ProcessorInfo->ProcessorLevel == 4) {
                lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_486;
            }
            else {
                lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_PENTIUM;
            }
        }
        else if (ProcessorInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) {
            lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_IA64;
        }
        else if (ProcessorInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
            lpSystemInfo->dwProcessorType = PROCESSOR_AMD_X8664;
        }
        else {
            lpSystemInfo->dwProcessorType = 0;
        }

        lpSystemInfo->dwAllocationGranularity = BasicInfo->AllocationGranularity;
    }

    _IRQL_requires_max_(PASSIVE_LEVEL)
    VOID WINAPI MI_NAME(GetNativeSystemInfo)(
        _Out_ LPSYSTEM_INFO SystemInfo
        )
    {
        PAGED_CODE();

        SYSTEM_BASIC_INFORMATION BasicInfo{};
        NTSTATUS Status = ZwQuerySystemInformation(
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
    MI_IAT_SYMBOL(GetNativeSystemInfo, 4);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    VOID WINAPI MI_NAME(GetSystemInfo)(
        _Out_ LPSYSTEM_INFO SystemInfo
        )
    {
        PAGED_CODE();

        return GetNativeSystemInfo(SystemInfo);
    }
    MI_IAT_SYMBOL(GetSystemInfo, 4);
















}
EXTERN_C_END
