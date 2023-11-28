#pragma once


namespace Mi
{
#ifdef _KERNEL_MODE
    NTSTATUS MICORE_API MI_NAME_PRIVATE(SetupEnvironmentBlock)(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath);
#else
    NTSTATUS MICORE_API MI_NAME_PRIVATE(SetupEnvironmentBlock)();
#endif

    NTSTATUS MICORE_API MI_NAME_PRIVATE(FreeEnvironmentBlock)();

}
