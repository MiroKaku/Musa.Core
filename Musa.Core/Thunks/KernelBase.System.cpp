#include "KernelBase.Private.h"
#include "Musa.Core/Musa.Core.SystemEnvironmentBlock.Process.h"
#include "Internal/KernelBase.System.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(GetNativeSystemInfo))
#pragma alloc_text(PAGE, MUSA_NAME(GetSystemInfo))
#pragma alloc_text(PAGE, MUSA_NAME(VerifyVersionInfoW))
#endif

using namespace Musa;


EXTERN_C_START

//
// System Information
//

static VOID GetSystemInfoInternal(
    _In_ PSYSTEM_BASIC_INFORMATION     BasicInfo,
    _In_ PSYSTEM_PROCESSOR_INFORMATION ProcessorInfo,
    _Out_ LPSYSTEM_INFO                lpSystemInfo
)
{
    RtlZeroMemory(lpSystemInfo, sizeof(*lpSystemInfo));

    lpSystemInfo->wProcessorArchitecture      = ProcessorInfo->ProcessorArchitecture;
    lpSystemInfo->wReserved                   = 0;
    lpSystemInfo->dwPageSize                  = BasicInfo->PageSize;
    lpSystemInfo->lpMinimumApplicationAddress = (LPVOID)BasicInfo->MinimumUserModeAddress;
    lpSystemInfo->lpMaximumApplicationAddress = (LPVOID)BasicInfo->MaximumUserModeAddress;
    lpSystemInfo->dwActiveProcessorMask       = BasicInfo->ActiveProcessorsAffinityMask;
    lpSystemInfo->dwNumberOfProcessors        = BasicInfo->NumberOfProcessors;
    lpSystemInfo->wProcessorLevel             = ProcessorInfo->ProcessorLevel;
    lpSystemInfo->wProcessorRevision          = ProcessorInfo->ProcessorRevision;

    if (ProcessorInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
        if (ProcessorInfo->ProcessorLevel == 3) {
            lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_386;
        } else if (ProcessorInfo->ProcessorLevel == 4) {
            lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_486;
        } else {
            lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_PENTIUM;
        }
    } else if (ProcessorInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) {
        lpSystemInfo->dwProcessorType = PROCESSOR_INTEL_IA64;
    } else if (ProcessorInfo->ProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
        lpSystemInfo->dwProcessorType = PROCESSOR_AMD_X8664;
    } else {
        lpSystemInfo->dwProcessorType = 0;
    }

    lpSystemInfo->dwAllocationGranularity = BasicInfo->AllocationGranularity;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MUSA_NAME(GetNativeSystemInfo)(
    _Out_ LPSYSTEM_INFO SystemInfo
)
{
    PAGED_CODE();

    SYSTEM_BASIC_INFORMATION BasicInfo{};
    NTSTATUS                 Status = ZwQuerySystemInformation(
        SystemBasicInformation,
        &BasicInfo,
        sizeof(BasicInfo),
        nullptr);
    if (!NT_SUCCESS(Status)) {
        return;
    }

    SYSTEM_PROCESSOR_INFORMATION ProcessorInfo{};
    Status = ZwQuerySystemInformation(
        SystemProcessorInformation,
        &ProcessorInfo,
        sizeof(ProcessorInfo),
        nullptr);
    if (!NT_SUCCESS(Status)) {
        return;
    }

    GetSystemInfoInternal(
        &BasicInfo,
        &ProcessorInfo,
        SystemInfo);
}

MUSA_IAT_SYMBOL(GetNativeSystemInfo, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MUSA_NAME(GetSystemInfo)(
    _Out_ LPSYSTEM_INFO SystemInfo
)
{
    PAGED_CODE();

    return GetNativeSystemInfo(SystemInfo);
}

MUSA_IAT_SYMBOL(GetSystemInfo, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(VerifyVersionInfoW)(
    _Inout_ LPOSVERSIONINFOEXW lpVersionInformation,
    _In_    DWORD dwTypeMask,
    _In_    DWORDLONG dwlConditionMask
)
{
    PAGED_CODE();

    return RtlVerifyVersionInfo(lpVersionInformation, dwTypeMask, dwlConditionMask) == STATUS_SUCCESS;
}

MUSA_IAT_SYMBOL(VerifyVersionInfoW, 12);


_IRQL_requires_max_(PASSIVE_LEVEL)
LPWSTR WINAPI MUSA_NAME(GetCommandLineW)(
    VOID
)
{
    PAGED_CODE();

    // Kernel mode: no command line from PEB. Return empty string.
    static WCHAR EmptyStr[] = L"";
    return EmptyStr;
}

MUSA_IAT_SYMBOL(GetCommandLineW, 0);

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetEnvironmentVariableW)(
    _In_ LPCWSTR lpName,
    _Out_writes_to_opt_(nSize, return + 1) LPWSTR lpBuffer,
    _In_ DWORD nSize
)
{
    PAGED_CODE();

    if (lpName == nullptr || lpName[0] == L'\0') {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return 0;
    }

    // Open system environment registry key
    UNICODE_STRING KeyPath = RTL_CONSTANT_STRING(
        L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");

    OBJECT_ATTRIBUTES ObjAttrs;
    InitializeObjectAttributes(&ObjAttrs, &KeyPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);

    HANDLE KeyHandle = nullptr;
    NTSTATUS Status = ZwOpenKey(&KeyHandle, KEY_READ, &ObjAttrs);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return 0;
    }

    UNICODE_STRING ValueName;
    RtlInitUnicodeString(&ValueName, lpName);

    ULONG ResultLength = 0;
    Status = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation,
        nullptr, 0, &ResultLength);

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
        ZwClose(KeyHandle);
        BaseSetLastNTError(STATUS_OBJECT_NAME_NOT_FOUND);
        return 0;
    }

    if (Status != STATUS_BUFFER_OVERFLOW && Status != STATUS_BUFFER_TOO_SMALL) {
        ZwClose(KeyHandle);
        BaseSetLastNTError(Status);
        return 0;
    }

    ULONG BufSize = ResultLength + sizeof(KEY_VALUE_PARTIAL_INFORMATION);
    auto Info = static_cast<PKEY_VALUE_PARTIAL_INFORMATION>(
        RtlAllocateHeap(RtlProcessHeap(), 0, BufSize));
    if (Info == nullptr) {
        ZwClose(KeyHandle);
        BaseSetLastNTError(STATUS_INSUFFICIENT_RESOURCES);
        return 0;
    }

    Status = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation,
        Info, BufSize, &ResultLength);
    ZwClose(KeyHandle);

    if (!NT_SUCCESS(Status)) {
        RtlFreeHeap(RtlProcessHeap(), 0, Info);
        BaseSetLastNTError(Status);
        return 0;
    }

    if (Info->Type != REG_SZ && Info->Type != REG_EXPAND_SZ) {
        RtlFreeHeap(RtlProcessHeap(), 0, Info);
        BaseSetLastNTError(STATUS_OBJECT_TYPE_MISMATCH);
        return 0;
    }

    // DataLength includes null terminator
    DWORD ValueChars = Info->DataLength / sizeof(WCHAR);

    // Return required size (including null)
    if (lpBuffer == nullptr || nSize == 0) {
        DWORD RequiredSize = ValueChars;
        RtlFreeHeap(RtlProcessHeap(), 0, Info);
        return RequiredSize;
    }

    if (ValueChars > nSize) {
        RtlFreeHeap(RtlProcessHeap(), 0, Info);
        BaseSetLastNTError(STATUS_BUFFER_TOO_SMALL);
        return ValueChars;
    }

    memcpy(lpBuffer, Info->Data, Info->DataLength);
    RtlFreeHeap(RtlProcessHeap(), 0, Info);

    return ValueChars - 1; // Length without null
}

MUSA_IAT_SYMBOL(GetEnvironmentVariableW, 12);


_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetCurrentDirectoryW)(
    _In_ DWORD nBufferLength,
    _Out_writes_to_opt_(nBufferLength, return + 1) LPWSTR lpBuffer
)
{
    PAGED_CODE();
    // Kernel mode has no current directory concept. Return system root.
    const wchar_t* SysRoot = L"C:\\Windows";
    const DWORD Len = static_cast<DWORD>(wcslen(SysRoot)) + 1;

    if (lpBuffer == nullptr || nBufferLength == 0) {
        return Len;
    }
    if (Len > nBufferLength) {
        BaseSetLastNTError(STATUS_BUFFER_TOO_SMALL);
        return Len;
    }
    RtlStringCchCopyW(lpBuffer, nBufferLength, SysRoot);
    return Len - 1;
}

MUSA_IAT_SYMBOL(GetCurrentDirectoryW, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SetCurrentDirectoryW)(
    _In_ LPCWSTR lpPathName
)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(lpPathName);
    // Kernel mode: no per-process CWD. Always succeeds for compatibility.
    return TRUE;
}

MUSA_IAT_SYMBOL(SetCurrentDirectoryW, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(ExpandEnvironmentStringsW)(
    _In_ LPCWSTR lpSrc,
    _Out_writes_to_opt_(nSize, return + 1) LPWSTR lpDst,
    _In_ DWORD nSize
)
{
    PAGED_CODE();

    if (lpSrc == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return 0;
    }

    DWORD TotalLen = 0;
    PWSTR Dst = lpDst;
    DWORD Remaining = nSize;

    for (PCWSTR p = lpSrc; *p; ) {
        if (p[0] == L'%') {
            // Find closing %
            PCWSTR end = wcschr(p + 1, L'%');
            if (end == nullptr) { end = p + wcslen(p); }
            // Extract variable name
            WCHAR NameBuf[128];
            size_t NameLen = static_cast<size_t>(end - p - 1);
            if (NameLen >= _countof(NameBuf)) NameLen = _countof(NameBuf) - 1;
            wcsncpy_s(NameBuf, p + 1, NameLen);
            NameBuf[NameLen] = L'\0';

            DWORD ValLen = GetEnvironmentVariableW(NameBuf, nullptr, 0);
            if (ValLen > 0) {
                if (Remaining > ValLen) {
                    GetEnvironmentVariableW(NameBuf, Dst, Remaining);
                    Dst += ValLen - 1;
                    Remaining -= ValLen - 1;
                } else {
                    Dst += ValLen - 1;
                    Remaining = 0;
                }
                TotalLen += ValLen - 1;
            }
            p = (*end == L'%') ? end + 1 : end;
        } else {
            if (Remaining > 0) {
                if (Dst) *Dst++ = *p;
                --Remaining;
            }
            ++TotalLen;
            ++p;
        }
    }

    if (Dst && nSize > 0) *Dst = L'\0';
    return TotalLen + 1;
}

MUSA_IAT_SYMBOL(ExpandEnvironmentStringsW, 12);
EXTERN_C_END
