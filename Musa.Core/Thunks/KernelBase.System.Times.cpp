#include "KernelBase.Private.h"
#include "Internal/KernelBase.System.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(GetSystemTimes))
#endif

using namespace Musa;


EXTERN_C_START

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

EXTERN_C_END
