#pragma once


#ifdef _KERNEL_MODE

EXTERN_C_START

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlMapResourceId)(
    _Out_ ULONG_PTR* To,
    _In_  LPCWSTR    From
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID NTAPI MUSA_NAME(RtlUnmapResourceId)(
    _In_ ULONG_PTR Id
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlFindResource)(
    _Out_ HRSRC* ResBase,
    _In_  PVOID   DllHandle,
    _In_  LPCWSTR Name,
    _In_  LPCWSTR Type,
    _In_  UINT16  Language
);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlLoadResource)(
    _Out_ PVOID* ResBuff,
    _Out_ ULONG* ResSize,
    _In_  HRSRC  ResBase,
    _In_  PVOID  DllHandle
);

EXTERN_C_END

#endif
