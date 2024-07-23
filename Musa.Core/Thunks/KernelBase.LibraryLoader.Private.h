#pragma once


EXTERN_C_START


#if defined(_KERNEL_MODE)
_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MUSA_NAME(FreeLibrary)(
    _In_ HMODULE DllHandle
    );
#endif

_IRQL_requires_max_(APC_LEVEL)
_Success_(return != 0)
_Ret_range_(1, nSize)
DWORD WINAPI MUSA_NAME(GetModuleFileNameW)(
    _In_opt_ HMODULE DllHandle,
    _Out_writes_to_(Size, ((return < Size) ? (return +1) : Size)) LPWSTR DllFullName,
    _In_ DWORD Size
    );

_IRQL_requires_max_(APC_LEVEL)
_When_(lpModuleName == NULL, _Ret_notnull_)
_When_(lpModuleName != NULL, _Ret_maybenull_)
HMODULE WINAPI MUSA_NAME(GetModuleHandleW)(
    _In_opt_ LPCWSTR ModuleName
    );

_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MUSA_NAME(GetModuleHandleExW)(
    _In_ DWORD Flags,
    _In_opt_ LPCWSTR ModuleName,
    _Out_ HMODULE * DllHandle
    );

_IRQL_requires_max_(APC_LEVEL)
FARPROC WINAPI MUSA_NAME(GetProcAddress)(
    _In_ HMODULE Module,
    _In_ LPCSTR ProcedureName
    );


EXTERN_C_END
