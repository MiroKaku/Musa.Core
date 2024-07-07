#ifdef _KERNEL_MODE


EXTERN_C_START
namespace Musa
{
    LOGICAL NTAPI MUSA_NAME(RtlQueryPerformanceCounter)(
        _Out_ PLARGE_INTEGER PerformanceCount
        )
    {
        *PerformanceCount = KeQueryPerformanceCounter(nullptr);
        return TRUE;
    }
    MUSA_IAT_SYMBOL(RtlQueryPerformanceCounter, 4);

    LOGICAL NTAPI MUSA_NAME(RtlQueryPerformanceFrequency)(
        _Out_ PLARGE_INTEGER Frequency)
    {
        KeQueryPerformanceCounter(Frequency);
        return TRUE;
    }
    MUSA_IAT_SYMBOL(RtlQueryPerformanceFrequency, 4);

}
EXTERN_C_END


#endif
