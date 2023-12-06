#include <intrin.h>


EXTERN_C_START
namespace Mi
{
    VOID WINAPI MI_NAME(RaiseException)(
        _In_ DWORD ExceptionCode,
        _In_ DWORD ExceptionFlags,
        _In_ DWORD NumberOfArguments,
        _In_reads_opt_(NumberOfArguments) CONST ULONG_PTR* Arguments
    )
    {
        EXCEPTION_RECORD ExceptionRecord{};
        ExceptionRecord.ExceptionCode    = static_cast<NTSTATUS>(ExceptionCode);
        ExceptionRecord.ExceptionFlags   = ExceptionFlags;
        ExceptionRecord.ExceptionAddress = _ReturnAddress();
        ExceptionRecord.NumberParameters = NumberOfArguments;

        if (Arguments != nullptr) {
            RtlCopyMemory(ExceptionRecord.ExceptionInformation, Arguments,
                min(NumberOfArguments, _countof(ExceptionRecord.ExceptionInformation)));
        }

        RtlRaiseException(&ExceptionRecord);
    }
    MI_IAT_SYMBOL(RaiseException, 16);

    _Check_return_
    _Post_equals_last_error_
    DWORD WINAPI MI_NAME(GetLastError)()
    {
        return RtlGetLastWin32Error();
    }
    MI_IAT_SYMBOL(GetLastError, 0);

    VOID WINAPI MI_NAME(SetLastError)(
        _In_ DWORD Win32Error
    )
    {
        return RtlSetLastWin32Error(static_cast<LONG>(Win32Error));
    }
    MI_IAT_SYMBOL(SetLastError, 4);
}
EXTERN_C_END
