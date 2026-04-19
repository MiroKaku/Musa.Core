#include "KernelBase.Private.h"
#include "Internal/KernelBase.Synchronize.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(CreateEventW))
#pragma alloc_text(PAGE, MUSA_NAME(CreateEventExW))
#pragma alloc_text(PAGE, MUSA_NAME(OpenEventW))
#pragma alloc_text(PAGE, MUSA_NAME(SetEvent))
#pragma alloc_text(PAGE, MUSA_NAME(ResetEvent))
#endif

EXTERN_C_START

//
// Event
//

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(CreateEventW)(
    _In_opt_ LPSECURITY_ATTRIBUTES EventAttributes,
    _In_ BOOL                      ManualReset,
    _In_ BOOL                      InitialState,
    _In_opt_ LPCWSTR               Name
)
{
    PAGED_CODE();

    DWORD Flags = 0;

    if (ManualReset) {
        Flags |= CREATE_EVENT_MANUAL_RESET;
    }
    if (InitialState) {
        Flags |= CREATE_EVENT_INITIAL_SET;
    }

    return CreateEventExW(EventAttributes, Name, Flags, EVENT_ALL_ACCESS);
}

MUSA_IAT_SYMBOL(CreateEventW, 16);

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(CreateEventExW)(
    _In_opt_ LPSECURITY_ATTRIBUTES EventAttributes,
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

    if (EventAttributes) {
        ObjectAttributes.SecurityDescriptor = EventAttributes->lpSecurityDescriptor;

        if (EventAttributes->bInheritHandle) {
            ObjectAttributes.Attributes |= OBJ_INHERIT;
        }
    }

    if (Name) {
        RtlInitUnicodeString(&NameString, Name);
        ObjectAttributes.ObjectName = &NameString;
    }

    HANDLE     Event  = nullptr;
    const auto Status = ZwCreateEvent(&Event, DesiredAccess, &ObjectAttributes,
        BooleanFlagOn(Flags, CREATE_EVENT_MANUAL_RESET) ? NotificationEvent : SynchronizationEvent,
        BooleanFlagOn(Flags, CREATE_EVENT_INITIAL_SET));
    if (NT_SUCCESS(Status)) {
        return Event;
    }

    BaseSetLastNTError(Status);
    return nullptr;
}

MUSA_IAT_SYMBOL(CreateEventExW, 16);

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(OpenEventW)(
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

    HANDLE     Event = nullptr;
    const auto Status = ZwOpenEvent(&Event, DesiredAccess, &ObjectAttributes);
    if (NT_SUCCESS(Status)) {
        return Event;
    }

    BaseSetLastNTError(Status);
    return nullptr;
}

MUSA_IAT_SYMBOL(OpenEventW, 12);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SetEvent)(
    _In_ HANDLE Event
)
{
    PAGED_CODE();

    const auto Status = ZwSetEvent(Event, nullptr);
    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(SetEvent, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(ResetEvent)(
    _In_ HANDLE Event
)
{
    PAGED_CODE();

    const auto Status = ZwResetEvent(Event, nullptr);
    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(ResetEvent, 4);

EXTERN_C_END
