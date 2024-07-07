#ifdef _KERNEL_MODE

#include "Musa.Core.SystemEnvironmentBlock.Private.h"
#include "Musa.Core.SystemEnvironmentBlock.Thread.h"
#include "Musa.Core.SystemEnvironmentBlock.Process.h"
#include "Thunks/Ntdll.FiberLocalStorage.Private.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MUSA_NAME_PRIVATE(SetupProcessEnvironmentBlock))
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(FreeProcessEnvironmentBlock))

#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(RtlAcquirePebLockExclusive))
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(RtlReleasePebLockExclusive))
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(RtlAcquirePebLockShared))
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(RtlReleasePebLockShared))
#endif


EXTERN_C_START
namespace Musa
{
    PKPEB MusaCoreProcessEnvironmentBlock = nullptr;


    _Must_inspect_result_
    _IRQL_requires_max_(APC_LEVEL)
    NTSTATUS MUSA_API MUSA_NAME_PRIVATE(SetupProcessEnvironmentBlock)(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        PAGED_CODE();

        NTSTATUS Status;

        do {
            const auto Peb = static_cast<PKPEB>(ExAllocatePoolZero(NonPagedPool, PAGE_SIZE, MUSA_TAG));
            if (Peb == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            InterlockedExchangePointer(
                reinterpret_cast<PVOID volatile*>(&MusaCoreProcessEnvironmentBlock), Peb);

            Peb->ProcessHeaps         = static_cast<PVOID*>(Add2Ptr(Peb, sizeof(KPEB)));
            Peb->MaximumNumberOfHeaps = (PAGE_SIZE - sizeof(KPEB)) / sizeof(PVOID);

            ExInitializeRundownProtection(&Peb->RundownProtect);

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

            Status = MUSA_NAME_PRIVATE(SetupThreadEnvironmentBlock)(DriverObject, RegistryPath);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Peb->DefaultHeap = RtlCreateHeap(HEAP_GROWABLE, nullptr,
                0, 0, nullptr, nullptr);
            if (Peb->DefaultHeap == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            };

            MUSA_NAME_PRIVATE(RtlFlsCreate)();

        } while (false);

        return Status;
    }

    _Must_inspect_result_
    _IRQL_requires_max_(APC_LEVEL)
    NTSTATUS MUSA_API MUSA_NAME_PRIVATE(FreeProcessEnvironmentBlock)()
    {
        PAGED_CODE();

        NTSTATUS Status = STATUS_SUCCESS;

        do {
            const auto Peb = static_cast<PKPEB>(InterlockedExchangePointer(
                reinterpret_cast<PVOID volatile*>(&MusaCoreProcessEnvironmentBlock), MusaCoreProcessEnvironmentBlock));
            if (Peb == nullptr) {
                break;
            }

            ExWaitForRundownProtectionRelease(&Peb->RundownProtect);

            MUSA_NAME_PRIVATE(RtlFlsCleanup)();

            for (auto Idx = static_cast<int>(Peb->MaximumNumberOfHeaps - 1); Idx >= 0 ; --Idx) {
                if (Peb->ProcessHeaps[Idx]) {
                    RtlDestroyHeap(Peb->ProcessHeaps[Idx]);
                }
            }

            Status = MUSA_NAME_PRIVATE(FreeThreadEnvironmentBlock)();
            if (!NT_SUCCESS(Status)) {
                break;
            }

            RtlFreeUnicodeString(&Peb->RegistryPath);
            RtlFreeUnicodeString(&Peb->ImagePathName);
            RtlFreeUnicodeString(&Peb->ImageBaseName);

            ExFreePoolWithTag(Peb, MUSA_TAG);

            InterlockedExchangePointer(
                reinterpret_cast<PVOID volatile*>(&MusaCoreProcessEnvironmentBlock), nullptr);

        } while (false);

        return Status;
    }

    _IRQL_requires_max_(DISPATCH_LEVEL)
    PKPEB MUSA_API MUSA_NAME_PRIVATE(RtlGetCurrentPeb)()
    {
        return MusaCoreProcessEnvironmentBlock;
    }

    _IRQL_raises_(APC_LEVEL)
    VOID MUSA_API MUSA_NAME_PRIVATE(RtlAcquirePebLockExclusive)()
    {
        PAGED_CODE();

        ExEnterCriticalRegionAndAcquireResourceExclusive(&MUSA_NAME_PRIVATE(RtlGetCurrentPeb)()->Lock);
    }

    _IRQL_requires_(APC_LEVEL)
    VOID MUSA_API MUSA_NAME_PRIVATE(RtlReleasePebLockExclusive)()
    {
        PAGED_CODE();

        ExReleaseResourceAndLeaveCriticalRegion(&MUSA_NAME_PRIVATE(RtlGetCurrentPeb)()->Lock);
    }

    _IRQL_raises_(APC_LEVEL)
    VOID MUSA_API MUSA_NAME_PRIVATE(RtlAcquirePebLockShared)()
    {
        PAGED_CODE();

        ExEnterCriticalRegionAndAcquireResourceShared(&MUSA_NAME_PRIVATE(RtlGetCurrentPeb)()->Lock);
    }

    _IRQL_requires_(APC_LEVEL)
    VOID MUSA_API MUSA_NAME_PRIVATE(RtlReleasePebLockShared)()
    {
        PAGED_CODE();

        ExReleaseResourceAndLeaveCriticalRegion(&MUSA_NAME_PRIVATE(RtlGetCurrentPeb)()->Lock);
    }

}
EXTERN_C_END


#endif // _KERNEL_MODE
