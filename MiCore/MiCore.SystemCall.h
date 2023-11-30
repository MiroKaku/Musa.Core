#pragma once


EXTERN_C_START


NTSTATUS MICORE_API MI_NAME_PRIVATE(SetupSystemCall)();

NTSTATUS MICORE_API MI_NAME_PRIVATE(FreeSystemCall)();

PVOID    MICORE_API MI_NAME_PRIVATE(GetSystemRoutineAddress)(
    _In_ const char* Name);

PVOID    MICORE_API MI_NAME_PRIVATE(GetSystemRoutineAddressByNameHash)(
    _In_ size_t NameHash);


EXTERN_C_END
