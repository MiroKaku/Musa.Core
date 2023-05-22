
#if defined(MICORE_DLL)

namespace Main
{
    EXTERN_C_START

    NTSTATUS DllInitialize(
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        UNREFERENCED_PARAMETER(RegistryPath);

        return STATUS_SUCCESS;
    }

    NTSTATUS DllUnload()
    {
        return STATUS_SUCCESS;
    }

    NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
    {
        UNREFERENCED_PARAMETER(DriverObject);
        UNREFERENCED_PARAMETER(RegistryPath);

        return STATUS_DRIVER_UNABLE_TO_LOAD;
    }

    EXTERN_C_END
}

#endif
