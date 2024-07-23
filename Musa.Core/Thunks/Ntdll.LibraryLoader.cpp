#ifdef _KERNEL_MODE

#include "Musa.Core\Musa.Core.PEParser.h"
#include "Ntdll.LibraryLoader.Private.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(LdrUnloadDll))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetDllHandle))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetDllHandleEx))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetDllHandleByMapping))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetDllHandleByName))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetDllFullName))
#pragma alloc_text(PAGE, MUSA_NAME(LdrAddRefDll))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetProcedureAddress))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetProcedureAddressEx))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetProcedureAddressForCaller))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetKnownDllSectionHandle))
#endif


EXTERN_C_START
namespace Musa
{

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

        NTSTATUS Status  = STATUS_DLL_NOT_FOUND;

        if ((Flags & GET_MODULE_HANDLE_EX_FLAG_PIN) &&
            (Flags & GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT)) {
            return STATUS_INVALID_PARAMETER_1;
        }

        /* Lock the list */
        ExEnterCriticalRegionAndAcquireResourceShared(PsLoadedModuleResource);
        {
            /* Loop the loaded module list */
            for (LIST_ENTRY const* NextEntry = PsLoadedModuleList->Flink; NextEntry != PsLoadedModuleList; NextEntry = NextEntry->Flink) {
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
                     Status    = STATUS_SUCCESS;

                    if (Flags & LDR_GET_DLL_HANDLE_EX_UNCHANGED_REFCOUNT) {
                        break;
                    }

                    if (Flags == LDR_ADDREF_DLL_PIN) {
                        LdrEntry->LoadCount = UINT16_MAX;
                    }
                    else {
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
        }
        else {
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
        _In_  PVOID           DllHandle,
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
                        LdrEntry->LoadCount  = UINT16_MAX;
                    }
                    else {
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

    _IRQL_requires_max_(APC_LEVEL)
    NTSTATUS NTAPI MUSA_NAME(LdrGetProcedureAddress)(
        _In_ PVOID            DllHandle,
        _In_opt_ PANSI_STRING ProcedureName,
        _In_opt_ ULONG        ProcedureNumber,
        _Out_ PVOID*          ProcedureAddress
    )
    {
        PAGED_CODE();

        return LdrGetProcedureAddressEx(DllHandle, ProcedureName, ProcedureNumber, ProcedureAddress,
            0);
    }
    MUSA_IAT_SYMBOL(LdrGetProcedureAddress, 16);

    _IRQL_requires_max_(APC_LEVEL)
    NTSTATUS NTAPI MUSA_NAME(LdrGetProcedureAddressEx)(
        _In_ PVOID            DllHandle,
        _In_opt_ PANSI_STRING ProcedureName,
        _In_opt_ ULONG        ProcedureNumber,
        _Out_ PVOID*          ProcedureAddress,
        _In_ ULONG            Flags
    )
    {
        PAGED_CODE();

        return LdrGetProcedureAddressForCaller(DllHandle, ProcedureName, ProcedureNumber, ProcedureAddress,
            Flags, (PVOID*)_ReturnAddress());
    }
    MUSA_IAT_SYMBOL(LdrGetProcedureAddressEx, 20);

    _IRQL_requires_max_(APC_LEVEL)
    NTSTATUS NTAPI MUSA_NAME(LdrGetProcedureAddressForCaller)(
        _In_ PVOID            DllHandle,
        _In_opt_ PANSI_STRING ProcedureName,
        _In_opt_ ULONG        ProcedureNumber,
        _Out_ PVOID*          ProcedureAddress,
        _In_ ULONG            Flags,
        _In_ PVOID*           Callback
    )
    {
        PAGED_CODE();
        UNREFERENCED_PARAMETER(Flags);

        if (ProcedureName == nullptr && ProcedureNumber == 0) {
            return STATUS_INVALID_PARAMETER;
        }

        struct ParameterBlock
        {
            PANSI_STRING ProcedureName;
            ULONG        ProcedureNumber;
            PVOID*       ProcedureAddress;
            PVOID*       Callback;

        } Block;

        Block.ProcedureName    = ProcedureName;
        Block.ProcedureNumber  = ProcedureNumber;
        Block.ProcedureAddress = ProcedureAddress;
        Block.Callback         = Callback; // TODO

        return Musa::PEParser::ImageEnumerateExports([](uint32_t Ordinal, const char* Name, const void* Address, void* Context) ->bool
        {
            auto Block = static_cast<ParameterBlock*>(Context);

            if (Block->ProcedureNumber) {
                if (Block->ProcedureNumber == Ordinal) {
                    *(Block->ProcedureAddress) = const_cast<void*>(Address);
                    return true;
                }
            }
            else {
                STRING ProcedureName_A{};
                if (NT_SUCCESS(RtlInitStringEx(&ProcedureName_A, Name))) {

                    if (RtlEqualString(Block->ProcedureName, &ProcedureName_A, TRUE)) {
                        *(Block->ProcedureAddress) = const_cast<void*>(Address);
                        return true;
                    }
                }
            }

            return false;

        }, &Block, DllHandle, true);
    }
    MUSA_IAT_SYMBOL(LdrGetProcedureAddressForCaller, 24);

    _IRQL_requires_max_(APC_LEVEL)
    NTSTATUS NTAPI MUSA_NAME(LdrGetKnownDllSectionHandle)(
        _In_ PCWSTR   DllName,
        _In_ BOOLEAN  KnownDlls32,
        _Out_ PHANDLE Section
    )
    {
        PAGED_CODE();

        NTSTATUS Status;
        HANDLE   DirectoryHandle = nullptr;

        do {
            UNICODE_STRING KnownDllName;

            if (KnownDlls32) {
                Status = RtlInitUnicodeStringEx(&KnownDllName, L"\\KnownDlls32");
            }
            else{
                Status = RtlInitUnicodeStringEx(&KnownDllName, L"\\KnownDlls");
            }
            if (!NT_SUCCESS(Status)) {
                break;
            }

            OBJECT_ATTRIBUTES ObjectAttributes;
            InitializeObjectAttributes(&ObjectAttributes, &KnownDllName, OBJ_CASE_INSENSITIVE,
                nullptr, nullptr)

            #ifdef _KERNEL_MODE
                ObjectAttributes.Attributes |= OBJ_KERNEL_HANDLE;
            #endif

            Status = ZwOpenDirectoryObject(&DirectoryHandle, DIRECTORY_QUERY, &ObjectAttributes);
            if (!NT_SUCCESS(Status)) {
                if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
                    break;
                }

                Status = RtlInitUnicodeStringEx(&KnownDllName, L"\\KnownDlls");
                if (!NT_SUCCESS(Status)) {
                    break;
                }

                Status = ZwOpenDirectoryObject(&DirectoryHandle, DIRECTORY_QUERY, &ObjectAttributes);
                if (!NT_SUCCESS(Status)) {
                    break;
                }
            }

            UNICODE_STRING SectionName{};
            Status = RtlInitUnicodeStringEx(&SectionName, DllName);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            InitializeObjectAttributes(&ObjectAttributes, &SectionName, OBJ_CASE_INSENSITIVE,
                DirectoryHandle, nullptr)

            #ifdef _KERNEL_MODE
                ObjectAttributes.Attributes |= OBJ_KERNEL_HANDLE;
            #endif

            Status = ZwOpenSection(Section, SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_EXECUTE, &ObjectAttributes);
            if (!NT_SUCCESS(Status)) {
                break;
            }

        } while (false);

        if (DirectoryHandle) {
            (void)ZwClose(DirectoryHandle);
        }

        return Status;
    }
    MUSA_IAT_SYMBOL(LdrGetKnownDllSectionHandle, 12);

}
EXTERN_C_END

#endif
