#ifdef _KERNEL_MODE

#include "Musa.Core/Musa.Core.Utility.h"

#include <bcrypt.h>
#pragma comment(lib, "Cng.lib")

EXTERN_C_START
namespace Musa
{
    PVOID NTAPI MUSA_NAME(RtlEncodePointer)(
        _In_ PVOID Ptr
        )
    {
        return FastEncodePointer(Ptr);
    }
    MUSA_IAT_SYMBOL(RtlEncodePointer, 4);

    PVOID NTAPI MUSA_NAME(RtlDecodePointer)(
        _In_ PVOID Ptr
        )
    {
        return FastDecodePointer(Ptr);
    }
    MUSA_IAT_SYMBOL(RtlDecodePointer, 4);

    PVOID NTAPI MUSA_NAME(RtlEncodeSystemPointer)(
        _In_ PVOID Ptr
    )
    {
        return FastEncodePointer(Ptr, SharedUserData->Cookie);
    }
    MUSA_IAT_SYMBOL(RtlEncodeSystemPointer, 4);

    PVOID NTAPI MUSA_NAME(RtlDecodeSystemPointer)(
        _In_ PVOID Ptr
    )
    {
        return FastDecodePointer(Ptr, SharedUserData->Cookie);
    }
    MUSA_IAT_SYMBOL(RtlDecodeSystemPointer, 4);

    BOOLEAN NTAPI MUSA_NAME(RtlGenRandom)(
        _Out_writes_bytes_(RandomBufferLength) PVOID RandomBuffer,
        _In_ ULONG RandomBufferLength
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


}
EXTERN_C_END


#endif
