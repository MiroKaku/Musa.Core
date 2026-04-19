#include "Musa.Core.SystemEnvironmentBlock.h"
#include "Musa.Core.SystemEnvironmentBlock.Process.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(EnvironmentBlockSetup))
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(EnvironmentBlockTeardown))
#endif

EXTERN_C_START

PVOID MusaCoreHeap = nullptr;

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
    const auto CallbackObject = MusaCoreThreadNotifyCallbackObject;
    if (CallbackObject == nullptr) {
        return;
    }
    ExNotifyCallback(CallbackObject, ThreadId, reinterpret_cast<PVOID>(Create));
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
    bool     ThreadNotifyRegistered = false;

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
                Status = PsSetCreateThreadNotifyRoutineEx(PsCreateThreadNotifySubsystems, &ThreadNotifyCallback);
                if (!NT_SUCCESS(Status)) {
                    MusaLOG("Failed to set thread notify callback: 0x%X", Status);
                    break;
                }
                ThreadNotifyRegistered = true;
            }
        }
    } while (false);

    if (!NT_SUCCESS(Status)) {
        if (ThreadNotifyRegistered) {
            (void)PsRemoveCreateThreadNotifyRoutine(ThreadNotifyCallback);
        }

        if (MusaCoreHeap) {
            MusaCoreHeap = RtlDestroyHeap(MusaCoreHeap);
        }

        const auto CallbackObject = static_cast<PCALLBACK_OBJECT>(InterlockedExchangePointer(
            reinterpret_cast<PVOID volatile*>(&MusaCoreThreadNotifyCallbackObject), nullptr));
        if (CallbackObject) {
            ObDereferenceObject(CallbackObject);
        }
    }

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

        const auto CallbackObject = static_cast<PCALLBACK_OBJECT>(InterlockedExchangePointer(
            reinterpret_cast<PVOID volatile*>(&MusaCoreThreadNotifyCallbackObject), nullptr));

        if (MusaCoreHeap) {
            MusaCoreHeap = RtlDestroyHeap(MusaCoreHeap);
        }

        Status = MUSA_NAME_PRIVATE(ProcessEnvironmentBlockTeardown)();
        if (!NT_SUCCESS(Status)) {
            break;
        }

        if (CallbackObject) {
            ObDereferenceObject(CallbackObject);
        }
    } while (false);

    return Status;
}

EXTERN_C_END
