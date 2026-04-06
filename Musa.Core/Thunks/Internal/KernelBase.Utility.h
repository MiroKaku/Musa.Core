#pragma once

EXTERN_C_START

_IRQL_requires_max_(PASSIVE_LEVEL)
HRESULT WINAPI MUSA_NAME(EncodeRemotePointer)(
    _In_ HANDLE ProcessHandle,
    _In_opt_ PVOID Ptr,
    _Out_ PVOID* EncodedPtr
);


_IRQL_requires_max_(PASSIVE_LEVEL)
HRESULT WINAPI MUSA_NAME(DecodeRemotePointer)(
    _In_ HANDLE ProcessHandle,
    _In_opt_ PVOID Ptr,
    _Out_ PVOID* DecodedPtr
);

EXTERN_C_END
