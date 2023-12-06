

EXTERN_C_START
namespace Mi
{
    BOOL WINAPI MI_NAME(IsDebuggerPresent)()
    {
    #ifdef _KERNEL_MODE
        return !KdRefreshDebuggerNotPresent();
    #else
        return NtCurrentPeb()->BeingDebugged;
    #endif
    }
    MI_IAT_SYMBOL(IsDebuggerPresent, 0);

    VOID WINAPI MI_NAME(DebugBreak)()
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
