#pragma once

EXTERN_C_START

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetFileType)(
    _In_ HANDLE hFile
    );

_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MUSA_NAME(ReadFile)(
    _In_ HANDLE       hFile,
    _Out_writes_bytes_to_opt_(nNumberOfBytesToRead, *lpNumberOfBytesRead) __out_data_source(FILE) LPVOID lpBuffer,
    _In_ DWORD        nNumberOfBytesToRead,
    _Out_opt_ LPDWORD lpNumberOfBytesRead,
    _Inout_opt_ LPOVERLAPPED lpOverlapped
    );

_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MUSA_NAME(WriteFile)(
    _In_ HANDLE       hFile,
    _In_reads_bytes_(nNumberOfBytesToWrite) LPCVOID lpBuffer,
    _In_ DWORD        nNumberOfBytesToWrite,
    _Out_opt_ LPDWORD lpNumberOfBytesWritten,
    _Inout_opt_ LPOVERLAPPED lpOverlapped
    );

_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MUSA_NAME(FlushFileBuffers)(
    _In_ HANDLE hFile
    );


_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(CreateFileW)(
    _In_ LPCWSTR lpFileName,
    _In_ DWORD dwDesiredAccess,
    _In_ DWORD dwShareMode,
    _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    _In_ DWORD dwCreationDisposition,
    _In_ DWORD dwFlagsAndAttributes,
    _In_opt_ HANDLE hTemplateFile
    );

_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MUSA_NAME(SetFilePointerEx)(
    _In_ HANDLE hFile,
    _In_ LARGE_INTEGER liDistanceToMove,
    _Out_opt_ PLARGE_INTEGER lpNewFilePointer,
    _In_ DWORD dwMoveMethod
    );

_IRQL_requires_max_(APC_LEVEL)
DWORD WINAPI MUSA_NAME(SetFilePointer)(
    _In_ HANDLE hFile,
    _In_ LONG lDistanceToMove,
    _Inout_opt_ PLONG lpDistanceToMoveHigh,
    _In_ DWORD dwMoveMethod
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetFileAttributesExW)(
    _In_ LPCWSTR lpFileName,
    _In_ GET_FILEEX_INFO_LEVELS fInfoLevelId,
    _Out_ LPVOID lpFileInformation
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(DeleteFileW)(
    _In_ LPCWSTR lpFileName
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SetFileAttributesW)(
    _In_ LPCWSTR lpFileName,
    _In_ DWORD dwFileAttributes
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetTempPathW)(
    _In_ DWORD nBufferLength,
    _Out_writes_to_opt_(nBufferLength, return + 1) LPWSTR lpBuffer
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(CreateDirectoryW)(
    _In_ LPCWSTR lpPathName,
    _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(RemoveDirectoryW)(
    _In_ LPCWSTR lpPathName
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(MoveFileExW)(
    _In_ LPCWSTR lpExistingFileName,
    _In_ LPCWSTR lpNewFileName,
    _In_ DWORD dwFlags
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
UINT WINAPI MUSA_NAME(GetDriveTypeW)(
    _In_opt_ LPCWSTR lpRootPathName
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(FindFirstFileExW)(
    _In_ LPCWSTR lpFileName,
    _In_ FINDEX_INFO_LEVELS fInfoLevelId,
    _Out_ LPVOID lpFindFileData,
    _In_ FINDEX_SEARCH_OPS fSearchOp,
    _Reserved_ LPVOID lpSearchFilter,
    _In_ DWORD dwAdditionalFlags
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(FindNextFileW)(
    _In_ HANDLE hFindFile,
    _Out_ LPWIN32_FIND_DATAW lpFindFileData
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(FindClose)(
    _Inout_ HANDLE hFindFile
    );

EXTERN_C_END
