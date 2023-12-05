#pragma once
#ifdef _KERNEL_MODE


// Fiber Local storage

namespace Mi
{
    constexpr auto RTL_FLS_MAXIMUM_AVAILABLE = 256 - (sizeof(LIST_ENTRY) / sizeof(PVOID));

    VEIL_DECLARE_STRUCT(RTL_FLS_DATA)
    {
        LIST_ENTRY  Entry;
        PVOID       Slots[RTL_FLS_MAXIMUM_AVAILABLE];
    };

    VEIL_DECLARE_STRUCT(RTL_FLS_CONTEXT)
    {
        EX_SPIN_LOCK            Lock;
        PFLS_CALLBACK_FUNCTION* FlsCallback;
        LIST_ENTRY              FlsListHead;
        RTL_BITMAP              FlsBitmap;
        ULONG                   FlsBitmapBits[ROUND_TO_SIZE(RTL_FLS_MAXIMUM_AVAILABLE, sizeof(PVOID)) / RTL_BITS_OF(ULONG)];
        ULONG                   FlsHighIndex;
    };

}

EXTERN_C_START


_IRQL_requires_max_(APC_LEVEL)
VOID NTAPI MI_NAME_PRIVATE(RtlFlsCreate)();

_IRQL_requires_max_(APC_LEVEL)
VOID NTAPI MI_NAME_PRIVATE(RtlFlsCleanup)();

_IRQL_requires_max_(APC_LEVEL)
VOID NTAPI MI_NAME_PRIVATE(RtlFlsDataCleanup)(
    _In_ Mi::PRTL_FLS_DATA FlsData,
    _In_ ULONG Flags
    );

_IRQL_requires_max_(APC_LEVEL)
VOID NTAPI MI_NAME(RtlProcessFlsData)(
    _In_ PVOID FlsData,
    _In_ ULONG Flags
    );

EXTERN_C_END


#endif // _KERNEL_MODE
