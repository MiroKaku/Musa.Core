#pragma once

#if defined(_KERNEL_MODE)

namespace Musa::Core
{
    //
    // Fiber Local storage
    //

    constexpr auto RTLP_FLS_MAXIMUM_AVAILABLE = 256 - (sizeof(LIST_ENTRY) / sizeof(PVOID));

    VEIL_DECLARE_STRUCT(RTL_FLS_DATA)
    {
        LIST_ENTRY  Entry;
        PVOID       Slots[RTLP_FLS_MAXIMUM_AVAILABLE];
    };

    VEIL_DECLARE_STRUCT(RTL_FLS_CONTEXT)
    {
        EX_SPIN_LOCK            Lock;
        PFLS_CALLBACK_FUNCTION* FlsCallback;
        LIST_ENTRY              FlsListHead;
        RTL_BITMAP              FlsBitmap;
        ULONG                   FlsBitmapBits[ROUND_TO_SIZE(RTLP_FLS_MAXIMUM_AVAILABLE, sizeof(PVOID)) / RTL_BITS_OF(ULONG)];
        ULONG                   FlsHighIndex;
    };
}


EXTERN_C_START

_IRQL_requires_max_(APC_LEVEL)
VOID NTAPI MUSA_NAME_PRIVATE(FlsCreate)();

_IRQL_requires_max_(APC_LEVEL)
VOID NTAPI MUSA_NAME_PRIVATE(FlsCleanup)();

_IRQL_requires_max_(APC_LEVEL)
VOID NTAPI MUSA_NAME_PRIVATE(FlsDataCleanup)(
    _In_ PVOID FlsData,
    _In_ ULONG Flags
    );

EXTERN_C_END

#endif // defined(_KERNEL_MODE)
