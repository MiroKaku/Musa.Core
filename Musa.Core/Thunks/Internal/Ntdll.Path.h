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


EXTERN_C_END
