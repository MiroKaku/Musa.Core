#include "Musa.Core.SystemEnvironmentBlock.h"
#include "Musa.Core.SystemEnvironmentBlock.Process.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MUSA_NAME_PRIVATE(EnvironmentBlockSetup))
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(EnvironmentBlockTeardown))
#endif

EXTERN_C_START

PVOID MusaCoreHeap = nullptr;

#if defined(_KERNEL_MODE)
extern bool MusaCoreUseThreadNotifyCallback;

PDRIVER_OBJECT   MusaCoreDriverObject = nullptr;
PCALLBACK_OBJECT MusaCoreThreadNotifyCallbackObject = nullptr;

static void NTAPI ThreadNotifyCallback(
    _In_ HANDLE  ProcessId,
    _In_ HANDLE  ThreadId,
    _In_ BOOLEAN Create
)
{
    UNREFERENCED_PARAMETER(ProcessId);
    if (MusaCoreThreadNotifyCallbackObject == nullptr) {
        return;
    }
    ExNotifyCallback(MusaCoreThreadNotifyCallbackObject, ThreadId, reinterpret_cast<PVOID>(Create));
}

_Must_inspect_result_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS MUSA_API MUSA_NAME_PRIVATE(EnvironmentBlockSetup)(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    PAGED_CODE();

    NTSTATUS Status;

    do {
        ExInitializeDriverRuntime(DrvRtPoolNxOptIn);
        InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(&MusaCoreDriverObject), DriverObject);

        constexpr auto ObjectAttributes = OBJECT_ATTRIBUTES RTL_CONSTANT_OBJECT_ATTRIBUTES(
             static_cast<PUNICODE_STRING>(nullptr), OBJ_CASE_INSENSITIVE);
        Status = ExCreateCallback(&MusaCoreThreadNotifyCallbackObject,
            const_cast<POBJECT_ATTRIBUTES>(&ObjectAttributes), TRUE, TRUE);
        if (!NT_SUCCESS(Status)) {
            MusaLOG("Failed to create callback object: 0x%X", Status);
            break;
        }

        Status = MUSA_NAME_PRIVATE(ProcessEnvironmentBlockSetup)(DriverObject, RegistryPath);
        if (!NT_SUCCESS(Status)) {
            MusaLOG("Failed to setup process environment block: 0x%X", Status);
            break;
        }

        MusaCoreHeap = RtlCreateHeap(HEAP_GROWABLE, nullptr,
            0, 0, nullptr, nullptr);
        if (MusaCoreHeap == nullptr) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            MusaLOG("Failed to create heap: 0x%X", Status);
            break;
        }

        if (MusaCoreUseThreadNotifyCallback) {
            if (DriverObject) {
                // Must be linked with the /IntegrityCheck flag
                Status = PsSetCreateThreadNotifyRoutineEx(PsCreateThreadNotifySubsystems, &ThreadNotifyCallback);
                if (!NT_SUCCESS(Status)) {
                    MusaLOG("Failed to set thread notify callback: 0x%X", Status);
                    break;
                }
            }
        }
    } while (false);

    return Status;
}

_Must_inspect_result_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS MUSA_API MUSA_NAME_PRIVATE(EnvironmentBlockTeardown)()
{
    PAGED_CODE();

    NTSTATUS Status;

    do {
        if (MusaCoreUseThreadNotifyCallback) {
            if (MusaCoreDriverObject) {
                (void)PsRemoveCreateThreadNotifyRoutine(ThreadNotifyCallback);
            }
        }

        if (MusaCoreHeap) {
            MusaCoreHeap = RtlDestroyHeap(MusaCoreHeap);
        }

        Status = MUSA_NAME_PRIVATE(ProcessEnvironmentBlockTeardown)();
        if (!NT_SUCCESS(Status)) {
            break;
        }

        if (MusaCoreThreadNotifyCallbackObject) {
            ObDereferenceObject(MusaCoreThreadNotifyCallbackObject);
            MusaCoreThreadNotifyCallbackObject = nullptr;
        }
    } while (false);

    return Status;
}
#endif // defined(_KERNEL_MODE)

#if !defined(_KERNEL_MODE)

_Must_inspect_result_
    _IRQL_requires_max_(APC_LEVEL)
    NTSTATUS MUSA_API MUSA_NAME_PRIVATE(EnvironmentBlockSetup)()
{
    NTSTATUS Status = STATUS_SUCCESS;

    do {
        MusaCoreHeap = RtlCreateHeap(HEAP_GROWABLE, nullptr, 0, 0, nullptr, nullptr);
        if (MusaCoreHeap == nullptr) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
    } while (false);

    return Status;
}

_Must_inspect_result_
    _IRQL_requires_max_(APC_LEVEL)
    NTSTATUS MUSA_API MUSA_NAME_PRIVATE(EnvironmentBlockTeardown)()
{
    if (MusaCoreHeap) {
        MusaCoreHeap = RtlDestroyHeap(MusaCoreHeap);
    }
    return STATUS_SUCCESS;
}

#endif // !defined(_KERNEL_MODE)

EXTERN_C_END
