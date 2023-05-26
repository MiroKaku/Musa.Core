#include "universal.h"
#include "micore.hpp"



///////////////////////////////////////////////////////////////////////////////////////////////////////
//                                              X64
//  ntdll.dll
// 
// .text:000000018009F890                                               public ZwCreateFile
// .text:000000018009F890                               ZwCreateFile    proc near
// .text:000000018009F890
// .text:000000018009F890 4C 8B D1                                      mov     r10, rcx
// .text:000000018009F893 B8 55 00 00 00                                mov     eax, 55h        ; Index
//                           ^^ ^^ ^^ ^^
// .text:000000018009F898 F6 04 25 08 03 FE 7F 01                       test    byte ptr ds:7FFE0308h, 1
// .text:000000018009F8A0 75 03                                         jnz     short loc_18009F8A5
// .text:000000018009F8A2 0F 05                                         syscall                 ; Low latency system call
// .text:000000018009F8A4 C3                                            retn
// .text:000000018009F8A5                               loc_18009F8A5:
// .text:000000018009F8A5 CD 2E                                         int     2Eh             ; DOS 2+ internal - EXECUTE COMMAND
// .text:000000018009F8A5                                                                       ; DS:SI -> counted CR-terminated command string
// .text:000000018009F8A7 C3                                            retn
// .text:000000018009F8A7                               ZwCreateFile    endp
//
//  ntoskrnl.exe
// 
// .text:00000001404254B0                                               public ZwCreateFile
// .text:00000001404254B0                               ZwCreateFile    proc near
// .text:00000001404254B0
// .text:00000001404254B0 48 8B C4                                      mov     rax, rsp
// .text:00000001404254B3 FA                                            cli
// .text:00000001404254B4 48 83 EC 10                                   sub     rsp, 10h
// .text:00000001404254B8 50                                            push    rax
// .text:00000001404254B9 9C                                            pushfq
// .text:00000001404254BA 6A 10                                         push    10h
// .text:00000001404254BC 48 8D 05 CD 8A 00 00                          lea     rax, KiServiceLinkage
// .text:00000001404254C3 50                                            push    rax
// .text:00000001404254C4 B8 55 00 00 00                                mov     eax, 55h        ; Index
//                           ^^ ^^ ^^ ^^
// .text:00000001404254C9 E9 B2 7A 01 00                                jmp     KiServiceInternal
// .text:00000001404254CE C3                                            retn
// .text:00000001404254CE                               ZwCreateFile    endp
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
//                                              X86
//  ntdll.dll
// 
// .text:77F04A10                                               public _ZwCreateFile@44
// .text:77F04A10                               _ZwCreateFile@44 proc near
// .text:77F04A10 B8 42 00 00 00                                mov     eax, 42h ; Index
//                   ^^ ^^ ^^ ^^
// .text:77F04A15 BA 00 03 FE 7F                                mov     edx, 7FFE0300h
// .text:77F04A1A FF 12                                         call    dword ptr [edx]
// .text:77F04A1C C2 2C 00                                      retn    2Ch
// .text:77F04A1C                               _ZwCreateFile@44 endp
//
//  ntoskrnl.exe
// 
// .text:00433228                                               public _ZwCreateFile@44
// .text:00433228                               _ZwCreateFile@44 proc near
// .text:00433228 B8 42 00 00 00                                mov     eax, 42h ; Index
//                   ^^ ^^ ^^ ^^
// .text:0043322D 8D 54 24 04                                   lea     edx, [esp+4]
// .text:00433231 9C                                            pushf
// .text:00433232 6A 08                                         push    8
// .text:00433234 E8 65 23 00 00                                call    _KiSystemService
// .text:00433239 C2 2C 00                                      retn    2Ch
// .text:00433239                               _ZwCreateFile@44 endp
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
//                                              ARM64
//  ntdll.dll
//
//
// .text:000000018000F8C0                                       EXPORT ZwCreateFile
// .text:000000018000F8C0                       ZwCreateFile
// .text:000000018000F8C0 A1 0A 00 D4                           SVC             0x55 ; Index
// .text:000000018000F8C4 C0 03 5F D6                           RET
// .text:000000018000F8C4                       ; End of function ZwCreateFile
//
//  ntoskrnl.exe
// 
// .text:0000000140200830                                       EXPORT ZwCreateFile
// .text:0000000140200830                       ZwCreateFile
// .text:0000000140200830 B0 0A 80 D2                           MOV             X16, #0x55 ; Index
// .text:0000000140200834 B3 2D 00 14                           B               KiServiceInternal
// .text:0000000140200834                       ; End of function ZwCreateFile



namespace Mi
{
    static decltype(::ZwClose)*                 ZwClose;
    static decltype(::ZwOpenDirectoryObject)*   ZwOpenDirectoryObject;
    static decltype(::ZwOpenSection)*           ZwOpenSection;

    namespace /*anonymous namespace*/
    {
        constexpr size_t Fnv1aHash(_In_ const char* const Buffer, _In_ const size_t Count)
        {
        #if defined(_WIN64)
            constexpr size_t FnvOffsetBasis = 14695981039346656037ULL;
            constexpr size_t FnvPrime       = 1099511628211ULL;
        #else
            constexpr size_t FnvOffsetBasis = 2166136261U;
            constexpr size_t FnvPrime       = 16777619U;
        #endif

            auto Value = FnvOffsetBasis;
            for (size_t Idx = 0; Idx < Count; ++Idx) {
                Value ^= static_cast<size_t>(Buffer[Idx]);
                Value *= FnvPrime;
            }
            return Value;
        }

        constexpr int MAXIMUM_POINTER_SHIFT = sizeof(uintptr_t) * 8;

        uintptr_t RotatePointerValue(uintptr_t Value, int const Shift) noexcept
        {
        #if _WIN64
            return RotateRight64(Value, Shift);
        #else
            return RotateRight32(Value, Shift);
        #endif
        }

        template <typename T>
        T FastDecodePointer(T const Ptr) noexcept
        {
            return reinterpret_cast<T>(RotatePointerValue(reinterpret_cast<uintptr_t>(Ptr) ^ __security_cookie,
                    __security_cookie % MAXIMUM_POINTER_SHIFT));
        }

        template <typename T>
        T FastEncodePointer(T const Ptr) noexcept
        {
            return reinterpret_cast<T>(RotatePointerValue(reinterpret_cast<uintptr_t>(Ptr),
                MAXIMUM_POINTER_SHIFT - static_cast<int>(__security_cookie % MAXIMUM_POINTER_SHIFT)) ^ __security_cookie);
        }

        NTSTATUS GetKnownDllSectionHandle(
            _In_  PCWSTR  DllName,
            _In_  BOOLEAN KnownDlls32,
            _Out_ PHANDLE SectionHandle
        )
        {
        #ifndef _WIN64
            KnownDlls32 = FALSE;
        #endif

            UNICODE_STRING KnownDllName;
            if (KnownDlls32) {
                RtlInitUnicodeStringEx(&KnownDllName, L"\\KnownDlls32");
            }
            else {
                RtlInitUnicodeStringEx(&KnownDllName, L"\\KnownDlls");
            }

            NTSTATUS Status;
            HANDLE   DirectoryHandle = nullptr;

            do {
                OBJECT_ATTRIBUTES ObjectAttributes;
                InitializeObjectAttributes(&ObjectAttributes, &KnownDllName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                    nullptr, nullptr);

                Status = ZwOpenDirectoryObject(&DirectoryHandle, STANDARD_RIGHTS_REQUIRED, &ObjectAttributes);
                if (!NT_SUCCESS(Status)) {
                    break;
                }

                UNICODE_STRING SectionName{};
                Status = RtlInitUnicodeStringEx(&SectionName, DllName);
                if (!NT_SUCCESS(Status)) {
                    break;
                }

                InitializeObjectAttributes(&ObjectAttributes, &SectionName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                    DirectoryHandle, nullptr);

                Status = ZwOpenSection(SectionHandle, SECTION_MAP_READ | SECTION_QUERY, &ObjectAttributes);
                if (!NT_SUCCESS(Status)) {
                    break;
                }

            } while (false);

            if (DirectoryHandle) {
                (void)ZwClose(DirectoryHandle);
            }

            return Status;
        }

        NTSTATUS ImageEnumerateExports(
            _In_ void* BaseOfImage,
            _In_ bool(CALLBACK *Callback)(uint32_t Ordinal, const char* Name, const void* Address, void* Context),
            _In_opt_ void* Context
        )
        {
            ULONG DataEntrySize   = 0;
            const void* DataEntry = RtlImageDirectoryEntryToData(BaseOfImage, TRUE,
                IMAGE_DIRECTORY_ENTRY_EXPORT, &DataEntrySize);
            if (DataEntry == nullptr) {
                return STATUS_INVALID_IMAGE_FORMAT;
            }

            const auto ExportEntry    = static_cast<PCIMAGE_EXPORT_DIRECTORY>(DataEntry);
            const auto AddressOfNames = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(BaseOfImage) + ExportEntry->AddressOfNames);
            const auto AddressOfFuncs = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(BaseOfImage) + ExportEntry->AddressOfFunctions);
            const auto AddressOfOrdis = reinterpret_cast<uint16_t*>(static_cast<uint8_t*>(BaseOfImage) + ExportEntry->AddressOfNameOrdinals);

            for (uint32_t Ordinal = 0; Ordinal < ExportEntry->NumberOfFunctions; ++Ordinal) {
                if (0 == AddressOfFuncs[Ordinal]) {
                    continue;
                }

                const char*    ExportName    = nullptr;
                const uint32_t ExportOrdinal = Ordinal + ExportEntry->Base;
                const void*    ExportAddress = static_cast<void*>(static_cast<uint8_t*>(BaseOfImage) + AddressOfFuncs[Ordinal]);

                for (uint32_t Idx = 0u; Idx <= Ordinal; ++Idx) {
                    if (Ordinal == AddressOfOrdis[Idx]) {
                        ExportName = reinterpret_cast<const char*>(static_cast<uint8_t*>(BaseOfImage) + AddressOfNames[Idx]);
                        break;
                    }
                }

                if (Callback(ExportOrdinal, ExportName, ExportAddress, Context)) {
                    break;
                }
            }

            return STATUS_SUCCESS;
        }

    }
}

namespace Mi
{
    typedef struct _ZWFUN_LIST_ENTRY{

        uint32_t    CmpMode; // 0: Index; 1: NameHash
        uint32_t    Index;
        size_t      NameHash;
    #if DBG
        char*       Name;
    #endif
        const void* Address;

    } ZWFUN_LIST_ENTRY, * PZWFUN_LIST_ENTRY;

    static RTL_AVL_TABLE         ZwFunTable;
    static NPAGED_LOOKASIDE_LIST ZwFunTablePool;

    namespace ZwFunTableRoutines
    {
        RTL_GENERIC_COMPARE_RESULTS NTAPI Compare(
            _In_ RTL_AVL_TABLE* Table,
            _In_ PVOID First,
            _In_ PVOID Second
        )
        {
            UNREFERENCED_PARAMETER(Table);

            const auto Entry1 = static_cast<PZWFUN_LIST_ENTRY>(First);
            const auto Entry2 = static_cast<PZWFUN_LIST_ENTRY>(Second);

            if (Entry1->CmpMode == 1 || Entry2->CmpMode == 1){

                return Entry1->NameHash == Entry2->NameHash
                    ? GenericEqual
                    : (Entry1->NameHash > Entry2->NameHash ? GenericGreaterThan : GenericLessThan);
            }
            else{

                return Entry1->Index == Entry2->Index
                    ? GenericEqual
                    : (Entry1->Index > Entry2->Index ? GenericGreaterThan : GenericLessThan);
            }
        }

        PVOID NTAPI Allocate(
            _In_ RTL_AVL_TABLE* Table,
            _In_ ULONG Size
        )
        {
            UNREFERENCED_PARAMETER(Table);
            UNREFERENCED_PARAMETER(Size);

            return ExAllocateFromNPagedLookasideList(&ZwFunTablePool);
        }

        VOID NTAPI Free(
            _In_ RTL_AVL_TABLE* Table,
            _In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Buffer
        )
        {
            UNREFERENCED_PARAMETER(Table);

        #if DBG
            auto Entry = reinterpret_cast<PZWFUN_LIST_ENTRY>(static_cast<uint8_t*>(Buffer) + sizeof(RTL_BALANCED_LINKS));
            if (Entry->Name){
                ExFreePoolWithTag(const_cast<char*>(Entry->Name), MI_TAG);
            }
        #endif

            return ExFreeToNPagedLookasideList(&ZwFunTablePool, Buffer);
        }
    }

    EXTERN_C_START

    NTSTATUS MiCoreStartup()
    {
        NTSTATUS Status;
        HANDLE   SectionHandle    = nullptr;
        PVOID    SectionObject    = nullptr;
        PVOID    ImageBaseOfNtdll = nullptr;
        SIZE_T   ImageSizeOfNtdll = 0;

        do {
            ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

            constexpr auto NameOfZwClose                = UNICODE_STRING RTL_CONSTANT_STRING(L"ZwClose");
            constexpr auto NameOfZwOpenSection          = UNICODE_STRING RTL_CONSTANT_STRING(L"ZwOpenSection");
            constexpr auto NameOfZwOpenDirectoryObject  = UNICODE_STRING RTL_CONSTANT_STRING(L"ZwOpenDirectoryObject");

            ZwClose               = static_cast<decltype(ZwClose)>(MmGetSystemRoutineAddress(const_cast<PUNICODE_STRING>(&NameOfZwClose)));
            ZwOpenSection         = static_cast<decltype(ZwOpenSection)>(MmGetSystemRoutineAddress(const_cast<PUNICODE_STRING>(&NameOfZwOpenSection)));
            ZwOpenDirectoryObject = static_cast<decltype(ZwOpenDirectoryObject)>(MmGetSystemRoutineAddress(const_cast<PUNICODE_STRING>(&NameOfZwOpenDirectoryObject)));

            if (ZwClose               == nullptr ||
                ZwOpenSection         == nullptr ||
                ZwOpenDirectoryObject == nullptr) {

                Status = STATUS_NOT_FOUND;
                break;
            }

            constexpr auto ZwFunEntrySize = ROUND_TO_SIZE(sizeof(ZWFUN_LIST_ENTRY) + sizeof(RTL_BALANCED_LINKS), sizeof(void*));
            ExInitializeNPagedLookasideList(&ZwFunTablePool, nullptr, nullptr,
                POOL_NX_ALLOCATION, ZwFunEntrySize, MI_TAG, 0);

            RtlInitializeGenericTableAvl(&ZwFunTable, &ZwFunTableRoutines::Compare, &ZwFunTableRoutines::Allocate,
                &ZwFunTableRoutines::Free, nullptr);

            // dump ntdll exports

            Status = GetKnownDllSectionHandle(L"ntdll.dll", FALSE, &SectionHandle);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = ObReferenceObjectByHandle(SectionHandle, SECTION_MAP_READ | SECTION_QUERY,
                *MmSectionObjectType, KernelMode, &SectionObject, nullptr);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = MmMapViewInSystemSpace(SectionObject, &ImageBaseOfNtdll, &ImageSizeOfNtdll);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = ImageEnumerateExports(ImageBaseOfNtdll, [](uint32_t Ordinal, const char* Name, const void* Address, void* Context)->bool
            {
                UNREFERENCED_PARAMETER(Ordinal);

                auto& Status = *static_cast<NTSTATUS*>(Context);

                if (Name) {
                    if (Name[0] == 'Z' && Name[1] == 'w') {

                        const auto OpCodeBase = static_cast<const uint8_t*>(Address);
                        const auto NameLength = strlen(Name);

                        ZWFUN_LIST_ENTRY Entry{};
                        Entry.CmpMode  = 0;
                        Entry.NameHash = Fnv1aHash(Name, NameLength);

                    #if _AMD64_
                        Entry.Index = *reinterpret_cast<const uint32_t*>(OpCodeBase + 4);
                    #elif _X86_
                        Entry.Index = *reinterpret_cast<const uint32_t*>(OpCodeBase + 1);
                    #else
                    #error Unsupported architecture
                    #endif

                    #if DBG
                        Entry.Name = static_cast<char*>(ExAllocatePoolZero(NonPagedPool, NameLength + sizeof(""), MI_TAG));
                        if (Entry.Name == nullptr){
                            Status = STATUS_INSUFFICIENT_RESOURCES;
                            return false;
                        }
                        strcpy_s(Entry.Name, NameLength + _countof(""), Name);
                    #endif

                        if (!RtlInsertElementGenericTableAvl(&ZwFunTable, &Entry, sizeof(ZWFUN_LIST_ENTRY), nullptr)) {
                        #if DBG
                            ExFreePoolWithTag(Entry.Name, MI_TAG);
                        #endif

                            Status = STATUS_INSUFFICIENT_RESOURCES;
                            return false;
                        }
                    }
                }
                return false;

            }, &Status);

            if (!NT_SUCCESS(Status)) {
                break;
            }

            // dump ntoskrnl Zw routines.

            constexpr auto NameOfZwCreateFile = UNICODE_STRING RTL_CONSTANT_STRING(L"ZwCreateFile");
            const auto AddressOfZwCreateFile  = static_cast<const uint8_t*>(MmGetSystemRoutineAddress(const_cast<PUNICODE_STRING>(&NameOfZwCreateFile)));

            const auto ZwCodeTemplate = *reinterpret_cast<const uint64_t*>(AddressOfZwCreateFile);

            size_t   SizeOfZwRoutine      = 0;
            size_t   IdxOffsetOfZwRoutine = 0;

            for (size_t Idx = 1; Idx < 0x100; ++Idx) {
                if (*reinterpret_cast<const uint64_t*>(AddressOfZwCreateFile + Idx) == ZwCodeTemplate) {
                    SizeOfZwRoutine = Idx;
                    break;
                }
            }

        #if _AMD64_
            for (size_t Idx = 0; Idx < 0x100; ++Idx) {
                if (*(AddressOfZwCreateFile + SizeOfZwRoutine - (Idx + 0)) == 0xE9 &&  // jmp imm32
                    *(AddressOfZwCreateFile + SizeOfZwRoutine - (Idx + 5)) == 0xB8) {  // mov rax, imm32
                    IdxOffsetOfZwRoutine = SizeOfZwRoutine - (Idx + 5) + 1;
                    break;
                }
            }
        #elif _X86_
            ZwIndexOffset = 1;
        #else
        #error Unsupported architecture
        #endif

            PVOID ImageBaseOfNtos = nullptr;
            RtlPcToFileHeader(const_cast<uint8_t*>(AddressOfZwCreateFile), &ImageBaseOfNtos);

            auto NtHeaderOfNtos = RtlImageNtHeader(ImageBaseOfNtos);
            if (NtHeaderOfNtos == nullptr){
                Status = STATUS_INVALID_IMAGE_FORMAT;
                break;
            }

            auto SectionOfNtos = RtlImageRvaToSection(NtHeaderOfNtos, ImageBaseOfNtos,
                static_cast<ULONG>(AddressOfZwCreateFile - static_cast<const uint8_t*>(ImageBaseOfNtos)));
            if (SectionOfNtos == nullptr) {
                Status = STATUS_INVALID_IMAGE_FORMAT;
                break;
            }

            const auto CodeSectionSize = SectionOfNtos->Misc.VirtualSize;
            const auto CodeSectionBase = static_cast<const uint8_t*>(ImageBaseOfNtos) + SectionOfNtos->VirtualAddress;

            const uint8_t* FirstZwRoutine = nullptr;
            for (size_t Idx = 0; Idx < CodeSectionSize; ++Idx) {
                if (*reinterpret_cast<const uint64_t*>(CodeSectionBase + Idx) == ZwCodeTemplate) {
                    FirstZwRoutine = CodeSectionBase + Idx;
                    break;
                }
            }

            auto CountOfZwRoutine = RtlNumberGenericTableElementsAvl(&ZwFunTable);
            for (size_t Idx = 0; Idx < CountOfZwRoutine; ++Idx) {
                const auto ZwRoutine = FirstZwRoutine + (Idx * SizeOfZwRoutine);

                ZWFUN_LIST_ENTRY Entry{};
                Entry.CmpMode = 0;
                Entry.Index   = *reinterpret_cast<const uint32_t*>(ZwRoutine + IdxOffsetOfZwRoutine);

                auto MatchEntry = static_cast<PZWFUN_LIST_ENTRY>(RtlLookupElementGenericTableAvl(&ZwFunTable, &Entry));
                if (MatchEntry) {
                    MatchEntry->Address = FastEncodePointer(ZwRoutine);
                }
            }

        #if DBG
            for (auto Entry = static_cast<const ZWFUN_LIST_ENTRY*>(RtlEnumerateGenericTableAvl(&ZwFunTable, TRUE));
                Entry;
                Entry = static_cast<const ZWFUN_LIST_ENTRY*>(RtlEnumerateGenericTableAvl(&ZwFunTable, FALSE))) {

                DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL,
                    "0x%04X -> 0x%p, %s \n", Entry->Index, Entry->Address, Entry->Name);
            }
        #endif

        } while (false);

        if (ImageBaseOfNtdll) {
            (void)MmUnmapViewInSystemSpace(ImageBaseOfNtdll);
        }

        if (SectionObject) {
            ObDereferenceObject(SectionObject);
        }

        if (SectionHandle) {
            (void)ZwClose(SectionHandle);
        }

        return Status;
    }

    NTSTATUS MiCoreShutdown()
    {
        auto Entry = RtlGetElementGenericTableAvl(&ZwFunTable, 0);
        while (Entry) {
            RtlDeleteElementGenericTableAvl(&ZwFunTable, Entry);
            Entry = RtlGetElementGenericTableAvl(&ZwFunTable, 0);
        }

        ExDeleteNPagedLookasideList(&ZwFunTablePool);
        return STATUS_SUCCESS;
    }

    EXTERN_C_END

}
