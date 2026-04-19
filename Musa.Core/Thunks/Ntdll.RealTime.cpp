EXTERN_C_START
LARGE_INTEGER NTAPI MUSA_NAME(RtlGetSystemTimePrecise)(VOID)
{
    LARGE_INTEGER SystemTime{};
    KeQuerySystemTimePrecise(&SystemTime);

    return SystemTime;
}

MUSA_IAT_SYMBOL(RtlGetSystemTimePrecise, 0);

BOOLEAN NTAPI MUSA_NAME(RtlGetInterruptTime)(
    _Out_ PLARGE_INTEGER InterruptTime
)
{
    InterruptTime->QuadPart = (LONGLONG)KeQueryInterruptTime();
    return TRUE;
}

MUSA_IAT_SYMBOL(RtlGetInterruptTime, 4);

LARGE_INTEGER /* InterruptTime */ NTAPI MUSA_NAME(RtlGetInterruptTimePrecise)(
    _Out_ PLARGE_INTEGER PerformanceCount
)
{
    LARGE_INTEGER InterruptTime;
    InterruptTime.QuadPart = (LONGLONG)KeQueryInterruptTimePrecise(reinterpret_cast<PULONG64>(PerformanceCount));

    return InterruptTime;
}

MUSA_IAT_SYMBOL(RtlGetInterruptTimePrecise, 4);

BOOLEAN NTAPI MUSA_NAME(RtlQueryUnbiasedInterruptTime)(
    _Out_ PLARGE_INTEGER InterruptTime
)
{
    InterruptTime->QuadPart = (LONGLONG)KeQueryUnbiasedInterruptTime();
    return TRUE;
}

MUSA_IAT_SYMBOL(RtlQueryUnbiasedInterruptTime, 4);

#if (NTDDI_VERSION >= NTDDI_WIN10_RS1)
LARGE_INTEGER NTAPI MUSA_NAME(RtlQueryUnbiasedInterruptTimePrecise)(
    _Out_ PLARGE_INTEGER PerformanceCount
)
{
    LARGE_INTEGER InterruptTime;
    InterruptTime.QuadPart = (LONGLONG)KeQueryUnbiasedInterruptTimePrecise(
        reinterpret_cast<PULONG64>(PerformanceCount));

    return InterruptTime;
}

MUSA_IAT_SYMBOL(RtlQueryUnbiasedInterruptTimePrecise, 4);
#endif // (NTDDI_VERSION >= NTDDI_WIN10_RS1)

NTSTATUS NTAPI MUSA_NAME(RtlQueryAuxiliaryCounterFrequency)(
    _Out_ PLARGE_INTEGER AuxiliaryCounterFrequency
)
{
    return KeQueryAuxiliaryCounterFrequency(reinterpret_cast<PULONG64>(AuxiliaryCounterFrequency));
}

MUSA_IAT_SYMBOL(RtlQueryAuxiliaryCounterFrequency, 4);

NTSTATUS NTAPI MUSA_NAME(RtlConvertAuxiliaryCounterToPerformanceCounter)(
    _In_ ULONG64             AuxiliaryCounterValue,
    _Out_ PLARGE_INTEGER     PerformanceCounterValue,
    _Out_opt_ PLARGE_INTEGER ConversionError
)
{
    return KeConvertAuxiliaryCounterToPerformanceCounter(AuxiliaryCounterValue,
        reinterpret_cast<PULONG64>(PerformanceCounterValue), reinterpret_cast<PULONG64>(ConversionError));
}

MUSA_IAT_SYMBOL(RtlConvertAuxiliaryCounterToPerformanceCounter, 16);

NTSTATUS NTAPI MUSA_NAME(RtlConvertPerformanceCounterToAuxiliaryCounter)(
    _In_ ULONG64             PerformanceCounterValue,
    _Out_ PLARGE_INTEGER     AuxiliaryCounterValue,
    _Out_opt_ PLARGE_INTEGER ConversionError
)
{
    return KeConvertPerformanceCounterToAuxiliaryCounter(PerformanceCounterValue,
        reinterpret_cast<PULONG64>(AuxiliaryCounterValue), reinterpret_cast<PULONG64>(ConversionError));
}

MUSA_IAT_SYMBOL(RtlConvertPerformanceCounterToAuxiliaryCounter, 16);

ULONG NTAPI MUSA_NAME(RtlGetTickCount)(VOID)
{
    return ZwGetTickCount();
}

MUSA_IAT_SYMBOL(RtlGetTickCount, 0);

LARGE_INTEGER NTAPI MUSA_NAME(RtlGetTickCount64)(VOID)
{
    return {.QuadPart = (LONGLONG)ZwGetTickCount64()};
}

MUSA_IAT_SYMBOL(RtlGetTickCount64, 0);

EXTERN_C_END
