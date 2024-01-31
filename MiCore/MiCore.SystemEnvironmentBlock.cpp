#include "MiCore.SystemEnvironmentBlock.h"
#include "MiCore.SystemEnvironmentBlock.Process.h"
#include "MiCore.Utility.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MI_NAME_PRIVATE(SetupEnvironmentBlock))
#pragma alloc_text(PAGE, MI_NAME_PRIVATE(FreeEnvironmentBlock))
#endif

EXTERN_C_START
namespace Mi
{
    PVOID MiCoreHeap   = nullptr;
    PVOID MiCoreNtBase = nullptr;

#ifdef _KERNEL_MODE
    PDRIVER_OBJECT   MiCoreDriverObject = nullptr;
    PCALLBACK_OBJECT MiCoreThreadNotifyCallbackObject = nullptr;

#ifdef MICORE_FLS_USE_THREAD_NOTIFY_CALLBACK
    static void NTAPI ThreadNotifyCallback(
        _In_ HANDLE  ProcessId,
        _In_ HANDLE  ThreadId,
        _In_ BOOLEAN Create
    )
    {
        UNREFERENCED_PARAMETER(ProcessId);

        return ExNotifyCallback(MiCoreThreadNotifyCallbackObject, ThreadId, reinterpret_cast<PVOID>(Create));
    }
#endif

    _Must_inspect_result_
    _IRQL_requires_max_(APC_LEVEL)
    NTSTATUS MICORE_API MI_NAME_PRIVATE(SetupEnvironmentBlock)(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        PAGED_CODE();

        NTSTATUS Status;

        do {
            ExInitializeDriverRuntime (DrvRtPoolNxOptIn);
            InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(&MiCoreDriverObject), DriverObject);

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

            Status = MI_NAME_PRIVATE(SetupProcessEnvironmentBlock)(DriverObject, RegistryPath);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            MiCoreHeap = RtlCreateHeap(HEAP_GROWABLE | HEAP_ZERO_MEMORY, nullptr,
                0, 0, nullptr, nullptr);
            if (MiCoreHeap == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

        #ifdef MICORE_FLS_USE_THREAD_NOTIFY_CALLBACK
            if (DriverObject) {
                Status = PsSetCreateThreadNotifyRoutineEx(PsCreateThreadNotifySubsystems, ThreadNotifyCallback);
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
    NTSTATUS MICORE_API MI_NAME_PRIVATE(FreeEnvironmentBlock)()
    {
        PAGED_CODE()

        NTSTATUS Status;

        do {
        #ifdef MICORE_FLS_USE_THREAD_NOTIFY_CALLBACK
            if (MiCoreDriverObject) {
                (void)PsRemoveCreateThreadNotifyRoutine(ThreadNotifyCallback);
            }
        #endif

            if (MiCoreHeap) {
                MiCoreHeap = RtlDestroyHeap(MiCoreHeap);
            }

            Status = MI_NAME_PRIVATE(FreeProcessEnvironmentBlock)();
            if (!NT_SUCCESS(Status)) {
                break;
            }

            if (MiCoreThreadNotifyCallbackObject) {
                ObDereferenceObject(MiCoreThreadNotifyCallbackObject);
                MiCoreThreadNotifyCallbackObject = nullptr;
            }

        } while (false);

        return Status;
    }

#else
    _Must_inspect_result_
    _IRQL_requires_max_(APC_LEVEL)
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

    _Must_inspect_result_
    _IRQL_requires_max_(APC_LEVEL)
    NTSTATUS MICORE_API MI_NAME_PRIVATE(FreeEnvironmentBlock)()
    {
        if (MiCoreHeap) {
            MiCoreHeap = RtlDestroyHeap(MiCoreHeap);
        }

        return STATUS_SUCCESS;
    }
#endif

}
EXTERN_C_END
