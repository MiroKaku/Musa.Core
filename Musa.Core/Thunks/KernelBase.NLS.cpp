//
// National Language Support
//

#include "KernelBase.Private.h"
#include "Internal/KernelBase.NLS.h"

#include "Internal/KernelBase.System.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(MultiByteToWideChar))
#pragma alloc_text(PAGE, MUSA_NAME(WideCharToMultiByte))
#pragma alloc_text(PAGE, MUSA_NAME(GetDateFormatEx))
#pragma alloc_text(PAGE, MUSA_NAME(GetTimeFormatEx))
#endif

#include "Internal/KernelBase.NLS.Table.cpp"

EXTERN_C_START

_IRQL_requires_max_(PASSIVE_LEVEL)
_Success_(return != 0)
_When_((CountOfMultiByte == -1) && (CountOfWideChar != 0), _Post_equal_to_(_String_length_(WideCharStr) + 1))
int WINAPI MUSA_NAME(MultiByteToWideChar)(
    _In_ UINT                                           CodePage,
    _In_ DWORD                                          Flags,
    _In_NLS_string_(CountOfMultiByte) LPCCH             MultiByteStr,
    _In_ int                                            CountOfMultiByte,
    _Out_writes_to_opt_(CountOfWideChar, return) LPWSTR WideCharStr,
    _In_ int                                            CountOfWideChar
)
{
    PAGED_CODE();

    int      ReturnLength = 0;
    NTSTATUS Status       = STATUS_NOT_SUPPORTED;

    do {
        ULONG BytesInUnicodeString = 0ul;

        if (CodePage == CP_ACP) {
            // user-mode MultiByteToWideChar treats -1 as "null-terminated":
            // length = strlen(s) + 1 (includes terminator). Match that.
            const int AcpByteCount = (CountOfMultiByte == -1)
                ? static_cast<int>(strlen(MultiByteStr) + 1)
                : CountOfMultiByte;

            Status = RtlMultiByteToUnicodeSize(&BytesInUnicodeString, MultiByteStr,
                static_cast<ULONG>(AcpByteCount));
            if (!NT_SUCCESS(Status)) {
                break;
            }

            ReturnLength = static_cast<int>(BytesInUnicodeString / sizeof(wchar_t));

            if (WideCharStr == nullptr || CountOfWideChar == 0) {
                break;
            }

            if (CountOfWideChar < ReturnLength) {
                ReturnLength = 0;
                break;
            }

            Status = RtlMultiByteToUnicodeN(WideCharStr, static_cast<ULONG>(CountOfWideChar * sizeof(wchar_t)),
                &BytesInUnicodeString, MultiByteStr, static_cast<ULONG>(AcpByteCount));
            if (!NT_SUCCESS(Status)) {
                ReturnLength = 0;
                break;
            }

            ReturnLength = static_cast<int>(BytesInUnicodeString / sizeof(wchar_t));
            break;
        }

        if (CodePage == CP_OEMCP) {
            const int OemByteCount = (CountOfMultiByte == -1)
                ? static_cast<int>(strlen(MultiByteStr) + 1)
                : CountOfMultiByte;

            Status = RtlMultiByteToUnicodeSize(&BytesInUnicodeString, MultiByteStr,
                static_cast<ULONG>(OemByteCount));
            if (!NT_SUCCESS(Status)) {
                break;
            }

            ReturnLength = static_cast<int>(BytesInUnicodeString / sizeof(wchar_t));

            if (WideCharStr == nullptr || CountOfWideChar == 0) {
                break;
            }

            if (CountOfWideChar < ReturnLength) {
                ReturnLength = 0;
                break;
            }

            Status = RtlOemToUnicodeN(WideCharStr, static_cast<ULONG>(CountOfWideChar * sizeof(wchar_t)),
                &BytesInUnicodeString, MultiByteStr, static_cast<ULONG>(OemByteCount));
            if (!NT_SUCCESS(Status)) {
                ReturnLength = 0;
                break;
            }

            ReturnLength = static_cast<int>(BytesInUnicodeString / sizeof(wchar_t));
            break;
        }

        if (CodePage == CP_UTF8) {
            // For UTF-8, MS docs say only 0 or MB_ERR_INVALID_CHARS is valid,
            // but UCRT/CRT internals routinely pass MB_PRECOMPOSED|MB_ERR_INVALID_CHARS.
            // MB_PRECOMPOSED is the implicit default for UTF-8; silently accept it.
            const DWORD AllowedFlags = MB_ERR_INVALID_CHARS | MB_PRECOMPOSED;
            if (Flags & ~AllowedFlags) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            const auto ByteCount = (CountOfMultiByte == -1)
                ? static_cast<int>(strlen(MultiByteStr) + 1)
                : CountOfMultiByte;

            // RtlUTF8ToUnicodeN doesn't support null output buffer for size query.
            const auto MaxBytes = static_cast<ULONG>((ByteCount + 1) * sizeof(wchar_t));
            auto TempBuffer = static_cast<PWCHAR>(LocalAlloc(LPTR, MaxBytes));
            if (TempBuffer == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            Status = RtlUTF8ToUnicodeN(TempBuffer, MaxBytes,
                &BytesInUnicodeString, MultiByteStr, ByteCount);

            ReturnLength = NT_SUCCESS(Status) ? static_cast<int>(BytesInUnicodeString / sizeof(wchar_t)) : 0;

            LocalFree(TempBuffer);

            if (!NT_SUCCESS(Status)) {
                break;
            }

            if (WideCharStr == nullptr || CountOfWideChar == 0) {
                break;
            }

            if (CountOfWideChar < ReturnLength) {
                ReturnLength = 0;
                break;
            }

            Status = RtlUTF8ToUnicodeN(WideCharStr, static_cast<ULONG>(CountOfWideChar * sizeof(wchar_t)),
                &BytesInUnicodeString, MultiByteStr, ByteCount);
            if (!NT_SUCCESS(Status)) {
                ReturnLength = 0;
                break;
            }

            ReturnLength = static_cast<int>(BytesInUnicodeString / sizeof(wchar_t));
            break;
        }
    } while (false);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
    }

    return ReturnLength;
}

MUSA_IAT_SYMBOL(MultiByteToWideChar, 24);

_IRQL_requires_max_(PASSIVE_LEVEL)
_Success_(return != 0)
_When_((CountOfWideChar == -1) && (CountOfMultiByte != 0), _Post_equal_to_(_String_length_(MultiByteStr) + 1))
int WINAPI MUSA_NAME(WideCharToMultiByte)(
    _In_ UINT                                                 CodePage,
    _In_ DWORD                                                Flags,
    _In_NLS_string_(CountOfWideChar) LPCWCH                   WideCharStr,
    _In_ int                                                  CountOfWideChar,
    _Out_writes_bytes_to_opt_(CountOfMultiByte, return) LPSTR MultiByteStr,
    _In_ int                                                  CountOfMultiByte,
    _In_opt_ LPCCH                                            DefaultChar,
    _Out_opt_ LPBOOL                                          UsedDefaultChar
)
{
    PAGED_CODE();

    int      ReturnLength = 0;
    NTSTATUS Status       = STATUS_NOT_SUPPORTED;

    do {
        if (UsedDefaultChar) {
            *UsedDefaultChar = FALSE;
        }

        ULONG BytesInMultiByteString = 0ul;

        if (CodePage == CP_ACP) {
            // -1 means null-terminated wide string; convert to char count.
            const int AcpCharCount = (CountOfWideChar == -1)
                ? static_cast<int>(wcslen(WideCharStr) + 1)
                : CountOfWideChar;

            Status = RtlUnicodeToMultiByteSize(&BytesInMultiByteString, WideCharStr,
                AcpCharCount * sizeof(wchar_t));
            if (!NT_SUCCESS(Status)) {
                break;
            }

            ReturnLength = static_cast<int>(BytesInMultiByteString);

            if (MultiByteStr == nullptr || CountOfMultiByte == 0) {
                break;
            }

            if (CountOfMultiByte < ReturnLength) {
                ReturnLength = 0;
                break;
            }

            Status = RtlUnicodeToMultiByteN(MultiByteStr, CountOfMultiByte,
                &BytesInMultiByteString, WideCharStr, AcpCharCount * sizeof(wchar_t));
            if (!NT_SUCCESS(Status)) {
                ReturnLength = 0;
                break;
            }

            ReturnLength = static_cast<int>(BytesInMultiByteString);
            break;
        }

        if (CodePage == CP_OEMCP) {
            const int OemCharCount = (CountOfWideChar == -1)
                ? static_cast<int>(wcslen(WideCharStr) + 1)
                : CountOfWideChar;

            Status = RtlUnicodeToMultiByteSize(&BytesInMultiByteString, WideCharStr,
                OemCharCount * sizeof(wchar_t));
            if (!NT_SUCCESS(Status)) {
                break;
            }

            ReturnLength = static_cast<int>(BytesInMultiByteString);

            if (MultiByteStr == nullptr || CountOfMultiByte == 0) {
                break;
            }

            if (CountOfMultiByte < ReturnLength) {
                ReturnLength = 0;
                break;
            }

            Status = RtlUnicodeToOemN(MultiByteStr, CountOfMultiByte,
                &BytesInMultiByteString, WideCharStr, OemCharCount * sizeof(wchar_t));
            if (!NT_SUCCESS(Status)) {
                ReturnLength = 0;
                break;
            }

            ReturnLength = static_cast<int>(BytesInMultiByteString);
            break;
        }

        if (CodePage == CP_UTF8) {
            if (Flags) {
                if (Flags != MB_ERR_INVALID_CHARS) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }
            }

            if (DefaultChar || UsedDefaultChar) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            const auto CharCount = (CountOfWideChar == -1)
                ? static_cast<int>(wcslen(WideCharStr) + 1)
                : CountOfWideChar;

            // RtlUnicodeToUTF8N doesn't support null output buffer for size query.
            const auto MaxBytes = static_cast<ULONG>(CharCount * 3 + 1);
            auto TempBuffer = static_cast<PCHAR>(LocalAlloc(LPTR, MaxBytes));
            if (TempBuffer == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            Status = RtlUnicodeToUTF8N(TempBuffer, MaxBytes,
                &BytesInMultiByteString, WideCharStr, CharCount * sizeof(wchar_t));

            ReturnLength = NT_SUCCESS(Status) ? static_cast<int>(BytesInMultiByteString) : 0;

            LocalFree(TempBuffer);

            if (!NT_SUCCESS(Status)) {
                break;
            }

            if (MultiByteStr == nullptr || CountOfMultiByte == 0) {
                break;
            }

            if (CountOfMultiByte < ReturnLength) {
                ReturnLength = 0;
                break;
            }

            Status = RtlUnicodeToUTF8N(MultiByteStr, CountOfMultiByte,
                &BytesInMultiByteString, WideCharStr, CharCount * sizeof(wchar_t));
            if (!NT_SUCCESS(Status)) {
                ReturnLength = 0;
                break;
            }

            ReturnLength = static_cast<int>(BytesInMultiByteString);
            break;
        }
    } while (false);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
    }

    return ReturnLength;
}

MUSA_IAT_SYMBOL(WideCharToMultiByte, 32);

UINT WINAPI MUSA_NAME(GetACP)()
{
    return NlsAnsiCodePage;
}

MUSA_IAT_SYMBOL(GetACP, 0);

UINT WINAPI MUSA_NAME(GetOEMCP)()
{
    return NlsOemCodePage;
}

MUSA_IAT_SYMBOL(GetOEMCP, 0);

BOOL WINAPI MUSA_NAME(GetCPInfo)(
    _In_ UINT      CodePage,
    _Out_ LPCPINFO CPInfo
)
{
    NTSTATUS Status = STATUS_NOT_FOUND;

    switch (CodePage) {
        default:
            break;
        case CP_ACP:
        case CP_THREAD_ACP:
            CodePage = NlsAnsiCodePage;
            break;
        case CP_OEMCP:
            CodePage = NlsOemCodePage;
            break;
    }

    for (const auto Item : NlsCodePageInfoTable) {
        if (CodePage == Item.CodePage) {
            CPInfo->MaxCharSize = Item.MaxCharSize;
            RtlCopyMemory(&CPInfo->DefaultChar, Item.DefaultChar, sizeof(CPInfo->DefaultChar));
            RtlCopyMemory(&CPInfo->LeadByte, Item.LeadByte, sizeof(CPInfo->LeadByte));

            Status = STATUS_SUCCESS;
            break;
        }
    }

    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(GetCPInfo, 8);

BOOL WINAPI MUSA_NAME(GetCPInfoExW)(
    _In_ UINT         CodePage,
    _In_ DWORD        Flags,
    _Out_ LPCPINFOEXW CPInfoEx
)
{
    NTSTATUS Status = STATUS_NOT_FOUND;

    do {
        if (Flags) {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        switch (CodePage) {
            default:
                break;
            case CP_ACP:
            case CP_THREAD_ACP:
                CodePage = NlsAnsiCodePage;
                break;
            case CP_OEMCP:
                CodePage = NlsOemCodePage;
                break;
        }

        for (const auto Item : NlsCodePageInfoTable) {
            if (CodePage == Item.CodePage) {
                *CPInfoEx = Item;

                Status = STATUS_SUCCESS;
                break;
            }
        }
    } while (false);

    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(GetCPInfoExW, 12);


_IRQL_requires_max_(PASSIVE_LEVEL)
int WINAPI MUSA_NAME(GetDateFormatEx)(
    _In_opt_ LPCWSTR  lpLocaleName,
    _In_     DWORD    dwFlags,
    _In_opt_ const SYSTEMTIME* lpDate,
    _In_opt_ LPCWSTR  lpFormat,
    _Out_writes_opt_(cchDate) LPWSTR lpDateStr,
    _In_     int      cchDate,
    _In_opt_ LPCWSTR  lpCalendar
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(lpLocaleName);
    UNREFERENCED_PARAMETER(lpCalendar);

    // Validate parameters
    if (cchDate < 0 || (!lpDateStr && cchDate > 0)) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return 0;
    }

    if (lpFormat && wcslen(lpFormat) >= 256) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return 0;
    }

    // Reject unsupported flags (kernel mode: no NLS worker)
    if ((dwFlags & 0xF0) && ((dwFlags & 0xF0) - 1) & 0x70) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return 0;
    }

    // Use current time if not provided
    SYSTEMTIME LocalTime{};
    if (lpDate == nullptr) {
        MUSA_NAME(GetLocalTime)(&LocalTime);
        lpDate = &LocalTime;
    }

    // If output buffer provided but too small, return required size
    if (cchDate > 0 && cchDate < 11) {
        BaseSetLastNTError(STATUS_BUFFER_TOO_SMALL);
        return 11;
    }

    // Format: YYYY-MM-DD (ISO 8601)
    if (lpDateStr && cchDate >= 11) {
        RtlStringCchPrintfW(lpDateStr, cchDate, L"%04d-%02d-%02d",
            lpDate->wYear, lpDate->wMonth, lpDate->wDay);
    }

    return 11;
}

MUSA_IAT_SYMBOL(GetDateFormatEx, 28);


_IRQL_requires_max_(PASSIVE_LEVEL)
int WINAPI MUSA_NAME(GetTimeFormatEx)(
    _In_opt_ LPCWSTR  lpLocaleName,
    _In_     DWORD    dwFlags,
    _In_opt_ const SYSTEMTIME* lpTime,
    _In_opt_ LPCWSTR  lpFormat,
    _Out_writes_opt_(cchTime) LPWSTR lpTimeStr,
    _In_     int      cchTime
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(lpLocaleName);

    // Validate parameters
    if (cchTime < 0 || (!lpTimeStr && cchTime > 0)) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return 0;
    }

    if (lpFormat && wcslen(lpFormat) >= 256) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return 0;
    }

    // Reject unsupported flags (kernel mode: no NLS worker)
    if ((dwFlags & 0x3FFFFFF0) != 0) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return 0;
    }

    // Use current time if not provided
    SYSTEMTIME LocalTime{};
    if (lpTime == nullptr) {
        MUSA_NAME(GetLocalTime)(&LocalTime);
        lpTime = &LocalTime;
    }

    // If output buffer provided but too small, return required size
    if (cchTime > 0 && cchTime < 9) {
        BaseSetLastNTError(STATUS_BUFFER_TOO_SMALL);
        return 9;
    }

    // Format: HH:MM:SS (ISO 8601)
    if (lpTimeStr && cchTime >= 9) {
        RtlStringCchPrintfW(lpTimeStr, cchTime, L"%02d:%02d:%02d",
            lpTime->wHour, lpTime->wMinute, lpTime->wSecond);
    }

    return 9;
}

MUSA_IAT_SYMBOL(GetTimeFormatEx, 24);

EXTERN_C_END
