#ifdef _KERNEL_MODE

#include "MiCore.SystemEnvironmentBlock.Private.h"
#include "MiCore.SystemEnvironmentBlock.Process.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MI_NAME_PRIVATE(SetupProcessEnvironmentBlock))
#pragma alloc_text(PAGE, MI_NAME_PRIVATE(FreeProcessEnvironmentBlock))

#pragma alloc_text(PAGE, MI_NAME_PRIVATE(RtlAcquirePebLockExclusive))
#pragma alloc_text(PAGE, MI_NAME_PRIVATE(RtlReleasePebLockExclusive))
#pragma alloc_text(PAGE, MI_NAME_PRIVATE(RtlAcquirePebLockShared))
#pragma alloc_text(PAGE, MI_NAME_PRIVATE(RtlReleasePebLockShared))
#endif


EXTERN_C_START
namespace Mi
{
    PKPEB MiCoreProcessEnvironmentBlock = nullptr;


    NTSTATUS MICORE_API MI_NAME_PRIVATE(SetupProcessEnvironmentBlock)(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        UNREFERENCED_PARAMETER(RegistryPath);

        PAGED_CODE();

        NTSTATUS Status;

        do {
            const auto Peb = static_cast<PKPEB>(ExAllocatePoolZero(NonPagedPool, PAGE_SIZE, MI_TAG));
            if (Peb == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            InterlockedExchangePointer(
                reinterpret_cast<PVOID volatile*>(&MiCoreProcessEnvironmentBlock), Peb);

            Peb->ProcessHeaps         = static_cast<PVOID*>(Add2Ptr(Peb, sizeof(KPEB)));
            Peb->MaximumNumberOfHeaps = (PAGE_SIZE - sizeof(KPEB)) / sizeof(PVOID);

            ExInitializeRundownProtection(&Peb->RundownProtect);

            InitializeListHead (&Peb->FlsListHead);
            RtlInitializeBitMap(&Peb->FlsBitmap, Peb->FlsBitmapBits, RTL_BITS_OF(Peb->FlsBitmapBits));

            Status = RtlDuplicateUnicodeString(RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
                RegistryPath, &Peb->RegistryPath);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            if (DriverObject) {
                #pragma warning(suppress: 28175)
                const auto LdrEntry = static_cast<PKLDR_DATA_TABLE_ENTRY>(DriverObject->DriverSection);

                Peb->DriverObject     = DriverObject;
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

        } while (false);

        return Status;
    }

    NTSTATUS MICORE_API MI_NAME_PRIVATE(FreeProcessEnvironmentBlock)()
    {
        PAGED_CODE();

        NTSTATUS Status = STATUS_SUCCESS;

        do {
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

        } while (false);

        return Status;
    }

    PKPEB MICORE_API MI_NAME_PRIVATE(RtlGetCurrentPeb)()
    {
        return MiCoreProcessEnvironmentBlock;
    }

    _IRQL_raises_(APC_LEVEL)
    VOID MICORE_API MI_NAME_PRIVATE(RtlAcquirePebLockExclusive)()
    {
        PAGED_CODE();

        ExEnterCriticalRegionAndAcquireResourceExclusive(&MI_NAME_PRIVATE(RtlGetCurrentPeb)()->Lock);
    }

    _IRQL_requires_(APC_LEVEL)
    VOID MICORE_API MI_NAME_PRIVATE(RtlReleasePebLockExclusive)()
    {
        PAGED_CODE();

        ExReleaseResourceAndLeaveCriticalRegion(&MI_NAME_PRIVATE(RtlGetCurrentPeb)()->Lock);
    }

    _IRQL_raises_(APC_LEVEL)
    VOID MICORE_API MI_NAME_PRIVATE(RtlAcquirePebLockShared)()
    {
        PAGED_CODE();

        ExEnterCriticalRegionAndAcquireResourceShared(&MI_NAME_PRIVATE(RtlGetCurrentPeb)()->Lock);
    }

    _IRQL_requires_(APC_LEVEL)
    VOID MICORE_API MI_NAME_PRIVATE(RtlReleasePebLockShared)()
    {
        PAGED_CODE();

        ExReleaseResourceAndLeaveCriticalRegion(&MI_NAME_PRIVATE(RtlGetCurrentPeb)()->Lock);
    }

}
EXTERN_C_END


#endif // _KERNEL_MODE
