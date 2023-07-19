#include "universal.h"
#include "thunks/thunks.hpp"


EXTERN_C_START

UNICODE_STRING NtSystemRoot;

NTSTATUS MICORE_API MiCoreStartup(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    NTSTATUS Status;

    do {
        ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

        RtlInitUnicodeString(&NtSystemRoot, RtlGetNtSystemRoot());

        Status = Mi::Thunk::LoadStubs();
        if (!NT_SUCCESS(Status)) {
            break;
        }

        Status = Mi::Thunk::CreatePeb(DriverObject, RegistryPath);
        if (!NT_SUCCESS(Status)) {
            break;
        }

    } while (false);

    return Status;
}

NTSTATUS MICORE_API MiCoreShutdown()
{
    Mi::Thunk::DeletePeb();
    Mi::Thunk::FreeStubs();
    return STATUS_SUCCESS;
}

PVOID MICORE_API MiGetZwRoutineAddress(const size_t NameHash)
{
    return Mi::Thunk::QueryStub(NameHash);
}

EXTERN_C_END
