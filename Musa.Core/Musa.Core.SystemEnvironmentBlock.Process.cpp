#include "Musa.Core.SystemEnvironmentBlock.Fls.h"
#include "Musa.Core.SystemEnvironmentBlock.Thread.h"
#include "Musa.Core.SystemEnvironmentBlock.Process.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MUSA_NAME_PRIVATE(ProcessEnvironmentBlockSetup))
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(ProcessEnvironmentBlockTeardown))
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(RtlAcquirePebLockExclusive))
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(RtlReleasePebLockExclusive))
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(RtlAcquirePebLockShared))
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(RtlReleasePebLockShared))
#endif

EXTERN_C_START

#if defined(_KERNEL_MODE)

using namespace Musa;
using namespace Musa::Core;

PKPEB MusaCoreProcessEnvironmentBlock = nullptr;

_Must_inspect_result_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS MUSA_API MUSA_NAME_PRIVATE(ProcessEnvironmentBlockSetup)(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    PAGED_CODE();

    NTSTATUS Status;
    PKPEB    Peb = nullptr;
    bool     TebInitialized  = false;
    bool     LockInitialized = false;

    do {
        Peb = static_cast<PKPEB>(ExAllocatePoolZero(NonPagedPool, PAGE_SIZE, MUSA_TAG));
        if (Peb == nullptr) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        Peb->ProcessHeaps         = static_cast<PVOID*>(Add2Ptr(Peb, sizeof(KPEB)));
        Peb->MaximumNumberOfHeaps = (PAGE_SIZE - sizeof(KPEB)) / sizeof(PVOID);

        Status = ExInitializeResourceLite(&Peb->Lock);
        if (!NT_SUCCESS(Status)) {
            break;
        }
        LockInitialized = true;

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

        // Publish only after basic fields are initialized, before TEB setup
        // which needs to access the PEB via RtlGetCurrentPeb()
        InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(&MusaCoreProcessEnvironmentBlock), Peb);

        Status = MUSA_NAME_PRIVATE(ThreadEnvironmentBlockSetup)(DriverObject, RegistryPath);
        if (!NT_SUCCESS(Status)) {
            InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(&MusaCoreProcessEnvironmentBlock), nullptr);
            break;
        }
        TebInitialized = true;

        Peb->DefaultHeap = RtlCreateHeap(HEAP_GROWABLE, nullptr,
            0, 0, nullptr, nullptr);
        if (Peb->DefaultHeap == nullptr) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        MUSA_NAME_PRIVATE(FlsCreate)();
    } while (false);

    if (!NT_SUCCESS(Status) && Peb) {
        if (Peb->DefaultHeap) {
            RtlDestroyHeap(Peb->DefaultHeap);
        }

        if (TebInitialized) {
            (void)MUSA_NAME_PRIVATE(ThreadEnvironmentBlockTeardown)();
        }

        InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(&MusaCoreProcessEnvironmentBlock), nullptr);

        RtlFreeUnicodeString(&Peb->ImageBaseName);
        RtlFreeUnicodeString(&Peb->ImagePathName);
        RtlFreeUnicodeString(&Peb->RegistryPath);

        if (LockInitialized) {
            ExDeleteResourceLite(&Peb->Lock);
        }

        ExFreePoolWithTag(Peb, MUSA_TAG);
    }

    return Status;
}

_Must_inspect_result_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS MUSA_API MUSA_NAME_PRIVATE(ProcessEnvironmentBlockTeardown)()
{
    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    do {
        const auto Peb = static_cast<PKPEB>(MusaCoreProcessEnvironmentBlock);
        if (Peb == nullptr) {
            break;
        }

        ExWaitForRundownProtectionRelease(&Peb->RundownProtect);

        InterlockedExchangePointer(
            reinterpret_cast<PVOID volatile*>(&MusaCoreProcessEnvironmentBlock), nullptr);

        MUSA_NAME_PRIVATE(FlsCleanup)();
        for (auto Idx = static_cast<int>(Peb->MaximumNumberOfHeaps - 1); Idx >= 0; --Idx) {
            if (Peb->ProcessHeaps[Idx]) {
                RtlDestroyHeap(Peb->ProcessHeaps[Idx]);
            }
        }

        Status = MUSA_NAME_PRIVATE(ThreadEnvironmentBlockTeardown)();
        if (!NT_SUCCESS(Status)) {
            break;
        }

        ExDeleteResourceLite(&Peb->Lock);

        RtlFreeUnicodeString(&Peb->RegistryPath);
        RtlFreeUnicodeString(&Peb->ImagePathName);
        RtlFreeUnicodeString(&Peb->ImageBaseName);

        ExFreePoolWithTag(Peb, MUSA_TAG);
    } while (false);

    return Status;
}

_IRQL_requires_max_(DISPATCH_LEVEL)
PVOID MUSA_API MUSA_NAME_PRIVATE(RtlGetCurrentPeb)()
{
    return MusaCoreProcessEnvironmentBlock;
}

_IRQL_raises_(APC_LEVEL)
VOID MUSA_API MUSA_NAME_PRIVATE(RtlAcquirePebLockExclusive)()
{
    PAGED_CODE();
    ExEnterCriticalRegionAndAcquireResourceExclusive(
        &static_cast<PKPEB>(MUSA_NAME_PRIVATE(RtlGetCurrentPeb)())->Lock);
}

_IRQL_requires_(APC_LEVEL)
VOID MUSA_API MUSA_NAME_PRIVATE(RtlReleasePebLockExclusive)()
{
    PAGED_CODE();
    ExReleaseResourceAndLeaveCriticalRegion(
        &static_cast<PKPEB>(MUSA_NAME_PRIVATE(RtlGetCurrentPeb)())->Lock);
}

_IRQL_raises_(APC_LEVEL)
VOID MUSA_API MUSA_NAME_PRIVATE(RtlAcquirePebLockShared)()
{
    PAGED_CODE();
    ExEnterCriticalRegionAndAcquireResourceShared(
        &static_cast<PKPEB>(MUSA_NAME_PRIVATE(RtlGetCurrentPeb)())->Lock);
}

_IRQL_requires_(APC_LEVEL)
VOID MUSA_API MUSA_NAME_PRIVATE(RtlReleasePebLockShared)()
{
    PAGED_CODE();
    ExReleaseResourceAndLeaveCriticalRegion(
        &static_cast<PKPEB>(MUSA_NAME_PRIVATE(RtlGetCurrentPeb)())->Lock);
}

#endif // defined(_KERNEL_MODE)

EXTERN_C_END
