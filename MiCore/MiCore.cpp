#include "MiCore.h"
#include "MiCore.SystemEnvironmentBlock.h"
#include "MiCore.SystemCall.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MiCoreStartup)
#pragma alloc_text(PAGE, MiCoreShutdown)
#endif


EXTERN_C_START
namespace Mi
{

#ifdef _KERNEL_MODE
    NTSTATUS MICORE_API MiCoreStartup(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        UNREFERENCED_PARAMETER(DriverObject);
        UNREFERENCED_PARAMETER(RegistryPath);

        PAGED_CODE()

        NTSTATUS Status;

        do {
            Status = MI_NAME_PRIVATE(SetupEnvironmentBlock)(DriverObject, RegistryPath);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = MI_NAME_PRIVATE(SetupSystemCall)();
            if (!NT_SUCCESS(Status)) {
                break;
            }

        } while (false);

        if (!NT_SUCCESS(Status)) {
            (void)MiCoreShutdown();
        }

        return Status;
    }

    NTSTATUS MICORE_API MiCoreShutdown()
    {
        PAGED_CODE()

        NTSTATUS Status;

        do {
            Status = MI_NAME_PRIVATE(FreeSystemCall)();
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = MI_NAME_PRIVATE(FreeEnvironmentBlock)();
            if (!NT_SUCCESS(Status)) {
                break;
            }

        } while (false);

        return Status;
    }

#else // !_KERNEL_MODE

    NTSTATUS MICORE_API MiCoreStartup()
    {
        NTSTATUS Status;

        do {
            Status = MI_NAME_PRIVATE(SetupEnvironmentBlock)();
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = MI_NAME_PRIVATE(SetupSystemCall)();
            if (!NT_SUCCESS(Status)) {
                break;
            }

        } while (false);

        if (!NT_SUCCESS(Status)) {
            (void)MiCoreShutdown();
        }

        return Status;
    }

    NTSTATUS MICORE_API MiCoreShutdown()
    {
        NTSTATUS Status;

        do {
            Status = MI_NAME_PRIVATE(FreeSystemCall)();
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = MI_NAME_PRIVATE(FreeEnvironmentBlock)();
            if (!NT_SUCCESS(Status)) {
                break;
            }

        } while (false);

        return Status;
    }

#endif // !_KERNEL_MODE

    PVOID MICORE_API MiCoreGetSystemRoutine(
        _In_z_ const char* Name
    )
    {
        return MI_NAME_PRIVATE(GetSystemRoutineAddress)(Name);
    }

    PVOID MICORE_API MiCoreGetSystemRoutineByNameHash(const size_t NameHash)
    {
        return MI_NAME_PRIVATE(GetSystemRoutineAddressByNameHash)(NameHash);
    }

}
EXTERN_C_END
