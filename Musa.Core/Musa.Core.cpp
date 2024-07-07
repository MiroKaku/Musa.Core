#include "Musa.Core.h"
#include "Musa.Core.SystemEnvironmentBlock.h"
#include "Musa.Core.SystemCall.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MusaCoreStartup)
#pragma alloc_text(PAGE, MusaCoreShutdown)
#endif

EXTERN_C_START

extern PVOID PsSystemDllBase = nullptr;

namespace Musa
{
#ifdef _KERNEL_MODE
    NTSTATUS MUSA_API MusaCoreStartup(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        UNREFERENCED_PARAMETER(DriverObject);
        UNREFERENCED_PARAMETER(RegistryPath);

        PAGED_CODE();

        NTSTATUS Status;

        do {
            Status = MUSA_NAME_PRIVATE(SetupEnvironmentBlock)(DriverObject, RegistryPath);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = MUSA_NAME_PRIVATE(SetupSystemCall)();
            if (!NT_SUCCESS(Status)) {
                break;
            }

        } while (false);

        if (!NT_SUCCESS(Status)) {
            (void)MusaCoreShutdown();
        }

        return Status;
    }

#else // !_KERNEL_MODE

    NTSTATUS MUSA_API MusaCoreStartup()
    {
        NTSTATUS Status;

        do {
            Status = MUSA_NAME_PRIVATE(SetupEnvironmentBlock)();
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = MUSA_NAME_PRIVATE(SetupSystemCall)();
            if (!NT_SUCCESS(Status)) {
                break;
            }

        } while (false);

        if (!NT_SUCCESS(Status)) {
            (void)MusaCoreShutdown();
        }

        return Status;
    }

#endif // !_KERNEL_MODE

    NTSTATUS MUSA_API MusaCoreShutdown()
    {
        PAGED_CODE();

        NTSTATUS Status;

        do {
            Status = MUSA_NAME_PRIVATE(FreeSystemCall)();
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = MUSA_NAME_PRIVATE(FreeEnvironmentBlock)();
            if (!NT_SUCCESS(Status)) {
                break;
            }

        } while (false);

        return Status;
    }

    PVOID MUSA_API MusaCoreGetSystemRoutine(
        _In_z_ const char* Name
    )
    {
        return MUSA_NAME_PRIVATE(GetSystemRoutineAddress)(Name);
    }

    PVOID MUSA_API MusaCoreGetSystemRoutineByNameHash(const size_t NameHash)
    {
        return MUSA_NAME_PRIVATE(GetSystemRoutineAddressByNameHash)(NameHash);
    }

}
EXTERN_C_END
