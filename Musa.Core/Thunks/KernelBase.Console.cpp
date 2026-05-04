//
// Console I/O
//

#include "KernelBase.Private.h"


#include "Internal/KernelBase.Console.h"
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(GetConsoleOutputCP))
#pragma alloc_text(PAGE, MUSA_NAME(GetConsoleMode))
#pragma alloc_text(PAGE, MUSA_NAME(ReadConsoleW))
#endif

EXTERN_C_START

_IRQL_requires_max_(PASSIVE_LEVEL)
UINT WINAPI MUSA_NAME(GetConsoleOutputCP)(
    VOID
)
{
    PAGED_CODE();

    // Kernel mode: no console. Return ANSI code page.
    return CP_ACP;

}

MUSA_IAT_SYMBOL(GetConsoleOutputCP, 0);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetConsoleMode)(
    _In_  HANDLE   hConsoleHandle,
    _Out_ LPDWORD  lpMode
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(hConsoleHandle);
    UNREFERENCED_PARAMETER(lpMode);

    // Kernel mode: no console.
    BaseSetLastNTError(STATUS_NOT_SUPPORTED);
    return FALSE;
}

MUSA_IAT_SYMBOL(GetConsoleMode, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(ReadConsoleW)(
    _In_    HANDLE  hConsoleInput,
    _Out_   LPVOID  lpBuffer,
    _In_    DWORD   nNumberOfCharsToRead,
    _Out_   LPDWORD lpNumberOfCharsRead,
    _In_opt_ LPVOID pInputControl
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(hConsoleInput);
    UNREFERENCED_PARAMETER(lpBuffer);
    UNREFERENCED_PARAMETER(nNumberOfCharsToRead);
    UNREFERENCED_PARAMETER(lpNumberOfCharsRead);
    UNREFERENCED_PARAMETER(pInputControl);

    // Kernel mode: no console.
    BaseSetLastNTError(STATUS_NOT_SUPPORTED);
    return FALSE;
}

MUSA_IAT_SYMBOL(ReadConsoleW, 20);

EXTERN_C_END
