#pragma once

EXTERN_C_START

_IRQL_requires_max_(PASSIVE_LEVEL)
UINT WINAPI MUSA_NAME(GetConsoleOutputCP)(
    VOID
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetConsoleMode)(
    _In_  HANDLE  hConsoleHandle,
    _Out_ LPDWORD lpMode
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(ReadConsoleW)(
    _In_    HANDLE  hConsoleInput,
    _Out_   LPVOID  lpBuffer,
    _In_    DWORD   nNumberOfCharsToRead,
    _Out_   LPDWORD lpNumberOfCharsRead,
    _In_opt_ LPVOID pInputControl
    );

EXTERN_C_END
