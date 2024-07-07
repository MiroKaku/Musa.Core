

EXTERN_C_START
namespace Musa
{
    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOLEAN WINAPI MUSA_NAME(RtlIsProcessorFeaturePresent)(
        _In_ ULONG ProcessorFeature
        )
    {
    #ifdef _KERNEL_MODE
        return ExIsProcessorFeaturePresent(ProcessorFeature);
    #else
        if (ProcessorFeature < PROCESSOR_FEATURE_MAX) {
            return SharedUserData->ProcessorFeatures[ProcessorFeature];
        }
        return FALSE;
    #endif
    }
    MUSA_IAT_SYMBOL(RtlIsProcessorFeaturePresent, 4);

    ULONG WINAPI MUSA_NAME(RtlGetCurrentProcessorNumber)(
        VOID
        )
    {
    #ifdef _KERNEL_MODE
        return KeGetCurrentProcessorNumber();
    #else
        return ZwGetCurrentProcessorNumber();
    #endif
    }
    MUSA_IAT_SYMBOL(RtlGetCurrentProcessorNumber, 0);

    VOID WINAPI MUSA_NAME(RtlGetCurrentProcessorNumberEx)(
        _Out_ PPROCESSOR_NUMBER ProcessorNumber
        )
    {
    #ifdef _KERNEL_MODE
        (void)KeGetCurrentProcessorNumberEx(ProcessorNumber);
    #else
        (void)ZwGetCurrentProcessorNumberEx(ProcessorNumber);
    #endif
    }
    MUSA_IAT_SYMBOL(RtlGetCurrentProcessorNumberEx, 4);


}
EXTERN_C_END
