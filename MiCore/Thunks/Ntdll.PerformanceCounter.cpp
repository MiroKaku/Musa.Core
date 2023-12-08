#ifdef _KERNEL_MODE


EXTERN_C_START
namespace Mi
{
    LOGICAL WINAPI MI_NAME(RtlQueryPerformanceCounter)(
        _Out_ PLARGE_INTEGER PerformanceCount
        )
    {
        *PerformanceCount = KeQueryPerformanceCounter(nullptr);
        return TRUE;
    }
    MI_IAT_SYMBOL(RtlQueryPerformanceCounter, 4);

    LOGICAL WINAPI MI_NAME(RtlQueryPerformanceFrequency)(
        _Out_ PLARGE_INTEGER Frequency)
    {
        KeQueryPerformanceCounter(Frequency);
        return TRUE;
    }
    MI_IAT_SYMBOL(RtlQueryPerformanceFrequency, 4);

}
EXTERN_C_END


#endif
