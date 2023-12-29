#include "KernelBase.Private.h"


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
    
    BOOL WINAPI MI_NAME(GetSystemTimes)(
        _Out_opt_ PFILETIME IdleTime,
        _Out_opt_ PFILETIME KernelTime,
        _Out_opt_ PFILETIME UserTime
        )
    {
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
                ProcessorTimes, SizeOfProcessorTimes, &ReturnLength);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            if (ReturnLength != SizeOfProcessorTimes) {
                Status = STATUS_INTERNAL_ERROR;
                break;
            }

            LONG Lupe;
            ULARGE_INTEGER Sum;

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


}
EXTERN_C_END
