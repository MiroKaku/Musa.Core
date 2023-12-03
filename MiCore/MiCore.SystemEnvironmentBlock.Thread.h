#pragma once
#ifdef _KERNEL_MODE


EXTERN_C_START


NTSTATUS MICORE_API MI_NAME_PRIVATE(SetupThreadEnvironmentBlock)(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );
NTSTATUS MICORE_API MI_NAME_PRIVATE(FreeThreadEnvironmentBlock)();


EXTERN_C_END


#endif
