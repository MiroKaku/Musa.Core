EXTERN_C_START

_IRQL_requires_max_(PASSIVE_LEVEL) BOOLEAN WINAPI MUSA_NAME(RtlIsProcessorFeaturePresent)(
    _In_ ULONG ProcessorFeature
)
{
    return ExIsProcessorFeaturePresent(ProcessorFeature);
}

MUSA_IAT_SYMBOL(RtlIsProcessorFeaturePresent, 4);

ULONG WINAPI MUSA_NAME(RtlGetCurrentProcessorNumber)(VOID)
{
    return KeGetCurrentProcessorNumber();
}

MUSA_IAT_SYMBOL(RtlGetCurrentProcessorNumber, 0);

VOID WINAPI MUSA_NAME(RtlGetCurrentProcessorNumberEx)(_Out_ PPROCESSOR_NUMBER ProcessorNumber)
{
    (void)KeGetCurrentProcessorNumberEx(ProcessorNumber);
}

MUSA_IAT_SYMBOL(RtlGetCurrentProcessorNumberEx, 4);

EXTERN_C_END
