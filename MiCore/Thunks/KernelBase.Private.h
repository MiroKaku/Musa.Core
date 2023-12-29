#pragma once


namespace Mi
{
    #define BaseSetLastNTError RtlSetLastWin32ErrorAndNtStatusFromNtStatus

    PVOID WINAPI BaseIsRealtimeAllowed(
        BOOLEAN LeaveEnabled,
        BOOLEAN Revert
        );

}
