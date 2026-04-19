#include "KernelBase.Private.h"
#include "Internal/KernelBase.Synchronize.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(CreateMutexW))
#pragma alloc_text(PAGE, MUSA_NAME(CreateMutexExW))
#pragma alloc_text(PAGE, MUSA_NAME(OpenMutexW))
#pragma alloc_text(PAGE, MUSA_NAME(ReleaseMutex))
#endif

EXTERN_C_START

//
// Mutant
//

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(CreateMutexW)(
    _In_opt_ LPSECURITY_ATTRIBUTES MutexAttributes,
    _In_ BOOL                      InitialOwner,
    _In_opt_ LPCWSTR               Name
)
{
    PAGED_CODE();

    return CreateMutexExW(MutexAttributes, Name, InitialOwner ? CREATE_MUTEX_INITIAL_OWNER : 0, MUTANT_ALL_ACCESS);
}

MUSA_IAT_SYMBOL(CreateMutexW, 12);

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(CreateMutexExW)(
    _In_opt_ LPSECURITY_ATTRIBUTES MutexAttributes,
    _In_opt_ LPCWSTR               Name,
    _In_ DWORD                     Flags,
    _In_ DWORD                     DesiredAccess
)
{
    PAGED_CODE();

    UNICODE_STRING    NameString{};
    OBJECT_ATTRIBUTES ObjectAttributes{};

    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectAttributes.Attributes = OBJ_KERNEL_HANDLE;

    if (MutexAttributes) {
        ObjectAttributes.SecurityDescriptor = MutexAttributes->lpSecurityDescriptor;

        if (MutexAttributes->bInheritHandle) {
            ObjectAttributes.Attributes |= OBJ_INHERIT;
        }
    }

    if (Name) {
        RtlInitUnicodeString(&NameString, Name);
        ObjectAttributes.ObjectName = &NameString;
    }

    HANDLE     Mutex  = nullptr;
    const auto Status = ZwCreateMutant(&Mutex, DesiredAccess, &ObjectAttributes,
        BooleanFlagOn(Flags, CREATE_MUTEX_INITIAL_OWNER));
    if (NT_SUCCESS(Status)) {
        return Mutex;
    }

    BaseSetLastNTError(Status);
    return nullptr;
}

MUSA_IAT_SYMBOL(CreateMutexExW, 16);

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(OpenMutexW)(
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

    HANDLE     Mutex  = nullptr;
    const auto Status = ZwOpenMutant(&Mutex, DesiredAccess, &ObjectAttributes);
    if (NT_SUCCESS(Status)) {
        return Mutex;
    }

    BaseSetLastNTError(Status);
    return nullptr;
}

MUSA_IAT_SYMBOL(OpenMutexW, 12);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(ReleaseMutex)(
    _In_ HANDLE Mutex
)
{
    PAGED_CODE();

    const auto Status = ZwReleaseMutant(Mutex, nullptr);
    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(ReleaseMutex, 4);

EXTERN_C_END
