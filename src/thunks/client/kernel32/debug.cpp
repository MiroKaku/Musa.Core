

EXTERN_C_START


VOID WINAPI MI_NAME(OutputDebugStringA)(
    _In_opt_ LPCSTR OutputString
)
{
    DbgPrint(OutputString);
}
MI_IAT_SYMBOL(OutputDebugStringA, 4);

VOID WINAPI MI_NAME(OutputDebugStringW)(
    _In_opt_ LPCWSTR OutputString
)
{
    DbgPrint("%ls", OutputString);
}
MI_IAT_SYMBOL(OutputDebugStringW, 4);


EXTERN_C_END
