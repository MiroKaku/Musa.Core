#include <miutil.hpp>
#include "thunks.hpp"


#pragma region "Stubs"
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



//
// Stub
//

namespace Mi::Thunk
{
    namespace /*anonymous namespace*/
    {
        decltype(::ZwClose)*                ZwClose;
        decltype(::ZwOpenDirectoryObject)*  ZwOpenDirectoryObject;
        decltype(::ZwOpenSection)*          ZwOpenSection;


        typedef struct _MI_ZWFUN_LIST_ENTRY
        {

            uint32_t    Index;
            size_t      NameHash;
        #if defined(DBG)
            char* Name;
        #endif
            const void* Address;

        } MI_ZWFUN_LIST_ENTRY, * PMI_ZWFUN_LIST_ENTRY;
        typedef MI_ZWFUN_LIST_ENTRY const* PCMI_ZWFUN_LIST_ENTRY;

        RTL_AVL_TABLE         ZwFuncTableByName;
        RTL_AVL_TABLE         ZwFuncTableByIndex;
        NPAGED_LOOKASIDE_LIST ZwTablePool;


        namespace ZwFunTableRoutines
        {
            RTL_GENERIC_COMPARE_RESULTS NTAPI CompareByIndex(
                _In_ RTL_AVL_TABLE* Table,
                _In_ PVOID First,
                _In_ PVOID Second
            )
            {
                UNREFERENCED_PARAMETER(Table);

                const auto Entry1 = static_cast<PCMI_ZWFUN_LIST_ENTRY>(First);
                const auto Entry2 = static_cast<PCMI_ZWFUN_LIST_ENTRY>(Second);

                return Entry1->Index == Entry2->Index
                    ? GenericEqual
                    : (Entry1->Index > Entry2->Index ? GenericGreaterThan : GenericLessThan);
            }

            RTL_GENERIC_COMPARE_RESULTS NTAPI CompareByNameHash(
                _In_ RTL_AVL_TABLE* Table,
                _In_ PVOID First,
                _In_ PVOID Second
            )
            {
                UNREFERENCED_PARAMETER(Table);

                const auto Entry1 = static_cast<PCMI_ZWFUN_LIST_ENTRY>(First);
                const auto Entry2 = static_cast<PCMI_ZWFUN_LIST_ENTRY>(Second);

                return Entry1->NameHash == Entry2->NameHash
                    ? GenericEqual
                    : (Entry1->NameHash > Entry2->NameHash ? GenericGreaterThan : GenericLessThan);
            }

            PVOID NTAPI Allocate(
                _In_ RTL_AVL_TABLE* Table,
                _In_ ULONG Size
            )
            {
                UNREFERENCED_PARAMETER(Table);
                UNREFERENCED_PARAMETER(Size);

                return ExAllocateFromNPagedLookasideList(&ZwTablePool);
            }

            VOID NTAPI Free(
                _In_ RTL_AVL_TABLE* Table,
                _In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Buffer
            )
            {
                UNREFERENCED_PARAMETER(Table);

            #if defined(DBG)
                const auto Entry = reinterpret_cast<PMI_ZWFUN_LIST_ENTRY>(static_cast<uint8_t*>(Buffer) + sizeof(RTL_BALANCED_LINKS));
                if (Entry->Name) {
                    ExFreePoolWithTag(Entry->Name, MI_TAG);
                }
            #endif

                return ExFreeToNPagedLookasideList(&ZwTablePool, Buffer);
            }
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
    }

    NTSTATUS MICORE_API LoadStubs()
    {
        NTSTATUS Status;
        HANDLE   SectionHandle    = nullptr;
        PVOID    SectionObject    = nullptr;
        PVOID    ImageBaseOfNtdll = nullptr;
        SIZE_T   ImageSizeOfNtdll = 0;

        do {
            constexpr auto NameOfZwClose               = UNICODE_STRING RTL_CONSTANT_STRING(L"ZwClose");
            constexpr auto NameOfZwOpenSection         = UNICODE_STRING RTL_CONSTANT_STRING(L"ZwOpenSection");
            constexpr auto NameOfZwOpenDirectoryObject = UNICODE_STRING RTL_CONSTANT_STRING(L"ZwOpenDirectoryObject");

            ZwClose                 = static_cast<decltype(ZwClose)              >(MmGetSystemRoutineAddress(const_cast<PUNICODE_STRING>(&NameOfZwClose)));
            ZwOpenSection           = static_cast<decltype(ZwOpenSection)        >(MmGetSystemRoutineAddress(const_cast<PUNICODE_STRING>(&NameOfZwOpenSection)));
            ZwOpenDirectoryObject   = static_cast<decltype(ZwOpenDirectoryObject)>(MmGetSystemRoutineAddress(const_cast<PUNICODE_STRING>(&NameOfZwOpenDirectoryObject)));

            if (ZwClose               == nullptr ||
                ZwOpenSection         == nullptr ||
                ZwOpenDirectoryObject == nullptr) {

                Status = STATUS_NOT_FOUND;
                break;
            }

            constexpr auto ZwFunEntrySize = ROUND_TO_SIZE(sizeof(MI_ZWFUN_LIST_ENTRY) + sizeof(RTL_BALANCED_LINKS), sizeof(void*));
            ExInitializeNPagedLookasideList(&ZwTablePool, nullptr, nullptr,
                POOL_NX_ALLOCATION, ZwFunEntrySize, MI_TAG, 0);

            RtlInitializeGenericTableAvl(&ZwFuncTableByName, &ZwFunTableRoutines::CompareByNameHash, &ZwFunTableRoutines::Allocate,
                &ZwFunTableRoutines::Free, nullptr);

            RtlInitializeGenericTableAvl(&ZwFuncTableByIndex, &ZwFunTableRoutines::CompareByIndex, &ZwFunTableRoutines::Allocate,
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

            Status = Util::ImageEnumerateExports(ImageBaseOfNtdll, [](const uint32_t Ordinal, const char* Name, const void* Address, void* Context)->bool
                {
                    UNREFERENCED_PARAMETER(Ordinal);

                    auto& Status = *static_cast<NTSTATUS*>(Context);

                    if (Name) {
                        if (Name[0] == 'Z' && Name[1] == 'w') {
                            const auto OpCodeBase = static_cast<const uint8_t*>(Address);
                            const auto NameLength = strlen(Name);

                            MI_ZWFUN_LIST_ENTRY Entry;
                            Entry.NameHash = Util::Fnv1aHash(Name, NameLength);
                            Entry.Address  = Util::FastEncodePointer(static_cast<void*>(nullptr));

                        #if defined(_X86_)
                            // B8 42 00 00 00   mov eax, 42h
                            Entry.Index = *reinterpret_cast<const uint32_t*>(OpCodeBase + 1);
                        #elif defined(_AMD64_)
                            // 4C 8B D1         mov r10, rcx
                            // B8 55 00 00 00   mov eax, 55h
                            Entry.Index = *reinterpret_cast<const uint32_t*>(OpCodeBase + 4);
                        #elif defined(_ARM64_)
                            // A1 0A 00 D4      SVC 0x55 ; imm16(5~20 bit)
                            Entry.Index = (*reinterpret_cast<const uint32_t*>(OpCodeBase) >> 5) & 0xFFFF;
                        #endif

                        #if defined(DBG)
                            Entry.Name = static_cast<char*>(ExAllocatePoolZero(NonPagedPool, NameLength + sizeof(""), MI_TAG));
                            if (Entry.Name == nullptr) {
                                Status = STATUS_INSUFFICIENT_RESOURCES;
                                return true;
                            }
                            strcpy_s(Entry.Name, NameLength + _countof(""), Name);
                        #endif

                            if (!RtlInsertElementGenericTableAvl(&ZwFuncTableByIndex, &Entry, sizeof(MI_ZWFUN_LIST_ENTRY), nullptr)) {
                            #if defined(DBG)
                                ExFreePoolWithTag(Entry.Name, MI_TAG);
                            #endif

                                Status = STATUS_INSUFFICIENT_RESOURCES;
                                return true;
                            }
                        }
                    }
                    return false;

                }, &Status);

            if (!NT_SUCCESS(Status)) {
                break;
            }

            // dump NTOS Zw routines.
            const auto BaseOfZwClose = reinterpret_cast<const uint8_t*>(ZwClose);

        #if defined(_X86_)
            size_t SizeOfZwRoutine = 0;
            constexpr size_t OffsetOfZwIndex = 1;
            constexpr size_t OffsetOfTemplate = 5;

            const auto ZwCodeTemplate = *reinterpret_cast<const uint64_t*>(BaseOfZwClose + OffsetOfTemplate);
            for (size_t Idx = OffsetOfTemplate + 1; Idx < 0x100; ++Idx) {
                if (*reinterpret_cast<const uint64_t*>(BaseOfZwClose + Idx) == ZwCodeTemplate) {
                    SizeOfZwRoutine = Idx - OffsetOfTemplate;
                    break;
                }
            }
        #elif defined(_AMD64_)
            size_t SizeOfZwRoutine = 0;
            size_t OffsetOfZwIndex = 0;
            constexpr size_t OffsetOfTemplate = 0;

            const auto ZwCodeTemplate = *reinterpret_cast<const uint64_t*>(BaseOfZwClose + OffsetOfTemplate);
            for (size_t Idx = OffsetOfTemplate + 1; Idx < 0x100; ++Idx) {
                if (*reinterpret_cast<const uint64_t*>(BaseOfZwClose + Idx) == ZwCodeTemplate) {
                    SizeOfZwRoutine = Idx - OffsetOfTemplate;
                    break;
                }
            }

            for (size_t Idx = 0; Idx < 0x100; ++Idx) {
                if (*(BaseOfZwClose + SizeOfZwRoutine - (Idx + 0)) == 0xE9 &&  // jmp imm32
                    *(BaseOfZwClose + SizeOfZwRoutine - (Idx + 5)) == 0xB8) {  // mov rax, imm32

                    OffsetOfZwIndex = SizeOfZwRoutine - (Idx + 5) + 1;
                    break;
                }
            }
        #elif defined(_ARM64_)
            size_t SizeOfZwRoutine = 0;

            // 10 00 80 D2  0xD2800010   MOV X16, #n  imm16(5~20)
            // 00 00 00 14  0x14000000   B   #offset  imm26(0~25)
            constexpr auto ZwCodeTemplate = 0x14000000D2800010ull;
            constexpr auto ZwCodeTemplateMask = 0xFC000000FFE0001Full;
            for (size_t Idx = 1; Idx < 0x100; ++Idx) {
                if ((*reinterpret_cast<const uint64_t*>(BaseOfZwClose + Idx) & ZwCodeTemplateMask) == ZwCodeTemplate) {
                    SizeOfZwRoutine = Idx;
                    break;
                }
            }
        #endif

            PVOID ImageBaseOfNtos = nullptr;
            RtlPcToFileHeader(const_cast<uint8_t*>(BaseOfZwClose), &ImageBaseOfNtos);

            auto NtHeaderOfNtos = RtlImageNtHeader(ImageBaseOfNtos);
            if (NtHeaderOfNtos == nullptr) {
                Status = STATUS_INVALID_IMAGE_FORMAT;
                break;
            }

            auto SectionOfNtos = RtlImageRvaToSection(NtHeaderOfNtos, ImageBaseOfNtos,
                static_cast<ULONG>(BaseOfZwClose - static_cast<const uint8_t*>(ImageBaseOfNtos)));
            if (SectionOfNtos == nullptr) {
                Status = STATUS_INVALID_IMAGE_FORMAT;
                break;
            }

            const auto CodeSectionSize = SectionOfNtos->Misc.VirtualSize;
            const auto CodeSectionBase = static_cast<const uint8_t*>(ImageBaseOfNtos) + SectionOfNtos->VirtualAddress;

            const uint8_t* FirstZwRoutine = nullptr;
            for (size_t Idx = 0; Idx < CodeSectionSize; ++Idx) {
            #if defined(_X86_) || defined(_AMD64_)
                if (*reinterpret_cast<const uint64_t*>(CodeSectionBase + Idx) == ZwCodeTemplate) {
                    FirstZwRoutine = CodeSectionBase + Idx - OffsetOfTemplate;
                    break;
                }
            #elif defined(_ARM64_)
                if ((*reinterpret_cast<const uint64_t*>(CodeSectionBase + Idx) & ZwCodeTemplateMask) == ZwCodeTemplate) {
                    FirstZwRoutine = CodeSectionBase + Idx;
                    break;
                }
            #endif
            }

            const auto CountOfZwRoutine = RtlNumberGenericTableElementsAvl(&ZwFuncTableByIndex);
            for (size_t Idx = 0; Idx < CountOfZwRoutine; ++Idx) {
                const auto ZwRoutine = FirstZwRoutine + (Idx * SizeOfZwRoutine);

                MI_ZWFUN_LIST_ENTRY Entry{};

            #if defined(_X86_) || defined(_AMD64_)
                if (*reinterpret_cast<const uint64_t*>(ZwRoutine + OffsetOfTemplate) != ZwCodeTemplate) {
                    break;
                }
                Entry.Index = *reinterpret_cast<const uint32_t*>(ZwRoutine + OffsetOfZwIndex);
            #elif defined(_ARM64_)
                if ((*reinterpret_cast<const uint64_t*>(ZwRoutine) & ZwCodeTemplateMask) != ZwCodeTemplate) {
                    break;
                }
                // B0 0A 80 D2      MOV X16, #0x55 ; imm16(5~20 bit)
                Entry.Index = (*reinterpret_cast<const uint32_t*>(ZwRoutine) >> 5) & 0xFFFF;
            #endif

                if (const auto MatchEntry = static_cast<PMI_ZWFUN_LIST_ENTRY>(RtlLookupElementGenericTableAvl(&ZwFuncTableByIndex, &Entry))) {
                    MatchEntry->Address = Mi::Util::FastEncodePointer(ZwRoutine);
                }
            }

            // from ZwFuncTableByIndex copy to ZwFuncTableByName
            for (auto Entry = static_cast<PCMI_ZWFUN_LIST_ENTRY>(RtlEnumerateGenericTableAvl(&ZwFuncTableByIndex, TRUE));
                Entry;
                Entry = static_cast<PCMI_ZWFUN_LIST_ENTRY>(RtlEnumerateGenericTableAvl(&ZwFuncTableByIndex, FALSE))) {

                MI_ZWFUN_LIST_ENTRY NewEntry = *Entry;

            #if defined(DBG)
                if (Entry->Name) {
                    const auto NameLength = strlen(Entry->Name);

                    NewEntry.Name = static_cast<char*>(ExAllocatePoolZero(NonPagedPool, NameLength + sizeof(""), MI_TAG));
                    if (NewEntry.Name == nullptr) {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        return true;
                    }
                    strcpy_s(NewEntry.Name, NameLength + _countof(""), Entry->Name);
                }
            #endif

                if (!RtlInsertElementGenericTableAvl(&ZwFuncTableByName, &NewEntry, sizeof(MI_ZWFUN_LIST_ENTRY), nullptr)) {
                #if defined(DBG)
                    ExFreePoolWithTag(NewEntry.Name, MI_TAG);
                #endif

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    return true;
                }
            }

        #if defined(DBG)
            for (auto Entry = static_cast<PCMI_ZWFUN_LIST_ENTRY>(RtlEnumerateGenericTableAvl(&ZwFuncTableByIndex, TRUE));
                Entry;
                Entry = static_cast<PCMI_ZWFUN_LIST_ENTRY>(RtlEnumerateGenericTableAvl(&ZwFuncTableByIndex, FALSE))) {

                DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL,
                    "0x%04X -> 0x%p, %s \n", Entry->Index, Util::FastDecodePointer(Entry->Address), Entry->Name);
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

    NTSTATUS MICORE_API FreeStubs()
    {
        for (auto Entry = RtlGetElementGenericTableAvl(&ZwFuncTableByName, 0);
            Entry;
            Entry = RtlGetElementGenericTableAvl(&ZwFuncTableByName, 0)) {

            RtlDeleteElementGenericTableAvl(&ZwFuncTableByName, Entry);
        }

        for (auto Entry = RtlGetElementGenericTableAvl(&ZwFuncTableByIndex, 0);
            Entry;
            Entry = RtlGetElementGenericTableAvl(&ZwFuncTableByIndex, 0)) {

            RtlDeleteElementGenericTableAvl(&ZwFuncTableByIndex, Entry);
        }

        ExDeleteNPagedLookasideList(&ZwTablePool);
        return STATUS_SUCCESS;
    }

    PVOID MICORE_API QueryStub(_In_ const size_t NameHash)
    {
        MI_ZWFUN_LIST_ENTRY Entry{};
        Entry.NameHash = NameHash;

        if (const auto MatchEntry = static_cast<PCMI_ZWFUN_LIST_ENTRY>(RtlLookupElementGenericTableAvl(&ZwFuncTableByName, &Entry))) {
            return const_cast<void*>(Util::FastDecodePointer(MatchEntry->Address));
        }

        return nullptr;
    }

    PVOID MICORE_API QueryStub(_In_ const char* Name)
    {
        return QueryStub(Util::Fnv1aHash(Name, strlen(Name)));
    }

}

#pragma endregion


#pragma region "KPEB"

//
// KPEB
//

EXTERN_C_START

extern LARGE_INTEGER  RtlTimeout;
extern UNICODE_STRING NtSystemRoot;

HANDLE StdInHandle;
HANDLE StdOutHandle;
HANDLE StdErrHandle;

EXTERN_C_END

namespace Mi::Thunk
{
    namespace /*anonymous namespace*/
    {
        MI_KPEB* KernelProcessEnvironmentBlock;

        NTSTATUS MICORE_API InitStdio()
        {
            NTSTATUS Status;

            do {
                static const UNICODE_STRING    NullDeviceName   = RTL_CONSTANT_STRING(LR"(\GLOBAL??\NUL)");
                static const OBJECT_ATTRIBUTES ObjectAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES(
                    &NullDeviceName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE);

                IO_STATUS_BLOCK IoStatus{};

                Status = ZwOpenFile(&StdInHandle, FILE_GENERIC_READ, const_cast<POBJECT_ATTRIBUTES>(&ObjectAttributes),
                    &IoStatus, FILE_SHARE_VALID_FLAGS, 0);
                if (!NT_SUCCESS(Status)) {
                    break;
                }

                Status = ZwOpenFile(&StdOutHandle, FILE_GENERIC_WRITE, const_cast<POBJECT_ATTRIBUTES>(&ObjectAttributes),
                    &IoStatus, FILE_SHARE_VALID_FLAGS, 0);
                if (!NT_SUCCESS(Status)) {
                    break;
                }

                Status = ZwOpenFile(&StdErrHandle, FILE_GENERIC_WRITE, const_cast<POBJECT_ATTRIBUTES>(&ObjectAttributes),
                    &IoStatus, FILE_SHARE_VALID_FLAGS, 0);
                if (!NT_SUCCESS(Status)) {
                    break;
                }

            } while (false);

            return Status;
        }

        RTL_AVL_TABLE         ThreadTable;
        EX_SPIN_LOCK          ThreadTableLock;
        NPAGED_LOOKASIDE_LIST ThreadTablePool;

        namespace ThreadTableRoutines
        {
            RTL_GENERIC_COMPARE_RESULTS NTAPI Compare(
                _In_ RTL_AVL_TABLE* Table,
                _In_ PVOID First,
                _In_ PVOID Second
            )
            {
                UNREFERENCED_PARAMETER(Table);

                const auto Entry1 = static_cast<PCMI_KTEB>(First);
                const auto Entry2 = static_cast<PCMI_KTEB>(Second);

                return
                    (Entry1->ThreadId < Entry2->ThreadId) ? GenericLessThan :
                    (Entry1->ThreadId > Entry2->ThreadId) ? GenericGreaterThan : GenericEqual;
            }

            PVOID NTAPI Allocate(
                _In_ RTL_AVL_TABLE* Table,
                _In_ ULONG Size
            )
            {
                UNREFERENCED_PARAMETER(Table);
                UNREFERENCED_PARAMETER(Size);

                return ExAllocateFromNPagedLookasideList(&ThreadTablePool);
            }

            VOID NTAPI Free(
                _In_ RTL_AVL_TABLE* Table,
                _In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Buffer
            )
            {
                UNREFERENCED_PARAMETER(Table);

                return ExFreeToNPagedLookasideList(&ThreadTablePool, Buffer);
            }
        }

        VOID CreateOrDeleteThreadNotifyRoutine(
            _In_ HANDLE  ProcessId,
            _In_ HANDLE  ThreadId,
            _In_ BOOLEAN Create
        )
        {
            if (Create) {
                return;
            }

            MI_FLS_DATA* FlsData = nullptr;

            MI_KTEB Block{};
            Block.ThreadId = ThreadId;
            Block.UniqueId = Util::GetUniqueIdViaClientId({ ProcessId, ThreadId });

            const auto Irql = ExAcquireSpinLockExclusive(&ThreadTableLock);
            {
                const auto Result = static_cast<PMI_KTEB>(RtlLookupElementGenericTableAvl(&ThreadTable, &Block));
                if (Result != nullptr) {
                    NT_ASSERT(Result->UniqueId == Block.UniqueId);

                    FlsData = Result->FlsData;
                    RtlDeleteElementGenericTableAvl(&ThreadTable, Result);
                }
            }
            ExReleaseSpinLockExclusive(&ThreadTableLock, Irql);

            const auto Peb = GetCurrentPeb();

            if (FlsData) {
                ExAcquireFastMutex(&Peb->FastPebLock);
                {
                    RemoveEntryList(&FlsData->Entry);

                    for (auto Idx = 0ul; Idx < Peb->FlsHighIndex; ++Idx) {
                        if (RtlAreBitsSet(&Peb->FlsBitmap, Idx, 1)) {
                            const auto Callback = Peb->FlsCallback[Idx];

                            if ((Callback != nullptr) && (FlsData->Slots[Idx])) {
                                (Callback)(FlsData->Slots[Idx]);
                            }
                            FlsData->Slots[Idx] = nullptr;
                        }
                    }

                    ExFreePoolWithTag(FlsData, MI_TAG);
                }
                ExReleaseFastMutex(&Peb->FastPebLock);
            }
        }

        NTSTATUS MICORE_API InitThreadEnvironmentBlock()
        {
            NTSTATUS Status;

            do {
                constexpr UNICODE_STRING RoutineName1 = RTL_CONSTANT_STRING(L"PsSetCreateThreadNotifyRoutine");
                constexpr UNICODE_STRING RoutineName2 = RTL_CONSTANT_STRING(L"PsSetCreateThreadNotifyRoutineEx");

                constexpr auto EntrySize = ROUND_TO_SIZE(sizeof(MI_KTEB) + sizeof(RTL_BALANCED_LINKS), sizeof(void*));
                ExInitializeNPagedLookasideList(&ThreadTablePool, nullptr, nullptr,
                    POOL_NX_ALLOCATION, EntrySize, MI_TAG, 0);

                RtlInitializeGenericTableAvl(&ThreadTable, &ThreadTableRoutines::Compare,
                    &ThreadTableRoutines::Allocate, &ThreadTableRoutines::Free, nullptr);

                if (const auto PsSetCreateThreadNotifyRoutineEx = static_cast<decltype(::PsSetCreateThreadNotifyRoutineEx)*>(
                    MmGetSystemRoutineAddress(const_cast<PUNICODE_STRING>(&RoutineName2)))) {

                    Status = PsSetCreateThreadNotifyRoutineEx(PsCreateThreadNotifySubsystems, &CreateOrDeleteThreadNotifyRoutine);
                    if (NT_SUCCESS(Status)) {
                        break;
                    }
                }

                if (const auto PsSetCreateThreadNotifyRoutine = static_cast<decltype(::PsSetCreateThreadNotifyRoutine)*>(
                    MmGetSystemRoutineAddress(const_cast<PUNICODE_STRING>(&RoutineName1)))) {

                    Status = PsSetCreateThreadNotifyRoutine(&CreateOrDeleteThreadNotifyRoutine);
                    if (NT_SUCCESS(Status)) {
                        break;
                    }
                }

                Status = STATUS_PROCEDURE_NOT_FOUND;

            } while (false);

            return Status;
        }

        NTSTATUS MICORE_API FreeThreadEnvironmentBlock()
        {
            NTSTATUS Status;

            do {
                constexpr UNICODE_STRING RoutineName = RTL_CONSTANT_STRING(L"PsRemoveCreateThreadNotifyRoutine");

                const auto PsRemoveCreateThreadNotifyRoutine = static_cast<decltype(::PsRemoveCreateThreadNotifyRoutine)*>(
                    MmGetSystemRoutineAddress(const_cast<PUNICODE_STRING>(&RoutineName)));
                if (PsRemoveCreateThreadNotifyRoutine == nullptr) {
                    Status = STATUS_PROCEDURE_NOT_FOUND;
                    break;
                }

                Status = PsRemoveCreateThreadNotifyRoutine(&CreateOrDeleteThreadNotifyRoutine);
                if (!NT_SUCCESS(Status)) {
                    if (Status == STATUS_PROCEDURE_NOT_FOUND) {
                        Status = STATUS_SUCCESS;
                    }
                    else {
                        break;
                    }
                }

                const auto Irql = ExAcquireSpinLockExclusive(&ThreadTableLock);
                {
                    for (auto Entry = RtlGetElementGenericTableAvl(&ThreadTable, 0);
                        Entry;
                        Entry = RtlGetElementGenericTableAvl(&ThreadTable, 0)) {

                        RtlDeleteElementGenericTableAvl(&ThreadTable, Entry);
                    }
                    ExDeleteNPagedLookasideList(&ThreadTablePool);
                }
                ExReleaseSpinLockExclusive(&ThreadTableLock, Irql);

                const auto Peb = GetCurrentPeb();

                ExAcquireFastMutex(&Peb->FastPebLock);
                if (!IsListEmpty(&Peb->FlsListHead)) {
                    PLIST_ENTRY Entry = RemoveHeadList(&Peb->FlsListHead);

                    while (Entry != &Peb->FlsListHead) {
                        const auto FlsData = CONTAINING_RECORD(Entry, MI_FLS_DATA, Entry);

                        for (auto Idx = 0ul; Idx < Peb->FlsHighIndex; ++Idx) {
                            if (RtlAreBitsSet(&Peb->FlsBitmap, Idx, 1)) {
                                const auto Callback = Peb->FlsCallback[Idx];

                                if ((Callback != nullptr) && (FlsData->Slots[Idx])) {
                                    (Callback)(FlsData->Slots[Idx]);
                                }
                                FlsData->Slots[Idx] = nullptr;
                            }
                        }
                        ExFreePoolWithTag(Entry, MI_TAG);

                        Entry = RemoveHeadList(&Peb->FlsListHead);
                    }
                }
                ExReleaseFastMutex(&Peb->FastPebLock);

            } while (false);

            return Status;
        }
    }

    NTSTATUS MICORE_API InitProcessEnvironmentBlock(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        NTSTATUS Status;

        do {
            RtlInitUnicodeString(&NtSystemRoot, RtlGetNtSystemRoot());

            Status = InitStdio();
            if (!NT_SUCCESS(Status)) {
                break;
            }

            const auto Ldr = static_cast<PKLDR_DATA_TABLE_ENTRY>(DriverObject->DriverSection);
            const auto Peb = static_cast<PMI_KPEB>(ExAllocatePoolZero(
                NonPagedPool, PAGE_SIZE, MI_TAG));
            if (KernelProcessEnvironmentBlock == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            ExInitializeFastMutex(&Peb->FastPebLock);

            Peb->ImageBaseAddress     = Ldr->DllBase;
            Peb->SizeOfImage          = Ldr->SizeOfImage;
            Peb->DriverObject         = DriverObject;
            Peb->ProcessHeaps         = reinterpret_cast<PVOID*>(Peb + 1);
            Peb->MaximumNumberOfHeaps = (PAGE_SIZE - sizeof(MI_KPEB)) / sizeof(PVOID);
            Peb->StandardInput        = StdInHandle;
            Peb->StandardOutput       = StdOutHandle;
            Peb->StandardError        = StdErrHandle;

            InitializeListHead (&Peb->FlsListHead);
            RtlInitializeBitMap(&Peb->FlsBitmap, Peb->FlsBitmapBits, RTL_BITS_OF(Peb->FlsBitmapBits));

            Status = RtlDuplicateUnicodeString(RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
                RegistryPath, &Peb->RegistryPath);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = RtlDuplicateUnicodeString(RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
                &Ldr->FullDllName, &Peb->ImagePathName);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            ULONG ImageConfigDataSize   = 0ul;
            const auto ImageConfigData  = static_cast<PIMAGE_LOAD_CONFIG_DIRECTORY>(RtlImageDirectoryEntryToData(
                Peb->ImageBaseAddress, TRUE, IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG, &ImageConfigDataSize));
            if (ImageConfigData) {
                if ((ImageConfigDataSize >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY, CriticalSectionDefaultTimeout)) &&
                    (ImageConfigData->CriticalSectionDefaultTimeout)) {

                    //
                    // Convert from milliseconds to NT time scale (100ns)
                    //

                    RtlTimeout.QuadPart = Int32x32To64(static_cast<LONG>(ImageConfigData->CriticalSectionDefaultTimeout),
                        -10000);
                }
            }

            RTL_SEGMENT_HEAP_PARAMETERS HeapParameters{};
            HeapParameters.Size     = sizeof(RTL_SEGMENT_HEAP_PARAMETERS);
            HeapParameters.Version  = SEGMENT_HEAP_PARAMETERS_VERSION;
            HeapParameters.MemorySource.Flags           = RTL_SEGHEAP_MEM_SOURCE_ANY_NODE;
            HeapParameters.MemorySource.MemoryTypeMask  = MemoryTypeNonPaged;
            HeapParameters.MemorySource.Reserved[1]     = MI_TAG;

            Peb->ProcessHeap = RtlCreateHeap(
                HEAP_SETTABLE_USER_VALUE | HEAP_GROWABLE | HEAP_CLASS_0,
                nullptr,
                0,
                0,
                nullptr,
                reinterpret_cast<PRTL_HEAP_PARAMETERS>(&HeapParameters));
            if (Peb->ProcessHeap == nullptr) {
                Status = STATUS_NO_MEMORY;
                break;
            }

            Status = RtlCreateEnvironment(FALSE, &Peb->Environment);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = RtlSetCurrentDirectory_U(&NtSystemRoot);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(&KernelProcessEnvironmentBlock), Peb);

            //
            // TEB
            //

            Status = InitThreadEnvironmentBlock();
            if (!NT_SUCCESS(Status)) {
                break;
            }

        } while (false);

        return Status;
    }

    NTSTATUS MICORE_API FreeProcessEnvironmentBlock()
    {
        NTSTATUS Status;

        do {
            Status = FreeThreadEnvironmentBlock();
            if (!NT_SUCCESS(Status)) {
                break;
            }

            const auto Peb = static_cast<PMI_KPEB>(InterlockedExchangePointer(
                reinterpret_cast<PVOID volatile*>(&KernelProcessEnvironmentBlock), nullptr));
            if (Peb != nullptr) {

                RtlFreeUnicodeString(&Peb->CurrentDirectory.DosPath);
                if (Peb->CurrentDirectory.Handle) {
                    Status = ::ZwClose(Peb->CurrentDirectory.Handle);
                    if (!NT_SUCCESS(Status)) {
                        break;
                    }
                }

                if (Peb->Environment) {
                    Status = RtlDestroyEnvironment(Peb->Environment);
                    if (!NT_SUCCESS(Status)) {
                        break;
                    }
                }

                for (size_t Idx = 0; Idx < Peb->MaximumNumberOfHeaps; ++Idx) {
                    if (Peb->ProcessHeaps[Idx]) {
                        RtlDestroyHeap(Peb->ProcessHeaps[Idx]);
                    }
                }
                RtlDestroyHeap(reinterpret_cast<PVOID>(reinterpret_cast<size_t>(
                    GetProcessHeap()) | 1));

                RtlFreeUnicodeString(&Peb->RegistryPath);
                RtlFreeUnicodeString(&Peb->ImagePathName);

                ExFreePoolWithTag(Peb, MI_TAG);
            }

            if (StdInHandle) {
                Status = ::ZwClose(StdInHandle);
                if (!NT_SUCCESS(Status)) {
                    break;
                }
            }
            if (StdOutHandle) {
                Status = ::ZwClose(StdOutHandle);
                if (!NT_SUCCESS(Status)) {
                    break;
                }
            }
            if (StdErrHandle) {
                Status = ::ZwClose(StdErrHandle);
                if (!NT_SUCCESS(Status)) {
                    break;
                }
            }

        } while (false);

        return Status;
    }

    PMI_KPEB MICORE_API GetCurrentPeb()
    {
        return KernelProcessEnvironmentBlock;
    }

    PMI_KTEB MICORE_API GetCurrentTeb()
    {
        MI_KTEB* Result;
        MI_KTEB  Block{};
        Block.ThreadId = PsGetCurrentThreadId();
        Block.UniqueId = Util::GetUniqueIdViaClientId(PsGetThreadClientId(PsGetCurrentThread()));
        Block.ProcessEnvironmentBlock = GetCurrentPeb();

        auto Exclusive = false;
        auto Irql = ExAcquireSpinLockShared(&ThreadTableLock);
        {
            Result = static_cast<PMI_KTEB>(RtlLookupElementGenericTableAvl(&ThreadTable, &Block));
            if (Result == nullptr) {
                Exclusive = true;

                if (!ExTryConvertSharedSpinLockExclusive(&ThreadTableLock)) {
                    ExReleaseSpinLockShared(&ThreadTableLock, Irql);

                    KeMemoryBarrier();

                    Irql = ExAcquireSpinLockExclusive(&ThreadTableLock);
                }

                Result = static_cast<PMI_KTEB>(RtlInsertElementGenericTableAvl(
                    &ThreadTable, &Block, sizeof Block, nullptr));
            }

            NT_ASSERT(Result->UniqueId == Block.UniqueId);
        }
        if (Exclusive) {
            ExReleaseSpinLockExclusive(&ThreadTableLock, Irql);
        }
        else {
            ExReleaseSpinLockShared(&ThreadTableLock, Irql);
        }

        return Result;
    }

}

#pragma endregion

