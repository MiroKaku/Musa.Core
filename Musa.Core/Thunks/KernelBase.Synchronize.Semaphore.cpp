#include "KernelBase.Private.h"
#include "Internal/KernelBase.Synchronize.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(CreateSemaphoreW))
#pragma alloc_text(PAGE, MUSA_NAME(CreateSemaphoreExW))
#pragma alloc_text(PAGE, MUSA_NAME(OpenSemaphoreW))
#pragma alloc_text(PAGE, MUSA_NAME(ReleaseSemaphore))
#endif

EXTERN_C_START

//
// Semaphore
//

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(CreateSemaphoreW)(
    _In_opt_ LPSECURITY_ATTRIBUTES SemaphoreAttributes,
    _In_ LONG                      InitialCount,
    _In_ LONG                      MaximumCount,
    _In_opt_ LPCWSTR               Name
)
{
    PAGED_CODE();

    return CreateSemaphoreExW(SemaphoreAttributes, InitialCount, MaximumCount, Name,
        0, SEMAPHORE_ALL_ACCESS);
}

MUSA_IAT_SYMBOL(CreateSemaphoreW, 16);

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(CreateSemaphoreExW)(
    _In_opt_ LPSECURITY_ATTRIBUTES SemaphoreAttributes,
    _In_ LONG                      InitialCount,
    _In_ LONG                      MaximumCount,
    _In_opt_ LPCWSTR               Name,
    _Reserved_ DWORD               Flags,
    _In_ DWORD                     DesiredAccess
)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(Flags);

    UNICODE_STRING    NameString{};
    OBJECT_ATTRIBUTES ObjectAttributes{};

    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectAttributes.Attributes = OBJ_KERNEL_HANDLE;

    if (SemaphoreAttributes) {
        ObjectAttributes.SecurityDescriptor = SemaphoreAttributes->lpSecurityDescriptor;

        if (SemaphoreAttributes->bInheritHandle) {
            ObjectAttributes.Attributes |= OBJ_INHERIT;
        }
    }

    if (Name) {
        RtlInitUnicodeString(&NameString, Name);
        ObjectAttributes.ObjectName = &NameString;
    }

    HANDLE     Semaphore = nullptr;
    const auto Status = ZwCreateSemaphore(&Semaphore, DesiredAccess, &ObjectAttributes, InitialCount, MaximumCount);
    if (NT_SUCCESS(Status)) {
        return Semaphore;
    }

    BaseSetLastNTError(Status);
    return nullptr;
}

MUSA_IAT_SYMBOL(CreateSemaphoreExW, 24);

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(OpenSemaphoreW)(
    _In_ DWORD   DesiredAccess,
    _In_ BOOL    InheritHandle,
    _In_ LPCWSTR Name
)
{
    PAGED_CODE();

    UNICODE_STRING    NameString{};
    OBJECT_ATTRIBUTES ObjectAttributes{};

    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectAttributes.Attributes = OBJ_KERNEL_HANDLE;

    if (InheritHandle) {
        ObjectAttributes.Attributes |= OBJ_INHERIT;
    }

    if (Name) {
        RtlInitUnicodeString(&NameString, Name);
        ObjectAttributes.ObjectName = &NameString;
    }

    HANDLE     Semaphore = nullptr;
    const auto Status    = ZwOpenSemaphore(&Semaphore, DesiredAccess, &ObjectAttributes);
    if (NT_SUCCESS(Status)) {
        return Semaphore;
    }

    BaseSetLastNTError(Status);
    return nullptr;
}

MUSA_IAT_SYMBOL(OpenSemaphoreW, 12);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(ReleaseSemaphore)(
    _In_ HANDLE      Semaphore,
    _In_ LONG        ReleaseCount,
    _Out_opt_ LPLONG PreviousCount
)
{
    PAGED_CODE();

    const auto Status = ZwReleaseSemaphore(Semaphore, ReleaseCount, PreviousCount);
    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(ReleaseSemaphore, 12);

EXTERN_C_END
