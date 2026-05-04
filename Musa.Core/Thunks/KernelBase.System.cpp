#include "KernelBase.Private.h"
#include "Musa.Core/Musa.Core.SystemEnvironmentBlock.Process.h"
#include "Internal/KernelBase.System.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(GetNativeSystemInfo))
#pragma alloc_text(PAGE, MUSA_NAME(GetSystemInfo))
#pragma alloc_text(PAGE, MUSA_NAME(VerifyVersionInfoW))
#pragma alloc_text(PAGE, MUSA_NAME(GetCommandLineW))
#pragma alloc_text(PAGE, MUSA_NAME(GetEnvironmentVariableW))
#pragma alloc_text(PAGE, MUSA_NAME(SetEnvironmentVariableW))
#pragma alloc_text(PAGE, MUSA_NAME(GetEnvironmentStringsW))
#pragma alloc_text(PAGE, MUSA_NAME(FreeEnvironmentStringsW))
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

    using namespace Musa::Core;
    const auto Peb = static_cast<PKPEB>(MUSA_NAME_PRIVATE(RtlGetCurrentPeb)());

    // 1. Check per-process environment list (shared lock)
    if (Peb) {
        MUSA_NAME_PRIVATE(RtlAcquirePebLockShared)();

        DWORD Result = 0;
        bool  Found  = false;
        for (PLIST_ENTRY Link = Peb->EnvironmentListHead.Flink;
             Link != &Peb->EnvironmentListHead;
             Link = Link->Flink) {
            auto Entry = CONTAINING_RECORD(Link, ENVIRONMENT_VARIABLE_ENTRY, Link);
            if (_wcsicmp(Entry->Name, lpName) == 0) {
                if (Entry->Value == nullptr) {
                    BaseSetLastNTError(STATUS_OBJECT_NAME_NOT_FOUND);
                    Result = 0;
                } else {
                    DWORD ValueLen = static_cast<DWORD>(wcslen(Entry->Value));
                    if (lpBuffer == nullptr || nSize == 0) {
                        Result = ValueLen + 1;
                    } else if (ValueLen + 1 > nSize) {
                        BaseSetLastNTError(STATUS_BUFFER_TOO_SMALL);
                        Result = ValueLen + 1;
                    } else {
                        memcpy(lpBuffer, Entry->Value, (ValueLen + 1) * sizeof(WCHAR));
                        Result = ValueLen;
                    }
                }
                Found = true;
                break;
            }
        }

        MUSA_NAME_PRIVATE(RtlReleasePebLockShared)();

        if (Found) return Result;
    }

    // 2. Fallback to system registry
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

#pragma warning(pop)

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SetEnvironmentVariableW)(
    _In_opt_ LPCWSTR lpName,
    _In_opt_ LPCWSTR lpValue
)
{
    PAGED_CODE();

    if (lpName == nullptr || lpName[0] == L'\0') {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    using namespace Musa::Core;
    const auto Peb = static_cast<PKPEB>(MUSA_NAME_PRIVATE(RtlGetCurrentPeb)());
    if (Peb == nullptr) {
        BaseSetLastNTError(STATUS_NOT_SUPPORTED);
        return FALSE;
    }

    MUSA_NAME_PRIVATE(RtlAcquirePebLockExclusive)();

    const auto Heap = RtlProcessHeap();
    BOOL Result = TRUE;

    do {
        // Find existing entry
        PENVIRONMENT_VARIABLE_ENTRY Existing = nullptr;
        for (PLIST_ENTRY Link = Peb->EnvironmentListHead.Flink;
             Link != &Peb->EnvironmentListHead;
             Link = Link->Flink) {
            auto Entry = CONTAINING_RECORD(Link, ENVIRONMENT_VARIABLE_ENTRY, Link);
            if (_wcsicmp(Entry->Name, lpName) == 0) {
                Existing = Entry;
                break;
            }
        }

        if (lpValue == nullptr) {
            // Delete
            if (Existing) {
                RemoveEntryList(&Existing->Link);
                RtlFreeHeap(Heap, 0, Existing->Name);
                if (Existing->Value) RtlFreeHeap(Heap, 0, Existing->Value);
                RtlFreeHeap(Heap, 0, Existing);
            }
            break;
        }

        // Set: update or insert
        auto ValueLen = (wcslen(lpValue) + 1) * sizeof(WCHAR);

        if (Existing) {
            auto NewValue = static_cast<PWSTR>(RtlAllocateHeap(Heap, 0, ValueLen));
            if (NewValue == nullptr) {
                BaseSetLastNTError(STATUS_NO_MEMORY);
                Result = FALSE;
                break;
            }
            memcpy(NewValue, lpValue, ValueLen);
            if (Existing->Value) RtlFreeHeap(Heap, 0, Existing->Value);
            Existing->Value = NewValue;
        } else {
            auto NameLen = (wcslen(lpName) + 1) * sizeof(WCHAR);
            auto Entry = static_cast<PENVIRONMENT_VARIABLE_ENTRY>(
                RtlAllocateHeap(Heap, 0, sizeof(ENVIRONMENT_VARIABLE_ENTRY)));
            if (Entry == nullptr) {
                BaseSetLastNTError(STATUS_NO_MEMORY);
                Result = FALSE;
                break;
            }
            Entry->Name = static_cast<PWSTR>(RtlAllocateHeap(Heap, 0, NameLen));
            Entry->Value = static_cast<PWSTR>(RtlAllocateHeap(Heap, 0, ValueLen));
            if (Entry->Name == nullptr || Entry->Value == nullptr) {
                if (Entry->Name) RtlFreeHeap(Heap, 0, Entry->Name);
                if (Entry->Value) RtlFreeHeap(Heap, 0, Entry->Value);
                RtlFreeHeap(Heap, 0, Entry);
                BaseSetLastNTError(STATUS_NO_MEMORY);
                Result = FALSE;
                break;
            }
            memcpy(Entry->Name, lpName, NameLen);
            memcpy(Entry->Value, lpValue, ValueLen);
            InsertTailList(&Peb->EnvironmentListHead, &Entry->Link);
        }
    } while (false);

    MUSA_NAME_PRIVATE(RtlReleasePebLockExclusive)();
    return Result;
}

MUSA_IAT_SYMBOL(SetEnvironmentVariableW, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
LPWCH WINAPI MUSA_NAME(GetEnvironmentStringsW)(
    VOID
)
{
    PAGED_CODE();

    using namespace Musa::Core;
    const auto Peb = static_cast<PKPEB>(MUSA_NAME_PRIVATE(RtlGetCurrentPeb)());
    if (Peb == nullptr) {
        BaseSetLastNTError(STATUS_NOT_SUPPORTED);
        return NULL;
    }

    // Try KPEB list first
    MUSA_NAME_PRIVATE(RtlAcquirePebLockShared)();
    SIZE_T TotalBytes = 0;
    for (PLIST_ENTRY Link = Peb->EnvironmentListHead.Flink;
         Link != &Peb->EnvironmentListHead;
         Link = Link->Flink) {
        auto Entry = CONTAINING_RECORD(Link, ENVIRONMENT_VARIABLE_ENTRY, Link);
        if (Entry->Value == nullptr) continue;
        TotalBytes += (wcslen(Entry->Name) + wcslen(Entry->Value) + 2) * sizeof(WCHAR);
    }

    if (TotalBytes > 0) {
        TotalBytes += sizeof(WCHAR);
        auto Block = static_cast<PWSTR>(RtlAllocateHeap(RtlProcessHeap(), 0, TotalBytes));
        if (Block) {
            RtlZeroMemory(Block, TotalBytes);
            PWSTR Dst = Block;
            for (PLIST_ENTRY Link = Peb->EnvironmentListHead.Flink;
                 Link != &Peb->EnvironmentListHead;
                 Link = Link->Flink) {
                auto Entry = CONTAINING_RECORD(Link, ENVIRONMENT_VARIABLE_ENTRY, Link);
                if (Entry->Value == nullptr) continue;
                auto NameLen = wcslen(Entry->Name);
                memcpy(Dst, Entry->Name, NameLen * sizeof(WCHAR));
                Dst += NameLen;
                *Dst++ = L'=';
                auto ValLen = wcslen(Entry->Value);
                memcpy(Dst, Entry->Value, (ValLen + 1) * sizeof(WCHAR));
                Dst += ValLen + 1;
            }
        }
        MUSA_NAME_PRIVATE(RtlReleasePebLockShared)();
        if (!Block) BaseSetLastNTError(STATUS_NO_MEMORY);
        return Block;
    }
    MUSA_NAME_PRIVATE(RtlReleasePebLockShared)();

#pragma warning(push)
#pragma warning(disable: 28121)  // SAL false positive: lock released above
    // Fallback to system registry
    static UNICODE_STRING KeyPaths[] = {
        RTL_CONSTANT_STRING(L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment"),
        RTL_CONSTANT_STRING(L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion")
    };

    // First pass: compute size
    for (size_t k = 0; k < _countof(KeyPaths); ++k) {
        OBJECT_ATTRIBUTES ObjAttrs;
        InitializeObjectAttributes(&ObjAttrs, &KeyPaths[k], OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);
        HANDLE KeyHandle = nullptr;
        if (!NT_SUCCESS(ZwOpenKey(&KeyHandle, KEY_READ, &ObjAttrs))) continue;
        ULONG Index = 0;
        for (;;) {
            UCHAR Buf[sizeof(KEY_VALUE_BASIC_INFORMATION) + 256 * sizeof(WCHAR)];
            ULONG ResultLength = 0;
            if (!NT_SUCCESS(ZwEnumerateValueKey(KeyHandle, Index, KeyValueBasicInformation, Buf, sizeof(Buf), &ResultLength)))
                break;
            auto Info = reinterpret_cast<PKEY_VALUE_BASIC_INFORMATION>(Buf);
            if (Info->Type != REG_SZ && Info->Type != REG_EXPAND_SZ) { ++Index; continue; }
            UNICODE_STRING ValueName;
            ValueName.Buffer = Info->Name;
            ValueName.Length = static_cast<USHORT>(Info->NameLength);
            ValueName.MaximumLength = ValueName.Length;
            ULONG QueryLength = 0;
            NTSTATUS St = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation, nullptr, 0, &QueryLength);
            if (St != STATUS_BUFFER_OVERFLOW && St != STATUS_BUFFER_TOO_SMALL) { ++Index; continue; }
            ULONG BufSize = QueryLength + sizeof(KEY_VALUE_PARTIAL_INFORMATION);
            auto ValInfo = static_cast<PKEY_VALUE_PARTIAL_INFORMATION>(RtlAllocateHeap(RtlProcessHeap(), 0, BufSize));
            if (!ValInfo) { ++Index; continue; }
            if (NT_SUCCESS(ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation, ValInfo, BufSize, &QueryLength))) {
                TotalBytes += Info->NameLength + sizeof(WCHAR);
                TotalBytes += ValInfo->DataLength;
            }
            RtlFreeHeap(RtlProcessHeap(), 0, ValInfo);
            ++Index;
        }
        ZwClose(KeyHandle);
    }

    if (TotalBytes == 0) {
        auto Block = static_cast<PWSTR>(RtlAllocateHeap(RtlProcessHeap(), 0, sizeof(WCHAR)));
        if (Block) Block[0] = L'\0';
        return Block;
    }

    TotalBytes += sizeof(WCHAR);
    auto Block = static_cast<PWSTR>(RtlAllocateHeap(RtlProcessHeap(), 0, TotalBytes));
    if (!Block) {
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return NULL;
    }
    RtlZeroMemory(Block, TotalBytes);

    // Second pass: fill
    PWSTR Dst = Block;
    for (size_t k = 0; k < _countof(KeyPaths); ++k) {
        OBJECT_ATTRIBUTES ObjAttrs;
        InitializeObjectAttributes(&ObjAttrs, &KeyPaths[k], OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);
        HANDLE KeyHandle = nullptr;
        if (!NT_SUCCESS(ZwOpenKey(&KeyHandle, KEY_READ, &ObjAttrs))) continue;
        ULONG Index = 0;
        for (;;) {
            UCHAR Buf[sizeof(KEY_VALUE_BASIC_INFORMATION) + 256 * sizeof(WCHAR)];
            ULONG ResultLength = 0;
            if (!NT_SUCCESS(ZwEnumerateValueKey(KeyHandle, Index, KeyValueBasicInformation, Buf, sizeof(Buf), &ResultLength)))
                break;
            auto Info = reinterpret_cast<PKEY_VALUE_BASIC_INFORMATION>(Buf);
            if (Info->Type != REG_SZ && Info->Type != REG_EXPAND_SZ) { ++Index; continue; }
            UNICODE_STRING ValueName;
            ValueName.Buffer = Info->Name;
            ValueName.Length = static_cast<USHORT>(Info->NameLength);
            ValueName.MaximumLength = ValueName.Length;
            ULONG QueryLength = 0;
            NTSTATUS St = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation, nullptr, 0, &QueryLength);
            if (St != STATUS_BUFFER_OVERFLOW && St != STATUS_BUFFER_TOO_SMALL) { ++Index; continue; }
            ULONG BufSize = QueryLength + sizeof(KEY_VALUE_PARTIAL_INFORMATION);
            auto ValInfo = static_cast<PKEY_VALUE_PARTIAL_INFORMATION>(RtlAllocateHeap(RtlProcessHeap(), 0, BufSize));
            if (!ValInfo) { ++Index; continue; }
            if (NT_SUCCESS(ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation, ValInfo, BufSize, &QueryLength))) {
                memcpy(Dst, Info->Name, Info->NameLength);
                Dst += Info->NameLength / sizeof(WCHAR);
                *Dst++ = L'=';
                RtlCopyMemory(Dst, ValInfo->Data, ValInfo->DataLength);
                Dst += ValInfo->DataLength / sizeof(WCHAR);
            }
            RtlFreeHeap(RtlProcessHeap(), 0, ValInfo);
            ++Index;
        }
        ZwClose(KeyHandle);
    }


#pragma warning(pop)
    return Block;
}

MUSA_IAT_SYMBOL(GetEnvironmentStringsW, 0);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(FreeEnvironmentStringsW)(
    _In_opt_ LPWCH lpszEnvironmentBlock
)
{
    PAGED_CODE();

    if (lpszEnvironmentBlock == nullptr) {
        return TRUE;
    }

    RtlFreeHeap(RtlProcessHeap(), 0, lpszEnvironmentBlock);
    return TRUE;
}

MUSA_IAT_SYMBOL(FreeEnvironmentStringsW, 4);

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
                    Dst += ValLen - 1;
                    Remaining -= ValLen - 1;
                } else if (Dst) {
                    Remaining = 0;
                }
                TotalLen += ValLen - 1;
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
