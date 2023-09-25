#include <thunks/thunks.hpp>


EXTERN_C_START


#define IS_PATH_SEPARATOR_U(ch)     (((ch) == L'\\') || ((ch) == L'/'))
#define IS_END_OF_COMPONENT_U(ch)   (IS_PATH_SEPARATOR_U(ch) || (ch) == UNICODE_NULL)
#define IS_DOT_U(s)                 ((s)[0] == L'.' && IS_END_OF_COMPONENT_U( (s)[1] ))
#define IS_DOT_DOT_U(s)             ((s)[0] == L'.' && IS_DOT_U( (s) + 1))
#define IS_DRIVE_LETTER(ch)         (((ch) >= 'a' && (ch) <= 'z') || ((ch) >= 'A' && (ch) <= 'Z'))

#define IS_END_OF_COMPONENT_USTR(s, len) \
                ((len) == 0 || IS_PATH_SEPARATOR_U((s)[0]))

#define IS_DOT_USTR(s, len) \
                ((len) >= sizeof(WCHAR) && (s)[0] == L'.' && IS_END_OF_COMPONENT_USTR( (s) + 1, (len) - sizeof(WCHAR) ))

#define IS_DOT_DOT_USTR(s, len) \
                ((len) >= sizeof(WCHAR) && (s)[0] == L'.' && IS_DOT_USTR( (s) + 1, (len) - sizeof(WCHAR) ))

extern const UNICODE_STRING RtlpDosLPTDevice = RTL_CONSTANT_STRING(L"LPT");
extern const UNICODE_STRING RtlpDosCOMDevice = RTL_CONSTANT_STRING(L"COM");
extern const UNICODE_STRING RtlpDosPRNDevice = RTL_CONSTANT_STRING(L"PRN");
extern const UNICODE_STRING RtlpDosAUXDevice = RTL_CONSTANT_STRING(L"AUX");
extern const UNICODE_STRING RtlpDosNULDevice = RTL_CONSTANT_STRING(L"NUL");
extern const UNICODE_STRING RtlpDosCONDevice = RTL_CONSTANT_STRING(L"CON");

extern const UNICODE_STRING RtlpDosSlashCONDevice   = RTL_CONSTANT_STRING(L"\\\\.\\CON");
extern const UNICODE_STRING RtlpSlashSlashDot       = RTL_CONSTANT_STRING(L"\\\\.\\");
extern const UNICODE_STRING RtlpDosDevicesPrefix    = RTL_CONSTANT_STRING(L"\\??\\");
extern const UNICODE_STRING RtlpDosDevicesUncPrefix = RTL_CONSTANT_STRING(L"\\??\\UNC\\");

#define RtlpLongestPrefix   RtlpDosDevicesUncPrefix.Length

const UNICODE_STRING RtlpEmptyString = RTL_CONSTANT_STRING(L"");

//
// \\? is referred to as the "Win32Nt" prefix or root.
// Paths that start with \\? are referred to as "Win32Nt" paths.
// Fudging the \\? to \?? converts the path to an Nt path.
//
extern const UNICODE_STRING RtlpWin32NtRoot         = RTL_CONSTANT_STRING(L"\\\\?");
extern const UNICODE_STRING RtlpWin32NtRootSlash    = RTL_CONSTANT_STRING(L"\\\\?\\");
extern const UNICODE_STRING RtlpWin32NtUncRoot      = RTL_CONSTANT_STRING(L"\\\\?\\UNC");
extern const UNICODE_STRING RtlpWin32NtUncRootSlash = RTL_CONSTANT_STRING(L"\\\\?\\UNC\\");

#define DPFLTR_LEVEL_STATUS(x) ((NT_SUCCESS(x) || (x) == STATUS_OBJECT_NAME_NOT_FOUND) \
                                ? DPFLTR_TRACE_LEVEL : DPFLTR_ERROR_LEVEL)


RTL_PATH_TYPE NTAPI MI_NAME(RtlDetermineDosPathNameType_U)(
    _In_ PCWSTR DosFileName
)
{
    RTL_PATH_TYPE ReturnValue;
    ASSERT(DosFileName != NULL);

    if (IS_PATH_SEPARATOR_U(*DosFileName)) {
        if (IS_PATH_SEPARATOR_U(*(DosFileName + 1))) {
            if (DosFileName[2] == '.' || DosFileName[2] == '?') {
                if (IS_PATH_SEPARATOR_U(*(DosFileName + 3))) {
                    ReturnValue = RtlPathTypeLocalDevice;
                }
                else if ((*(DosFileName + 3)) == UNICODE_NULL) {
                    ReturnValue = RtlPathTypeRootLocalDevice;
                }
                else {
                    ReturnValue = RtlPathTypeUncAbsolute;
                }
            }
            else {
                ReturnValue = RtlPathTypeUncAbsolute;
            }
        }
        else {
            ReturnValue = RtlPathTypeRooted;
        }
    }
    else if ((*DosFileName) && (*(DosFileName + 1) == L':')) {
        if (IS_PATH_SEPARATOR_U(*(DosFileName + 2))) {
            ReturnValue = RtlPathTypeDriveAbsolute;
        }
        else {
            ReturnValue = RtlPathTypeDriveRelative;
        }
    }
    else {
        ReturnValue = RtlPathTypeRelative;
    }

    return ReturnValue;
}
MI_IAT_SYMBOL(RtlDetermineDosPathNameType_U, 4);

RTL_PATH_TYPE NTAPI MI_NAME(RtlDetermineDosPathNameType_Ustr)(
    _In_ PCUNICODE_STRING String
)
{
    RTL_PATH_TYPE ReturnValue;
    const PCWSTR DosFileName = String->Buffer;

#define ENOUGH_CHARS(_cch) (String->Length >= ((_cch) * sizeof(WCHAR)))

    if (ENOUGH_CHARS(1) && IS_PATH_SEPARATOR_U(*DosFileName)) {
        if (ENOUGH_CHARS(2) && IS_PATH_SEPARATOR_U(*(DosFileName + 1))) {
            if (ENOUGH_CHARS(3) && (DosFileName[2] == '.' ||
                DosFileName[2] == '?')) {

                if (ENOUGH_CHARS(4) && IS_PATH_SEPARATOR_U(*(DosFileName + 3))) {
                    // "\\.\" or "\\?\"
                    ReturnValue = RtlPathTypeLocalDevice;
                }
                else if (String->Length == (3 * sizeof(WCHAR))) {
                    // "\\." or \\?"
                    ReturnValue = RtlPathTypeRootLocalDevice;
                }
                else {
                    // "\\.x" or "\\?x"
                    ReturnValue = RtlPathTypeUncAbsolute;
                }
            }
            else {
                // "\\x"
                ReturnValue = RtlPathTypeUncAbsolute;
            }
        }
        else {
            // "\x"
            ReturnValue = RtlPathTypeRooted;
        }
    }
    //
    // the "*DosFileName" is left over from the PCWSTR version
    // Win32 and DOS don't allow embedded nuls and much code limits
    // drive letters to strictly 7bit a-zA-Z so it's ok.
    //
    else if (ENOUGH_CHARS(2) && *DosFileName && *(DosFileName + 1) == L':') {
        if (ENOUGH_CHARS(3) && IS_PATH_SEPARATOR_U(*(DosFileName + 2))) {
            // "x:\"
            ReturnValue = RtlPathTypeDriveAbsolute;
        }
        else {
            // "c:x"
            ReturnValue = RtlPathTypeDriveRelative;
        }
    }
    else {
        // "x", first char is not a slash / second char is not colon
        ReturnValue = RtlPathTypeRelative;
    }
    return ReturnValue;

#undef ENOUGH_CHARS
}
MI_IAT_SYMBOL(RtlDetermineDosPathNameType_Ustr, 4);

ULONG NTAPI MI_NAME(RtlIsDosDeviceName_Ustr)(
    _In_ PCUNICODE_STRING DosFileName
)
{
    UNICODE_STRING UnicodeString;
    USHORT NumberOfCharacters, OriginalLength;
    ULONG ReturnLength;
    ULONG ReturnOffset;
    LPWSTR p;
    USHORT ColonBias;
    RTL_PATH_TYPE PathType;
    WCHAR wch;

    ColonBias = 0;

    //
    // NOTICE-2002/03/18-ELi
    // DosFileName->Buffer might not be a null-terminated string
    // RtlSetCurrentDirectory_U can called this function with such a string
    //
    PathType = RtlDetermineDosPathNameType_Ustr(DosFileName);

    switch (PathType) {

    case RtlPathTypeLocalDevice:
        //
        // For Unc Absolute, Check for \\.\CON
        // since this really is not a device
        //

        if (RtlEqualUnicodeString(DosFileName, &RtlpDosSlashCONDevice, TRUE)) {
            return 0x00080006;
        }

        //
        // FALLTHRU
        //

    case RtlPathTypeUncAbsolute:
    case RtlPathTypeUnknown:
        return 0;
    }

    UnicodeString = *DosFileName;
    OriginalLength = UnicodeString.Length;
    NumberOfCharacters = OriginalLength >> 1;

    if (NumberOfCharacters && UnicodeString.Buffer[NumberOfCharacters - 1] == L':') {
        UnicodeString.Length -= sizeof(WCHAR);
        NumberOfCharacters--;
        ColonBias = 1;
    }

    //
    // The above strip the trailing colon logic could have left us with 0
    // for NumberOfCharacters, so that needs to be tested
    //

    if (NumberOfCharacters == 0) {
        return 0;
    }

    wch = UnicodeString.Buffer[NumberOfCharacters - 1];
    while (NumberOfCharacters && (wch == L'.' || wch == L' ')) {
        UnicodeString.Length -= sizeof(WCHAR);
        NumberOfCharacters--;
        ColonBias++;

        if (NumberOfCharacters > 0) {
            wch = UnicodeString.Buffer[NumberOfCharacters - 1];
        }
    }

    ReturnLength = NumberOfCharacters << 1;

    //
    //  Walk backwards through the string finding the
    //  first slash or the beginning of the string.  We also stop
    //  at the drive: if it is present.
    //

    ReturnOffset = 0;
    if (NumberOfCharacters) {
        p = UnicodeString.Buffer + NumberOfCharacters - 1;
        while (p >= UnicodeString.Buffer) {
            if (*p == L'\\' || *p == L'/'
                || (*p == L':' && p == UnicodeString.Buffer + 1)) {
                p++;

                //
                // NOTICE-2002/02/12-ELi
                // if p points to the end of the string, then cannot access p++
                // because string may not be NULL-terminated
                // if at the end of the string, don't have any more chars in
                // the name to check so just return 0
                //
                if (p >= (UnicodeString.Buffer +
                    (OriginalLength / sizeof(WCHAR)))) {
                    return 0;
                }

                //
                //  Get the first char of the file name and convert it to
                //  lower case.  This will be safe since we will be comparing
                //  it to only lower-case ASCII.
                //

                wch = (*p) | 0x20;

                //
                //  check to see if we possibly have a hit on
                //  lpt, prn, con, com, aux, or nul
                //

                if (!(wch == L'l' || wch == L'c' || wch == L'p' || wch == L'a'
                    || wch == L'n')
                    ) {
                    return 0;
                }
                ReturnOffset = (ULONG)((PSZ)p - (PSZ)UnicodeString.Buffer);

                //
                // NOTICE-2002/03/18-ELi
                // assuming that at this point
                // UnicodeString.Buffer < p < (UnicodeString.Buffer +
                //     OriginalLength / sizeof(WCHAR))
                // Therefore the USHORT cast and subtractions are ok
                //
                UnicodeString.Length = OriginalLength - (USHORT)((PCHAR)p -
                    (PCHAR)UnicodeString.Buffer);
                UnicodeString.Buffer = p;

                NumberOfCharacters = UnicodeString.Length >> 1;
                NumberOfCharacters = NumberOfCharacters - ColonBias;
                ReturnLength = NumberOfCharacters << 1;
                UnicodeString.Length -= ColonBias * sizeof(WCHAR);
                break;
            }
            p--;
        }

        wch = UnicodeString.Buffer[0] | 0x20;

        //
        // check to see if we possibly have a hit on
        // lpt, prn, con, com, aux, or nul
        //

        if (!(wch == L'l' || wch == L'c' || wch == L'p' || wch == L'a'
            || wch == L'n')) {
            return 0;
        }
    }

    //
    //  Now we need to see if we are dealing with a device name that has
    //  an extension or a stream name. If so, we need to limit the search to the
    //  file portion only
    //

    p = UnicodeString.Buffer;
    while (p < UnicodeString.Buffer + NumberOfCharacters && *p != L'.' && *p != L':') {
        p++;
    }

    //
    //  p either points past end of string or to a dot or :.  We back up over
    //  trailing spaces
    //

    while (p > UnicodeString.Buffer && p[-1] == L' ') {
        p--;
    }

    //
    //  p either points to the beginning of the string or p[-1] is
    //  the first non-space char found above.
    //

    NumberOfCharacters = (USHORT)(p - UnicodeString.Buffer);
    UnicodeString.Length = NumberOfCharacters * sizeof(WCHAR);

    if (NumberOfCharacters == 4 && iswdigit(UnicodeString.Buffer[3])) {
        if ((WCHAR)UnicodeString.Buffer[3] == L'0') {
            return 0;
        }
        else {
            UnicodeString.Length -= sizeof(WCHAR);
            if (RtlEqualUnicodeString(&UnicodeString, &RtlpDosLPTDevice, TRUE) ||
                RtlEqualUnicodeString(&UnicodeString, &RtlpDosCOMDevice, TRUE)) {
                ReturnLength = NumberOfCharacters << 1;
            }
            else {
                return 0;
            }
        }
    }
    else if (NumberOfCharacters != 3) {
        return 0;
    }
    else if (RtlEqualUnicodeString(&UnicodeString, &RtlpDosPRNDevice, TRUE)) {
        ReturnLength = NumberOfCharacters << 1;
    }
    else if (RtlEqualUnicodeString(&UnicodeString, &RtlpDosAUXDevice, TRUE)) {
        ReturnLength = NumberOfCharacters << 1;
    }
    else if (RtlEqualUnicodeString(&UnicodeString, &RtlpDosNULDevice, TRUE)) {
        ReturnLength = NumberOfCharacters << 1;
    }
    else if (RtlEqualUnicodeString(&UnicodeString, &RtlpDosCONDevice, TRUE)) {
        ReturnLength = NumberOfCharacters << 1;
    }
    else {
        return 0;
    }

    return ReturnLength | (ReturnOffset << 16);
}
MI_IAT_SYMBOL(RtlIsDosDeviceName_Ustr, 4);

ULONG NTAPI MI_NAME(RtlIsDosDeviceName_U)(
    _In_ PCWSTR DosFileName
)
{
    UNICODE_STRING UnicodeString{};
    if (!NT_SUCCESS(RtlInitUnicodeStringEx(&UnicodeString, DosFileName))) {
        return 0;
    }

    return RtlIsDosDeviceName_Ustr(&UnicodeString);
}
MI_IAT_SYMBOL(RtlIsDosDeviceName_U, 4);

ULONG NTAPI MI_NAME(RtlGetCurrentDirectory_U)(
    _In_ ULONG BufferLength,
    _Out_writes_bytes_(BufferLength) PWSTR Buffer
    )
{
    ULONG Result;

    const auto CurDir = &NtCurrentPeb()->CurrentDirectory;
    RtlAcquirePebLock();
    do
    {
        const auto CurDirName = CurDir->DosPath.Buffer;

        //
        // Make sure user's buffer is big enough to hold the null
        // terminated current directory
        //

        ULONG Length = CurDir->DosPath.Length >> 1;

        //
        // Assuming CurDir->DosPath.Buffer always has a trailing slash
        // if the root of a drive,
        // then return x:\
        // else return x:\directory\subdir
        //

        NT_ASSERT((CurDirName != nullptr) && (Length > 0));

        if ((Length > 1) && (CurDirName[Length - 2] != L':')) {
            if (BufferLength < (Length << 1)) {
                Result = Length << 1;
                break;
            }
        }
        else {
            if (BufferLength <= (Length << 1)) {
                Result = (Length + 1) << 1;
                break;
            }
        }

        __try {
            RtlCopyMemory(Buffer, CurDirName, Length << 1);

            NT_ASSERT(Buffer[Length - 1] == L'\\');

            if ((Length > 1) && (Buffer[Length - 2] == L':')) {
                //
                // root of a drive, append a null
                //
                Buffer[Length] = UNICODE_NULL;
            }
            else {
                //
                // some directory, change the trailing slash to a null
                //
                Buffer[--Length] = UNICODE_NULL;
            }

            Result = Length << 1;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Result = 0;
            break;
        }

    } while(false);
    RtlReleasePebLock();

    return Result;
}
MI_IAT_SYMBOL(RtlGetCurrentDirectory_U, 8);

NTSTATUS NTAPI MI_NAME(RtlSetCurrentDirectory_U)(
    _In_ PCUNICODE_STRING PathName
    )
{

    PCURDIR CurDir;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    ULONG DosDirLength;
    ULONG DosDirCharCount;
    UNICODE_STRING DosDir;
    UNICODE_STRING NtFileName;
    HANDLE NewDirectoryHandle;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;
    RTL_PATH_TYPE InputPathType;
    HANDLE HandleToClose;
    HANDLE Heap;

    const auto Peb = NtCurrentPeb();
    Heap    = Peb->ProcessHeap;
    CurDir  = &Peb->CurrentDirectory;

    DosDir.Buffer          = nullptr;
    FreeBuffer             = nullptr;
    NewDirectoryHandle     = nullptr;
    HandleToClose          = nullptr;


    if (RtlIsDosDeviceName_Ustr(PathName)) {
        return STATUS_NOT_A_DIRECTORY;
    }

    RtlAcquirePebLock();
    do {

        //
        // Compute the length of the Dos style fully qualified current
        // directory
        //

        DosDirLength  = CurDir->DosPath.MaximumLength;
        DosDir.Buffer = static_cast<PWCH>(RtlAllocateHeap(Heap, 0, DosDirLength));
        if (!DosDir.Buffer) {
            Status = STATUS_NO_MEMORY;
            break;
        }

        DosDir.Length = 0;
        DosDir.MaximumLength = static_cast<USHORT>(DosDirLength);

        //
        // Now get the full pathname for the Dos style current
        // directory
        //

        DosDirLength = RtlGetFullPathName_Ustr(
            PathName,
            DosDirLength,
            DosDir.Buffer,
            nullptr,
            nullptr,
            &InputPathType
        );
        if (!DosDirLength) {
            Status = STATUS_OBJECT_NAME_INVALID;
            break;
        }

        if (DosDirLength > DosDir.MaximumLength) {
            Status = STATUS_NAME_TOO_LONG;
            break;
        }

        DosDirCharCount = DosDirLength >> 1;

        //
        // Get the Nt filename of the new current directory
        //
        TranslationStatus = RtlDosPathNameToNtPathName_U(
            DosDir.Buffer,
            &NtFileName,
            nullptr,
            nullptr
        );

        if (!TranslationStatus) {
            Status = STATUS_OBJECT_NAME_INVALID;
            break;
        }
        FreeBuffer = NtFileName.Buffer;

        InitializeObjectAttributes(
            &Obja,
            &NtFileName,
            OBJ_CASE_INSENSITIVE | OBJ_INHERIT | OBJ_KERNEL_HANDLE,
            nullptr,
            nullptr
        );

        //
        // If we are inheriting current directory, then
        // avoid the open
        //

        if ((reinterpret_cast<ULONG_PTR>(CurDir->Handle) & OBJ_HANDLE_TAGBITS) == RTL_USER_PROC_CURDIR_INHERIT) {
            NewDirectoryHandle = reinterpret_cast<HANDLE>(reinterpret_cast<ULONG_PTR>(CurDir->Handle) & ~OBJ_HANDLE_TAGBITS);
            CurDir->Handle     = nullptr;

            //
            // Test to see if this is removable media. If so
            // tag the handle this may fail if the process was
            // created with inherit handles set to false
            //

            Status = NtQueryVolumeInformationFile(
                NewDirectoryHandle,
                &IoStatusBlock,
                &DeviceInfo,
                sizeof(DeviceInfo),
                FileFsDeviceInformation
            );
            if (!NT_SUCCESS(Status)) {
                Status = RtlSetCurrentDirectory_U(PathName);
                break;
            }
            else {
                if (DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA) {
                    NewDirectoryHandle = reinterpret_cast<HANDLE>(reinterpret_cast<ULONG_PTR>(NewDirectoryHandle) | 1);
                }
            }

        }
        else {
            //
            // Open a handle to the current directory. Don't allow
            // deletes of the directory.
            //

            Status = ZwOpenFile(
                &NewDirectoryHandle,
                FILE_TRAVERSE | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
            );

            if (!NT_SUCCESS(Status)) {
                break;
            }

            //
            // Test to see if this is removable media. If so
            // tag the handle
            //
            Status = ZwQueryVolumeInformationFile(
                NewDirectoryHandle,
                &IoStatusBlock,
                &DeviceInfo,
                sizeof(DeviceInfo),
                FileFsDeviceInformation
            );
            if (!NT_SUCCESS(Status)) {
                break;
            }
            else {
                if (DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA) {
                    NewDirectoryHandle = reinterpret_cast<HANDLE>(reinterpret_cast<ULONG_PTR>(NewDirectoryHandle) | 1);
                }
            }
        }

        //
        // If there is no trailing '\', than place one
        //

        DosDir.Length = static_cast<USHORT>(DosDirLength);
        if (DosDir.Buffer[DosDirCharCount - 1] != L'\\') {

            if ((DosDirCharCount + 2) > (DosDir.MaximumLength / sizeof(WCHAR))) {
                Status = STATUS_NAME_TOO_LONG;
                break;
            }
            DosDir.Buffer[DosDirCharCount] = L'\\';
            DosDir.Buffer[DosDirCharCount + 1] = UNICODE_NULL;
            DosDir.Length += sizeof(UNICODE_NULL);
        }

        //
        // Now we are set to change to the new directory.
        //

        CurDir->Handle = NewDirectoryHandle;
        NewDirectoryHandle = nullptr;

        RtlCopyMemory(CurDir->DosPath.Buffer, DosDir.Buffer, DosDir.Length + sizeof(UNICODE_NULL));
        CurDir->DosPath.Length = DosDir.Length;

    } while (false);
    RtlReleasePebLock();

    if (DosDir.Buffer != nullptr) {
        RtlFreeHeap(Heap, 0, DosDir.Buffer);
    }

    if (FreeBuffer != nullptr) {
        RtlFreeHeap(Heap, 0, FreeBuffer);
    }

    if (NewDirectoryHandle != nullptr) {
        (void)ZwClose(NewDirectoryHandle);
    }

    if (HandleToClose != nullptr) {
        (void)ZwClose(HandleToClose);
    }

    return Status;
}
MI_IAT_SYMBOL(RtlSetCurrentDirectory_U, 4);


EXTERN_C_END
