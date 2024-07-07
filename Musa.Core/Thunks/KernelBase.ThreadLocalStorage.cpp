

EXTERN_C_START
namespace Musa
{
    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Must_inspect_result_
    DWORD WINAPI MUSA_NAME(TlsAlloc)()
    {
        return FlsAlloc(nullptr);
    }
    MUSA_IAT_SYMBOL(TlsAlloc, 0);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    LPVOID WINAPI MUSA_NAME(TlsGetValue)(
        _In_ DWORD TlsIndex
        )
    {
        return FlsGetValue(TlsIndex);
    }
    MUSA_IAT_SYMBOL(TlsGetValue, 4);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    BOOL WINAPI MUSA_NAME(TlsSetValue)(
        _In_ DWORD TlsIndex,
        _In_opt_ LPVOID TlsValue
        )
    {
        return FlsSetValue(TlsIndex, TlsValue);
    }
    MUSA_IAT_SYMBOL(TlsSetValue, 8);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    BOOL WINAPI MUSA_NAME(TlsFree)(
        _In_ DWORD TlsIndex
        )
    {
        return FlsFree(TlsIndex);
    }
    MUSA_IAT_SYMBOL(TlsFree, 4);

}
EXTERN_C_END
