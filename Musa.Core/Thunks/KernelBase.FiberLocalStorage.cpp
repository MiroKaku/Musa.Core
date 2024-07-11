#include "KernelBase.Private.h"


EXTERN_C_START
namespace Musa
{
    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Must_inspect_result_
    DWORD WINAPI MUSA_NAME(FlsAlloc)(
        _In_opt_ PFLS_CALLBACK_FUNCTION Callback
    )
    {
        ULONG FlsIndex = FLS_OUT_OF_INDEXES;

    #pragma warning(suppress: 6387)
        const auto Status = RtlFlsAlloc(Callback, &FlsIndex);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
        }

        return FlsIndex;
    }
    MUSA_IAT_SYMBOL(FlsAlloc, 4);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    PVOID WINAPI MUSA_NAME(FlsGetValue)(
        _In_ DWORD FlsIndex
    )
    {
        PVOID Value = nullptr;

        const auto Status = RtlFlsGetValue(FlsIndex, &Value);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
        }

        return Value;
    }
    MUSA_IAT_SYMBOL(FlsGetValue, 4);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    BOOL WINAPI MUSA_NAME(FlsSetValue)(
        _In_ DWORD FlsIndex,
        _In_opt_ PVOID FlsData
    )
    {
    #pragma warning(suppress: 6387)
        const auto Status = RtlFlsSetValue(FlsIndex, FlsData);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MUSA_IAT_SYMBOL(FlsSetValue, 8);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    BOOL WINAPI MUSA_NAME(FlsFree)(
        _In_ DWORD FlsIndex
    )
    {
        const auto Status = RtlFlsFree(FlsIndex);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MUSA_IAT_SYMBOL(FlsFree, 4);
    
    _IRQL_requires_max_(DISPATCH_LEVEL)
    NTSTATUS NTAPI MUSA_NAME(RtlFlsGetValue)(
        _In_  ULONG  FlsIndex,
        _Out_ PVOID* FlsData
    );

#if defined _M_IX86
    _VEIL_DEFINE_IAT_RAW_SYMBOL(FlsGetValue2 ## @ ## 4, MUSA_NAME(RtlFlsGetValue))
    #else
    _VEIL_DEFINE_IAT_SYMBOL(FlsGetValue2, MUSA_NAME(RtlFlsGetValue));
#endif

}
EXTERN_C_END
