#include <thunks/thunks.hpp>


EXTERN_C_START


NTSTATUS NTAPI MI_NAME(RtlCreateEnvironment)(
    _In_ BOOLEAN CloneCurrentEnvironment,
    _Out_ PVOID* Environment
    )
{
}
MI_IAT_SYMBOL(RtlCreateEnvironment, 8);

NTSTATUS NTAPI MI_NAME(RtlDestroyEnvironment)(
    _In_ PVOID Environment
)
{
}
MI_IAT_SYMBOL(RtlDestroyEnvironment, 4);

NTSTATUS NTAPI MI_NAME(RtlSetCurrentEnvironment)(
    _In_ PVOID Environment,
    _Out_opt_ PVOID* PreviousEnvironment
)
{
}
MI_IAT_SYMBOL(RtlSetCurrentEnvironment, 8);

NTSTATUS NTAPI MI_NAME(RtlSetEnvironmentVar)(
    _Inout_opt_ PVOID* Environment,
    _In_reads_(NameLength) PCWSTR Name,
    _In_ SIZE_T NameLength,
    _In_reads_(ValueLength) PCWSTR Value,
    _In_ SIZE_T ValueLength
)
{
}
MI_IAT_SYMBOL(RtlSetEnvironmentVar, 20);

NTSTATUS NTAPI MI_NAME(RtlSetEnvironmentVariable)(
    _Inout_opt_ PVOID* Environment,
    _In_ PCUNICODE_STRING Name,
    _In_opt_ PCUNICODE_STRING Value
)
{
}
MI_IAT_SYMBOL(RtlSetEnvironmentVariable, 12);

NTSTATUS NTAPI MI_NAME(RtlQueryEnvironmentVariable)(
    _In_opt_ PVOID Environment,
    _In_reads_(NameLength) PCWSTR Name,
    _In_ SIZE_T NameLength,
    _Out_writes_(ValueLength) PWSTR Value,
    _In_ SIZE_T ValueLength,
    _Out_ PSIZE_T ReturnLength
)
{

}
MI_IAT_SYMBOL(RtlQueryEnvironmentVariable, 24);

NTSTATUS NTAPI MI_NAME(RtlQueryEnvironmentVariable_U)(
    _In_opt_ PVOID Environment,
    _In_ PCUNICODE_STRING Name,
    _Inout_ PCUNICODE_STRING Value
)
{
}
MI_IAT_SYMBOL(RtlQueryEnvironmentVariable_U, 12);

NTSTATUS NTAPI MI_NAME(RtlExpandEnvironmentStrings)(
    _In_opt_ PVOID Environment,
    _In_reads_(SourceLength) PCWSTR Source,
    _In_ SIZE_T SourceLength,
    _Out_writes_(DestinationLength) PWSTR Destination,
    _In_ SIZE_T DestinationLength,
    _Out_opt_ PSIZE_T ReturnLength
)
{

}
MI_IAT_SYMBOL(RtlExpandEnvironmentStrings, 24);

NTSTATUS NTAPI MI_NAME(RtlExpandEnvironmentStrings_U)(
    _In_opt_ PVOID Environment,
    _In_ PCUNICODE_STRING Source,
    _Inout_ PUNICODE_STRING Destination,
    _Out_opt_ PULONG ReturnedLength
)
{
}
MI_IAT_SYMBOL(RtlExpandEnvironmentStrings_U, 16);

NTSTATUS NTAPI MI_NAME(RtlSetEnvironmentStrings)(
    _In_ PCWCHAR NewEnvironment,
    _In_ SIZE_T NewEnvironmentSize
)
{
}
MI_IAT_SYMBOL(RtlSetEnvironmentStrings, 8);

EXTERN_C_END
