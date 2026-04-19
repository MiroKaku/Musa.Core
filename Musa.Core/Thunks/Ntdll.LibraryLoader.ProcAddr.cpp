#include "Musa.Core/Musa.Utilities.PEParser.h"
#include "Internal/Ntdll.LibraryLoader.h"

using namespace Musa;
using namespace Musa::Utils;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetProcedureAddress))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetProcedureAddressEx))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetProcedureAddressForCaller))
#pragma alloc_text(PAGE, MUSA_NAME(LdrGetKnownDllSectionHandle))
#endif

EXTERN_C_START

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

    return PEParser::ImageEnumerateExports([](
        uint32_t Ordinal, const char* Name, const void* Address, void* Context
    ) -> NTSTATUS
    {
        auto Block = static_cast<ParameterBlock*>(Context);

        if (Block->ProcedureNumber) {
            if (Block->ProcedureNumber == Ordinal) {
                *(Block->ProcedureAddress) = const_cast<void*>(Address);
                return STATUS_SUCCESS;
            }
        } else {
            STRING ProcedureName_A{};
            if (NT_SUCCESS(RtlInitStringEx(&ProcedureName_A, Name))) {
                if (RtlEqualString(Block->ProcedureName, &ProcedureName_A, TRUE)) {
                    *(Block->ProcedureAddress) = const_cast<void*>(Address);
                    return STATUS_SUCCESS;
                }
            }
        }
        return STATUS_CALLBACK_BYPASS;
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
        } else {
            Status = RtlInitUnicodeStringEx(&KnownDllName, L"\\KnownDlls");
        }
        if (!NT_SUCCESS(Status)) {
            break;
        }

        OBJECT_ATTRIBUTES ObjectAttributes;
        InitializeObjectAttributes(&ObjectAttributes, &KnownDllName, OBJ_CASE_INSENSITIVE,
            nullptr, nullptr)

        ObjectAttributes.Attributes |= OBJ_KERNEL_HANDLE;

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

        ObjectAttributes.Attributes |= OBJ_KERNEL_HANDLE;

        Status = ZwOpenSection(Section, SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_EXECUTE,
            &ObjectAttributes);
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

EXTERN_C_END
