#ifdef _KERNEL_MODE

#include "MiCore/MiCore.Utility.h"


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

}
EXTERN_C_END


#endif
