#include "Ntdll.RealTime.Private.h"


EXTERN_C_START
namespace Mi
{
#ifdef _KERNEL_MODE
    LARGE_INTEGER NTAPI MI_NAME(RtlGetSystemTimePrecise)(
        VOID
        )
    {
        LARGE_INTEGER SystemTime{};
        KeQuerySystemTimePrecise(&SystemTime);

        return SystemTime;
    }
    MI_IAT_SYMBOL(RtlGetSystemTimePrecise, 0);
#endif

    BOOLEAN NTAPI MI_NAME(RtlGetInterruptTime)(
        _Out_ PLARGE_INTEGER InterruptTime
        )
    {
    #ifdef _KERNEL_MODE
        InterruptTime->QuadPart = KeQueryInterruptTime();
        return TRUE;
    #else
        InterruptTime->QuadPart = *reinterpret_cast<const volatile ULONG64*>(&SharedUserData->InterruptTime);
        return TRUE;
    #endif
    }
    MI_IAT_SYMBOL(RtlGetInterruptTime, 4);

#ifdef _KERNEL_MODE
    LARGE_INTEGER /* InterruptTime */ NTAPI MI_NAME(RtlGetInterruptTimePrecise)(
        _Out_ PLARGE_INTEGER PerformanceCount
        )
    {
        LARGE_INTEGER InterruptTime;
        InterruptTime.QuadPart = KeQueryInterruptTimePrecise(reinterpret_cast<PULONG64>(PerformanceCount));

        return InterruptTime;
    }
    MI_IAT_SYMBOL(RtlGetInterruptTimePrecise, 4);

    BOOLEAN NTAPI MI_NAME(RtlQueryUnbiasedInterruptTime)(
        _Out_ PLARGE_INTEGER InterruptTime)
    {
        InterruptTime->QuadPart = KeQueryUnbiasedInterruptTime();
        return TRUE;
    }
    MI_IAT_SYMBOL(RtlQueryUnbiasedInterruptTime, 4);
#endif

#if (NTDDI_VERSION >= NTDDI_WIN10_RS1)
    LARGE_INTEGER NTAPI MI_NAME(RtlQueryUnbiasedInterruptTimePrecise)(
        _Out_ PLARGE_INTEGER PerformanceCount)
    {
    #ifdef _KERNEL_MODE
        LARGE_INTEGER InterruptTime;
        InterruptTime.QuadPart = KeQueryUnbiasedInterruptTimePrecise(reinterpret_cast<PULONG64>(PerformanceCount));

        return InterruptTime;
    #else
        ULONGLONG InterruptTimeBias;
        ULONGLONG InterruptTimePrecise;

        do
        {
            InterruptTimeBias    = SharedUserData->InterruptTimeBias;
            InterruptTimePrecise = RtlGetInterruptTimePrecise(PerformanceCount).QuadPart;

        } while (InterruptTimeBias != SharedUserData->InterruptTimeBias);

        LARGE_INTEGER Result;
        Result.QuadPart = InterruptTimePrecise - InterruptTimeBias;

        return Result;
    #endif
    }
    MI_IAT_SYMBOL(RtlQueryUnbiasedInterruptTimePrecise, 4);
#endif

    NTSTATUS NTAPI MI_NAME(RtlQueryAuxiliaryCounterFrequency)(
        _Out_ PLARGE_INTEGER AuxiliaryCounterFrequency
    )
    {
    #ifdef _KERNEL_MODE
        return KeQueryAuxiliaryCounterFrequency(reinterpret_cast<PULONG64>(AuxiliaryCounterFrequency));
    #else
        return ZwQueryAuxiliaryCounterFrequency(AuxiliaryCounterFrequency);
    #endif
    }
    MI_IAT_SYMBOL(RtlQueryAuxiliaryCounterFrequency, 4);

    NTSTATUS NTAPI MI_NAME(RtlConvertAuxiliaryCounterToPerformanceCounter)(
        _In_      ULONG64  AuxiliaryCounterValue,
        _Out_     PLARGE_INTEGER PerformanceCounterValue,
        _Out_opt_ PLARGE_INTEGER ConversionError
    )
    {
    #ifdef _KERNEL_MODE
        return KeConvertAuxiliaryCounterToPerformanceCounter(AuxiliaryCounterValue,
            reinterpret_cast<PULONG64>(PerformanceCounterValue), reinterpret_cast<PULONG64>(ConversionError));
    #else
        LARGE_INTEGER Value;
        Value.QuadPart = AuxiliaryCounterValue;

        return ZwConvertBetweenAuxiliaryCounterAndPerformanceCounter(
            TRUE, &Value, PerformanceCounterValue, ConversionError);

    #endif
    }
    MI_IAT_SYMBOL(RtlConvertAuxiliaryCounterToPerformanceCounter, 16);

    NTSTATUS NTAPI MI_NAME(RtlConvertPerformanceCounterToAuxiliaryCounter)(
        _In_      ULONG64  PerformanceCounterValue,
        _Out_     PLARGE_INTEGER AuxiliaryCounterValue,
        _Out_opt_ PLARGE_INTEGER ConversionError
    )
    {
    #ifdef _KERNEL_MODE
        return KeConvertPerformanceCounterToAuxiliaryCounter(PerformanceCounterValue,
            reinterpret_cast<PULONG64>(AuxiliaryCounterValue), reinterpret_cast<PULONG64>(ConversionError));
    #else
        LARGE_INTEGER Value;
        Value.QuadPart = PerformanceCounterValue;

        return ZwConvertBetweenAuxiliaryCounterAndPerformanceCounter(
            FALSE, &Value, AuxiliaryCounterValue, ConversionError);
    #endif
    }
    MI_IAT_SYMBOL(RtlConvertPerformanceCounterToAuxiliaryCounter, 16);


}
EXTERN_C_END


