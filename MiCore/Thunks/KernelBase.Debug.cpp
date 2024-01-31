#include "KernelBase.Private.h"
#include "Ntdll.Debug.Private.h"


EXTERN_C_START
namespace Mi
{
    BOOL WINAPI MI_NAME(IsDebuggerPresent)(
        VOID
        )
    {
        return RtlIsAnyDebuggerPresent();
    }
    MI_IAT_SYMBOL(IsDebuggerPresent, 0);

    VOID WINAPI MI_NAME(DebugBreak)(
        VOID
        )
    {
        __debugbreak();
    }
    MI_IAT_SYMBOL(DebugBreak, 0);

    VOID WINAPI MI_NAME(OutputDebugStringA)(
        _In_opt_ LPCSTR OutputString
    )
    {
        DbgPrint("%hs", OutputString);
    }
    MI_IAT_SYMBOL(OutputDebugStringA, 4);

    VOID WINAPI MI_NAME(OutputDebugStringW)(
        _In_opt_ LPCWSTR OutputString
    )
    {
        DbgPrint("%ls", OutputString);
    }
    MI_IAT_SYMBOL(OutputDebugStringW, 4);

#if defined _KERNEL_MODE

#pragma warning(push)
#pragma warning(disable: 6054 6387)
    BOOL WINAPI MI_NAME(FormatMessageA)(
        _In_     DWORD      Flags,
        _In_opt_ LPCVOID    Source,
        _In_     DWORD      MessageId,
        _In_     DWORD      LanguageId,
        _When_((Flags& FORMAT_MESSAGE_ALLOCATE_BUFFER) != 0, _At_((LPSTR*)Buffer, _Outptr_result_z_))
        _When_((Flags& FORMAT_MESSAGE_ALLOCATE_BUFFER) == 0, _Out_writes_z_(Size))
                 LPSTR      Buffer,
        _In_     DWORD      Size,
        _In_opt_ va_list*   Arguments
        )
    {
        NTSTATUS Status;
        DWORD    WrittenSize    = Size;
        PWSTR    UnicodeSource  = nullptr;
        PWSTR    UnicodeBuffer  = nullptr;

        ANSI_STRING     AnsiString   {};
        UNICODE_STRING  UnicodeString{};

        do {
            if (BooleanFlagOn(Flags, FORMAT_MESSAGE_FROM_STRING)) {
                if (strlen(static_cast<LPCSTR>(Source)) >= MAXSHORT) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }
                else {
                    RtlInitAnsiString(&AnsiString, static_cast<LPCSTR>(Source));
                    Status = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
                    if (!NT_SUCCESS(Status)) {
                        break;
                    }

                    UnicodeSource = UnicodeString.Buffer;
                }
            }
            else {
                UnicodeSource = static_cast<LPWSTR>(const_cast<LPVOID>(Source));
            }

            if (BooleanFlagOn(Flags, FORMAT_MESSAGE_ALLOCATE_BUFFER)) {
                UnicodeBuffer = reinterpret_cast<LPWSTR>(Buffer);
            }
            else {
                UnicodeBuffer = static_cast<LPWSTR>(LocalAlloc(LPTR, Size * sizeof(wchar_t)));
            }

            if (UnicodeBuffer == nullptr) {
                Status = STATUS_NO_MEMORY;
                break;
            }

            SetFlag(Flags, FORMAT_MESSAGE_ARGUMENT_ANSI);

            Status = RtlFindAndFormatMessage(
                Flags,
                UnicodeSource,
                MessageId,
                LanguageId,
                UnicodeBuffer,
                &WrittenSize,
                Arguments);

            RtlFreeUnicodeString(&UnicodeString);

            if (!NT_SUCCESS(Status)) {
                if (BooleanFlagOn(Flags, FORMAT_MESSAGE_ALLOCATE_BUFFER)) {
                    UnicodeBuffer = nullptr;
                }

                break;
            }

            if (WrittenSize >= MAXSHORT) {
                if (BooleanFlagOn(Flags, FORMAT_MESSAGE_ALLOCATE_BUFFER)) {
                    UnicodeBuffer = *reinterpret_cast<LPWSTR*>(Buffer);
                    *reinterpret_cast<LPWSTR*>(Buffer) = nullptr;
                }

                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            if (BooleanFlagOn(Flags, FORMAT_MESSAGE_ALLOCATE_BUFFER)) {
                UnicodeBuffer = *reinterpret_cast<LPWSTR*>(Buffer);

                UnicodeString.Buffer        = UnicodeBuffer;
                UnicodeString.Length        = static_cast<USHORT>(WrittenSize * sizeof(WCHAR));
                UnicodeString.MaximumLength = static_cast<USHORT>(UnicodeString.Length + sizeof(UNICODE_NULL));

                Status = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, TRUE);
                if (!NT_SUCCESS(Status)) {
                    *reinterpret_cast<LPSTR*>(Buffer) = nullptr;
                    break;
                }

                *reinterpret_cast<LPSTR*>(Buffer) = AnsiString.Buffer;
            }
            else {
                UnicodeString.Buffer = UnicodeBuffer;

                RtlInitEmptyAnsiString(&AnsiString, Buffer, static_cast<USHORT>(Size));
                Status = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, FALSE);
                if (!NT_SUCCESS(Status)) {
                    break;
                }
            }

            WrittenSize = AnsiString.Length;

        } while (false);

        if (UnicodeBuffer) {
            LocalFree(UnicodeBuffer);
        }

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
        }

        return WrittenSize;
    }
    MI_IAT_SYMBOL(FormatMessageA, 28);

    BOOL WINAPI MI_NAME(FormatMessageW)(
        _In_     DWORD      Flags,
        _In_opt_ LPCVOID    Source,
        _In_     DWORD      MessageId,
        _In_     DWORD      LanguageId,
        _When_((Flags& FORMAT_MESSAGE_ALLOCATE_BUFFER) != 0, _At_((LPWSTR*)Buffer, _Outptr_result_z_))
        _When_((Flags& FORMAT_MESSAGE_ALLOCATE_BUFFER) == 0, _Out_writes_z_(Size))
                 LPWSTR     Buffer,
        _In_     DWORD      Size,
        _In_opt_ va_list*   Arguments
    )
    {
        NTSTATUS Status;
        DWORD    WrittenSize = Size;

        do {
            if (BooleanFlagOn(Flags, FORMAT_MESSAGE_ARGUMENT_ANSI)) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            Status = RtlFindAndFormatMessage(
                Flags,
                Source,
                MessageId,
                LanguageId,
                Buffer,
                &WrittenSize,
                Arguments);
            if (!NT_SUCCESS(Status)) {
                break;
            }

        } while (false);

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
        }

        return WrittenSize;
    }
    MI_IAT_SYMBOL(FormatMessageW, 28);
#pragma warning(pop)

#endif


}
EXTERN_C_END
