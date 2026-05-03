#include "KernelBase.Private.h"
#include "Internal/KernelBase.System.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(GetSystemTimes))
#endif

using namespace Musa;


EXTERN_C_START

//
// System Times
//

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetSystemTimes)(
    _Out_opt_ PFILETIME IdleTime,
    _Out_opt_ PFILETIME KernelTime,
    _Out_opt_ PFILETIME UserTime
)
{
    PAGED_CODE();

    NTSTATUS                                  Status;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION ProcessorTimes = nullptr;

    do {
        SYSTEM_BASIC_INFORMATION BaseInformation{};
        Status = ZwQuerySystemInformation(SystemBasicInformation,
            &BaseInformation, sizeof(BaseInformation), nullptr);
        if (!NT_SUCCESS(Status)) {
            break;
        }

        const auto NumberOfProcessors   = BaseInformation.NumberOfProcessors;
        const auto SizeOfProcessorTimes = NumberOfProcessors * sizeof(*ProcessorTimes);

        ProcessorTimes = static_cast<PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION>(
            LocalAlloc(LPTR, SizeOfProcessorTimes));
        if (ProcessorTimes == nullptr) {
            Status = STATUS_NO_MEMORY;
            break;
        }

        ULONG ReturnLength = 0;
        Status = ZwQuerySystemInformation(SystemProcessorPerformanceInformation,
            ProcessorTimes, static_cast<ULONG>(SizeOfProcessorTimes), &ReturnLength);
        if (!NT_SUCCESS(Status)) {
            break;
        }

        if (ReturnLength != SizeOfProcessorTimes) {
            Status = STATUS_INTERNAL_ERROR;
            break;
        }

        LONG           Lupe;
        ULARGE_INTEGER Sum;

        #pragma warning(suppress: 6385)
        #define BASEP_GST_SUM(DST, SRC)                                \
        if ( DST ) {                                                   \
            Sum.QuadPart = 0;                                          \
            for (Lupe = 0;                                             \
                 Lupe < NumberOfProcessors;                            \
                 Lupe++) {                                             \
                Sum.QuadPart += ProcessorTimes[Lupe]. SRC .QuadPart ;  \
            }                                                          \
            (DST)->dwLowDateTime  = Sum.LowPart;                       \
            (DST)->dwHighDateTime = Sum.HighPart;                      \
        }

        BASEP_GST_SUM(IdleTime, IdleTime)
        BASEP_GST_SUM(KernelTime, KernelTime)
        BASEP_GST_SUM(UserTime, UserTime)
        #undef BASEP_GST_SUM

        Status = STATUS_SUCCESS;
    } while (false);

    if (ProcessorTimes) {
        LocalFree(ProcessorTimes);
    }

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

MUSA_IAT_SYMBOL(GetSystemTimes, 12);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MUSA_NAME(GetSystemTime)(
    _Out_ LPSYSTEMTIME lpSystemTime
)
{
    PAGED_CODE();

    LARGE_INTEGER SystemTime{};
    KeQuerySystemTimePrecise(&SystemTime);

    TIME_FIELDS TimeFields{};
    RtlTimeToTimeFields(&SystemTime, &TimeFields);

    lpSystemTime->wYear = TimeFields.Year;
    lpSystemTime->wMonth = TimeFields.Month;
    lpSystemTime->wDayOfWeek = TimeFields.Weekday;
    lpSystemTime->wDay = TimeFields.Day;
    lpSystemTime->wHour = TimeFields.Hour;
    lpSystemTime->wMinute = TimeFields.Minute;
    lpSystemTime->wSecond = TimeFields.Second;
    lpSystemTime->wMilliseconds = TimeFields.Milliseconds;
}

MUSA_IAT_SYMBOL(GetSystemTime, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MUSA_NAME(GetLocalTime)(
    _Out_ LPSYSTEMTIME lpSystemTime
)
{
    PAGED_CODE();

    LARGE_INTEGER SystemTime{};
    KeQuerySystemTimePrecise(&SystemTime);
    LARGE_INTEGER LocalTime{};
    ExSystemTimeToLocalTime(&SystemTime, &LocalTime);

    TIME_FIELDS TimeFields{};
    RtlTimeToTimeFields(&LocalTime, &TimeFields);

    lpSystemTime->wYear = TimeFields.Year;
    lpSystemTime->wMonth = TimeFields.Month;
    lpSystemTime->wDayOfWeek = TimeFields.Weekday;
    lpSystemTime->wDay = TimeFields.Day;
    lpSystemTime->wHour = TimeFields.Hour;
    lpSystemTime->wMinute = TimeFields.Minute;
    lpSystemTime->wSecond = TimeFields.Second;
    lpSystemTime->wMilliseconds = TimeFields.Milliseconds;
}

MUSA_IAT_SYMBOL(GetLocalTime, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(FileTimeToSystemTime)(
    _In_ const FILETIME* lpFileTime,
    _Out_ LPSYSTEMTIME lpSystemTime
)
{
    PAGED_CODE();
    if (lpFileTime == nullptr || lpSystemTime == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    LARGE_INTEGER Time{};
    Time.LowPart = lpFileTime->dwLowDateTime;
    Time.HighPart = static_cast<LONG>(lpFileTime->dwHighDateTime);
    TIME_FIELDS TimeFields{};
    RtlTimeToTimeFields(&Time, &TimeFields);
    lpSystemTime->wYear = TimeFields.Year;
    lpSystemTime->wMonth = TimeFields.Month;
    lpSystemTime->wDayOfWeek = TimeFields.Weekday;
    lpSystemTime->wDay = TimeFields.Day;
    lpSystemTime->wHour = TimeFields.Hour;
    lpSystemTime->wMinute = TimeFields.Minute;
    lpSystemTime->wSecond = TimeFields.Second;
    lpSystemTime->wMilliseconds = TimeFields.Milliseconds;
    return TRUE;
}

MUSA_IAT_SYMBOL(FileTimeToSystemTime, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SystemTimeToFileTime)(
    _In_ const SYSTEMTIME* lpSystemTime,
    _Out_ LPFILETIME lpFileTime
)
{
    PAGED_CODE();
    if (lpSystemTime == nullptr || lpFileTime == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    TIME_FIELDS TimeFields{};
    TimeFields.Year = lpSystemTime->wYear;
    TimeFields.Month = lpSystemTime->wMonth;
    TimeFields.Day = lpSystemTime->wDay;
    TimeFields.Hour = lpSystemTime->wHour;
    TimeFields.Minute = lpSystemTime->wMinute;
    TimeFields.Second = lpSystemTime->wSecond;
    TimeFields.Milliseconds = lpSystemTime->wMilliseconds;
    LARGE_INTEGER Time{};
    BOOLEAN Result = RtlTimeFieldsToTime(&TimeFields, &Time);
    if (!Result) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    lpFileTime->dwLowDateTime = Time.LowPart;
    lpFileTime->dwHighDateTime = Time.HighPart;
    return TRUE;
}

MUSA_IAT_SYMBOL(SystemTimeToFileTime, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(FileTimeToLocalFileTime)(
    _In_ const FILETIME* lpFileTime,
    _Out_ LPFILETIME lpLocalFileTime
)
{
    PAGED_CODE();
    if (lpFileTime == nullptr || lpLocalFileTime == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    LARGE_INTEGER UtcTime{};
    UtcTime.LowPart = lpFileTime->dwLowDateTime;
    UtcTime.HighPart = static_cast<LONG>(lpFileTime->dwHighDateTime);
    LARGE_INTEGER LocalTime{};
    ExSystemTimeToLocalTime(&UtcTime, &LocalTime);
    lpLocalFileTime->dwLowDateTime = LocalTime.LowPart;
    lpLocalFileTime->dwHighDateTime = LocalTime.HighPart;
    return TRUE;
}

MUSA_IAT_SYMBOL(FileTimeToLocalFileTime, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(LocalFileTimeToFileTime)(
    _In_ const FILETIME* lpLocalFileTime,
    _Out_ LPFILETIME lpFileTime
)
{
    PAGED_CODE();
    if (lpLocalFileTime == nullptr || lpFileTime == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    LARGE_INTEGER LocalTime{};
    LocalTime.LowPart = lpLocalFileTime->dwLowDateTime;
    LocalTime.HighPart = static_cast<LONG>(lpLocalFileTime->dwHighDateTime);
    LARGE_INTEGER UtcTime{};
    ExLocalTimeToSystemTime(&LocalTime, &UtcTime);
    lpFileTime->dwLowDateTime = UtcTime.LowPart;
    lpFileTime->dwHighDateTime = UtcTime.HighPart;
    return TRUE;
}

MUSA_IAT_SYMBOL(LocalFileTimeToFileTime, 8);


_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetTimeZoneInformation)(
    _Out_ LPTIME_ZONE_INFORMATION lpTimeZoneInformation
)
{
    PAGED_CODE();

    if (lpTimeZoneInformation == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return TIME_ZONE_ID_INVALID;
    }

    // Read time zone info from registry
    UNICODE_STRING KeyPath = RTL_CONSTANT_STRING(
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\TimeZoneInformation");

    OBJECT_ATTRIBUTES ObjAttrs;
    InitializeObjectAttributes(&ObjAttrs, &KeyPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);

    HANDLE KeyHandle = nullptr;
    NTSTATUS Status = ZwOpenKey(&KeyHandle, KEY_READ, &ObjAttrs);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return TIME_ZONE_ID_INVALID;
    }

    // Read ActiveTimeBias (LONG) -- the main bias value
    RtlZeroMemory(lpTimeZoneInformation, sizeof(TIME_ZONE_INFORMATION));

    UNICODE_STRING ValueName;
    ULONG ResultLength = 0;

    // Bias
    LONG Bias = 0;
    RtlInitUnicodeString(&ValueName, L"ActiveTimeBias");
    Status = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation,
        nullptr, 0, &ResultLength);
    if (NT_SUCCESS(Status) || Status == STATUS_BUFFER_OVERFLOW) {
        ULONG BufSize = ResultLength + sizeof(KEY_VALUE_PARTIAL_INFORMATION);
        auto Info = static_cast<PKEY_VALUE_PARTIAL_INFORMATION>(
            RtlAllocateHeap(RtlProcessHeap(), 0, BufSize));
        if (Info) {
            Status = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation,
                Info, BufSize, &ResultLength);
            if (NT_SUCCESS(Status) && Info->Type == REG_DWORD) {
                Bias = *reinterpret_cast<PLONG>(Info->Data);
            }
            RtlFreeHeap(RtlProcessHeap(), 0, Info);
        }
    }

    lpTimeZoneInformation->Bias = Bias;
    lpTimeZoneInformation->StandardBias = 0;
    lpTimeZoneInformation->DaylightBias = -60; // Default: 1 hour

    // StandardName
    RtlInitUnicodeString(&ValueName, L"StandardName");
    Status = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation,
        nullptr, 0, &ResultLength);
    if (NT_SUCCESS(Status) || Status == STATUS_BUFFER_OVERFLOW) {
        ULONG BufSize = ResultLength + sizeof(KEY_VALUE_PARTIAL_INFORMATION);
        auto Info = static_cast<PKEY_VALUE_PARTIAL_INFORMATION>(
            RtlAllocateHeap(RtlProcessHeap(), 0, BufSize));
        if (Info) {
            Status = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation,
                Info, BufSize, &ResultLength);
            if (NT_SUCCESS(Status) && Info->Type == REG_SZ) {
                USHORT NameLen = static_cast<USHORT>(Info->DataLength);
                if (NameLen > sizeof(lpTimeZoneInformation->StandardName) - sizeof(WCHAR))
                    NameLen = sizeof(lpTimeZoneInformation->StandardName) - sizeof(WCHAR);
                memcpy(lpTimeZoneInformation->StandardName, Info->Data, NameLen);
            }
            RtlFreeHeap(RtlProcessHeap(), 0, Info);
        }
    }

    // DaylightName
    RtlInitUnicodeString(&ValueName, L"DaylightName");
    Status = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation,
        nullptr, 0, &ResultLength);
    if (NT_SUCCESS(Status) || Status == STATUS_BUFFER_OVERFLOW) {
        ULONG BufSize = ResultLength + sizeof(KEY_VALUE_PARTIAL_INFORMATION);
        auto Info = static_cast<PKEY_VALUE_PARTIAL_INFORMATION>(
            RtlAllocateHeap(RtlProcessHeap(), 0, BufSize));
        if (Info) {
            Status = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation,
                Info, BufSize, &ResultLength);
            if (NT_SUCCESS(Status) && Info->Type == REG_SZ) {
                USHORT NameLen = static_cast<USHORT>(Info->DataLength);
                if (NameLen > sizeof(lpTimeZoneInformation->DaylightName) - sizeof(WCHAR))
                    NameLen = sizeof(lpTimeZoneInformation->DaylightName) - sizeof(WCHAR);
                memcpy(lpTimeZoneInformation->DaylightName, Info->Data, NameLen);
            }
            RtlFreeHeap(RtlProcessHeap(), 0, Info);
        }
    }

    ZwClose(KeyHandle);
    return TIME_ZONE_ID_UNKNOWN;
}

MUSA_IAT_SYMBOL(GetTimeZoneInformation, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetDynamicTimeZoneInformation)(
    _Out_ PDYNAMIC_TIME_ZONE_INFORMATION pTimeZoneInformation
)
{
    PAGED_CODE();
    if (pTimeZoneInformation == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return TIME_ZONE_ID_INVALID;
    }
    TIME_ZONE_INFORMATION Tzi{};
    DWORD Result = MUSA_NAME(GetTimeZoneInformation)(&Tzi);
    if (Result == TIME_ZONE_ID_INVALID) {
        return TIME_ZONE_ID_INVALID;
    }
    memcpy(pTimeZoneInformation, &Tzi, sizeof(TIME_ZONE_INFORMATION));
    pTimeZoneInformation->DynamicDaylightTimeDisabled = FALSE;
    wcscpy_s(pTimeZoneInformation->TimeZoneKeyName, 128, L"");
    return Result;
}

MUSA_IAT_SYMBOL(GetDynamicTimeZoneInformation, 4);
EXTERN_C_END
