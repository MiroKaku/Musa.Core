#include "MiCore.SystemEnvironmentBlock.h"
#include "MiCore.SystemEnvironmentBlock.Private.h"
#include "MiCore.Utility.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MI_NAME_PRIVATE(SetupEnvironmentBlock))
#pragma alloc_text(PAGE, MI_NAME_PRIVATE(FreeEnvironmentBlock))
#pragma alloc_text(PAGE, MI_NAME_PRIVATE(RtlAcquirePebLock))
#pragma alloc_text(PAGE, MI_NAME_PRIVATE(RtlReleasePebLock))
#pragma alloc_text(PAGE, MI_NAME_PRIVATE(RtlTryAcquirePebLock))
#endif


EXTERN_C_START
namespace Mi
{
    PVOID   MiCoreHeap   = nullptr;
    PVOID   MiCoreNtBase = nullptr;

#ifdef _KERNEL_MODE
    LOGICAL          MiCoreVerifierEnabled            = FALSE;
    PKPEB            MiCoreProcessEnvironmentBlock    = nullptr;
    PDRIVER_OBJECT   MiCoreDriverObject               = nullptr;
    PCALLBACK_OBJECT MiCoreThreadNotifyCallbackObject = nullptr;

    static void NTAPI ThreadNotifyCallback(
        _In_ HANDLE  ProcessId,
        _In_ HANDLE  ThreadId,
        _In_ BOOLEAN Create
    )
    {
        UNREFERENCED_PARAMETER(ProcessId);

        return ExNotifyCallback(MiCoreThreadNotifyCallbackObject, ThreadId, reinterpret_cast<PVOID>(Create));
    }

    NTSTATUS MICORE_API MI_NAME_PRIVATE(SetupEnvironmentBlock)(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        UNREFERENCED_PARAMETER(RegistryPath);

        PAGED_CODE()

        NTSTATUS Status;

        do {
            ExInitializeDriverRuntime (DrvRtPoolNxOptIn);
            InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(&MiCoreDriverObject), DriverObject);

            if (MiCoreDriverObject) {
                MiCoreVerifierEnabled = MmIsDriverSuspectForVerifier(MiCoreDriverObject);
            }

            constexpr auto KernelName = UNICODE_STRING RTL_CONSTANT_STRING(L"ntoskrnl.exe");
            MiCoreNtBase = GetLoadedModuleBase(&KernelName);
            if (MiCoreNtBase == nullptr) {
                Status = STATUS_NOT_FOUND;
                break;
            }

            constexpr auto ObjectAttributes = OBJECT_ATTRIBUTES RTL_CONSTANT_OBJECT_ATTRIBUTES(
                static_cast<PUNICODE_STRING>(nullptr), OBJ_CASE_INSENSITIVE);

            Status = ExCreateCallback(&MiCoreThreadNotifyCallbackObject,
                const_cast<POBJECT_ATTRIBUTES>(&ObjectAttributes), TRUE, TRUE);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            if (MiCoreDriverObject) {
                Status = PsSetCreateThreadNotifyRoutineEx(PsCreateThreadNotifySubsystems, ThreadNotifyCallback);
                if (!NT_SUCCESS(Status)) {
                    break;
                }
            }

            const auto Peb = static_cast<PKPEB>(ExAllocatePoolZero(NonPagedPool, PAGE_SIZE, MI_TAG));
            if (Peb == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            InterlockedExchangePointer(
                reinterpret_cast<PVOID volatile*>(&MiCoreProcessEnvironmentBlock), Peb);

            Peb->ProcessHeaps         = static_cast<PVOID*>(Add2Ptr(Peb, sizeof(KPEB)));
            Peb->MaximumNumberOfHeaps = (PAGE_SIZE - sizeof(KPEB)) / sizeof(PVOID);

            ExInitializeFastMutex(&Peb->Lock);
            ExInitializeRundownProtection(&Peb->RundownProtect);

            Status = RtlDuplicateUnicodeString(RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
                RegistryPath, &Peb->RegistryPath);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            if (MiCoreDriverObject) {
                #pragma warning(suppress: 28175)
                const auto LdrEntry = static_cast<PKLDR_DATA_TABLE_ENTRY>(MiCoreDriverObject->DriverSection);

                Peb->DriverObject     = MiCoreDriverObject;
                Peb->SizeOfImage      = LdrEntry->SizeOfImage;
                Peb->ImageBaseAddress = LdrEntry->DllBase;

                Status = RtlDuplicateUnicodeString(RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
                    &LdrEntry->FullDllName, &Peb->ImagePathName);
                if (!NT_SUCCESS(Status)) {
                    break;
                }

                Status = RtlDuplicateUnicodeString(RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
                    &LdrEntry->FullDllName, &Peb->ImageBaseName);
                if (!NT_SUCCESS(Status)) {
                    break;
                }
            }

            Peb->DefaultHeap = RtlCreateHeap(HEAP_GROWABLE, nullptr,
                0, 0, nullptr, nullptr);
            if (Peb->DefaultHeap == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            };

            //Status = RtlCreateEnvironment(FALSE, &Peb->Environment);
            //if (!NT_SUCCESS(Status)) {
            //    break;
            //}

            //UNICODE_STRING SystemRoot{};
            //RtlInitUnicodeString(&SystemRoot, RtlGetNtSystemRoot());
            //
            //Status = RtlSetCurrentDirectory_U(&SystemRoot);
            //if (!NT_SUCCESS(Status)) {
            //    break;
            //}

            MiCoreHeap = RtlCreateHeap(HEAP_GROWABLE | HEAP_ZERO_MEMORY, nullptr,
                0, 0, nullptr, nullptr);
            if (MiCoreHeap == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

        } while (false);

        return Status;
    }

    NTSTATUS MICORE_API MI_NAME_PRIVATE(FreeEnvironmentBlock)()
    {
        PAGED_CODE()

        NTSTATUS Status = STATUS_SUCCESS;

        do {
            if (MiCoreHeap) {
                MiCoreHeap = RtlDestroyHeap(MiCoreHeap);
            }

            const auto Peb = static_cast<PKPEB>(InterlockedExchangePointer(
                reinterpret_cast<PVOID volatile*>(&MiCoreProcessEnvironmentBlock), MiCoreProcessEnvironmentBlock));
            if (Peb == nullptr) {
                break;
            }

            ExWaitForRundownProtectionRelease(&Peb->RundownProtect);

            //if (Peb->Environment) {
            //    Status = RtlDestroyEnvironment(Peb->Environment);
            //    if (!NT_SUCCESS(Status)) {
            //        break;
            //    }
            //}

            //Status = RtlSetCurrentDirectory_U(nullptr);
            //if (!NT_SUCCESS(Status)) {
            //    break;
            //}

            RtlFreeUnicodeString(&Peb->RegistryPath);
            RtlFreeUnicodeString(&Peb->ImagePathName);
            RtlFreeUnicodeString(&Peb->ImageBaseName);

            for (auto Idx = static_cast<int>(Peb->MaximumNumberOfHeaps - 1); Idx >= 0 ; --Idx) {
                if (Peb->ProcessHeaps[Idx]) {
                    RtlDestroyHeap(Peb->ProcessHeaps[Idx]);
                }
            }

            ExFreePoolWithTag(Peb, MI_TAG);

            InterlockedExchangePointer(
                reinterpret_cast<PVOID volatile*>(&MiCoreProcessEnvironmentBlock), nullptr);

            if (MiCoreDriverObject) {
                (void)PsRemoveCreateThreadNotifyRoutine(ThreadNotifyCallback);
            }

            if (MiCoreThreadNotifyCallbackObject) {
                ObDereferenceObject(MiCoreThreadNotifyCallbackObject);
                MiCoreThreadNotifyCallbackObject = nullptr;
            }

        } while (false);

        return Status;
    }

    PKPEB MICORE_API MI_NAME_PRIVATE(RtlGetCurrentPeb)()
    {
        return MiCoreProcessEnvironmentBlock;
    }

    _IRQL_raises_(APC_LEVEL)
    VOID MICORE_API MI_NAME_PRIVATE(RtlAcquirePebLock)()
    {
        PAGED_CODE()

        ExAcquireFastMutex(&MI_NAME_PRIVATE(RtlGetCurrentPeb)()->Lock);
    }

    _IRQL_requires_(APC_LEVEL)
    VOID MICORE_API MI_NAME_PRIVATE(RtlReleasePebLock)()
    {
        PAGED_CODE()

        ExReleaseFastMutex(&MI_NAME_PRIVATE(RtlGetCurrentPeb)()->Lock);
    }

    _Must_inspect_result_
    _Success_(return != FALSE)
    _IRQL_raises_(APC_LEVEL)
    BOOLEAN MICORE_API MI_NAME_PRIVATE(RtlTryAcquirePebLock)()
    {
        PAGED_CODE()

        return ExTryToAcquireFastMutex(&MI_NAME_PRIVATE(RtlGetCurrentPeb)()->Lock);
    }

#else
    NTSTATUS MICORE_API MI_NAME_PRIVATE(SetupEnvironmentBlock)()
    {
        NTSTATUS Status;

        do {
            constexpr auto NameOfNtdll = UNICODE_STRING RTL_CONSTANT_STRING(L"ntdll.dll");
            Status = LdrGetDllHandleEx(LDR_GET_DLL_HANDLE_EX_UNCHANGED_REFCOUNT, nullptr, nullptr,
                const_cast<PUNICODE_STRING>(&NameOfNtdll), &MiCoreNtBase);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            MiCoreHeap = RtlCreateHeap(HEAP_GROWABLE, nullptr, 0, 0, nullptr, nullptr);
            if (MiCoreHeap == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

        } while (false);

        return Status;
    }

    NTSTATUS MICORE_API MI_NAME_PRIVATE(FreeEnvironmentBlock)()
    {
        if (MiCoreHeap) {
            RtlDestroyHeap(MiCoreHeap);

            MiCoreHeap = nullptr;
        }

        return STATUS_SUCCESS;
    }
#endif

}
EXTERN_C_END
