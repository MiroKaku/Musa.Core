

EXTERN_C_START
namespace Mi
{
    BOOL WINAPI MI_NAME(IsDebuggerPresent)(
        VOID
        )
    {
        return RtlIsAnyDebuggerPresent();
    }
    MI_IAT_SYMBOL(IsDebuggerPresent, 0);

    VOID WINAPI MI_NAME(DebugBreak)(
        VOID
        )
    {
        __debugbreak();
    }
    MI_IAT_SYMBOL(DebugBreak, 0);

    VOID WINAPI MI_NAME(OutputDebugStringA)(
        _In_opt_ LPCSTR OutputString
    )
    {
        DbgPrint("%hs", OutputString);
    }
    MI_IAT_SYMBOL(OutputDebugStringA, 4);

    VOID WINAPI MI_NAME(OutputDebugStringW)(
        _In_opt_ LPCWSTR OutputString
    )
    {
        DbgPrint("%ls", OutputString);
    }
    MI_IAT_SYMBOL(OutputDebugStringW, 4);

}
EXTERN_C_END
