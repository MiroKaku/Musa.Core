#pragma once

EXTERN_C_START

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN NTAPI MUSA_NAME(RtlDosPathNameToNtPathName_U)(
    _In_ PCWSTR DosPathName,
    _Out_ PUNICODE_STRING NtPathName,
    _Out_opt_ PCWSTR* FilePart,
    _Out_opt_ PRTL_RELATIVE_NAME_U RelativeName
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlDosPathNameToNtPathName_U_WithStatus)(
    _In_ PCWSTR DosPathName,
    _Out_ PUNICODE_STRING NtPathName,
    _Out_opt_ PCWSTR* FilePart,
    _Out_opt_ PRTL_RELATIVE_NAME_U RelativeName
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
ULONG NTAPI MUSA_NAME(RtlGetCurrentDirectory_U)(
    _In_ ULONG BufferLength,
    _Out_writes_bytes_(BufferLength) PWSTR Buffer
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlSetCurrentDirectory_U)(
    _In_ PCUNICODE_STRING PathName
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlGetFullPathName_UEx)(
    _In_ PWSTR FileName,
    _In_ ULONG BufferLength,
    _Out_writes_bytes_opt_(BufferLength) PWSTR Buffer,
    _Out_opt_ PWSTR* FilePart,
    _Out_opt_ RTL_PATH_TYPE* InputPathType
    );

EXTERN_C_END
