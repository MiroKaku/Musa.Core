#include "Ntdll.RealTime.Private.h"


EXTERN_C_START
namespace Mi
{
    VOID WINAPI MI_NAME(QueryInterruptTime)(
        _Out_ PLARGE_INTEGER PerformanceCount
    )
    {
        RtlGetInterruptTime(PerformanceCount);
    }
    MI_IAT_SYMBOL(QueryInterruptTime, 4);

    VOID WINAPI MI_NAME(QueryInterruptTimePrecise)(
        _Out_ PULONGLONG InterruptTimePrecise
    )
    {
        LARGE_INTEGER PerformanceCount{};
        *InterruptTimePrecise = RtlGetInterruptTimePrecise(&PerformanceCount).QuadPart;
    }
    MI_IAT_SYMBOL(QueryInterruptTimePrecise, 4);

    BOOL WINAPI MI_NAME(QueryUnbiasedInterruptTime)(
        _Out_ PULONGLONG UnbiasedTime
    )
    {
        return RtlQueryUnbiasedInterruptTime(reinterpret_cast<PLARGE_INTEGER>(UnbiasedTime));
    }
    MI_IAT_SYMBOL(QueryUnbiasedInterruptTime, 4);

#if (NTDDI_VERSION >= NTDDI_WIN10_RS1)
    VOID WINAPI MI_NAME(QueryUnbiasedInterruptTimePrecise)(
        _Out_ PULONGLONG UnbiasedInterruptTimePrecise
    )
    {
        LARGE_INTEGER PerformanceCount{};
        *UnbiasedInterruptTimePrecise = RtlQueryUnbiasedInterruptTimePrecise(&PerformanceCount).QuadPart;
    }
    MI_IAT_SYMBOL(QueryUnbiasedInterruptTimePrecise, 4);
#endif

    HRESULT WINAPI MI_NAME(QueryAuxiliaryCounterFrequency)(
        _Out_ PULONGLONG AuxiliaryCounterFrequency
    )
    {
        const auto Status = RtlQueryAuxiliaryCounterFrequency(AuxiliaryCounterFrequency);
        if (Status == STATUS_NOT_SUPPORTED) {
            return E_NOTIMPL;
        }

        const auto DosCode = RtlNtStatusToDosErrorNoTeb(Status);
        if (DosCode) {
            return HRESULT_FROM_WIN32(DosCode);
        }

        return S_OK;
    }
    MI_IAT_SYMBOL(QueryAuxiliaryCounterFrequency, 4);

    HRESULT WINAPI MI_NAME(ConvertAuxiliaryCounterToPerformanceCounter)(
        _In_ ULONGLONG AuxiliaryCounterValue,
        _Out_ PULONGLONG PerformanceCounterValue,
        _Out_opt_ PULONGLONG ConversionError
    )
    {
        const auto Status = RtlConvertAuxiliaryCounterToPerformanceCounter(
            AuxiliaryCounterValue, PerformanceCounterValue, ConversionError);
        if (Status == STATUS_NOT_SUPPORTED) {
            return E_NOTIMPL;
        }
        if (Status == STATUS_INVALID_PARAMETER) {
            return E_BOUNDS;
        }

        const auto Win32Code = RtlNtStatusToDosErrorNoTeb(Status);
        if (Win32Code) {
            return HRESULT_FROM_WIN32(Win32Code);
        }

        return S_OK;
    }
    MI_IAT_SYMBOL(ConvertAuxiliaryCounterToPerformanceCounter, 16);

    HRESULT WINAPI MI_NAME(ConvertPerformanceCounterToAuxiliaryCounter)(
        _In_ ULONGLONG PerformanceCounterValue,
        _Out_ PULONGLONG AuxiliaryCounterValue,
        _Out_opt_ PULONGLONG ConversionError
    )
    {
        const auto Status = RtlConvertPerformanceCounterToAuxiliaryCounter(
            PerformanceCounterValue, AuxiliaryCounterValue, ConversionError);
        if (Status == STATUS_NOT_SUPPORTED) {
            return E_NOTIMPL;
        }
        if (Status == STATUS_INVALID_PARAMETER) {
            return E_BOUNDS;
        }

        const auto Win32Code = RtlNtStatusToDosErrorNoTeb(Status);
        if (Win32Code) {
            return HRESULT_FROM_WIN32(Win32Code);
        }

        return S_OK;
    }
    MI_IAT_SYMBOL(ConvertPerformanceCounterToAuxiliaryCounter, 16);


}
EXTERN_C_END
