#pragma once

EXTERN_C_START



BOOLEAN NTAPI RtlGetInterruptTime(
    _Out_ PLARGE_INTEGER InterruptTime);

BOOLEAN NTAPI MI_NAME(RtlGetInterruptTime)(
    _Out_ PLARGE_INTEGER InterruptTime);

#ifdef _X86_
_VEIL_DECLARE_ALTERNATE_NAME(RtlGetInterruptTime@4, _Mi_RtlGetInterruptTime@4);
#else
_VEIL_DECLARE_ALTERNATE_NAME(RtlGetInterruptTime, _Mi_RtlGetInterruptTime);
#endif


#if (NTDDI_VERSION >= NTDDI_WIN10_RS1)
LARGE_INTEGER NTAPI RtlQueryUnbiasedInterruptTimePrecise(
    _Out_ PLARGE_INTEGER PerformanceCount);

LARGE_INTEGER NTAPI MI_NAME(RtlQueryUnbiasedInterruptTimePrecise)(
    _Out_ PLARGE_INTEGER PerformanceCount);

#ifdef _X86_
_VEIL_DECLARE_ALTERNATE_NAME(RtlQueryUnbiasedInterruptTimePrecise@4, _Mi_RtlQueryUnbiasedInterruptTimePrecise@4);
#else
_VEIL_DECLARE_ALTERNATE_NAME(RtlQueryUnbiasedInterruptTimePrecise, _Mi_RtlQueryUnbiasedInterruptTimePrecise);
#endif
#endif // (NTDDI_VERSION >= NTDDI_WIN10_RS1)

NTSTATUS NTAPI RtlQueryAuxiliaryCounterFrequency(
    _Out_ PULONG64 AuxiliaryCounterFrequency
);

NTSTATUS NTAPI RtlConvertAuxiliaryCounterToPerformanceCounter(
    _In_      ULONG64  AuxiliaryCounterValue,
    _Out_     PULONG64 PerformanceCounterValue,
    _Out_opt_ PULONG64 ConversionError
);

NTSTATUS NTAPI RtlConvertPerformanceCounterToAuxiliaryCounter(
    _In_      ULONG64  PerformanceCounterValue,
    _Out_     PULONG64 AuxiliaryCounterValue,
    _Out_opt_ PULONG64 ConversionError
);



EXTERN_C_END
