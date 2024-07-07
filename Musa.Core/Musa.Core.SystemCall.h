#pragma once


EXTERN_C_START


_Must_inspect_result_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS MUSA_API MUSA_NAME_PRIVATE(SetupSystemCall)();

_Must_inspect_result_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS MUSA_API MUSA_NAME_PRIVATE(FreeSystemCall)();

_IRQL_requires_max_(DISPATCH_LEVEL)
PVOID MUSA_API MUSA_NAME_PRIVATE(GetSystemRoutineAddress)(
    _In_ const char* Name);

_IRQL_requires_max_(DISPATCH_LEVEL)
PVOID MUSA_API MUSA_NAME_PRIVATE(GetSystemRoutineAddressByNameHash)(
    _In_ size_t NameHash);


EXTERN_C_END
