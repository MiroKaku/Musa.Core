#pragma once
#ifdef _KERNEL_MODE


EXTERN_C_START


NTSTATUS MICORE_API MI_NAME_PRIVATE(SetupProcessEnvironmentBlock)(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );
NTSTATUS MICORE_API MI_NAME_PRIVATE(FreeProcessEnvironmentBlock)();


EXTERN_C_END


#endif
