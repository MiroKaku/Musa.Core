

EXTERN_C_START


//
// FLS - Fiber Local Storage
//

DWORD WINAPI MI_NAME(FlsAlloc)(
    _In_opt_ PFLS_CALLBACK_FUNCTION Callback
)
{
    ULONG FlsIndex = FLS_OUT_OF_INDEXES;

    const auto Status = RtlFlsAlloc(Callback, &FlsIndex);
    if (!NT_SUCCESS(Status)) {
        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
    }

    return FlsIndex;
}
MI_IAT_SYMBOL(FlsAlloc, 4);


PVOID WINAPI MI_NAME(FlsGetValue)(
    _In_ DWORD FlsIndex
)
{
    PVOID FlsData = nullptr;

    const auto Status  = RtlFlsGetValue(FlsIndex, &FlsData);
    if (!NT_SUCCESS(Status)) {
        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
    }

    return FlsData;
}
MI_IAT_SYMBOL(FlsGetValue, 4);


BOOL WINAPI MI_NAME(FlsSetValue)(
    _In_ DWORD FlsIndex,
    _In_opt_ PVOID FlsData
)
{
    const auto Status = RtlFlsSetValue(FlsIndex, FlsData);
    if (!NT_SUCCESS(Status)) {
        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        return FALSE;
    }

    return TRUE;
}
MI_IAT_SYMBOL(FlsSetValue, 8);


BOOL WINAPI MI_NAME(FlsFree)(
    _In_ DWORD FlsIndex
)
{
    const auto Status = RtlFlsFree(FlsIndex);
    if (!NT_SUCCESS(Status)) {
        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        return FALSE;
    }

    return TRUE;
}
MI_IAT_SYMBOL(FlsFree, 4);

//
// TLS - Thread Local Storage 
//

_Must_inspect_result_
DWORD WINAPI MI_NAME(TlsAlloc)(
    VOID
)
{
    return FlsAlloc(nullptr);
}
MI_IAT_SYMBOL(TlsAlloc, 0);


LPVOID WINAPI MI_NAME(TlsGetValue)(
    _In_ DWORD TlsIndex
)
{
    return FlsGetValue(TlsIndex);
}
MI_IAT_SYMBOL(TlsGetValue, 4);

BOOL WINAPI MI_NAME(TlsSetValue)(
    _In_ DWORD TlsIndex,
    _In_opt_ LPVOID TlsValue
)
{
    return FlsSetValue(TlsIndex, TlsValue);
}
MI_IAT_SYMBOL(TlsSetValue, 8);

BOOL WINAPI MI_NAME(TlsFree)(
    _In_ DWORD TlsIndex
)
{
    return FlsFree(TlsIndex);
}
MI_IAT_SYMBOL(TlsFree, 4);


EXTERN_C_END
