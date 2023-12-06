#ifdef _KERNEL_MODE

#include "MiCore/MiCore.SystemEnvironmentBlock.Private.h"


EXTERN_C_START
namespace Mi
{
    LONG NTAPI MI_NAME(RtlGetLastWin32Error)()
    {
        const auto Teb = MI_NAME_PRIVATE(RtlGetCurrentTeb)();
        if (Teb) {
            return static_cast<LONG>(Teb->LastErrorValue);
        }
        return STATUS_INTERNAL_ERROR;
    }
    MI_IAT_SYMBOL(RtlGetLastWin32Error, 0);

    VOID NTAPI MI_NAME(RtlSetLastWin32Error)(
        _In_ LONG Win32Error
        )
    {
        const auto Teb = MI_NAME_PRIVATE(RtlGetCurrentTeb)();
        if (Teb) {
            Teb->LastErrorValue = Win32Error;
        }
    }
    MI_IAT_SYMBOL(RtlSetLastWin32Error, 4);

    VOID NTAPI MI_NAME(RtlRestoreLastWin32Error)(
        _In_ LONG Win32Error
        )
    {
        return MI_NAME(RtlSetLastWin32Error)(Win32Error);
    }
    MI_IAT_SYMBOL(RtlRestoreLastWin32Error, 4);

    NTSTATUS NTAPI MI_NAME(RtlGetLastNtStatus)()
    {
        const auto Teb = MI_NAME_PRIVATE(RtlGetCurrentTeb)();
        if (Teb) {
            return Teb->LastStatusValue;
        }
        return STATUS_INTERNAL_ERROR;
    }
    MI_IAT_SYMBOL(RtlGetLastNtStatus, 0);

    VOID NTAPI MI_NAME(RtlSetLastNtStatus)(_In_ NTSTATUS Status)
    {
        const auto Teb = MI_NAME_PRIVATE(RtlGetCurrentTeb)();
        if (Teb) {
            Teb->LastStatusValue = Status;
        }
    }
    MI_IAT_SYMBOL(RtlSetLastNtStatus, 0);

    VOID NTAPI MI_NAME(RtlSetLastWin32ErrorAndNtStatusFromNtStatus)(
        _In_ NTSTATUS Status
        )
    {
        const auto Teb = MI_NAME_PRIVATE(RtlGetCurrentTeb)();
        if (Teb) {
            Teb->LastErrorValue  = RtlNtStatusToDosErrorNoTeb(Status);
            Teb->LastStatusValue = Status;
        }
    }
    MI_IAT_SYMBOL(RtlSetLastWin32ErrorAndNtStatusFromNtStatus, 4);

}
EXTERN_C_END


#endif
