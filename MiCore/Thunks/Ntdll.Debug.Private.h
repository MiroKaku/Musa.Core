#pragma once

EXTERN_C_START


#ifdef _KERNEL_MODE

NTSTATUS NTAPI RtlFindAndFormatMessage(
    _In_     DWORD      Flags,
    _In_opt_ LPCVOID    Source,
    _In_     DWORD      MessageId,
    _In_     DWORD      LanguageId,
    _When_((Flags& FORMAT_MESSAGE_ALLOCATE_BUFFER) != 0, _At_((LPWSTR*)Buffer, _Outptr_result_z_))
    _When_((Flags& FORMAT_MESSAGE_ALLOCATE_BUFFER) == 0, _Out_writes_z_(*Size))
             LPWSTR     Buffer,
    _Inout_  DWORD*     Size,
    _In_opt_ va_list*   Arguments
    );

BOOLEAN NTAPI MI_NAME(RtlFindAndFormatMessage)(
    _In_     DWORD      Flags,
    _In_opt_ LPCVOID    Source,
    _In_     DWORD      MessageId,
    _In_     DWORD      LanguageId,
    _When_((Flags& FORMAT_MESSAGE_ALLOCATE_BUFFER) != 0, _At_((LPWSTR*)Buffer, _Outptr_result_z_))
    _When_((Flags& FORMAT_MESSAGE_ALLOCATE_BUFFER) == 0, _Out_writes_z_(*Size))
             LPWSTR     Buffer,
    _Inout_  DWORD*     Size,
    _In_opt_ va_list*   Arguments
    );

#ifdef _X86_
_VEIL_DECLARE_ALTERNATE_NAME(RtlFindAndFormatMessage@28, _Mi_RtlFindAndFormatMessage@28);
#else
_VEIL_DECLARE_ALTERNATE_NAME(RtlFindAndFormatMessage, _Mi_RtlFindAndFormatMessage);
#endif

#endif

EXTERN_C_END
