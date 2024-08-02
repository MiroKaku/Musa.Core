#pragma once


#ifdef _KERNEL_MODE

EXTERN_C_START

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrUnloadDll)(
    _In_ PVOID DllHandle
    );

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrGetDllHandle)(
    _In_opt_ PWSTR       DllPath,
    _In_opt_ PULONG      DllCharacteristics,
    _In_ PUNICODE_STRING DllName,
    _Out_ PVOID* DllHandle
    );

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrGetDllHandleEx)(
    _In_ ULONG           Flags,
    _In_opt_ PWSTR       DllPath,
    _In_opt_ PULONG      DllCharacteristics,
    _In_ PUNICODE_STRING DllName,
    _Out_ PVOID* DllHandle
    );

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrGetDllHandleByMapping)(
    _In_ PVOID   BaseAddress,
    _Out_ PVOID* DllHandle
    );

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrGetDllHandleByName)(
    _In_opt_ PUNICODE_STRING BaseDllName,
    _In_opt_ PUNICODE_STRING FullDllName,
    _Out_ PVOID* DllHandle
    );

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrGetDllFullName)(
    _In_ PVOID            DllHandle,
    _Out_ PUNICODE_STRING FullDllName
    );

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrAddRefDll)(
    _In_ ULONG Flags,
    _In_ PVOID DllHandle
    );

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrGetProcedureAddress)(
    _In_ PVOID            DllHandle,
    _In_opt_ PANSI_STRING ProcedureName,
    _In_opt_ ULONG        ProcedureNumber,
    _Out_ PVOID* ProcedureAddress
    );

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrGetProcedureAddressEx)(
    _In_ PVOID            DllHandle,
    _In_opt_ PANSI_STRING ProcedureName,
    _In_opt_ ULONG        ProcedureNumber,
    _Out_ PVOID* ProcedureAddress,
    _In_ ULONG            Flags
    );

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrGetProcedureAddressForCaller)(
    _In_ PVOID            DllHandle,
    _In_opt_ PANSI_STRING ProcedureName,
    _In_opt_ ULONG        ProcedureNumber,
    _Out_ PVOID* ProcedureAddress,
    _In_ ULONG            Flags,
    _In_ PVOID* Callback
    );

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrGetKnownDllSectionHandle)(
    _In_ PCWSTR   DllName,
    _In_ BOOLEAN  KnownDlls32,
    _Out_ PHANDLE Section
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrLoadDataFile)(
    _In_  PCWSTR DllName,
    _Out_ PVOID* DllHandle
    );

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrUnloadDataFile)(
    _In_ PVOID DllHandle
    );



EXTERN_C_END

#endif
