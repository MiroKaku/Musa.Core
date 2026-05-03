#include "Musa.Core/Musa.Core.SystemEnvironmentBlock.Process.h"
#include "Internal/Ntdll.Path.h"
#include "Internal/Ntdll.Heap.h"
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(RtlDosPathNameToNtPathName_U_WithStatus))
#pragma alloc_text(PAGE, MUSA_NAME(RtlDosPathNameToNtPathName_U))
#pragma alloc_text(PAGE, MUSA_NAME(RtlGetCurrentDirectory_U))
#pragma alloc_text(PAGE, MUSA_NAME(RtlSetCurrentDirectory_U))
#endif


EXTERN_C_START

// Path type constants matching ntdll.dll RtlDetermineDosPathNameType_Ustr
#define MUSA_RTL_PATH_UNC_ROOT        1
#define MUSA_RTL_PATH_DRIVE_ABSOLUTE  2
#define MUSA_RTL_PATH_DRIVE_RELATIVE  3
#define MUSA_RTL_PATH_UNC             4
#define MUSA_RTL_PATH_RELATIVE        5
#define MUSA_RTL_PATH_EXTENDED        6
#define MUSA_RTL_PATH_SHORT_PREFIX    7

static int RtlpDetermineDosPathNameType(PCWSTR Path)
{
    if (!Path || Path[0] == L'\0')
        return MUSA_RTL_PATH_RELATIVE;

    if (Path[0] != L'\\' && Path[0] != L'/') {
        if (Path[0] != L'\0' && Path[1] == L':' && Path[2] != L'\0') {
            if (Path[2] == L'\\' || Path[2] == L'/')
                return MUSA_RTL_PATH_DRIVE_ABSOLUTE;
            return MUSA_RTL_PATH_DRIVE_RELATIVE;
        }
        return MUSA_RTL_PATH_RELATIVE;
    }

    if (Path[1] != L'\\' && Path[1] != L'/')
        return MUSA_RTL_PATH_UNC;

    if (Path[2] == L'\0')
        return MUSA_RTL_PATH_SHORT_PREFIX;

    if (Path[2] == L'.' || Path[2] == L'?') {
        if (Path[3] == L'\\' || Path[3] == L'/')
            return MUSA_RTL_PATH_EXTENDED;
        return MUSA_RTL_PATH_SHORT_PREFIX;
    }

    return MUSA_RTL_PATH_UNC_ROOT;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlDosPathNameToNtPathName_U_WithStatus)(
    _In_ PCWSTR DosPathName,
    _Out_ PUNICODE_STRING NtPathName,
    _Out_opt_ PCWSTR* FilePart,
    _Out_opt_ PRTL_RELATIVE_NAME_U RelativeName)
{
    PAGED_CODE();
    if (NtPathName == nullptr)
        return STATUS_INVALID_PARAMETER;

    if (DosPathName == nullptr || DosPathName[0] == L'\0') {
        NtPathName->Buffer = nullptr;
        NtPathName->Length = 0;
        NtPathName->MaximumLength = 0;
        if (FilePart) *FilePart = nullptr;
        if (RelativeName) RelativeName->RelativeName.Buffer = nullptr;
        return STATUS_OBJECT_PATH_SYNTAX_BAD;
    }

    // Validate input length (ntdll max is 0x7FFE WCHARs)
    const size_t MaxPathChars = 0x7FFE;
    size_t InputLen = 0;
    while (InputLen < MaxPathChars && DosPathName[InputLen] != L'\0')
        ++InputLen;

    if (InputLen == 0 || InputLen >= MaxPathChars)
        return STATUS_NAME_TOO_LONG;

    int PathType = RtlpDetermineDosPathNameType(DosPathName);
    PCWSTR Source = DosPathName;
    size_t SourceLen = InputLen;
    PCWSTR Prefix = nullptr;
    size_t PrefixLen = 0;

    switch (PathType) {
    case MUSA_RTL_PATH_EXTENDED:
        if (InputLen >= 8 && DosPathName[4] == L'U' && DosPathName[5] == L'N' &&
            DosPathName[6] == L'C' && (DosPathName[7] == L'\\' || DosPathName[7] == L'/')) {
            Prefix = L"\\??\\UNC\\";
            PrefixLen = 8;
            Source = DosPathName + 8;
            SourceLen = InputLen - 8;
        } else if (DosPathName[2] == L'.' && InputLen >= 12 &&
            _wcsnicmp(DosPathName + 4, L"GLOBALROOT", 10) == 0 &&
            (DosPathName[14] == L'\\' || DosPathName[14] == L'/')) {
            Prefix = L"\\";
            PrefixLen = 1;
            Source = DosPathName + 14;
            SourceLen = InputLen - 14;
        } else {
            Prefix = L"\\??\\";
            PrefixLen = 4;
            Source = DosPathName + 4;
            SourceLen = InputLen - 4;
        }
        break;

    case MUSA_RTL_PATH_UNC_ROOT:
        Prefix = L"\\??\\UNC\\";
        PrefixLen = 8;
        Source = DosPathName + 2;
        SourceLen = InputLen - 2;
        break;

    case MUSA_RTL_PATH_DRIVE_ABSOLUTE:
        Prefix = L"\\??\\";
        PrefixLen = 4;
        Source = DosPathName;
        SourceLen = InputLen;
        break;

    case MUSA_RTL_PATH_DRIVE_RELATIVE:
    case MUSA_RTL_PATH_RELATIVE:
        return STATUS_OBJECT_PATH_SYNTAX_BAD;

    case MUSA_RTL_PATH_UNC: {
        // Always allocate a copy so caller can safely RtlFreeUnicodeString
        const size_t AllocSize = (InputLen + 1) * sizeof(WCHAR);
        PWSTR Buffer = static_cast<PWSTR>(
            RtlAllocateHeap(RtlProcessHeap(), 0, AllocSize));
        if (Buffer == nullptr)
            return STATUS_INSUFFICIENT_RESOURCES;
        memcpy(Buffer, DosPathName, InputLen * sizeof(WCHAR));
        Buffer[InputLen] = L'\0';
        NtPathName->Buffer = Buffer;
        NtPathName->Length = static_cast<USHORT>(InputLen * sizeof(WCHAR));
        NtPathName->MaximumLength = static_cast<USHORT>(AllocSize);
        if (FilePart) *FilePart = nullptr;
        if (RelativeName) RelativeName->RelativeName.Buffer = nullptr;
        return STATUS_SUCCESS;
    }

    case MUSA_RTL_PATH_SHORT_PREFIX:
    default:
        return STATUS_OBJECT_PATH_SYNTAX_BAD;
    }

    // Validate path characters
    for (size_t i = 0; i < SourceLen; ++i) {
        wchar_t c = Source[i];
        if (c < 0x20) return STATUS_OBJECT_NAME_INVALID;
        if (c == L'<' || c == L'>' || c == L'"' || c == L'|' ||
            c == L'?' || c == L'*') return STATUS_OBJECT_NAME_INVALID;
    }

    // Compute total length
    const size_t TotalLen = PrefixLen + SourceLen;
    if (TotalLen >= MaxPathChars)
        return STATUS_NAME_TOO_LONG;

    // Allocate via RtlAllocateHeap (matching ntdll.dll behavior)
    const size_t AllocSize = (TotalLen + 1) * sizeof(WCHAR);
    PWSTR Buffer = static_cast<PWSTR>(
        RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, AllocSize));
    if (Buffer == nullptr)
        return STATUS_INSUFFICIENT_RESOURCES;

#pragma warning(suppress: 6386)
    // Build NT path
    memcpy(Buffer, Prefix, PrefixLen * sizeof(WCHAR));
    memcpy(Buffer + PrefixLen, Source, SourceLen * sizeof(WCHAR));
    Buffer[TotalLen] = L'\0';

    NtPathName->Buffer = Buffer;
    NtPathName->Length = static_cast<USHORT>(TotalLen * sizeof(WCHAR));
    NtPathName->MaximumLength = static_cast<USHORT>(AllocSize);

    if (FilePart) *FilePart = nullptr;
    if (RelativeName) RelativeName->RelativeName.Buffer = nullptr;

    return STATUS_SUCCESS;
}

MUSA_IAT_SYMBOL(RtlDosPathNameToNtPathName_U_WithStatus, 16);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN NTAPI MUSA_NAME(RtlDosPathNameToNtPathName_U)(
    _In_ PCWSTR DosPathName,
    _Out_ PUNICODE_STRING NtPathName,
    _Out_opt_ PCWSTR* FilePart,
    _Out_opt_ PRTL_RELATIVE_NAME_U RelativeName)
{
    PAGED_CODE();
    return NT_SUCCESS(MUSA_NAME(RtlDosPathNameToNtPathName_U_WithStatus)(
        DosPathName, NtPathName, FilePart, RelativeName));
}

_IRQL_requires_max_(PASSIVE_LEVEL)
ULONG NTAPI MUSA_NAME(RtlGetCurrentDirectory_U)(
    _In_ ULONG BufferLength,
    _Out_writes_bytes_(BufferLength) PWSTR Buffer
)
{
    PAGED_CODE();

    if (BufferLength == 0 || Buffer == nullptr) {
        const auto Peb = static_cast<Musa::Core::KPEB*>(MUSA_NAME_PRIVATE(RtlGetCurrentPeb)());
        if (Peb == nullptr) return 0;
        return static_cast<ULONG>(wcslen(Peb->CurrentDirectory)) + 1;
    }

    const auto Peb = static_cast<Musa::Core::KPEB*>(MUSA_NAME_PRIVATE(RtlGetCurrentPeb)());
    if (Peb == nullptr) return 0;

    RtlStringCchCopyW(Buffer, BufferLength / sizeof(WCHAR), Peb->CurrentDirectory);
    return static_cast<ULONG>(wcslen(Peb->CurrentDirectory));
}

MUSA_IAT_SYMBOL(RtlGetCurrentDirectory_U, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlSetCurrentDirectory_U)(
    _In_ PCUNICODE_STRING PathName
)
{
    PAGED_CODE();

    if (PathName == nullptr || PathName->Buffer == nullptr || PathName->Length == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    const auto Peb = static_cast<Musa::Core::KPEB*>(MUSA_NAME_PRIVATE(RtlGetCurrentPeb)());
    if (Peb == nullptr) {
        return STATUS_UNSUCCESSFUL;
    }

    const USHORT CopyLen = min(PathName->Length, (MAX_PATH - 1) * sizeof(WCHAR));
    RtlStringCchCopyNW(Peb->CurrentDirectory, MAX_PATH, PathName->Buffer, CopyLen / sizeof(WCHAR));
    return STATUS_SUCCESS;
}

MUSA_IAT_SYMBOL(RtlSetCurrentDirectory_U, 4);

MUSA_IAT_SYMBOL(RtlDosPathNameToNtPathName_U, 16);
EXTERN_C_END
