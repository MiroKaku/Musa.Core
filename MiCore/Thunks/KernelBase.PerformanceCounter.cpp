

EXTERN_C_START
namespace Mi
{
    BOOL WINAPI MI_NAME(QueryPerformanceCounter)(
        _Out_ PLARGE_INTEGER PerformanceCount
    )
    {
        return RtlQueryPerformanceCounter(PerformanceCount);
    }
    MI_IAT_SYMBOL(QueryPerformanceCounter, 4);

    BOOL WINAPI MI_NAME(QueryPerformanceFrequency)(
        _Out_ PLARGE_INTEGER Frequency)
    {
        return RtlQueryPerformanceFrequency(Frequency);
    }
    MI_IAT_SYMBOL(QueryPerformanceFrequency, 4);

}
EXTERN_C_END
