

EXTERN_C_START
namespace Mi
{
    _Ret_maybenull_
    PVOID WINAPI MI_NAME(EncodePointer)(
        _In_opt_ PVOID Ptr
        )
    {
        return RtlEncodePointer(Ptr);
    }
    MI_IAT_SYMBOL(EncodePointer, 4);

    _Ret_maybenull_
    PVOID WINAPI MI_NAME(DecodePointer)(
        _In_opt_ PVOID Ptr
        )
    {
        return RtlDecodePointer(Ptr);
    }
    MI_IAT_SYMBOL(DecodePointer, 4);

    _Ret_maybenull_
    PVOID WINAPI MI_NAME(EncodeSystemPointer)(
        _In_opt_ PVOID Ptr
        )
    {
        return RtlEncodeSystemPointer(Ptr);
    }
    MI_IAT_SYMBOL(EncodeSystemPointer, 4);

    _Ret_maybenull_
    PVOID WINAPI MI_NAME(DecodeSystemPointer)(
        _In_opt_ PVOID Ptr
        )
    {
        return RtlDecodeSystemPointer(Ptr);
    }
    MI_IAT_SYMBOL(DecodeSystemPointer, 4);

    //HRESULT WINAPI MI_NAME(EncodeRemotePointer)(
    //    _In_ HANDLE ProcessHandle,
    //    _In_opt_ PVOID Ptr,
    //    _Out_ PVOID* EncodedPtr
    //    )
    //{
    //    return HRESULT_FROM_NT(RtlEncodeRemotePointer(ProcessHandle, Ptr, EncodedPtr));
    //}
    //MI_IAT_SYMBOL(EncodeRemotePointer, 12);

    //HRESULT WINAPI MI_NAME(DecodeRemotePointer)(
    //    _In_ HANDLE ProcessHandle,
    //    _In_opt_ PVOID Ptr,
    //    _Out_ PVOID* DecodedPtr
    //    )
    //{
    //    return HRESULT_FROM_NT(RtlDecodeRemotePointer(ProcessHandle, Ptr, DecodedPtr));
    //}
    //MI_IAT_SYMBOL(DecodeRemotePointer, 12);

}
EXTERN_C_END
