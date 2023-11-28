#include "MiCore.EnvironmentBlock.h"
#include "MiCore.Internal.h"


namespace Mi
{
    EXTERN_C PVOID   MiCoreHeap   = nullptr;
    EXTERN_C PVOID   MiCoreNtBase = nullptr;
    EXTERN_C LOGICAL MiCoreVerifierEnabled = FALSE;

#ifdef _KERNEL_MODE
    NTSTATUS MICORE_API MI_NAME_PRIVATE(SetupEnvironmentBlock)(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        UNREFERENCED_PARAMETER(RegistryPath);

        NTSTATUS Status = STATUS_SUCCESS;

        do {
            ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

            if (DriverObject) {
                MiCoreVerifierEnabled = MmIsDriverSuspectForVerifier(DriverObject);
            }

            constexpr auto KernelName = UNICODE_STRING RTL_CONSTANT_STRING(L"ntoskrnl.exe");
            MiCoreNtBase = GetLoadedModuleBase(&KernelName);
            if (MiCoreNtBase == nullptr) {
                Status = STATUS_NOT_FOUND;
                break;
            }

        } while (false);

        return Status;
    }

    NTSTATUS MICORE_API MI_NAME_PRIVATE(FreeEnvironmentBlock)()
    {
        return STATUS_SUCCESS;
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
