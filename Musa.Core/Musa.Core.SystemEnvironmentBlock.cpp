#include "Musa.Core.SystemEnvironmentBlock.h"
#include "Musa.Core.SystemEnvironmentBlock.Process.h"
#include "Musa.Core.Utility.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MUSA_NAME_PRIVATE(SetupEnvironmentBlock))
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(FreeEnvironmentBlock))
#endif

EXTERN_C_START
namespace Musa
{
    PVOID MusaCoreHeap   = nullptr;
    PVOID MusaCoreNtBase = nullptr;

#ifdef _KERNEL_MODE
    PDRIVER_OBJECT   MusaCoreDriverObject = nullptr;
    PCALLBACK_OBJECT MusaCoreThreadNotifyCallbackObject = nullptr;

#if MUSA_CORE_USE_THREAD_NOTIFY_CALLBACK
    static void NTAPI ThreadNotifyCallback(
        _In_ HANDLE  ProcessId,
        _In_ HANDLE  ThreadId,
        _In_ BOOLEAN Create
    )
    {
        UNREFERENCED_PARAMETER(ProcessId);

        return ExNotifyCallback(MusaCoreThreadNotifyCallbackObject, ThreadId, reinterpret_cast<PVOID>(Create));
    }
#endif

    _Must_inspect_result_
    _IRQL_requires_max_(APC_LEVEL)
    NTSTATUS MUSA_API MUSA_NAME_PRIVATE(SetupEnvironmentBlock)(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        PAGED_CODE();

        NTSTATUS Status;

        do {
            ExInitializeDriverRuntime (DrvRtPoolNxOptIn);
            InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(&MusaCoreDriverObject), DriverObject);

            constexpr auto KernelName = UNICODE_STRING RTL_CONSTANT_STRING(L"ntoskrnl.exe");
            MusaCoreNtBase = GetLoadedModuleBase(&KernelName);
            if (MusaCoreNtBase == nullptr) {
                Status = STATUS_NOT_FOUND;
                break;
            }

            constexpr auto ObjectAttributes = OBJECT_ATTRIBUTES RTL_CONSTANT_OBJECT_ATTRIBUTES(
                static_cast<PUNICODE_STRING>(nullptr), OBJ_CASE_INSENSITIVE);

            Status = ExCreateCallback(&MusaCoreThreadNotifyCallbackObject,
                const_cast<POBJECT_ATTRIBUTES>(&ObjectAttributes), TRUE, TRUE);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = MUSA_NAME_PRIVATE(SetupProcessEnvironmentBlock)(DriverObject, RegistryPath);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            MusaCoreHeap = RtlCreateHeap(HEAP_GROWABLE | HEAP_ZERO_MEMORY, nullptr,
                0, 0, nullptr, nullptr);
            if (MusaCoreHeap == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

        #if MUSA_CORE_USE_THREAD_NOTIFY_CALLBACK
            if (DriverObject) {
                // Must be linked with the /IntegrityCheck flag
                Status = PsSetCreateThreadNotifyRoutineEx(PsCreateThreadNotifySubsystems, &ThreadNotifyCallback);
                if (!NT_SUCCESS(Status)) {
                    break;
                }
            }
        #endif

        } while (false);

        return Status;
    }

    _Must_inspect_result_
    _IRQL_requires_max_(APC_LEVEL)
    NTSTATUS MUSA_API MUSA_NAME_PRIVATE(FreeEnvironmentBlock)()
    {
        PAGED_CODE()

        NTSTATUS Status;

        do {
        #if MUSA_CORE_USE_THREAD_NOTIFY_CALLBACK
            if (MusaCoreDriverObject) {
                (void)PsRemoveCreateThreadNotifyRoutine(ThreadNotifyCallback);
            }
        #endif

            if (MusaCoreHeap) {
                MusaCoreHeap = RtlDestroyHeap(MusaCoreHeap);
            }

            Status = MUSA_NAME_PRIVATE(FreeProcessEnvironmentBlock)();
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

#else
    _Must_inspect_result_
    _IRQL_requires_max_(APC_LEVEL)
    NTSTATUS MUSA_API MUSA_NAME_PRIVATE(SetupEnvironmentBlock)()
    {
        NTSTATUS Status;

        do {
            constexpr auto NameOfNtdll = UNICODE_STRING RTL_CONSTANT_STRING(L"ntdll.dll");
            Status = LdrGetDllHandleEx(LDR_GET_DLL_HANDLE_EX_UNCHANGED_REFCOUNT, nullptr, nullptr,
                const_cast<PUNICODE_STRING>(&NameOfNtdll), &MusaCoreNtBase);
            if (!NT_SUCCESS(Status)) {
                break;
            }

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
    NTSTATUS MUSA_API MUSA_NAME_PRIVATE(FreeEnvironmentBlock)()
    {
        if (MusaCoreHeap) {
            MusaCoreHeap = RtlDestroyHeap(MusaCoreHeap);
        }

        return STATUS_SUCCESS;
    }
#endif

}
EXTERN_C_END
