#include "Musa.Core/Musa.Utilities.PEParser.h"
#include "Internal/Ntdll.Image.h"

using namespace Musa;
using namespace Musa::Utils;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(RtlMapResourceId))
#pragma alloc_text(PAGE, MUSA_NAME(RtlUnmapResourceId))
#pragma alloc_text(PAGE, MUSA_NAME(RtlFindResource))
#pragma alloc_text(PAGE, MUSA_NAME(RtlLoadResource))
#endif

EXTERN_C_START

#if defined(_KERNEL_MODE)

PIMAGE_SECTION_HEADER NTAPI MUSA_NAME(RtlImageRvaToSection)(
    _In_ PIMAGE_NT_HEADERS NtHeaders,
    _In_ PVOID             BaseOfImage,
    _In_ ULONG             Rva
)
{
    UNREFERENCED_PARAMETER(BaseOfImage);

    auto NtSection = IMAGE_FIRST_SECTION(NtHeaders);
    for (auto Idx = 0; Idx < NtHeaders->FileHeader.NumberOfSections; Idx++) {
        if (Rva >= NtSection->VirtualAddress &&
            Rva < NtSection->VirtualAddress + NtSection->SizeOfRawData
        ) {
            return NtSection;
        }
        ++NtSection;
    }

    return nullptr;
}

MUSA_IAT_SYMBOL(RtlImageRvaToSection, 12);

PVOID NTAPI MUSA_NAME(RtlImageRvaToVa)(
    _In_ PIMAGE_NT_HEADERS             NtHeaders,
    _In_ PVOID                         BaseOfImage,
    _In_ ULONG                         Rva,
    _Inout_opt_ PIMAGE_SECTION_HEADER* LastRvaSection
)
{
    PIMAGE_SECTION_HEADER NtSection = nullptr;

    if (LastRvaSection != nullptr) {
        NtSection = *LastRvaSection;
    }

    if ((NtSection == nullptr) ||
        (Rva < NtSection->VirtualAddress) ||
        (Rva >= NtSection->VirtualAddress + NtSection->SizeOfRawData)) {
        NtSection = RtlImageRvaToSection(NtHeaders, BaseOfImage, Rva);
        if (NtSection == nullptr) {
            return nullptr;
        }

        if (LastRvaSection != nullptr) {
            *LastRvaSection = NtSection;
        }
    }

    return Add2Ptr(BaseOfImage, Rva + NtSection->PointerToRawData - NtSection->VirtualAddress);
}

MUSA_IAT_SYMBOL(RtlImageRvaToVa, 16);

//
// Resource
//

#define LDR_RESOURCE_ID_NAME_MASK   ((~(ULONG_PTR)0) << 16) /* lower 16bits clear */
#define LDR_RESOURCE_ID_NAME_MINVAL (( (ULONG_PTR)1) << 16) /* 17th bit set */

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlMapResourceId)(
    _Out_ ULONG_PTR* To,
    _In_ LPCWSTR     From
)
{
    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    __try {
        *To = (ULONG_PTR)-1;

        if ((ULONG_PTR)From >= LDR_RESOURCE_ID_NAME_MINVAL) {
            if (*From == L'#') {
                UNICODE_STRING UnicodeString = {0};
                RtlInitUnicodeString(&UnicodeString, From + 1);

                ULONG Integer = 0ul;
                Status        = RtlUnicodeStringToInteger(&UnicodeString, 10, &Integer);

                #pragma warning(suppress: 26450)
                if (!NT_SUCCESS(Status) || Integer > LDR_RESOURCE_ID_NAME_MASK) {
                    if (NT_SUCCESS(Status)) {
                        Status = STATUS_INVALID_PARAMETER;
                    }
                } else {
                    *To = Integer;
                }
            } else {
                PWSTR String = (PWSTR)RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, (wcslen(From) + 1) * sizeof(WCHAR));
                if (String == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    *To = (ULONG_PTR)String;

                    while (*From != UNICODE_NULL) {
                        *String++ = RtlUpcaseUnicodeChar(*From++);
                    }

                    *String = UNICODE_NULL;
                }
            }
        } else {
            *To = (ULONG_PTR)From;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        *To = (ULONG_PTR)-1;
    }

    return Status;
}

MUSA_IAT_SYMBOL(RtlMapResourceId, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID NTAPI MUSA_NAME(RtlUnmapResourceId)(
    _In_ ULONG_PTR Id
)
{
    PAGED_CODE();

    if (Id >= LDR_RESOURCE_ID_NAME_MINVAL && Id != -1) {
        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)Id);
    }
}

MUSA_IAT_SYMBOL(RtlUnmapResourceId, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlFindResource)(
    _Out_ HRSRC* ResBase,
    _In_ PVOID   DllHandle,
    _In_ LPCWSTR Name,
    _In_ LPCWSTR Type,
    _In_ UINT16  Language
)
{
    PAGED_CODE();

    NTSTATUS  Status;
    ULONG_PTR IdPath[3]{};

    do {
        __try {
            Status = RtlMapResourceId(&IdPath[0], Type);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = RtlMapResourceId(&IdPath[1], Name);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            IdPath[2] = Language;

            Status = LdrFindResource_U(LDR_DATAFILE_TO_MAPPEDVIEW(DllHandle), IdPath,
                _countof(IdPath), reinterpret_cast<PIMAGE_RESOURCE_DATA_ENTRY*>(ResBase));
            if (!NT_SUCCESS(Status)) {
                break;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
        }
    } while (FALSE);

    RtlUnmapResourceId(IdPath[0]);
    RtlUnmapResourceId(IdPath[1]);

    return Status;
}

MUSA_IAT_SYMBOL(RtlFindResource, 20);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlLoadResource)(
    _Out_ PVOID* ResBuff,
    _Out_ ULONG* ResSize,
    _In_ HRSRC   ResBase,
    _In_ PVOID   DllHandle
)
{
    PAGED_CODE();

    NTSTATUS Status;

    __try {
        Status = LdrAccessResource(LDR_DATAFILE_TO_MAPPEDVIEW(DllHandle),
            reinterpret_cast<PIMAGE_RESOURCE_DATA_ENTRY>(ResBase), ResBuff, ResSize);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    return Status;
}

MUSA_IAT_SYMBOL(RtlLoadResource, 16);

#endif // defined(_KERNEL_MODE)

EXTERN_C_END
