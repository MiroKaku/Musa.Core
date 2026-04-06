#include "Internal/Ntdll.Utility.h"
#include "Musa.Core/Musa.Utilities.h"
#include <bcrypt.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(RtlEncodeRemotePointer))
#pragma alloc_text(PAGE, MUSA_NAME(RtlDecodeRemotePointer))
#endif

using namespace Musa;
using namespace Musa::Utils;

EXTERN_C_START

#if defined(_KERNEL_MODE)
#pragma comment(lib, "Cng.lib")

PVOID NTAPI MUSA_NAME(RtlEncodePointer)(_In_ PVOID Ptr)
{
    return FastEncodePointer(Ptr);
}

MUSA_IAT_SYMBOL(RtlEncodePointer, 4);

PVOID NTAPI MUSA_NAME(RtlDecodePointer)(_In_ PVOID Ptr)
{
    return FastDecodePointer(Ptr);
}

MUSA_IAT_SYMBOL(RtlDecodePointer, 4);

PVOID NTAPI MUSA_NAME(RtlEncodeSystemPointer)(_In_ PVOID Ptr)
{
    return FastEncodePointer(Ptr, SharedUserData->Cookie);
}

MUSA_IAT_SYMBOL(RtlEncodeSystemPointer, 4);

PVOID NTAPI MUSA_NAME(RtlDecodeSystemPointer)(_In_ PVOID Ptr)
{
    return FastDecodePointer(Ptr, SharedUserData->Cookie);
}

MUSA_IAT_SYMBOL(RtlDecodeSystemPointer, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlEncodeRemotePointer)(
    _In_ HANDLE ProcessHandle,
    _In_opt_ PVOID Ptr,
    _Out_ PVOID* EncodedPtr
)
{
    PAGED_CODE();

    ULONG    Cookie = 0;
    NTSTATUS Status = ZwQueryInformationProcess(ProcessHandle, ProcessCookie,
        &Cookie, sizeof(Cookie), nullptr);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    *EncodedPtr = FastEncodePointer(Ptr, Cookie);
    return STATUS_SUCCESS;
}

MUSA_IAT_SYMBOL(RtlEncodeRemotePointer, 12);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS NTAPI MUSA_NAME(RtlDecodeRemotePointer)(
    _In_ HANDLE ProcessHandle,
    _In_opt_ PVOID Ptr,
    _Out_ PVOID* DecodedPtr
)
{
    PAGED_CODE();

    ULONG    Cookie = 0;
    NTSTATUS Status = ZwQueryInformationProcess(ProcessHandle, ProcessCookie,
        &Cookie, sizeof(Cookie), nullptr);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    *DecodedPtr = FastDecodePointer(Ptr, Cookie);
    return STATUS_SUCCESS;
}

MUSA_IAT_SYMBOL(RtlDecodeRemotePointer, 12);

BOOLEAN NTAPI MUSA_NAME(RtlGenRandom)(
    _Out_writes_bytes_(RandomBufferLength) PVOID RandomBuffer, _In_ ULONG RandomBufferLength
)
{
    const auto Status = BCryptGenRandom(nullptr, static_cast<PUCHAR>(RandomBuffer), RandomBufferLength,
        BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(RtlGenRandom, 8);

#endif // defined(_KERNEL_MODE)

EXTERN_C_END
