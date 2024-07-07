#pragma once
#ifdef _KERNEL_MODE


EXTERN_C_START


_Must_inspect_result_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS MUSA_API MUSA_NAME_PRIVATE(SetupThreadEnvironmentBlock)(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    );

_Must_inspect_result_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS MUSA_API MUSA_NAME_PRIVATE(FreeThreadEnvironmentBlock)();


EXTERN_C_END


#endif
