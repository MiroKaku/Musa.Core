

EXTERN_C_START
namespace Mi
{
    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Must_inspect_result_
    DWORD WINAPI MI_NAME(FlsAlloc)(
        _In_opt_ PFLS_CALLBACK_FUNCTION Callback
    )
    {
        ULONG FlsIndex = FLS_OUT_OF_INDEXES;

    #pragma warning(suppress: 6387)
        const auto Status = RtlFlsAlloc(Callback, &FlsIndex);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        }

        return FlsIndex;
    }
    MI_IAT_SYMBOL(FlsAlloc, 4);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    PVOID WINAPI MI_NAME(FlsGetValue)(
        _In_ DWORD FlsIndex
    )
    {
        PVOID Value = nullptr;

        const auto Status = RtlFlsGetValue(FlsIndex, &Value);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
        }

        return Value;
    }
    MI_IAT_SYMBOL(FlsGetValue, 4);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    BOOL WINAPI MI_NAME(FlsSetValue)(
        _In_ DWORD FlsIndex,
        _In_opt_ PVOID FlsData
    )
    {
    #pragma warning(suppress: 6387)
        const auto Status = RtlFlsSetValue(FlsIndex, FlsData);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(FlsSetValue, 8);

    _IRQL_requires_max_(DISPATCH_LEVEL)
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

}
EXTERN_C_END
