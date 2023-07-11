

EXTERN_C_START


_Success_(return != 0)
_When_((CountOfMultiByte == -1) && (CountOfWideChar != 0), _Post_equal_to_(_String_length_(WideCharStr) + 1))
int WINAPI MI_NAME(MultiByteToWideChar)(
    _In_ UINT CodePage,
    _In_ DWORD Flags,
    _In_NLS_string_(CountOfMultiByte) LPCCH MultiByteStr,
    _In_ int CountOfMultiByte,
    _Out_writes_to_opt_(CountOfWideChar, return) LPWSTR WideCharStr,
    _In_ int CountOfWideChar
)
{
    UNREFERENCED_PARAMETER(Flags);

    int ReturnLength = 0;
    NTSTATUS Status  = STATUS_NOT_SUPPORTED;

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
            Status = RtlUTF8ToUnicodeN(nullptr, 0,
                &BytesInUnicodeString, MultiByteStr, CountOfMultiByte);
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

            Status = RtlUTF8ToUnicodeN(WideCharStr, static_cast<ULONG>(CountOfWideChar * sizeof(wchar_t)),
                &BytesInUnicodeString, MultiByteStr, CountOfMultiByte);
            if (!Status) {
                ReturnLength = 0;
                break;
            }

            ReturnLength = static_cast<int>(BytesInUnicodeString / sizeof(wchar_t));
            break;
        }

    } while (false);

    if (!NT_SUCCESS(Status)) {
        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
    }

    return ReturnLength;
}
MI_IAT_SYMBOL(MultiByteToWideChar, 24);


_Success_(return != 0)
_When_((CountOfWideChar == -1) && (CountOfMultiByte != 0), _Post_equal_to_(_String_length_(MultiByteStr) + 1))
int WINAPI MI_NAME(WideCharToMultiByte)(
    _In_ UINT CodePage,
    _In_ DWORD Flags,
    _In_NLS_string_(CountOfWideChar) LPCWCH WideCharStr,
    _In_ int CountOfWideChar,
    _Out_writes_bytes_to_opt_(CountOfMultiByte, return) LPSTR MultiByteStr,
    _In_ int CountOfMultiByte,
    _In_opt_ LPCCH DefaultChar,
    _Out_opt_ LPBOOL UsedDefaultChar
)
{
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(DefaultChar);

    int ReturnLength = 0;
    NTSTATUS Status  = STATUS_NOT_SUPPORTED;

    do {
        if (UsedDefaultChar) {
            *UsedDefaultChar = FALSE;
        }

        ULONG BytesInMultiByteString = 0ul;

        if (CodePage == CP_ACP) {
            Status = RtlUnicodeToMultiByteSize(&BytesInMultiByteString, WideCharStr, CountOfWideChar * sizeof(wchar_t));
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
            Status = RtlUnicodeToMultiByteSize(&BytesInMultiByteString, WideCharStr, CountOfWideChar * sizeof(wchar_t));
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
            Status = RtlUnicodeToUTF8N(nullptr, 0,
                &BytesInMultiByteString, WideCharStr, CountOfWideChar * sizeof(wchar_t));
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

            Status = RtlUnicodeToUTF8N(MultiByteStr, CountOfMultiByte,
                &BytesInMultiByteString, WideCharStr, CountOfWideChar * sizeof(wchar_t));
            if (!Status) {
                ReturnLength = 0;
                break;
            }

            ReturnLength = static_cast<int>(BytesInMultiByteString);
            break;
        }

    } while (false);

    if (!NT_SUCCESS(Status)) {
        RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
    }

    return ReturnLength;
}
MI_IAT_SYMBOL(WideCharToMultiByte, 32);


EXTERN_C_END
