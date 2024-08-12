#pragma once


EXTERN_C_START


#ifdef _KERNEL_MODE

_IRQL_requires_max_(PASSIVE_LEVEL)
_Success_(return != 0)
_When_((CountOfMultiByte == -1) && (CountOfWideChar != 0), _Post_equal_to_(_String_length_(WideCharStr) + 1))
int WINAPI MUSA_NAME(MultiByteToWideChar)(
    _In_ UINT CodePage,
    _In_ DWORD Flags,
    _In_NLS_string_(CountOfMultiByte) LPCCH MultiByteStr,
    _In_ int CountOfMultiByte,
    _Out_writes_to_opt_(CountOfWideChar, return) LPWSTR WideCharStr,
    _In_ int CountOfWideChar
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
_Success_(return != 0)
_When_((CountOfWideChar == -1) && (CountOfMultiByte != 0), _Post_equal_to_(_String_length_(MultiByteStr) + 1))
int WINAPI MUSA_NAME(WideCharToMultiByte)(
    _In_ UINT CodePage,
    _In_ DWORD Flags,
    _In_NLS_string_(CountOfWideChar) LPCWCH WideCharStr,
    _In_ int CountOfWideChar,
    _Out_writes_bytes_to_opt_(CountOfMultiByte, return) LPSTR MultiByteStr,
    _In_ int CountOfMultiByte,
    _In_opt_ LPCCH DefaultChar,
    _Out_opt_ LPBOOL UsedDefaultChar
    );

#endif

EXTERN_C_END


