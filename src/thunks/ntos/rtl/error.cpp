#include <thunks/thunks.hpp>


EXTERN_C_START


NTSTATUS NTAPI MI_NAME(RtlGetLastNtStatus)(
    VOID
)
{
    if (const auto Teb = Mi::Thunk::GetCurrentTeb()) {
        return Teb->LastStatusValue;
    }
    return STATUS_INTERNAL_ERROR;
}
MI_IAT_SYMBOL(RtlGetLastNtStatus, 0);

LONG NTAPI MI_NAME(RtlGetLastWin32Error)(
    VOID
)
{
    if (const auto Teb = Mi::Thunk::GetCurrentTeb()) {
        return static_cast<LONG>(Teb->LastErrorValue);
    }
    return STATUS_INTERNAL_ERROR;
}
MI_IAT_SYMBOL(RtlGetLastWin32Error, 0);

VOID NTAPI MI_NAME(RtlSetLastWin32ErrorAndNtStatusFromNtStatus)(
    _In_ NTSTATUS Status
)
{
    if (const auto Teb = Mi::Thunk::GetCurrentTeb()) {
        Teb->LastStatusValue = Status;
        Teb->LastErrorValue  = RtlNtStatusToDosErrorNoTeb(Status);
    }
}
MI_IAT_SYMBOL(RtlSetLastWin32ErrorAndNtStatusFromNtStatus, 4);

VOID NTAPI MI_NAME(RtlSetLastWin32Error)(
    _In_ LONG Win32Error
)
{
    if (const auto Teb = Mi::Thunk::GetCurrentTeb()) {
        Teb->LastErrorValue = Win32Error;
    }
}
MI_IAT_SYMBOL(RtlSetLastWin32Error, 4);


EXTERN_C_END
