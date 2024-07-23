#include "Ntdll.RealTime.Private.h"


EXTERN_C_START
namespace Musa
{
    VOID WINAPI MUSA_NAME(GetSystemTimePreciseAsFileTime)(
        _Out_ LPFILETIME SystemTimeAsFileTime
        )
    {
        const auto SystemTimePrecise = RtlGetSystemTimePrecise();

        SystemTimeAsFileTime->dwHighDateTime = SystemTimePrecise.HighPart;
        SystemTimeAsFileTime->dwLowDateTime = SystemTimePrecise.LowPart;
    }
    MUSA_IAT_SYMBOL(GetSystemTimePreciseAsFileTime, 4);

    VOID WINAPI MUSA_NAME(QueryInterruptTime)(
        _Out_ PLARGE_INTEGER PerformanceCount
    )
    {
        RtlGetInterruptTime(PerformanceCount);
    }
    MUSA_IAT_SYMBOL(QueryInterruptTime, 4);

    VOID WINAPI MUSA_NAME(QueryInterruptTimePrecise)(
        _Out_ PULONGLONG InterruptTimePrecise
    )
    {
        LARGE_INTEGER PerformanceCount{};
        *InterruptTimePrecise = RtlGetInterruptTimePrecise(&PerformanceCount).QuadPart;
    }
    MUSA_IAT_SYMBOL(QueryInterruptTimePrecise, 4);

    BOOL WINAPI MUSA_NAME(QueryUnbiasedInterruptTime)(
        _Out_ PULONGLONG UnbiasedTime
    )
    {
        return RtlQueryUnbiasedInterruptTime(reinterpret_cast<PLARGE_INTEGER>(UnbiasedTime));
    }
    MUSA_IAT_SYMBOL(QueryUnbiasedInterruptTime, 4);

#if (NTDDI_VERSION >= NTDDI_WIN10_RS1)
    VOID WINAPI MUSA_NAME(QueryUnbiasedInterruptTimePrecise)(
        _Out_ PULONGLONG UnbiasedInterruptTimePrecise
    )
    {
        LARGE_INTEGER PerformanceCount{};
        *UnbiasedInterruptTimePrecise = RtlQueryUnbiasedInterruptTimePrecise(&PerformanceCount).QuadPart;
    }
    MUSA_IAT_SYMBOL(QueryUnbiasedInterruptTimePrecise, 4);
#endif

    HRESULT WINAPI MUSA_NAME(QueryAuxiliaryCounterFrequency)(
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
    MUSA_IAT_SYMBOL(QueryAuxiliaryCounterFrequency, 4);

    HRESULT WINAPI MUSA_NAME(ConvertAuxiliaryCounterToPerformanceCounter)(
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
    MUSA_IAT_SYMBOL(ConvertAuxiliaryCounterToPerformanceCounter, 16);

    HRESULT WINAPI MUSA_NAME(ConvertPerformanceCounterToAuxiliaryCounter)(
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

    MUSA_IAT_SYMBOL(ConvertPerformanceCounterToAuxiliaryCounter, 16);

    DWORD WINAPI MUSA_NAME(GetTickCount)(
        VOID
    )
    {
        return ZwGetTickCount();
    }
    MUSA_IAT_SYMBOL(GetTickCount, 16);

    ULONGLONG WINAPI MUSA_NAME(GetTickCount64)(
        VOID
        )
    {
        return ZwGetTickCount64();
    }
    MUSA_IAT_SYMBOL(GetTickCount64, 16);

}
EXTERN_C_END
