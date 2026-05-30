#include "Musa.Core/Musa.Core.SystemEnvironmentBlock.Thread.h"

EXTERN_C_START

using namespace Musa;
using namespace Musa::Core;

LONG NTAPI MUSA_NAME(RtlGetLastWin32Error)()
{
    const auto Teb = (PKTEB)MUSA_NAME_PRIVATE(RtlGetCurrentTeb)();
    if (Teb) {
        return static_cast<LONG>(Teb->LastErrorValue);
    }
    return STATUS_INTERNAL_ERROR;
}

MUSA_IAT_SYMBOL(RtlGetLastWin32Error, 0);

VOID NTAPI MUSA_NAME(RtlSetLastWin32Error)(
    _In_ LONG Win32Error
)
{
    const auto Teb = (PKTEB)MUSA_NAME_PRIVATE(RtlGetCurrentTeb)();
    if (Teb) {
        Teb->LastErrorValue = Win32Error;
    }
}

MUSA_IAT_SYMBOL(RtlSetLastWin32Error, 4);

VOID NTAPI MUSA_NAME(RtlRestoreLastWin32Error)(
    _In_ LONG Win32Error
)
{
    return MUSA_NAME(RtlSetLastWin32Error)(Win32Error);
}

MUSA_IAT_SYMBOL(RtlRestoreLastWin32Error, 4);

NTSTATUS NTAPI MUSA_NAME(RtlGetLastNtStatus)()
{
    const auto Teb = (PKTEB)MUSA_NAME_PRIVATE(RtlGetCurrentTeb)();
    if (Teb) {
        return Teb->LastStatusValue;
    }
    return STATUS_INTERNAL_ERROR;
}

MUSA_IAT_SYMBOL(RtlGetLastNtStatus, 0);

VOID NTAPI MUSA_NAME(RtlSetLastNtStatus)(
    _In_ NTSTATUS Status
)
{
    const auto Teb = (PKTEB)MUSA_NAME_PRIVATE(RtlGetCurrentTeb)();
    if (Teb) {
        Teb->LastStatusValue = Status;
    }
}

MUSA_IAT_SYMBOL(RtlSetLastNtStatus, 4);

VOID NTAPI MUSA_NAME(RtlSetLastWin32ErrorAndNtStatusFromNtStatus)(
    _In_ NTSTATUS Status
)
{
    const auto Teb = (PKTEB)MUSA_NAME_PRIVATE(RtlGetCurrentTeb)();
    if (Teb) {
        Teb->LastErrorValue  = RtlNtStatusToDosErrorNoTeb(Status);
        Teb->LastStatusValue = Status;
    }
}

MUSA_IAT_SYMBOL(RtlSetLastWin32ErrorAndNtStatusFromNtStatus, 4);

LPTOP_LEVEL_EXCEPTION_FILTER MUSA_NAME(TopLevelExceptionFilter) = nullptr;

VOID NTAPI MUSA_NAME(RtlSetUnhandledExceptionFilter)(
    _In_opt_ PTOP_LEVEL_EXCEPTION_FILTER UnhandledExceptionFilter
)
{
    InterlockedExchangePointer(
        reinterpret_cast<PVOID volatile*>(&MUSA_NAME(TopLevelExceptionFilter)), UnhandledExceptionFilter);
}

MUSA_IAT_SYMBOL(RtlSetUnhandledExceptionFilter, 4);

LONG NTAPI MUSA_NAME(RtlUnhandledExceptionFilter)(
    _In_ PEXCEPTION_POINTERS ExceptionPointers
)
{
    auto Filter = MUSA_NAME(TopLevelExceptionFilter);
    if (Filter) {
        LONG Result = Filter(ExceptionPointers);
        if (Result == EXCEPTION_EXECUTE_HANDLER) {
            return Result;
        }
    }

    return ExSystemExceptionFilter();
}

MUSA_IAT_SYMBOL(RtlUnhandledExceptionFilter, 4);

ULONG NTAPI MUSA_NAME(RtlGetThreadErrorMode)(
    VOID
)
{
    const auto Teb = (PKTEB)MUSA_NAME_PRIVATE(RtlGetCurrentTeb)();
    if (Teb) {
        return Teb->HardErrorMode;
    }
    return 0;
}

MUSA_IAT_SYMBOL(RtlGetThreadErrorMode, 0);

#pragma warning(suppress: 6101)
NTSTATUS NTAPI MUSA_NAME(RtlSetThreadErrorMode)(
    _In_ ULONG       NewMode,
    _Out_opt_ PULONG OldMode
)
{
    const auto Teb = (PKTEB)MUSA_NAME_PRIVATE(RtlGetCurrentTeb)();
    if (Teb) {
        const auto PreviousMode = InterlockedExchange(
            reinterpret_cast<LONG volatile*>(&Teb->HardErrorMode), (LONG)NewMode);

        if (OldMode) {
            *OldMode = PreviousMode;
        }
    }
    return STATUS_SUCCESS;
}

MUSA_IAT_SYMBOL(RtlSetThreadErrorMode, 8);

EXTERN_C_END
