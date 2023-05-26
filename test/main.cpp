#include <wdm.h>
#include <src/micore.hpp>


namespace Main
{
    EXTERN_C_START

    NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
    {
        UNREFERENCED_PARAMETER(RegistryPath);

        DriverObject->DriverUnload = [](PDRIVER_OBJECT)
        {
            (void)Mi::MiCoreShutdown();
        };

        return Mi::MiCoreStartup();
    }

    EXTERN_C_END
}
