#pragma once
#ifdef _KERNEL_MODE


EXTERN_C_START


_IRQL_requires_max_(APC_LEVEL)
VOID NTAPI MI_NAME_PRIVATE(RtlRemoveFlsData)(
    _In_ PFLS_DATA FlsData
    );

_IRQL_requires_max_(APC_LEVEL)
VOID NTAPI MI_NAME_PRIVATE(RtlRemoveFlsDataAll)();


EXTERN_C_END


#endif // _KERNEL_MODE
