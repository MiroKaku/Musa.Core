#include "universal.h"
#include "thunks/thunks.hpp"


EXTERN_C_START

PDRIVER_OBJECT MI_DRIVER_OBJECT;

NTSTATUS MICORE_API MiCoreStartup(_In_ PDRIVER_OBJECT DriverObject)
{
    NTSTATUS Status;

    MI_DRIVER_OBJECT = DriverObject;

    do {
        ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

        Status = Mi::Thunk::LoadStubs();
        if (!NT_SUCCESS(Status)) {
            break;
        }

        Status = Mi::Thunk::InitThreadTable();
        if (!NT_SUCCESS(Status)) {
            break;
        }

    } while (false);

    return Status;
}

NTSTATUS MICORE_API MiCoreShutdown()
{
    Mi::Thunk::FreeThreadTable();
    Mi::Thunk::FreeStubs();
    return STATUS_SUCCESS;
}

PVOID MICORE_API MiGetZwRoutineAddress(const size_t NameHash)
{
    return Mi::Thunk::QueryStub(NameHash);
}

EXTERN_C_END
