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

    VOID NTAPI MI_NAME(RtlSetUnhandledExceptionFilter)(
        _In_opt_ PTOP_LEVEL_EXCEPTION_FILTER UnhandledExceptionFilter
        )
    {
        UNREFERENCED_PARAMETER(UnhandledExceptionFilter);
    }
    MI_IAT_SYMBOL(RtlSetUnhandledExceptionFilter, 4);

    LONG NTAPI MI_NAME(RtlUnhandledExceptionFilter)(
        _In_ PEXCEPTION_POINTERS ExceptionPointers
        )
    {
        UNREFERENCED_PARAMETER(ExceptionPointers);

        return ExSystemExceptionFilter();
    }
    MI_IAT_SYMBOL(RtlUnhandledExceptionFilter, 4);

    ULONG NTAPI MI_NAME(RtlGetThreadErrorMode)(
        VOID
        )
    {
        const auto Teb = MI_NAME_PRIVATE(RtlGetCurrentTeb)();
        if (Teb) {
            return Teb->HardErrorMode;
        }
        return 0;
    }
    MI_IAT_SYMBOL(RtlGetThreadErrorMode, 0);

#pragma warning(suppress: 6101)
    NTSTATUS NTAPI MI_NAME(RtlSetThreadErrorMode)(
        _In_ ULONG NewMode,
        _Out_opt_ PULONG OldMode
        )
    {
        const auto Teb = MI_NAME_PRIVATE(RtlGetCurrentTeb)();
        if (Teb) {
            const auto PreviousMode = InterlockedExchange(
                reinterpret_cast<LONG volatile*>(&Teb->HardErrorMode), NewMode);

            if (OldMode) {
                *OldMode = PreviousMode;
            }
        }
        return STATUS_SUCCESS;
    }
    MI_IAT_SYMBOL(RtlSetThreadErrorMode, 8);

    PVOID NTAPI MI_NAME(RtlAddVectoredExceptionHandler)(
        _In_ ULONG First,
        _In_ PVECTORED_EXCEPTION_HANDLER Handler
        )
    {
        UNREFERENCED_PARAMETER(First);

        return RtlEncodePointer(Handler);
    }
    MI_IAT_SYMBOL(RtlAddVectoredExceptionHandler, 8);

    ULONG NTAPI MI_NAME(RtlRemoveVectoredExceptionHandler)(
        _In_ PVOID Handle
        )
    {
        return static_cast<ULONG>(reinterpret_cast<ULONG_PTR>(RtlDecodePointer(Handle)));
    }
    MI_IAT_SYMBOL(RtlRemoveVectoredExceptionHandler, 4);

    PVOID NTAPI MI_NAME(RtlAddVectoredContinueHandler)(
        _In_ ULONG First,
        _In_ PVECTORED_EXCEPTION_HANDLER Handler
        )
    {
        UNREFERENCED_PARAMETER(First);

        return RtlEncodePointer(Handler);
    }
    MI_IAT_SYMBOL(RtlAddVectoredContinueHandler, 8);

    ULONG NTAPI MI_NAME(RtlRemoveVectoredContinueHandler)(
        _In_ PVOID Handle
        )
    {
        return static_cast<ULONG>(reinterpret_cast<ULONG_PTR>(RtlDecodePointer(Handle)));
    }
    MI_IAT_SYMBOL(RtlRemoveVectoredContinueHandler, 4);


}
EXTERN_C_END


#endif
