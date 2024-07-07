

EXTERN_C_START
namespace Musa
{
    _Ret_maybenull_
    PVOID WINAPI MUSA_NAME(EncodePointer)(
        _In_opt_ PVOID Ptr
        )
    {
        return RtlEncodePointer(Ptr);
    }
    MUSA_IAT_SYMBOL(EncodePointer, 4);

    _Ret_maybenull_
    PVOID WINAPI MUSA_NAME(DecodePointer)(
        _In_opt_ PVOID Ptr
        )
    {
        return RtlDecodePointer(Ptr);
    }
    MUSA_IAT_SYMBOL(DecodePointer, 4);

    _Ret_maybenull_
    PVOID WINAPI MUSA_NAME(EncodeSystemPointer)(
        _In_opt_ PVOID Ptr
        )
    {
        return RtlEncodeSystemPointer(Ptr);
    }
    MUSA_IAT_SYMBOL(EncodeSystemPointer, 4);

    _Ret_maybenull_
    PVOID WINAPI MUSA_NAME(DecodeSystemPointer)(
        _In_opt_ PVOID Ptr
        )
    {
        return RtlDecodeSystemPointer(Ptr);
    }
    MUSA_IAT_SYMBOL(DecodeSystemPointer, 4);

    //HRESULT WINAPI MUSA_NAME(EncodeRemotePointer)(
    //    _In_ HANDLE ProcessHandle,
    //    _In_opt_ PVOID Ptr,
    //    _Out_ PVOID* EncodedPtr
    //    )
    //{
    //    return HRESULT_FROM_NT(RtlEncodeRemotePointer(ProcessHandle, Ptr, EncodedPtr));
    //}
    //MUSA_IAT_SYMBOL(EncodeRemotePointer, 12);

    //HRESULT WINAPI MUSA_NAME(DecodeRemotePointer)(
    //    _In_ HANDLE ProcessHandle,
    //    _In_opt_ PVOID Ptr,
    //    _Out_ PVOID* DecodedPtr
    //    )
    //{
    //    return HRESULT_FROM_NT(RtlDecodeRemotePointer(ProcessHandle, Ptr, DecodedPtr));
    //}
    //MUSA_IAT_SYMBOL(DecodeRemotePointer, 12);

}
EXTERN_C_END
