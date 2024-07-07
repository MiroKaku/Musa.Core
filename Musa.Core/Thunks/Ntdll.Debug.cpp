
EXTERN_C_START

extern PVOID PsSystemDllBase;

namespace Musa
{
#ifdef _KERNEL_MODE

#pragma warning(push)
#pragma warning(disable: 6387)
    NTSTATUS NTAPI MUSA_NAME(RtlFindAndFormatMessage)(
        _In_     DWORD      Flags,
        _In_opt_ LPCVOID    Source,
        _In_     DWORD      MessageId,
        _In_     DWORD      LanguageId,
        _When_((Flags& FORMAT_MESSAGE_ALLOCATE_BUFFER) != 0, _At_((LPWSTR*)Buffer, _Outptr_result_z_))
        _When_((Flags& FORMAT_MESSAGE_ALLOCATE_BUFFER) == 0, _Out_writes_z_(*Size))
                 LPWSTR     Buffer,
        _Inout_  DWORD*     Size,
        _In_opt_ va_list*   Arguments
        )
    {
        NTSTATUS Status = STATUS_SUCCESS;
        PVOID    AllocatedBuffer = nullptr;

        ANSI_STRING     AnsiMessage   {};
        UNICODE_STRING  UnicodeMessage{};

        do {
            /* If this is a Win32 error wrapped as an OLE HRESULT then unwrap it */
            if (((MessageId & 0xffff0000) == 0x80070000) &&
                 BooleanFlagOn(Flags, FORMAT_MESSAGE_FROM_SYSTEM ) &&
                !BooleanFlagOn(Flags, FORMAT_MESSAGE_FROM_HMODULE) &&
                !BooleanFlagOn(Flags, FORMAT_MESSAGE_FROM_STRING )) {

                MessageId &= 0x0000ffff;
            }

            if (Buffer == nullptr) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            if (Flags & FORMAT_MESSAGE_ALLOCATE_BUFFER) {
                *reinterpret_cast<PVOID*>(Buffer) = nullptr;
            }

            PMESSAGE_RESOURCE_ENTRY MessageEntry  = nullptr;

            __try {
                PWSTR       MessageFormat;
                ULONG       MaximumWidth  = 0ul;
                const PVOID BaseDllHandle = PsSystemDllBase;

                MaximumWidth = Flags & FORMAT_MESSAGE_MAX_WIDTH_MASK;
                if (MaximumWidth == FORMAT_MESSAGE_MAX_WIDTH_MASK) {
                    MaximumWidth = ULONG_MAX;
                }

                if (BooleanFlagOn(Flags, FORMAT_MESSAGE_FROM_STRING)) {
                    MessageFormat = static_cast<LPWSTR>(const_cast<LPVOID>(Source));
                }
                else {
                    PVOID DllHandle;
                    if (BooleanFlagOn(Flags, FORMAT_MESSAGE_FROM_HMODULE)) {
                        if (Source == nullptr) {
                            DllHandle = BaseDllHandle;
                        }
                        else {
                            DllHandle = const_cast<LPVOID>(Source);
                        }
                    }
                    else if (BooleanFlagOn(Flags, FORMAT_MESSAGE_FROM_SYSTEM)) {
                        DllHandle = BaseDllHandle;
                    }
                    else {
                        Status = STATUS_INVALID_PARAMETER;
                        break;
                    }

                    Status = RtlFindMessage(
                        DllHandle,
                        PtrToUlong(RT_MESSAGETABLE),
                        LanguageId,
                        MessageId,
                        &MessageEntry);

                    if (Status == STATUS_MESSAGE_NOT_FOUND) {
                        if (BooleanFlagOn(Flags, FORMAT_MESSAGE_FROM_HMODULE) &&
                            BooleanFlagOn(Flags, FORMAT_MESSAGE_FROM_SYSTEM)) {

                            DllHandle = BaseDllHandle;
                            ClearFlag(Flags, FORMAT_MESSAGE_FROM_HMODULE);

                            Status = RtlFindMessage(
                                DllHandle,
                                PtrToUlong(RT_MESSAGETABLE),
                                LanguageId,
                                MessageId,
                                &MessageEntry);
                        }
                    }

                    if (!NT_SUCCESS(Status)) {
                        break;
                    }

                    if (!BooleanFlagOn(MessageEntry->Flags, MESSAGE_RESOURCE_UNICODE)) {
                        RtlInitAnsiString(&AnsiMessage, reinterpret_cast<PCSZ>(MessageEntry->Text));
                        Status = RtlAnsiStringToUnicodeString(&UnicodeMessage, &AnsiMessage, TRUE);
                        if (!NT_SUCCESS(Status)) {
                            break;
                        }

                        MessageFormat = UnicodeMessage.Buffer;
                    }
                    else {
                        MessageFormat = reinterpret_cast<PWSTR>(MessageEntry->Text);
                    }
                }

                auto       WrittenSize         = 256ul;
                const bool IgnoreInserts       = BooleanFlagOn(Flags, FORMAT_MESSAGE_IGNORE_INSERTS);
                const bool ArgumentsAreAnAnsi  = BooleanFlagOn(Flags, FORMAT_MESSAGE_ARGUMENT_ANSI);
                const bool ArgumentsAreAnArray = BooleanFlagOn(Flags, FORMAT_MESSAGE_ARGUMENT_ARRAY);

                do {
                    if (AllocatedBuffer) {
                        LocalFree(AllocatedBuffer);
                    }

                    AllocatedBuffer = LocalAlloc(LPTR, WrittenSize);
                    if (AllocatedBuffer == nullptr) {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }

                    Status = RtlFormatMessage(
                        MessageFormat,
                        MaximumWidth,
                        IgnoreInserts,
                        ArgumentsAreAnAnsi,
                        ArgumentsAreAnArray,
                        Arguments,
                        static_cast<PWSTR>(AllocatedBuffer),
                        WrittenSize,
                        &WrittenSize);
                    if (NT_SUCCESS(Status)) {
                        break;
                    }

                    if (Status != STATUS_BUFFER_OVERFLOW) {
                        break;
                    }

                    WrittenSize += 256;

                } while (true);

                if (!NT_SUCCESS(Status)) {
                    break;
                }

                if (BooleanFlagOn(Flags, FORMAT_MESSAGE_ALLOCATE_BUFFER)) {
                    *reinterpret_cast<PVOID*>(Buffer) = AllocatedBuffer;
                    AllocatedBuffer = nullptr;
                }
                else if ((WrittenSize / sizeof(WCHAR)) > *Size) {
                    Status = STATUS_BUFFER_TOO_SMALL;
                    break;
                }
                else {
                    RtlMoveMemory(Buffer, AllocatedBuffer, WrittenSize);
                }

                *Size = (WrittenSize - sizeof(WCHAR)) / sizeof(WCHAR);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
                break;
            }

        } while (false);

        RtlFreeUnicodeString(&UnicodeMessage);

        if (AllocatedBuffer) {
            LocalFree(AllocatedBuffer);
        }

        return Status;
    }
    MUSA_IAT_SYMBOL(RtlFindAndFormatMessage, 28);
#pragma warning(pop)

#endif

}
EXTERN_C_END
