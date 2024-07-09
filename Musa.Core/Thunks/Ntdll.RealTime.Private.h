#pragma once

EXTERN_C_START



BOOLEAN NTAPI RtlGetInterruptTime(
    _Out_ PLARGE_INTEGER InterruptTime);

BOOLEAN NTAPI MUSA_NAME(RtlGetInterruptTime)(
    _Out_ PLARGE_INTEGER InterruptTime);

#ifdef _X86_
_VEIL_DECLARE_ALTERNATE_NAME(RtlGetInterruptTime@4, _Musa_RtlGetInterruptTime@4);
#else
_VEIL_DECLARE_ALTERNATE_NAME(RtlGetInterruptTime, _Musa_RtlGetInterruptTime);
#endif


#if (NTDDI_VERSION >= NTDDI_WIN10_RS1)
LARGE_INTEGER NTAPI RtlQueryUnbiasedInterruptTimePrecise(
    _Out_ PLARGE_INTEGER PerformanceCount);

LARGE_INTEGER NTAPI MUSA_NAME(RtlQueryUnbiasedInterruptTimePrecise)(
    _Out_ PLARGE_INTEGER PerformanceCount);

#ifdef _X86_
_VEIL_DECLARE_ALTERNATE_NAME(RtlQueryUnbiasedInterruptTimePrecise@4, _Musa_RtlQueryUnbiasedInterruptTimePrecise@4);
#else
_VEIL_DECLARE_ALTERNATE_NAME(RtlQueryUnbiasedInterruptTimePrecise, _Musa_RtlQueryUnbiasedInterruptTimePrecise);
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

#ifdef _X86_
_VEIL_DECLARE_ALTERNATE_NAME(RtlQueryAuxiliaryCounterFrequency@4, _Musa_RtlQueryAuxiliaryCounterFrequency@4);
_VEIL_DECLARE_ALTERNATE_NAME(RtlConvertAuxiliaryCounterToPerformanceCounter@16, _Musa_RtlConvertAuxiliaryCounterToPerformanceCounter@16);
_VEIL_DECLARE_ALTERNATE_NAME(RtlConvertPerformanceCounterToAuxiliaryCounter@16, _Musa_RtlConvertPerformanceCounterToAuxiliaryCounter@16);
#else
_VEIL_DECLARE_ALTERNATE_NAME(RtlQueryAuxiliaryCounterFrequency, _Musa_RtlQueryAuxiliaryCounterFrequency);
_VEIL_DECLARE_ALTERNATE_NAME(RtlConvertAuxiliaryCounterToPerformanceCounter, _Musa_RtlConvertAuxiliaryCounterToPerformanceCounter);
_VEIL_DECLARE_ALTERNATE_NAME(RtlConvertPerformanceCounterToAuxiliaryCounter, _Musa_RtlConvertPerformanceCounterToAuxiliaryCounter);
#endif



EXTERN_C_END
