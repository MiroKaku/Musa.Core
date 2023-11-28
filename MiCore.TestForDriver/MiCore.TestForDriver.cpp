// unnecessary, fix ReSharper's code analysis.
#pragma warning(suppress: 4117)
#define _KERNEL_MODE 1

#include <Veil/Veil.h>
#include <MiCore/MiCore.h>


namespace Main
{
    EXTERN_C VOID DriverUnload(
        _In_ PDRIVER_OBJECT  DriverObject
    )
    {
        UNREFERENCED_PARAMETER(DriverObject);

        (void)MiCoreShutdown();
    }

    EXTERN_C NTSTATUS DriverEntry(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        UNREFERENCED_PARAMETER(DriverObject);
        UNREFERENCED_PARAMETER(RegistryPath);

        NTSTATUS Status;

        do {
            DriverObject->DriverUnload = DriverUnload;

            Status = MiCoreStartup(DriverObject, RegistryPath);
            if (!NT_SUCCESS(Status)) {
                break;
            }

        } while (false);

        return Status;
    }
}
