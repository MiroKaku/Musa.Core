#include "KernelBase.Private.h"
#include "Musa.Core/Musa.Core.SystemEnvironmentBlock.Private.h"

#include <intrin.h>


EXTERN_C_START
namespace Musa
{
    VOID WINAPI MUSA_NAME(RaiseException)(
        _In_ DWORD ExceptionCode,
        _In_ DWORD ExceptionFlags,
        _In_ DWORD NumberOfArguments,
        _In_reads_opt_(NumberOfArguments) CONST ULONG_PTR* Arguments
    )
    {
        /*
         *  Exception Record
         */

        EXCEPTION_RECORD ExceptionRecord{};
        ExceptionRecord.ExceptionCode    = static_cast<NTSTATUS>(ExceptionCode);
        ExceptionRecord.ExceptionFlags   = ExceptionFlags & EXCEPTION_NONCONTINUABLE;
        ExceptionRecord.ExceptionAddress = _ReturnAddress();

        /* Check if we have arguments */
        if (!Arguments) {
            /* We don't */
            ExceptionRecord.NumberParameters = 0;
        }
        else {
            /* We do, normalize the count */
            if (NumberOfArguments > EXCEPTION_MAXIMUM_PARAMETERS)
                NumberOfArguments = EXCEPTION_MAXIMUM_PARAMETERS;

            /* Set the count of parameters and copy them */
            ExceptionRecord.NumberParameters = NumberOfArguments;
            RtlCopyMemory(ExceptionRecord.ExceptionInformation,
                Arguments, NumberOfArguments * sizeof(ULONG_PTR));
        }

        RtlRaiseException(&ExceptionRecord);
    }
    MUSA_IAT_SYMBOL(RaiseException, 16);

    __callback
    LONG WINAPI MUSA_NAME(UnhandledExceptionFilter)(
        _In_ PEXCEPTION_POINTERS ExceptionInfo
        )
    {
        return RtlUnhandledExceptionFilter(ExceptionInfo);
    }
    MUSA_IAT_SYMBOL(UnhandledExceptionFilter, 4);

    volatile static LPTOP_LEVEL_EXCEPTION_FILTER MUSA_NAME(TopLevelExceptionFilter) = MUSA_NAME(UnhandledExceptionFilter);

    LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MUSA_NAME(SetUnhandledExceptionFilter)(
        _In_opt_ LPTOP_LEVEL_EXCEPTION_FILTER TopLevelExceptionFilter
        )
    {
        const auto PreviousExceptionFilter =InterlockedExchangePointer(
            reinterpret_cast<PVOID volatile*>(&MUSA_NAME(TopLevelExceptionFilter)), TopLevelExceptionFilter);

        RtlSetUnhandledExceptionFilter(TopLevelExceptionFilter);

        return static_cast<LPTOP_LEVEL_EXCEPTION_FILTER>(PreviousExceptionFilter);
    }
    MUSA_IAT_SYMBOL(SetUnhandledExceptionFilter, 4);

    _Check_return_
    _Post_equals_last_error_
    DWORD WINAPI MUSA_NAME(GetLastError)()
    {
        return RtlGetLastWin32Error();
    }
    MUSA_IAT_SYMBOL(GetLastError, 0);

    VOID WINAPI MUSA_NAME(SetLastError)(
        _In_ DWORD Win32Error
    )
    {
        return RtlSetLastWin32Error(static_cast<LONG>(Win32Error));
    }
    MUSA_IAT_SYMBOL(SetLastError, 4);

    UINT WINAPI MUSA_NAME(GetErrorMode)()
    {
    #if !defined _KERNEL_MODE
        UINT Mode = 0;

        const auto Status = ZwQueryInformationProcess(ZwCurrentProcess(),
            ProcessDefaultHardErrorMode, &Mode, sizeof(Mode),
            nullptr);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return 0;
        }

        if (BooleanFlagOn(Mode, SEM_FAILCRITICALERRORS)) {
            ClearFlag(Mode, SEM_FAILCRITICALERRORS);
        }
        else {
            SetFlag(Mode, SEM_FAILCRITICALERRORS);
        }
        return Mode;
    #else
        return MUSA_NAME_PRIVATE(RtlGetCurrentPeb)()->HardErrorMode;
    #endif
    }
    MUSA_IAT_SYMBOL(GetErrorMode, 0);

    UINT WINAPI MUSA_NAME(SetErrorMode)(
        _In_ UINT Mode
        )
    {
    #if !defined _KERNEL_MODE
        const auto PreviousMode = GetErrorMode();

        if (BooleanFlagOn(Mode, SEM_FAILCRITICALERRORS)) {
            ClearFlag(Mode, SEM_FAILCRITICALERRORS);
        }
        else {
            SetFlag(Mode, SEM_FAILCRITICALERRORS);
        }

        const auto Status = NtSetInformationProcess(NtCurrentProcess(),
            ProcessDefaultHardErrorMode, &Mode, sizeof(Mode));
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
        }

        return(PreviousMode);
    #else
        return InterlockedExchange(
            reinterpret_cast<long volatile*>(&MUSA_NAME_PRIVATE(RtlGetCurrentPeb)()->HardErrorMode), Mode);
    #endif
    }
    MUSA_IAT_SYMBOL(SetErrorMode, 4);

    DWORD WINAPI MUSA_NAME(GetThreadErrorMode)(
        VOID
        )
    {
        return RtlGetThreadErrorMode();
    }
    MUSA_IAT_SYMBOL(GetThreadErrorMode, 0);

    BOOL WINAPI MUSA_NAME(SetThreadErrorMode)(
        _In_     DWORD   NewMode,
        _In_opt_ LPDWORD OldMode
        )
    {
        const auto Status = RtlSetThreadErrorMode(NewMode, OldMode);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MUSA_IAT_SYMBOL(SetThreadErrorMode, 8);

    _Ret_maybenull_
    PVOID WINAPI MUSA_NAME(AddVectoredExceptionHandler)(
        _In_ ULONG First,
        _In_ PVECTORED_EXCEPTION_HANDLER Handler
        )
    {
        return RtlAddVectoredExceptionHandler(First, Handler);
    }
    MUSA_IAT_SYMBOL(AddVectoredExceptionHandler, 8);

    ULONG WINAPI MUSA_NAME(RemoveVectoredExceptionHandler)(
        _In_ PVOID Handle
        )
    {
        return RtlRemoveVectoredExceptionHandler(Handle);
    }
    MUSA_IAT_SYMBOL(RemoveVectoredExceptionHandler, 4);

    _Ret_maybenull_
    PVOID WINAPI MUSA_NAME(AddVectoredContinueHandler)(
        _In_ ULONG First,
        _In_ PVECTORED_EXCEPTION_HANDLER Handler
        )
    {
        return RtlAddVectoredContinueHandler(First, Handler);
    }
    MUSA_IAT_SYMBOL(AddVectoredContinueHandler, 8);

    ULONG WINAPI MUSA_NAME(RemoveVectoredContinueHandler)(
        _In_ PVOID Handle
        )
    {
        return RtlRemoveVectoredContinueHandler(Handle);
    }
    MUSA_IAT_SYMBOL(RemoveVectoredContinueHandler, 4);


}
EXTERN_C_END
