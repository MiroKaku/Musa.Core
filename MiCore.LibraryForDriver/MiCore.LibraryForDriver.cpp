#include <Veil/Veil.h>
#include <fltKernel.h>


namespace Main
{
    EXTERN_C NTSTATUS DllInitialize(
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        UNREFERENCED_PARAMETER(RegistryPath);

        return STATUS_SUCCESS;
    }

    EXTERN_C NTSTATUS DllUnload()
    {
        return STATUS_SUCCESS;
    }

    EXTERN_C NTSTATUS DriverEntry(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        UNREFERENCED_PARAMETER(DriverObject);
        UNREFERENCED_PARAMETER(RegistryPath);

        return STATUS_DRIVER_UNABLE_TO_LOAD;
    }

}
