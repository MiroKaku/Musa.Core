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
    static decltype(::ZwClose)*                ZwClose;
    static decltype(::ZwOpenDirectoryObject)*  ZwOpenDirectoryObject;
    static decltype(::ZwOpenSection)*          ZwOpenSection;


    namespace /*anonymous namespace*/
    {
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


    static RTL_AVL_TABLE         ZwFuncTableByName;
    static RTL_AVL_TABLE         ZwFuncTableByIndex;
    static NPAGED_LOOKASIDE_LIST ZwTablePool;


    typedef struct _MI_ZWFUN_LIST_ENTRY
    {

        uint32_t    Index;
        size_t      NameHash;
    #if defined(DBG)
        char*       Name;
    #endif
        const void* Address;

    } MI_ZWFUN_LIST_ENTRY, * PMI_ZWFUN_LIST_ENTRY;
    typedef MI_ZWFUN_LIST_ENTRY const* PCMI_ZWFUN_LIST_ENTRY;


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


    NTSTATUS MICORE_API LoadStubs()
    {
        NTSTATUS Status;
        HANDLE   SectionHandle = nullptr;
        PVOID    SectionObject = nullptr;
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


#pragma region "FLS"

//
// FLS
//

namespace Mi::Thunk
{
    RTL_BITMAP            FlsBitmap;
    ULONG                 FlsBitmapBits[MI_FLS_MAXIMUM_AVAILABLE / RTL_BITS_OF(ULONG)];
    PVOID*                FlsCallback;
    LIST_ENTRY            FlsListHead;
    KSPIN_LOCK            FlsListLock;

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

            const auto Entry1 = static_cast<PCMI_THREAD_BLOCK>(First);
            const auto Entry2 = static_cast<PCMI_THREAD_BLOCK>(Second);

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

            const auto Block = reinterpret_cast<PMI_THREAD_BLOCK>(static_cast<uint8_t*>(Buffer) + sizeof(RTL_BALANCED_LINKS));
            if (Block->FlsData) {
                const auto FlsData = Block->FlsData;

                for (auto FlsIndex = 0u; FlsIndex < _countof(FlsData->Slots); ++FlsIndex) {

                    if (RtlAreBitsSet(&FlsBitmap, FlsIndex, 1)) {

                        const auto Callback = static_cast<PFLS_CALLBACK_FUNCTION>(FlsCallback[FlsIndex]);
                        if ((Callback != nullptr) && (FlsData->Slots[FlsIndex])) {
                            (Callback)(FlsData->Slots[FlsIndex]);
                        }

                        FlsData->Slots[FlsIndex] = nullptr;
                    }
                }

                KLOCK_QUEUE_HANDLE LockHandle{};
                KeAcquireInStackQueuedSpinLock(&FlsListLock, &LockHandle);
                __try {
                    RemoveEntryList(&FlsData->Entry);
                }
                __finally {
                    KeReleaseInStackQueuedSpinLock(&LockHandle);
                }

                ExFreePoolWithTag(FlsData, MI_TAG);
            }

            return ExFreeToNPagedLookasideList(&ThreadTablePool, Buffer);
        }
    }


    NTSTATUS MICORE_API InitThreadTable()
    {
        do {
            RtlInitializeBitMap(&FlsBitmap, FlsBitmapBits, RTL_BITS_OF(FlsBitmapBits));

            FlsCallback = static_cast<PVOID*>(ExAllocatePoolZero(NonPagedPool,
                MI_FLS_MAXIMUM_AVAILABLE * sizeof(PVOID), MI_TAG));
            if (FlsCallback == nullptr) {
                return STATUS_NO_MEMORY;
            }

            InitializeListHead  (&FlsListHead);
            KeInitializeSpinLock(&FlsListLock);

            constexpr auto EntrySize = ROUND_TO_SIZE(sizeof(MI_THREAD_BLOCK) + sizeof(RTL_BALANCED_LINKS), sizeof(void*));
            ExInitializeNPagedLookasideList(&ThreadTablePool, nullptr, nullptr,
                POOL_NX_ALLOCATION, EntrySize, MI_TAG, 0);

            RtlInitializeGenericTableAvl(&ThreadTable, &ThreadTableRoutines::Compare,
                &ThreadTableRoutines::Allocate, &ThreadTableRoutines::Free, nullptr);

        } while (false);

        return STATUS_SUCCESS;
    }

    NTSTATUS MICORE_API FreeThreadTable()
    {
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

        if (FlsCallback) {
            ExFreePoolWithTag(FlsCallback, MI_TAG);
        }

        return STATUS_SUCCESS;
    }

    PMI_THREAD_BLOCK MICORE_API GetThreadBlock()
    {
        MI_THREAD_BLOCK* Result;

        MI_THREAD_BLOCK  Block{};
        Block.ThreadId    = PsGetCurrentThreadId();
        Block.UniqueId    = Util::GetUniqueIdViaThread(PsGetCurrentThread());

        bool Exclusive = false;

        auto Irql = ExAcquireSpinLockShared(&ThreadTableLock);
        {
            Result = static_cast<PMI_THREAD_BLOCK>(RtlLookupElementGenericTableAvl(&ThreadTable, &Block));
            if (Result == nullptr || Result->UniqueId != Block.UniqueId) {
                if (!ExTryConvertSharedSpinLockExclusive(&ThreadTableLock)) {
                    ExReleaseSpinLockShared(&ThreadTableLock, Irql);

                    KeMemoryBarrier();

                    Irql = ExAcquireSpinLockExclusive(&ThreadTableLock);
                }

                Exclusive = true;

                if (Result) {
                    RtlDeleteElementGenericTableAvl(&ThreadTable, Result);
                }

                Result = static_cast<PMI_THREAD_BLOCK>(RtlInsertElementGenericTableAvl(
                    &ThreadTable, &Block, sizeof Block, nullptr));
            }
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

