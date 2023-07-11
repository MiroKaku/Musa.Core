#include <thunks/thunks.hpp>


EXTERN_C_START


NTSTATUS NTAPI MI_NAME(RtlGetLastNtStatus)(
    VOID
)
{
    if (const auto MiThreadBlock = Mi::Thunk::GetThreadBlock()) {
        return MiThreadBlock->LastNtStatus;
    }
    return STATUS_INTERNAL_ERROR;
}
MI_IAT_SYMBOL(RtlGetLastNtStatus, 0);

LONG NTAPI MI_NAME(RtlGetLastWin32Error)(
    VOID
)
{
    if (const auto MiThreadBlock = Mi::Thunk::GetThreadBlock()) {
        return static_cast<LONG>(MiThreadBlock->LastWin32Error);
    }
    return STATUS_INTERNAL_ERROR;
}
MI_IAT_SYMBOL(RtlGetLastWin32Error, 0);

VOID NTAPI MI_NAME(RtlSetLastWin32ErrorAndNtStatusFromNtStatus)(
    _In_ NTSTATUS Status
)
{
    if (const auto MiThreadBlock = Mi::Thunk::GetThreadBlock()) {
        MiThreadBlock->LastNtStatus   = Status;
        MiThreadBlock->LastWin32Error = RtlNtStatusToDosErrorNoTeb(Status);
    }
}
MI_IAT_SYMBOL(RtlSetLastWin32ErrorAndNtStatusFromNtStatus, 4);

VOID NTAPI MI_NAME(RtlSetLastWin32Error)(
    _In_ LONG Win32Error
)
{
    if (const auto MiThreadBlock = Mi::Thunk::GetThreadBlock()) {
        MiThreadBlock->LastWin32Error = Win32Error;
    }
}
MI_IAT_SYMBOL(RtlSetLastWin32Error, 4);


EXTERN_C_END
