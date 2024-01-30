#include "KernelBase.Private.h"


namespace Mi
{
#if !defined _KERNEL_MODE
    PVOID WINAPI BaseIsRealtimeAllowed(
        BOOLEAN LeaveEnabled,
        BOOLEAN Revert
        )
    {
        PVOID State;
        ULONG Privilege = SE_INC_BASE_PRIORITY_PRIVILEGE;

        const auto Status = RtlAcquirePrivilege(&Privilege, 1,
            Revert ? RTL_ACQUIRE_PRIVILEGE_REVERT : 0, &State);
        if (!NT_SUCCESS(Status)) {
            return nullptr;
        }

        if (LeaveEnabled) {
            return State;
        }

        RtlReleasePrivilege(State);
        return reinterpret_cast<PVOID>(TRUE);
    }
#endif


}
