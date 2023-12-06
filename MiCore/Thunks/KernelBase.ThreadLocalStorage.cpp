

EXTERN_C_START
namespace Mi
{
    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Must_inspect_result_
    DWORD WINAPI MI_NAME(TlsAlloc)()
    {
        return FlsAlloc(nullptr);
    }
    MI_IAT_SYMBOL(TlsAlloc, 0);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    LPVOID WINAPI MI_NAME(TlsGetValue)(
        _In_ DWORD TlsIndex
        )
    {
        return FlsGetValue(TlsIndex);
    }
    MI_IAT_SYMBOL(TlsGetValue, 4);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    BOOL WINAPI MI_NAME(TlsSetValue)(
        _In_ DWORD TlsIndex,
        _In_opt_ LPVOID TlsValue
        )
    {
        return FlsSetValue(TlsIndex, TlsValue);
    }
    MI_IAT_SYMBOL(TlsSetValue, 8);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    BOOL WINAPI MI_NAME(TlsFree)(
        _In_ DWORD TlsIndex
        )
    {
        return FlsFree(TlsIndex);
    }
    MI_IAT_SYMBOL(TlsFree, 4);

}
EXTERN_C_END
