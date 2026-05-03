#include "KernelBase.Private.h"
#include "Musa.Core/Musa.Core.SystemEnvironmentBlock.Process.h"
#include "Internal/KernelBase.System.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(GetNativeSystemInfo))
#pragma alloc_text(PAGE, MUSA_NAME(GetSystemInfo))
#pragma alloc_text(PAGE, MUSA_NAME(VerifyVersionInfoW))
#pragma alloc_text(PAGE, MUSA_NAME(GetCommandLineW))
#pragma alloc_text(PAGE, MUSA_NAME(GetEnvironmentVariableW))
#pragma alloc_text(PAGE, MUSA_NAME(GetCurrentDirectoryW))
#pragma alloc_text(PAGE, MUSA_NAME(SetCurrentDirectoryW))
#pragma alloc_text(PAGE, MUSA_NAME(ExpandEnvironmentStringsW))
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

#pragma warning(push)
#pragma warning(disable: 6385)
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

    // Try system environment key first
    static UNICODE_STRING EnvKeyPath =
        RTL_CONSTANT_STRING(L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");
    static UNICODE_STRING NtKeyPath =
        RTL_CONSTANT_STRING(L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");

    UNICODE_STRING ValueName;
    RtlInitUnicodeString(&ValueName, lpName);

    PUNICODE_STRING KeyPaths[] = { &EnvKeyPath, &NtKeyPath };
    for (size_t i = 0; i < _countof(KeyPaths); ++i) {
        PUNICODE_STRING KeyPath = KeyPaths[i];
        OBJECT_ATTRIBUTES ObjAttrs;
        InitializeObjectAttributes(&ObjAttrs, KeyPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);

        HANDLE KeyHandle = nullptr;
        NTSTATUS Status = ZwOpenKey(&KeyHandle, KEY_READ, &ObjAttrs);
        if (!NT_SUCCESS(Status)) continue;

        ULONG ResultLength = 0;
        Status = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation, nullptr, 0, &ResultLength);
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            ZwClose(KeyHandle);
            continue;
        }
        if (Status != STATUS_BUFFER_OVERFLOW && Status != STATUS_BUFFER_TOO_SMALL) {
            ZwClose(KeyHandle);
            continue;
        }

        ULONG BufSize = ResultLength + sizeof(KEY_VALUE_PARTIAL_INFORMATION);
        auto Info = static_cast<PKEY_VALUE_PARTIAL_INFORMATION>(
            RtlAllocateHeap(RtlProcessHeap(), 0, BufSize));
        if (Info == nullptr) {
            ZwClose(KeyHandle);
            continue;
        }

        Status = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation, Info, BufSize, &ResultLength);
        ZwClose(KeyHandle);
        if (!NT_SUCCESS(Status)) {
            RtlFreeHeap(RtlProcessHeap(), 0, Info);
            continue;
        }

        if (Info->Type != REG_SZ && Info->Type != REG_EXPAND_SZ) {
            RtlFreeHeap(RtlProcessHeap(), 0, Info);
            continue;
        }

        DWORD ValueChars = Info->DataLength / sizeof(WCHAR);
        if (lpBuffer == nullptr || nSize == 0) {
            RtlFreeHeap(RtlProcessHeap(), 0, Info);
            return ValueChars;
        }
        if (ValueChars > nSize) {
            RtlFreeHeap(RtlProcessHeap(), 0, Info);
            BaseSetLastNTError(STATUS_BUFFER_TOO_SMALL);
            return ValueChars;
        }

#pragma warning(suppress: 6385)
        memcpy(lpBuffer, Info->Data, Info->DataLength);
        RtlFreeHeap(RtlProcessHeap(), 0, Info);
        return ValueChars - 1;
    }

    BaseSetLastNTError(STATUS_OBJECT_NAME_NOT_FOUND);
    return 0;

}

MUSA_IAT_SYMBOL(GetEnvironmentVariableW, 12);


_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetCurrentDirectoryW)(
    _In_ DWORD nBufferLength,
    _Out_writes_to_opt_(nBufferLength, return + 1) LPWSTR lpBuffer
)
{
    PAGED_CODE();

    // Wrap RtlGetCurrentDirectory_U (returns character count)
#pragma warning(suppress: 6387)
    ULONG Result = RtlGetCurrentDirectory_U(nBufferLength * sizeof(WCHAR),
        reinterpret_cast<PWSTR>(lpBuffer));
    return Result;
}

MUSA_IAT_SYMBOL(GetCurrentDirectoryW, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SetCurrentDirectoryW)(
    _In_ LPCWSTR lpPathName
)
{
    PAGED_CODE();

    if (lpPathName == nullptr || lpPathName[0] == L'\0') {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    UNICODE_STRING PathStr;
    RtlInitUnicodeString(&PathStr, lpPathName);
    NTSTATUS Status = RtlSetCurrentDirectory_U(&PathStr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}

MUSA_IAT_SYMBOL(SetCurrentDirectoryW, 4);

#pragma warning(push)
#pragma warning(disable: 6385)
_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(ExpandEnvironmentStringsW)(
    _In_ LPCWSTR lpSrc,
    _Out_writes_to_opt_(nSize, return + 1) LPWSTR lpDst,
    _In_ DWORD nSize
)
{
#pragma warning(suppress: 6385)
    PAGED_CODE();

    if (lpSrc == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return 0;
    }

    DWORD TotalLen = 0;
    DWORD Remaining = (lpDst && nSize > 0) ? nSize : 0;
    PWSTR Dst = lpDst;

    for (PCWSTR p = lpSrc; *p; ) {
        if (p[0] == L'%') {
            PCWSTR end = wcschr(p + 1, L'%');
            if (end == nullptr) {
                if (Remaining > 0 && Dst) { *Dst++ = *p; --Remaining; }
                TotalLen++;
                p++;
                continue;
            }

            size_t NameLen = static_cast<size_t>(end - p - 1);
            if (NameLen == 0) {
                if (Remaining > 0 && Dst) { *Dst++ = L'%'; --Remaining; }
                TotalLen++;
                p = end + 1;
                continue;
            }

            WCHAR NameBuf[128];
            if (NameLen >= _countof(NameBuf)) NameLen = _countof(NameBuf) - 1;
            RtlStringCchCopyNW(NameBuf, _countof(NameBuf), p + 1, NameLen);

            DWORD ValLen = GetEnvironmentVariableW(NameBuf, nullptr, 0);
            if (ValLen > 0) {
                if (ValLen <= Remaining && Dst) {
                    GetEnvironmentVariableW(NameBuf, Dst, Remaining);
                    Dst += ValLen;
                    Remaining -= ValLen;
                } else if (Dst) {
                    Remaining = 0;
                }
                TotalLen += ValLen;
            }
            p = end + 1;
        } else {
            if (Remaining > 0 && Dst) {
                *Dst++ = *p;
                --Remaining;
            }
            TotalLen++;
            p++;
        }
    }

    if (Dst && nSize > 0 && Remaining > 0) {
        *Dst = L'\0';
    }
    return TotalLen + 1;
}
#pragma warning(pop)

MUSA_IAT_SYMBOL(ExpandEnvironmentStringsW, 12);
EXTERN_C_END
