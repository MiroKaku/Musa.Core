#include "KernelBase.Private.h"
#include "Internal/KernelBase.Thread.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(SwitchToThread))
#pragma alloc_text(PAGE, MUSA_NAME(CreateThread))
#pragma alloc_text(PAGE, MUSA_NAME(CreateRemoteThread))
#pragma alloc_text(PAGE, MUSA_NAME(CreateRemoteThreadEx))
#endif

EXTERN_C_START

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SwitchToThread)()
{
    PAGED_CODE();

    if (ZwYieldExecution() != STATUS_NO_YIELD_PERFORMED) {
        return FALSE;
    }

    return TRUE;
}

MUSA_IAT_SYMBOL(SwitchToThread, 0);

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(CreateThread)(
    _In_opt_ LPSECURITY_ATTRIBUTES   ThreadAttributes,
    _In_ SIZE_T                      StackSize,
    _In_ LPTHREAD_START_ROUTINE      StartAddress,
    _In_opt_ __drv_aliasesMem LPVOID Parameter,
    _In_ DWORD                       CreationFlags,
    _Out_opt_ LPDWORD                ThreadId
)
{
    PAGED_CODE();

    return CreateRemoteThreadEx(GetCurrentProcess(), ThreadAttributes, StackSize,
        StartAddress, Parameter, CreationFlags & (STACK_SIZE_PARAM_IS_A_RESERVATION | CREATE_SUSPENDED),
        nullptr, ThreadId);
}

MUSA_IAT_SYMBOL(CreateThread, 24);

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(CreateRemoteThread)(
    _In_ HANDLE                    Process,
    _In_opt_ LPSECURITY_ATTRIBUTES ThreadAttributes,
    _In_ SIZE_T                    StackSize,
    _In_ LPTHREAD_START_ROUTINE    StartAddress,
    _In_opt_ LPVOID                Parameter,
    _In_ DWORD                     CreationFlags,
    _Out_opt_ LPDWORD              ThreadId
)
{
    PAGED_CODE();

    return CreateRemoteThreadEx(Process, ThreadAttributes, StackSize,
        StartAddress, Parameter, CreationFlags & (STACK_SIZE_PARAM_IS_A_RESERVATION | CREATE_SUSPENDED),
        nullptr, ThreadId);
}

MUSA_IAT_SYMBOL(CreateRemoteThread, 28);

extern PDRIVER_OBJECT MusaCoreDriverObject;

VEIL_DECLARE_STRUCT(MUSA_USER_THREAD_PARAMETER)
{
    LPTHREAD_START_ROUTINE StartAddress;
    LPVOID                 Parameter;
    KSEMAPHORE             Signal;
};

static void MUSA_NAME_PRIVATE(ThreadInitThunk)(
    _In_ PVOID StartContext
)
{
    const auto ThreadParameter = static_cast<PMUSA_USER_THREAD_PARAMETER>(StartContext);
    if (ThreadParameter == nullptr) {
        return;
    }
    const auto StartAddress = ThreadParameter->StartAddress;
    const auto Parameter    = ThreadParameter->Parameter;

    KeReleaseSemaphore(&ThreadParameter->Signal,
        IO_NO_INCREMENT, 1, FALSE);

    return (void)StartAddress(Parameter);
}

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(CreateRemoteThreadEx)(
    _In_ HANDLE                           Process,
    _In_opt_ LPSECURITY_ATTRIBUTES        ThreadAttributes,
    _In_ SIZE_T                           StackSize,
    _In_ LPTHREAD_START_ROUTINE           StartAddress,
    _In_opt_ LPVOID                       Parameter,
    _In_ DWORD                            CreationFlags,
    _In_opt_ LPPROC_THREAD_ATTRIBUTE_LIST AttributeList,
    _Out_opt_ LPDWORD                     ThreadId
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(StackSize);
    UNREFERENCED_PARAMETER(CreationFlags);
    UNREFERENCED_PARAMETER(AttributeList);

    if (BooleanFlagOn(CreationFlags, CREATE_SUSPENDED)) {
        BaseSetLastNTError(STATUS_NOT_SUPPORTED);
        return nullptr;
    }

    auto ObjectAttributes = OBJECT_ATTRIBUTES RTL_CONSTANT_OBJECT_ATTRIBUTES(
        static_cast<PUNICODE_STRING>(nullptr), OBJ_KERNEL_HANDLE);

    if (ThreadAttributes) {
        if (ThreadAttributes->bInheritHandle) {
            ObjectAttributes.Attributes |= OBJ_INHERIT;
        }

        ObjectAttributes.SecurityDescriptor = ThreadAttributes->lpSecurityDescriptor;
    }

    CLIENT_ID ClientId{};
    HANDLE    ThreadHandle = nullptr;
    PVOID     OwnerObject  = MusaCoreDriverObject;

    if (OwnerObject) {
        if (!ObReferenceObjectSafeWithTag(OwnerObject, MUSA_TAG)) {
            BaseSetLastNTError(STATUS_THREAD_IS_TERMINATING);
            return nullptr;
        }
    }

    __try {
        MUSA_USER_THREAD_PARAMETER ThreadParameter;
        ThreadParameter.StartAddress = StartAddress;
        ThreadParameter.Parameter    = Parameter;
        KeInitializeSemaphore(&ThreadParameter.Signal, 0, 1);

        const auto Status = PsCreateSystemThread(&ThreadHandle, THREAD_ALL_ACCESS, &ObjectAttributes, Process,
            &ClientId, MUSA_NAME_PRIVATE(ThreadInitThunk), &ThreadParameter);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return nullptr;
        }

        (void)KeWaitForSingleObject(&ThreadParameter.Signal,
            Executive, KernelMode, FALSE, nullptr);
    } __finally {
        if (OwnerObject) {
            ObDereferenceObjectWithTag(OwnerObject, MUSA_TAG);
        }
    }

    if (ThreadId) {
        *ThreadId = HandleToULong(ClientId.UniqueThread);
    }

    return ThreadHandle;
}

MUSA_IAT_SYMBOL(CreateRemoteThreadEx, 32);

EXTERN_C_END
