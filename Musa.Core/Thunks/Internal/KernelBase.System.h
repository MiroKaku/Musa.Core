#pragma once

EXTERN_C_START

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetLogicalProcessorInformation)(
    _Out_writes_bytes_to_opt_(*ReturnedLength, *ReturnedLength) PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer,
    _Inout_ PDWORD ReturnedLength
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetLogicalProcessorInformationEx)(
    _In_ LOGICAL_PROCESSOR_RELATIONSHIP RelationshipType,
    _Out_writes_bytes_to_opt_(*ReturnedLength, *ReturnedLength) PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX Buffer,
    _Inout_ PDWORD ReturnedLength
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetSystemTimes)(
    _Out_opt_ PFILETIME IdleTime,
    _Out_opt_ PFILETIME KernelTime,
    _Out_opt_ PFILETIME UserTime
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MUSA_NAME(GetNativeSystemInfo)(
    _Out_ LPSYSTEM_INFO SystemInfo
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MUSA_NAME(GetSystemInfo)(
    _Out_ LPSYSTEM_INFO SystemInfo
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(VerifyVersionInfoW)(
    _Inout_ LPOSVERSIONINFOEXW lpVersionInformation,
    _In_    DWORD dwTypeMask,
    _In_    DWORDLONG dwlConditionMask
    );


_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MUSA_NAME(GetSystemTime)(
    _Out_ LPSYSTEMTIME lpSystemTime
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MUSA_NAME(GetLocalTime)(
    _Out_ LPSYSTEMTIME lpSystemTime
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(FileTimeToSystemTime)(
    _In_ const FILETIME* lpFileTime,
    _Out_ LPSYSTEMTIME lpSystemTime
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SystemTimeToFileTime)(
    _In_ const SYSTEMTIME* lpSystemTime,
    _Out_ LPFILETIME lpFileTime
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(FileTimeToLocalFileTime)(
    _In_ const FILETIME* lpFileTime,
    _Out_ LPFILETIME lpLocalFileTime
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(LocalFileTimeToFileTime)(
    _In_ const FILETIME* lpLocalFileTime,
    _Out_ LPFILETIME lpFileTime
    );
_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetTimeZoneInformation)(
    _Out_ LPTIME_ZONE_INFORMATION lpTimeZoneInformation
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetDynamicTimeZoneInformation)(
    _Out_ PDYNAMIC_TIME_ZONE_INFORMATION pTimeZoneInformation
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
LPWSTR WINAPI MUSA_NAME(GetCommandLineW)(
    VOID
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetEnvironmentVariableW)(
    _In_ LPCWSTR lpName,
    _Out_writes_to_opt_(nSize, return + 1) LPWSTR lpBuffer,
    _In_ DWORD nSize
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetCurrentDirectoryW)(
    _In_ DWORD nBufferLength,
    _Out_writes_to_opt_(nBufferLength, return + 1) LPWSTR lpBuffer
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SetCurrentDirectoryW)(
    _In_ LPCWSTR lpPathName
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(ExpandEnvironmentStringsW)(
    _In_ LPCWSTR lpSrc,
    _Out_writes_to_opt_(nSize, return + 1) LPWSTR lpDst,
    _In_ DWORD nSize
    );
EXTERN_C_END
