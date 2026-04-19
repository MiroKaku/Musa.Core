#include "Musa.Core/Musa.Utilities.PEParser.h"
#include "Internal/Ntdll.LibraryLoader.h"

using namespace Musa;
using namespace Musa::Utils;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(LdrUnloadDll))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetDllHandle))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetDllHandleEx))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetDllHandleByMapping))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetDllHandleByName))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetDllFullName))
#pragma alloc_text(PAGE, MUSA_NAME(LdrAddRefDll))
#pragma alloc_text(PAGE, MUSA_NAME(LdrLoadDataFile))
#pragma alloc_text(PAGE, MUSA_NAME(LdrUnloadDataFile))
#endif

EXTERN_C_START

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrUnloadDll)(
    _In_ PVOID DllHandle
)
{
    PAGED_CODE();

    NTSTATUS Status = STATUS_DLL_NOT_FOUND;

    /* Lock the list */
    ExEnterCriticalRegionAndAcquireResourceExclusive(PsLoadedModuleResource);
    {
        /* Loop the loaded module list */
        for (LIST_ENTRY const* NextEntry = PsLoadedModuleList->Flink; NextEntry != PsLoadedModuleList;) {
            /* Get the entry */
            const auto LdrEntry = CONTAINING_RECORD(NextEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

            /* Check if it's the module */
            if (DllHandle == LdrEntry->DllBase) {
                /* Found it */

                if (LdrEntry->LoadCount > 1) {
                    LdrEntry->LoadCount -= 1;
                }
                // TODO: if LdrEntry->LoadCount == 1 then Unload

                Status = STATUS_SUCCESS;
                break;
            }

            /* Keep looping */
            NextEntry = NextEntry->Flink;
        }
    }
    /* Release the lock */
    ExReleaseResourceAndLeaveCriticalRegion(PsLoadedModuleResource);

    return Status;
}

MUSA_IAT_SYMBOL(LdrUnloadDll, 4);

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrGetDllHandle)(
    _In_opt_ PWSTR       DllPath,
    _In_opt_ PULONG      DllCharacteristics,
    _In_ PUNICODE_STRING DllName,
    _Out_ PVOID*         DllHandle
)
{
    PAGED_CODE();

    return LdrGetDllHandleEx(LDR_GET_DLL_HANDLE_EX_UNCHANGED_REFCOUNT,
        DllPath, DllCharacteristics, DllName, DllHandle);
}

MUSA_IAT_SYMBOL(LdrGetDllHandle, 16);

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrGetDllHandleEx)(
    _In_ ULONG           Flags,
    _In_opt_ PWSTR       DllPath,
    _In_opt_ PULONG      DllCharacteristics,
    _In_ PUNICODE_STRING DllName,
    _Out_ PVOID*         DllHandle
)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(DllCharacteristics);

    NTSTATUS Status = STATUS_DLL_NOT_FOUND;

    if ((Flags & GET_MODULE_HANDLE_EX_FLAG_PIN) &&
        (Flags & GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT)) {
        return STATUS_INVALID_PARAMETER_1;
    }

    /* Lock the list */
    ExEnterCriticalRegionAndAcquireResourceShared(PsLoadedModuleResource);
    {
        /* Loop the loaded module list */
        for (LIST_ENTRY const* NextEntry = PsLoadedModuleList->Flink; NextEntry != PsLoadedModuleList; NextEntry
             = NextEntry->Flink) {
            /* Get the entry */
            const auto LdrEntry = CONTAINING_RECORD(NextEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

            /* Check if it's the module */
            if (RtlFindUnicodeSubstring(&LdrEntry->FullDllName, DllName, TRUE)) {
                /* Found it */
                if (DllPath != nullptr) {
                    UNICODE_STRING DllPath_U{};
                    Status = RtlInitUnicodeStringEx(&DllPath_U, DllPath);
                    if (!NT_SUCCESS(Status)) {
                        continue;
                    }

                    if (!RtlPrefixUnicodeString(&DllPath_U, &LdrEntry->FullDllName, TRUE)) {
                        continue;
                    }
                }

                *DllHandle = LdrEntry->DllBase;
                Status     = STATUS_SUCCESS;

                if (Flags & LDR_GET_DLL_HANDLE_EX_UNCHANGED_REFCOUNT) {
                    break;
                }

                if (Flags == LDR_ADDREF_DLL_PIN) {
                    LdrEntry->LoadCount = UINT16_MAX;
                } else {
                    LdrEntry->LoadCount += 1;
                }

                break;
            }

            /* Keep looping */
        }
    }
    /* Release the lock */
    ExReleaseResourceAndLeaveCriticalRegion(PsLoadedModuleResource);

    return Status;
}

MUSA_IAT_SYMBOL(LdrGetDllHandleEx, 20);

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrGetDllHandleByMapping)(
    _In_ PVOID   BaseAddress,
    _Out_ PVOID* DllHandle
)
{
    PAGED_CODE();

    if (RtlPcToFileHeader(BaseAddress, DllHandle)) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_DLL_NOT_FOUND;
    }
}

MUSA_IAT_SYMBOL(LdrGetDllHandleByMapping, 8);

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrGetDllHandleByName)(
    _In_opt_ PUNICODE_STRING BaseDllName,
    _In_opt_ PUNICODE_STRING FullDllName,
    _Out_ PVOID*             DllHandle
)
{
    PAGED_CODE();

    NTSTATUS Status = STATUS_DLL_NOT_FOUND;

    if (BaseDllName == nullptr && FullDllName == nullptr) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Lock the list */
    ExEnterCriticalRegionAndAcquireResourceShared(PsLoadedModuleResource);
    {
        /* Loop the loaded module list */
        for (LIST_ENTRY const* NextEntry = PsLoadedModuleList->Flink; NextEntry != PsLoadedModuleList;) {
            /* Get the entry */
            const auto LdrEntry = CONTAINING_RECORD(NextEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

            /* Check if it's the module */
            if (RtlEqualUnicodeString(FullDllName ? FullDllName : BaseDllName,
                &LdrEntry->BaseDllName, TRUE)) {
                /* Found it */
                *DllHandle = LdrEntry->DllBase;

                Status = STATUS_SUCCESS;
                break;
            }

            /* Keep looping */
            NextEntry = NextEntry->Flink;
        }
    }
    /* Release the lock */
    ExReleaseResourceAndLeaveCriticalRegion(PsLoadedModuleResource);

    return Status;
}

MUSA_IAT_SYMBOL(LdrGetDllHandleByName, 12);

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrGetDllFullName)(
    _In_ PVOID            DllHandle,
    _Out_ PUNICODE_STRING FullDllName
)
{
    PAGED_CODE();

    NTSTATUS Status = STATUS_DLL_NOT_FOUND;

    if (FullDllName) {
        *FullDllName = {};
    }

    /* Lock the list */
    ExEnterCriticalRegionAndAcquireResourceShared(PsLoadedModuleResource);
    {
        /* Loop the loaded module list */
        for (LIST_ENTRY const* NextEntry = PsLoadedModuleList->Flink; NextEntry != PsLoadedModuleList;) {
            /* Get the entry */
            const auto LdrEntry = CONTAINING_RECORD(NextEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

            /* Check if it's the module */
            if (DllHandle == LdrEntry->DllBase) {
                /* Found it */
                Status = RtlUnicodeStringCopy(FullDllName, &LdrEntry->FullDllName);
                break;
            }

            /* Keep looping */
            NextEntry = NextEntry->Flink;
        }
    }
    /* Release the lock */
    ExReleaseResourceAndLeaveCriticalRegion(PsLoadedModuleResource);

    return Status;
}

MUSA_IAT_SYMBOL(LdrGetDllFullName, 8);

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrAddRefDll)(
    _In_ ULONG Flags,
    _In_ PVOID DllHandle
)
{
    PAGED_CODE();

    NTSTATUS Status = STATUS_DLL_NOT_FOUND;

    if (Flags & ~LDR_ADDREF_DLL_PIN) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Lock the list */
    ExEnterCriticalRegionAndAcquireResourceExclusive(PsLoadedModuleResource);
    {
        /* Loop the loaded module list */
        for (LIST_ENTRY const* NextEntry = PsLoadedModuleList->Flink; NextEntry != PsLoadedModuleList;) {
            /* Get the entry */
            const auto LdrEntry = CONTAINING_RECORD(NextEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

            /* Check if it's the module */
            if (DllHandle == LdrEntry->DllBase) {
                /* Found it */

                if (LdrEntry->LoadCount == UINT16_MAX) {
                    break;
                }

                if (Flags == LDR_ADDREF_DLL_PIN) {
                    LdrEntry->LoadCount = UINT16_MAX;
                } else {
                    LdrEntry->LoadCount += 1;
                }

                Status = STATUS_SUCCESS;
                break;
            }

            /* Keep looping */
            NextEntry = NextEntry->Flink;
        }
    }
    /* Release the lock */
    ExReleaseResourceAndLeaveCriticalRegion(PsLoadedModuleResource);

    return Status;
}

MUSA_IAT_SYMBOL(LdrAddRefDll, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrLoadDataFile)(
    _In_ PCWSTR  DllName,
    _Out_ PVOID* DllHandle
)
{
    PAGED_CODE();

    NTSTATUS Status;

    HANDLE FileHandle    = nullptr;
    PVOID  SectionObject = nullptr;

    do {
        *DllHandle = nullptr;

        UNICODE_STRING DllName_U{};
        Status = RtlInitUnicodeStringEx(&DllName_U, DllName);
        if (!NT_SUCCESS(Status)) {
            break;
        }

        IO_STATUS_BLOCK   IoStatusBlock{};
        OBJECT_ATTRIBUTES ObjectAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES(
            &DllName_U, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE);

        Status = ZwOpenFile(
            &FileHandle,
            FILE_READ_DATA | SYNCHRONIZE,
            &ObjectAttributes,
            &IoStatusBlock,
            FILE_SHARE_READ | FILE_SHARE_DELETE,
            FILE_NON_DIRECTORY_FILE);
        if (!NT_SUCCESS(Status)) {
            break;
        }

        LARGE_INTEGER MaximumSize{};
        Status = MmCreateSection(&SectionObject, SECTION_MAP_READ, nullptr,
            &MaximumSize, PAGE_READONLY, SEC_IMAGE_NO_EXECUTE, FileHandle, nullptr);
        if (!NT_SUCCESS(Status)) {
            break;
        }

        PVOID  ViewAddress = nullptr;
        SIZE_T ViewSize    = 0;
        Status             = MmMapViewInSystemSpace(SectionObject, &ViewAddress, &ViewSize);
        if (!NT_SUCCESS(Status)) {
            break;
        }

        *DllHandle = LDR_MAPPEDVIEW_TO_DATAFILE(ViewAddress);
    } while (FALSE);

    if (SectionObject) {
        ObDereferenceObject(SectionObject);
    }
    if (FileHandle) {
        (void)ZwClose(FileHandle);
    }

    return Status;
}

MUSA_IAT_SYMBOL(LdrLoadDataFile, 8);

_IRQL_requires_max_(APC_LEVEL)
NTSTATUS NTAPI MUSA_NAME(LdrUnloadDataFile)(
    _In_ PVOID DllHandle
)
{
    PAGED_CODE();

    if (LDR_IS_DATAFILE(DllHandle)) {
        return MmUnmapViewInSystemSpace(LDR_DATAFILE_TO_MAPPEDVIEW(DllHandle));
    }
    return STATUS_DLL_NOT_FOUND;
}

MUSA_IAT_SYMBOL(LdrUnloadDataFile, 4);

EXTERN_C_END
