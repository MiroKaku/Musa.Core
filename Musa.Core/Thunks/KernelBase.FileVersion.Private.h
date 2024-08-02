#pragma once


EXTERN_C_START

#if defined(_KERNEL_MODE)

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL APIENTRY MUSA_NAME(GetFileVersionInfoW)(
    _In_                LPCWSTR Filename,   /* Filename of version stamped file */
    _Reserved_          DWORD Handle,       /* Information from GetFileVersionSize */
    _In_                DWORD Length,       /* Length of buffer for info */
    _Out_writes_bytes_(Length) LPVOID Data   /* Buffer to place the data structure */
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD APIENTRY MUSA_NAME(GetFileVersionInfoSizeW)(
    _In_      LPCWSTR Filename,  /* Filename of version stamped file */
    _Out_opt_ LPDWORD Handle     /* Information for use by GetFileVersionInfo */
    );

BOOL APIENTRY MUSA_NAME(VerQueryValueW)(
    _In_ LPCVOID Block,
    _In_ LPCWSTR SubBlock,
    _Outptr_result_buffer_(_Inexpressible_("buffer can be PWSTR or DWORD*")) LPVOID* Buffer,
    _Out_ PUINT Length
    );

#endif

EXTERN_C_END
