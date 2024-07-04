#ifdef _KERNEL_MODE

#include "MiCore/MiCore.Utility.h"

#include <bcrypt.h>
#pragma comment(lib, "Cng.lib")

EXTERN_C_START
namespace Mi
{
    PVOID NTAPI MI_NAME(RtlEncodePointer)(
        _In_ PVOID Ptr
        )
    {
        return FastEncodePointer(Ptr);
    }
    MI_IAT_SYMBOL(RtlEncodePointer, 4);

    PVOID NTAPI MI_NAME(RtlDecodePointer)(
        _In_ PVOID Ptr
        )
    {
        return FastDecodePointer(Ptr);
    }
    MI_IAT_SYMBOL(RtlDecodePointer, 4);

    PVOID NTAPI MI_NAME(RtlEncodeSystemPointer)(
        _In_ PVOID Ptr
    )
    {
        return FastEncodePointer(Ptr, SharedUserData->Cookie);
    }
    MI_IAT_SYMBOL(RtlEncodeSystemPointer, 4);

    PVOID NTAPI MI_NAME(RtlDecodeSystemPointer)(
        _In_ PVOID Ptr
    )
    {
        return FastDecodePointer(Ptr, SharedUserData->Cookie);
    }
    MI_IAT_SYMBOL(RtlDecodeSystemPointer, 4);

    BOOLEAN NTAPI MI_NAME(RtlGenRandom)(
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
    MI_IAT_SYMBOL(RtlGenRandom, 8);


}
EXTERN_C_END


#endif
