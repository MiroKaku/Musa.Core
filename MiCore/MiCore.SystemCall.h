#pragma once


namespace Mi
{
    NTSTATUS MICORE_API MI_NAME_PRIVATE(SetupSystemCall)();

    NTSTATUS MICORE_API MI_NAME_PRIVATE(FreeSystemCall)();

    PVOID    MICORE_API MI_NAME_PRIVATE(GetSystemRoutineAddress)(
        _In_ size_t NameHash);

    PVOID    MICORE_API MI_NAME_PRIVATE(GetSystemRoutineAddress)(
        _In_ const char* Name);

}
