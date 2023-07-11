#include <thunks/thunks.hpp>


EXTERN_C_START


_Check_return_
_Post_equals_last_error_
DWORD WINAPI MI_NAME(GetLastError)(
    VOID
)
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


EXTERN_C_END
