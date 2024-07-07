

EXTERN_C_START
namespace Musa
{
    BOOL WINAPI MUSA_NAME(QueryPerformanceCounter)(
        _Out_ PLARGE_INTEGER PerformanceCount
    )
    {
        return RtlQueryPerformanceCounter(PerformanceCount);
    }
    MUSA_IAT_SYMBOL(QueryPerformanceCounter, 4);

    BOOL WINAPI MUSA_NAME(QueryPerformanceFrequency)(
        _Out_ PLARGE_INTEGER Frequency)
    {
        return RtlQueryPerformanceFrequency(Frequency);
    }
    MUSA_IAT_SYMBOL(QueryPerformanceFrequency, 4);

}
EXTERN_C_END
