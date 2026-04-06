#include "Musa.Core.h"
#include "Musa.Core.SystemEnvironmentBlock.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MusaCoreStartup)
#pragma alloc_text(PAGE, MusaCoreShutdown)
#endif

EXTERN_C_START

#if defined(_KERNEL_MODE)
bool MusaCoreUseThreadNotifyCallback = false;

NTSTATUS MUSA_API MusaCoreStartup(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath,
    _In_ BOOL TLSWithThreadNotifyCallback
)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    PAGED_CODE();

    NTSTATUS Status;

    do {
        MusaCoreUseThreadNotifyCallback = !!TLSWithThreadNotifyCallback;

        Status = MusaCoreLiteStartup();
        if (!NT_SUCCESS(Status)) {
            MusaLOG("MusaCoreLiteStartup failed: 0x%X", Status);
            break;
        }

        Status = MUSA_NAME_PRIVATE(EnvironmentBlockSetup)(DriverObject, RegistryPath);
        if (!NT_SUCCESS(Status)) {
            MusaLOG("SetupEnvironmentBlock failed: 0x%X", Status);
            break;
        }

    } while (false);

    if (!NT_SUCCESS(Status)) {
        (void)MusaCoreShutdown();
    }

    return Status;
}
#endif // defined(_KERNEL_MODE)

#if !defined(_KERNEL_MODE)
NTSTATUS MUSA_API MusaCoreStartup()
{
    NTSTATUS Status;

    do {
        Status = MusaCoreLiteStartup();
        if (!NT_SUCCESS(Status)) {
            MusaLOG("MusaCoreLiteStartup failed: 0x%X", Status);
            break;
        }

        Status = MUSA_NAME_PRIVATE(EnvironmentBlockSetup)();
        if (!NT_SUCCESS(Status)) {
            MusaLOG("SetupEnvironmentBlock failed: 0x%X", Status);
            break;
        }

        MusaLOG("MusaCore initialized successfully");
    } while (false);

    if (!NT_SUCCESS(Status)) {
        (void)MusaCoreShutdown();
    }

    return Status;
}
#endif // !defined(_KERNEL_MODE)


NTSTATUS MUSA_API MusaCoreShutdown()
{
    PAGED_CODE();

    NTSTATUS Status;

    do {
        Status = MUSA_NAME_PRIVATE(EnvironmentBlockTeardown)();
        if (!NT_SUCCESS(Status)) {
            MusaLOG("FreeEnvironmentBlock failed: 0x%X", Status);
            break;
        }

        Status = MusaCoreLiteShutdown();
        if (!NT_SUCCESS(Status)) {
            MusaLOG("MusaCoreLiteShutdown failed: 0x%X", Status);
            break;
        }

        MusaLOG("MusaCore shutdown successfully");
    } while (false);

    return Status;
}

EXTERN_C_END
