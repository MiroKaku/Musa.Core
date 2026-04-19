//
// National Language Support
//

#include "KernelBase.Private.h"
#include "Internal/KernelBase.NLS.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(MultiByteToWideChar))
#pragma alloc_text(PAGE, MUSA_NAME(WideCharToMultiByte))
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
            Status = RtlMultiByteToUnicodeSize(&BytesInUnicodeString, MultiByteStr, CountOfMultiByte);
            if (!Status) {
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
                &BytesInUnicodeString, MultiByteStr, CountOfMultiByte);
            if (!Status) {
                ReturnLength = 0;
                break;
            }

            ReturnLength = static_cast<int>(BytesInUnicodeString / sizeof(wchar_t));
            break;
        }

        if (CodePage == CP_OEMCP) {
            Status = RtlMultiByteToUnicodeSize(&BytesInUnicodeString, MultiByteStr, CountOfMultiByte);
            if (!Status) {
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
                &BytesInUnicodeString, MultiByteStr, CountOfMultiByte);
            if (!Status) {
                ReturnLength = 0;
                break;
            }

            ReturnLength = static_cast<int>(BytesInUnicodeString / sizeof(wchar_t));
            break;
        }

        if (CodePage == CP_UTF8) {
            if (Flags) {
                if (Flags != MB_ERR_INVALID_CHARS) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }
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
            Status = RtlUnicodeToMultiByteSize(&BytesInMultiByteString, WideCharStr,
                CountOfWideChar * sizeof(wchar_t));
            if (!Status) {
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
                &BytesInMultiByteString, WideCharStr, CountOfWideChar * sizeof(wchar_t));
            if (!Status) {
                ReturnLength = 0;
                break;
            }

            ReturnLength = static_cast<int>(BytesInMultiByteString);
            break;
        }

        if (CodePage == CP_OEMCP) {
            Status = RtlUnicodeToMultiByteSize(&BytesInMultiByteString, WideCharStr,
                CountOfWideChar * sizeof(wchar_t));
            if (!Status) {
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
                &BytesInMultiByteString, WideCharStr, CountOfWideChar * sizeof(wchar_t));
            if (!Status) {
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

EXTERN_C_END
