#include <wdm.h>
#include <src/micore.hpp>


namespace Main
{
    EXTERN_C_START

    _IRQL_requires_max_(PASSIVE_LEVEL)
    NTSYSAPI
    NTSTATUS
    NTAPI
    ZwYieldExecution(
        VOID
    );

    NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
    {
        UNREFERENCED_PARAMETER(RegistryPath);

        DriverObject->DriverUnload = [](PDRIVER_OBJECT)
        {
            (void)MiCoreShutdown();
        };

        NTSTATUS Status = MiCoreStartup(DriverObject);
        if (NT_SUCCESS(Status)) {

            (void)ZwYieldExecution();
        }

        return Status;
    }

    EXTERN_C_END
}
