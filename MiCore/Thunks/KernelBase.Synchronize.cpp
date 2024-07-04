#include "KernelBase.Private.h"
#include "KernelBase.Synchronize.Private.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MI_NAME(CreateMutexW))
#pragma alloc_text(PAGE, MI_NAME(CreateMutexExW))
#pragma alloc_text(PAGE, MI_NAME(OpenMutexW))
#pragma alloc_text(PAGE, MI_NAME(ReleaseMutex))

#pragma alloc_text(PAGE, MI_NAME(CreateEventW))
#pragma alloc_text(PAGE, MI_NAME(CreateEventExW))
#pragma alloc_text(PAGE, MI_NAME(OpenEventW))
#pragma alloc_text(PAGE, MI_NAME(SetEvent))
#pragma alloc_text(PAGE, MI_NAME(ResetEvent))

#pragma alloc_text(PAGE, MI_NAME(CreateSemaphoreW))
#pragma alloc_text(PAGE, MI_NAME(CreateSemaphoreExW))
#pragma alloc_text(PAGE, MI_NAME(OpenSemaphoreW))
#pragma alloc_text(PAGE, MI_NAME(ReleaseSemaphore))

#pragma alloc_text(PAGE, MI_NAME(Sleep))
#pragma alloc_text(PAGE, MI_NAME(SleepEx))
#pragma alloc_text(PAGE, MI_NAME(WaitForSingleObject))
#pragma alloc_text(PAGE, MI_NAME(WaitForSingleObjectEx))
#pragma alloc_text(PAGE, MI_NAME(WaitForMultipleObjects))
#pragma alloc_text(PAGE, MI_NAME(WaitForMultipleObjectsEx))

#pragma alloc_text(PAGE, MI_NAME(InitOnceInitialize))
#pragma alloc_text(PAGE, MI_NAME(InitOnceExecuteOnce))
#pragma alloc_text(PAGE, MI_NAME(InitOnceBeginInitialize))
#pragma alloc_text(PAGE, MI_NAME(InitOnceComplete))

#pragma alloc_text(PAGE, MI_NAME(AcquireSRWLockExclusive))
#pragma alloc_text(PAGE, MI_NAME(AcquireSRWLockShared))
#pragma alloc_text(PAGE, MI_NAME(ReleaseSRWLockExclusive))
#pragma alloc_text(PAGE, MI_NAME(ReleaseSRWLockShared))
#pragma alloc_text(PAGE, MI_NAME(TryAcquireSRWLockExclusive))
#pragma alloc_text(PAGE, MI_NAME(TryAcquireSRWLockShared))

#pragma alloc_text(PAGE, MI_NAME(EnterCriticalSection))
#pragma alloc_text(PAGE, MI_NAME(LeaveCriticalSection))
#pragma alloc_text(PAGE, MI_NAME(TryEnterCriticalSection))
#pragma alloc_text(PAGE, MI_NAME(SetCriticalSectionSpinCount))

#pragma alloc_text(PAGE, MI_NAME(WakeConditionVariable))
#pragma alloc_text(PAGE, MI_NAME(WakeAllConditionVariable))
#pragma alloc_text(PAGE, MI_NAME(SleepConditionVariableCS))
#pragma alloc_text(PAGE, MI_NAME(SleepConditionVariableSRW))
#endif

EXTERN_C_START
namespace Mi
{
    //
    // Mutant
    //

    _IRQL_requires_max_(PASSIVE_LEVEL)
    HANDLE WINAPI MI_NAME(CreateMutexW)(
        _In_opt_ LPSECURITY_ATTRIBUTES MutexAttributes,
        _In_     BOOL    InitialOwner,
        _In_opt_ LPCWSTR Name
        )
    {
        PAGED_CODE();

        return CreateMutexExW(MutexAttributes, Name, InitialOwner ? CREATE_MUTEX_INITIAL_OWNER : 0, MUTANT_ALL_ACCESS);
    }
    MI_IAT_SYMBOL(CreateMutexW, 12);
    
    _IRQL_requires_max_(PASSIVE_LEVEL)
    HANDLE WINAPI MI_NAME(CreateMutexExW)(
        _In_opt_ LPSECURITY_ATTRIBUTES MutexAttributes,
        _In_opt_ LPCWSTR Name,
        _In_ DWORD Flags,
        _In_ DWORD DesiredAccess
        )
    {
        PAGED_CODE();

        UNICODE_STRING    NameString{};
        OBJECT_ATTRIBUTES ObjectAttributes{};

        ObjectAttributes.Length     = sizeof(OBJECT_ATTRIBUTES);

    #if defined _KERNEL_MODE
        ObjectAttributes.Attributes = OBJ_KERNEL_HANDLE;
    #endif

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

        HANDLE Mutex = nullptr;
        const auto Status = ZwCreateMutant(&Mutex, DesiredAccess, &ObjectAttributes, BooleanFlagOn(Flags, CREATE_MUTEX_INITIAL_OWNER));
        if (NT_SUCCESS(Status)) {
            return Mutex;
        }

        BaseSetLastNTError(Status);
        return nullptr;
    }
    MI_IAT_SYMBOL(CreateMutexExW, 16);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    HANDLE WINAPI MI_NAME(OpenMutexW)(
        _In_ DWORD   DesiredAccess,
        _In_ BOOL    InheritHandle,
        _In_ LPCWSTR Name
        )
    {
        PAGED_CODE();

        UNICODE_STRING    NameString{};
        OBJECT_ATTRIBUTES ObjectAttributes{};

        ObjectAttributes.Length     = sizeof(OBJECT_ATTRIBUTES);

    #if defined _KERNEL_MODE
        ObjectAttributes.Attributes = OBJ_KERNEL_HANDLE;
    #endif

        if (InheritHandle) {
            ObjectAttributes.Attributes |= OBJ_INHERIT;
        }

        if (Name) {
            RtlInitUnicodeString(&NameString, Name);
            ObjectAttributes.ObjectName = &NameString;
        }

        HANDLE Mutex = nullptr;
        const auto Status = ZwOpenMutant(&Mutex, DesiredAccess, &ObjectAttributes);
        if (NT_SUCCESS(Status)) {
            return Mutex;
        }

        BaseSetLastNTError(Status);
        return nullptr;
    }
    MI_IAT_SYMBOL(OpenMutexW, 12);
    
    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(ReleaseMutex)(
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
    MI_IAT_SYMBOL(ReleaseMutex, 4);

    //
    // Event
    //

    _IRQL_requires_max_(PASSIVE_LEVEL)
    HANDLE WINAPI MI_NAME(CreateEventW)(
        _In_opt_ LPSECURITY_ATTRIBUTES EventAttributes,
        _In_ BOOL ManualReset,
        _In_ BOOL InitialState,
        _In_opt_ LPCWSTR Name
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
    MI_IAT_SYMBOL(CreateEventW, 16);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    HANDLE WINAPI MI_NAME(CreateEventExW)(
        _In_opt_ LPSECURITY_ATTRIBUTES EventAttributes,
        _In_opt_ LPCWSTR Name,
        _In_ DWORD Flags,
        _In_ DWORD DesiredAccess
        )
    {
        PAGED_CODE();

        UNICODE_STRING    NameString{};
        OBJECT_ATTRIBUTES ObjectAttributes{};

        ObjectAttributes.Length     = sizeof(OBJECT_ATTRIBUTES);

    #if defined _KERNEL_MODE
        ObjectAttributes.Attributes = OBJ_KERNEL_HANDLE;
    #endif

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

        HANDLE Event = nullptr;
        const auto Status = ZwCreateEvent(&Event, DesiredAccess, &ObjectAttributes,
            BooleanFlagOn(Flags, CREATE_EVENT_MANUAL_RESET) ? NotificationEvent : SynchronizationEvent, BooleanFlagOn(Flags, CREATE_EVENT_INITIAL_SET));
        if (NT_SUCCESS(Status)) {
            return Event;
        }

        BaseSetLastNTError(Status);
        return nullptr;
    }
    MI_IAT_SYMBOL(CreateEventExW, 16);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    HANDLE WINAPI MI_NAME(OpenEventW)(
        _In_ DWORD   DesiredAccess,
        _In_ BOOL    InheritHandle,
        _In_ LPCWSTR Name
        )
    {
        PAGED_CODE();

        UNICODE_STRING    NameString{};
        OBJECT_ATTRIBUTES ObjectAttributes{};

        ObjectAttributes.Length     = sizeof(OBJECT_ATTRIBUTES);

    #if defined _KERNEL_MODE
        ObjectAttributes.Attributes = OBJ_KERNEL_HANDLE;
    #endif

        if (InheritHandle) {
            ObjectAttributes.Attributes |= OBJ_INHERIT;
        }

        if (Name) {
            RtlInitUnicodeString(&NameString, Name);
            ObjectAttributes.ObjectName = &NameString;
        }

        HANDLE Event = nullptr;
        const auto Status = ZwOpenEvent(&Event, DesiredAccess, &ObjectAttributes);
        if (NT_SUCCESS(Status)) {
            return Event;
        }

        BaseSetLastNTError(Status);
        return nullptr;
    }
    MI_IAT_SYMBOL(OpenEventW, 12);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(SetEvent)(
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
    MI_IAT_SYMBOL(SetEvent, 4);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(ResetEvent)(
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
    MI_IAT_SYMBOL(ResetEvent, 4);

    //
    // Semaphore
    //

    _IRQL_requires_max_(PASSIVE_LEVEL)
    HANDLE WINAPI MI_NAME(CreateSemaphoreW)(
        _In_opt_ LPSECURITY_ATTRIBUTES SemaphoreAttributes,
        _In_ LONG InitialCount,
        _In_ LONG MaximumCount,
        _In_opt_ LPCWSTR Name
        )
    {
        PAGED_CODE();

        return CreateSemaphoreExW(SemaphoreAttributes, InitialCount, MaximumCount, Name,
            0, SEMAPHORE_ALL_ACCESS);
    }
    MI_IAT_SYMBOL(CreateSemaphoreW, 16);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    HANDLE WINAPI MI_NAME(CreateSemaphoreExW)(
        _In_opt_    LPSECURITY_ATTRIBUTES SemaphoreAttributes,
        _In_        LONG InitialCount,
        _In_        LONG MaximumCount,
        _In_opt_    LPCWSTR Name,
        _Reserved_  DWORD Flags,
        _In_        DWORD DesiredAccess
        )
    {
        PAGED_CODE();
        UNREFERENCED_PARAMETER(Flags);

        UNICODE_STRING    NameString{};
        OBJECT_ATTRIBUTES ObjectAttributes{};

        ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);

    #if defined _KERNEL_MODE
        ObjectAttributes.Attributes = OBJ_KERNEL_HANDLE;
    #endif

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

        HANDLE Semaphore = nullptr;
        const auto Status = ZwCreateSemaphore(&Semaphore, DesiredAccess, &ObjectAttributes, InitialCount, MaximumCount);
        if (NT_SUCCESS(Status)) {
            return Semaphore;
        }

        BaseSetLastNTError(Status);
        return nullptr;
    }
    MI_IAT_SYMBOL(CreateSemaphoreExW, 24);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    HANDLE WINAPI MI_NAME(OpenSemaphoreW)(
        _In_ DWORD   DesiredAccess,
        _In_ BOOL    InheritHandle,
        _In_ LPCWSTR Name
        )
    {
        PAGED_CODE();

        UNICODE_STRING    NameString{};
        OBJECT_ATTRIBUTES ObjectAttributes{};

        ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);

    #if defined _KERNEL_MODE
        ObjectAttributes.Attributes = OBJ_KERNEL_HANDLE;
    #endif

        if (InheritHandle) {
            ObjectAttributes.Attributes |= OBJ_INHERIT;
        }

        if (Name) {
            RtlInitUnicodeString(&NameString, Name);
            ObjectAttributes.ObjectName = &NameString;
        }

        HANDLE Semaphore = nullptr;
        const auto Status = ZwOpenSemaphore(&Semaphore, DesiredAccess, &ObjectAttributes);
        if (NT_SUCCESS(Status)) {
            return Semaphore;
        }

        BaseSetLastNTError(Status);
        return nullptr;
    }
    MI_IAT_SYMBOL(OpenSemaphoreW, 12);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(ReleaseSemaphore)(
        _In_ HANDLE Semaphore,
        _In_ LONG   ReleaseCount,
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
    MI_IAT_SYMBOL(ReleaseSemaphore, 12);

    //
    // Wait
    //

    _IRQL_requires_max_(APC_LEVEL)
    VOID WINAPI MI_NAME(Sleep)(
        _In_ DWORD  Milliseconds
        )
    {
        PAGED_CODE();
        SleepEx(Milliseconds, FALSE);
    }
    MI_IAT_SYMBOL(Sleep, 4);

#if defined _KERNEL_MODE
    _IRQL_requires_max_(APC_LEVEL)
    DWORD WINAPI MI_NAME(SleepEx)(
        _In_ DWORD Milliseconds,
        _In_ BOOL  Alertable
        )
    {
        PAGED_CODE();

        LARGE_INTEGER  Time;
        LARGE_INTEGER* Timeout = &Time;

        if (Milliseconds != INFINITE) {
            Time.QuadPart = Int32x32To64(Milliseconds, -10000);
        }
        else {
            Time.QuadPart = 0x8000000000000000;
        }

        NTSTATUS Status = KeDelayExecutionThread(KernelMode, static_cast<BOOLEAN>(Alertable), Timeout);
        if (Alertable) {
            if (Status != STATUS_USER_APC) {
                Status = 0;
            }
        }

        return Status;
    }
    MI_IAT_SYMBOL(SleepEx, 8);
#endif

    _IRQL_requires_max_(PASSIVE_LEVEL)
    DWORD WINAPI MI_NAME(WaitForSingleObject)(
        _In_ HANDLE Handle,
        _In_ DWORD  Milliseconds
        )
    {
        PAGED_CODE();

        return WaitForSingleObjectEx(Handle, Milliseconds, FALSE);
    }
    MI_IAT_SYMBOL(WaitForSingleObject, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    DWORD WINAPI MI_NAME(WaitForSingleObjectEx)(
        _In_ HANDLE Handle,
        _In_ DWORD  Milliseconds,
        _In_ BOOL   Alertable
        )
    {
        PAGED_CODE();

        LARGE_INTEGER  Time;
        LARGE_INTEGER* Timeout = nullptr;

        if (Milliseconds != INFINITE) {
            Time.QuadPart = Int32x32To64(Milliseconds, -10000);
            Timeout = &Time;
        }

        const auto Status = ZwWaitForSingleObject(Handle, static_cast<BOOLEAN>(Alertable), Timeout);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;

    }
    MI_IAT_SYMBOL(WaitForSingleObjectEx, 12);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    DWORD WINAPI MI_NAME(WaitForMultipleObjects)(
        _In_ DWORD Count,
        _In_reads_(Count) CONST HANDLE* Handles,
        _In_ BOOL  WaitAll,
        _In_ DWORD Milliseconds
        )
    {
        PAGED_CODE();

        return WaitForMultipleObjectsEx(Count, Handles, WaitAll, Milliseconds, FALSE);
    }
    MI_IAT_SYMBOL(WaitForMultipleObjects, 16);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    DWORD WINAPI MI_NAME(WaitForMultipleObjectsEx)(
        _In_ DWORD Count,
        _In_reads_(Count) CONST HANDLE* Handles,
        _In_ BOOL  WaitAll,
        _In_ DWORD Milliseconds,
        _In_ BOOL  Alertable
        )
    {
        PAGED_CODE();

        LARGE_INTEGER  Time;
        LARGE_INTEGER* Timeout = nullptr;

        if (Milliseconds != INFINITE) {
            Time.QuadPart = Int32x32To64(Milliseconds, -10000);
            Timeout = &Time;
        }

        const auto Status = ZwWaitForMultipleObjects(Count, const_cast<HANDLE*>(Handles), WaitAll ? WAIT_TYPE::WaitAll : WaitAny,
            static_cast<BOOLEAN>(Alertable), Timeout);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;

    }
    MI_IAT_SYMBOL(WaitForMultipleObjectsEx, 20);

    //
    // Run once
    //

    _IRQL_requires_max_(APC_LEVEL)
    VOID WINAPI MI_NAME(InitOnceInitialize)(
        _Out_ PINIT_ONCE InitOnce
        )
    {
        PAGED_CODE();

        RtlRunOnceInitialize(InitOnce);
    }
    MI_IAT_SYMBOL(InitOnceInitialize, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Must_inspect_result_
    BOOL WINAPI MI_NAME(InitOnceExecuteOnce)(
        _Inout_ PINIT_ONCE InitOnce,
        _In_ __callback PINIT_ONCE_FN InitFn,
        _Inout_opt_ PVOID Parameter,
        _Outptr_opt_result_maybenull_ LPVOID* Context
        )
    {
        PAGED_CODE();

        NTSTATUS Status;

        __try {
        #pragma warning(suppress: 28023)
            Status = RtlRunOnceExecuteOnce(InitOnce, reinterpret_cast<PRTL_RUN_ONCE_INIT_FN>(InitFn),
                Parameter, Context);
        }
        __except(EXCEPTION_EXECUTE_HANDLER){
            Status = GetExceptionCode();
        }

        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MI_IAT_SYMBOL(InitOnceExecuteOnce, 16);

    _IRQL_requires_max_(APC_LEVEL)
    _Must_inspect_result_
    BOOL WINAPI MI_NAME(InitOnceBeginInitialize)(
        _Inout_ LPINIT_ONCE InitOnce,
        _In_ DWORD Flags,
        _Out_ PBOOL Pending,
        _Outptr_opt_result_maybenull_ LPVOID* Context
        )
    {
        PAGED_CODE();

        const auto Status = RtlRunOnceBeginInitialize(InitOnce, Flags, Context);
        if (NT_SUCCESS(Status)) {
            *Pending = Status == STATUS_PENDING;
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MI_IAT_SYMBOL(InitOnceBeginInitialize, 16);

    _IRQL_requires_max_(APC_LEVEL)
    BOOL WINAPI MI_NAME(InitOnceComplete)(
        _Inout_ LPINIT_ONCE InitOnce,
        _In_ DWORD Flags,
        _In_opt_ LPVOID Context
        )
    {
        PAGED_CODE();

        const auto Status = RtlRunOnceComplete(InitOnce, Flags, Context);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MI_IAT_SYMBOL(InitOnceComplete, 12);

    //
    // R/W lock
    //

    _IRQL_requires_max_(DISPATCH_LEVEL)
    VOID WINAPI MI_NAME(InitializeSRWLock)(
        _Out_ PSRWLOCK SRWLock
        )
    {
        RtlInitializeSRWLock(SRWLock);
    }
    MI_IAT_SYMBOL(InitializeSRWLock, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Acquires_exclusive_lock_(*SRWLock)
    VOID WINAPI MI_NAME(AcquireSRWLockExclusive)(
        _Inout_ PSRWLOCK SRWLock
        )
    {
        PAGED_CODE();
        RtlAcquireSRWLockExclusive(SRWLock);
    }
    MI_IAT_SYMBOL(AcquireSRWLockExclusive, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Acquires_shared_lock_(*SRWLock)
    VOID WINAPI MI_NAME(AcquireSRWLockShared)(
        _Inout_ PSRWLOCK SRWLock
        )
    {
        PAGED_CODE();
        RtlAcquireSRWLockShared(SRWLock);
    }
    MI_IAT_SYMBOL(AcquireSRWLockShared, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Releases_exclusive_lock_(*SRWLock)
    VOID WINAPI MI_NAME(ReleaseSRWLockExclusive)(
        _Inout_ PSRWLOCK SRWLock
        )
    {
        PAGED_CODE();
        RtlReleaseSRWLockExclusive(SRWLock);
    }
    MI_IAT_SYMBOL(ReleaseSRWLockExclusive, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Releases_shared_lock_(*SRWLock)
    VOID WINAPI MI_NAME(ReleaseSRWLockShared)(
        _Inout_ PSRWLOCK SRWLock
        )
    {
        PAGED_CODE();
        RtlReleaseSRWLockShared(SRWLock);
    }
    MI_IAT_SYMBOL(ReleaseSRWLockShared, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _When_(return != 0, _Acquires_exclusive_lock_(*SRWLock))
    BOOLEAN WINAPI MI_NAME(TryAcquireSRWLockExclusive)(
        _Inout_ PSRWLOCK SRWLock
    )
    {
        PAGED_CODE();
        return RtlTryAcquireSRWLockExclusive(SRWLock);
    }
    MI_IAT_SYMBOL(TryAcquireSRWLockExclusive, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _When_(return != 0, _Acquires_shared_lock_(*SRWLock))
    BOOLEAN WINAPI MI_NAME(TryAcquireSRWLockShared)(
        _Inout_ PSRWLOCK SRWLock
    )
    {
        PAGED_CODE();
        return RtlTryAcquireSRWLockShared(SRWLock);
    }
    MI_IAT_SYMBOL(TryAcquireSRWLockShared, 4);

    //
    // Critical Section
    //

    _IRQL_requires_max_(DISPATCH_LEVEL)
    VOID WINAPI MI_NAME(InitializeCriticalSection)(
        _Out_ LPCRITICAL_SECTION CriticalSection
        )
    {
        (void)RtlInitializeCriticalSection(CriticalSection);
    }
    MI_IAT_SYMBOL(InitializeCriticalSection, 4);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _Must_inspect_result_
    BOOL WINAPI MI_NAME(InitializeCriticalSectionAndSpinCount)(
        _Out_ LPCRITICAL_SECTION CriticalSection,
        _In_ DWORD SpinCount
    )
    {
        const auto Status = RtlInitializeCriticalSectionAndSpinCount(CriticalSection, SpinCount);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MI_IAT_SYMBOL(InitializeCriticalSectionAndSpinCount, 8);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    _IRQL_requires_max_(APC_LEVEL)
    BOOL WINAPI MI_NAME(InitializeCriticalSectionEx)(
        _Out_ LPCRITICAL_SECTION CriticalSection,
        _In_ DWORD SpinCount,
        _In_ DWORD Flags
    )
    {
        const auto Status = RtlInitializeCriticalSectionEx(CriticalSection, SpinCount, Flags);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MI_IAT_SYMBOL(InitializeCriticalSectionEx, 12);

    _IRQL_requires_max_(DISPATCH_LEVEL)
    VOID WINAPI MI_NAME(DeleteCriticalSection)(
        _Inout_ LPCRITICAL_SECTION CriticalSection
        )
    {
        (void)RtlDeleteCriticalSection(CriticalSection);
    }
    MI_IAT_SYMBOL(DeleteCriticalSection, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Acquires_exclusive_lock_(*CriticalSection)
    VOID WINAPI MI_NAME(EnterCriticalSection)(
        _Inout_ LPCRITICAL_SECTION CriticalSection
        )
    {
        PAGED_CODE();
        (void)RtlEnterCriticalSection(CriticalSection);
    }
    MI_IAT_SYMBOL(EnterCriticalSection, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _Releases_exclusive_lock_(*CriticalSection)
    VOID WINAPI MI_NAME(LeaveCriticalSection)(
        _Inout_ LPCRITICAL_SECTION CriticalSection
        )
    {
        PAGED_CODE();
        (void)RtlLeaveCriticalSection(CriticalSection);
    }
    MI_IAT_SYMBOL(LeaveCriticalSection, 4);

    _IRQL_requires_max_(APC_LEVEL)
    _When_(return != 0, _Acquires_exclusive_lock_(*CriticalSection))
    BOOL WINAPI MI_NAME(TryEnterCriticalSection)(
        _Inout_ LPCRITICAL_SECTION CriticalSection
    )
    {
        PAGED_CODE();
        return RtlTryEnterCriticalSection(CriticalSection);
    }
    MI_IAT_SYMBOL(TryEnterCriticalSection, 4);

    //_IRQL_requires_max_(APC_LEVEL)
    //DWORD WINAPI MI_NAME(SetCriticalSectionSpinCount)(
    //    _Inout_ LPCRITICAL_SECTION CriticalSection,
    //    _In_ DWORD SpinCount
    //    )
    //{
    //    PAGED_CODE();
    //    return RtlSetCriticalSectionSpinCount(CriticalSection, SpinCount);
    //}
    //MI_IAT_SYMBOL(SetCriticalSectionSpinCount, 8);

    //
    // Condition variable
    //

    _IRQL_requires_max_(DISPATCH_LEVEL)
    VOID WINAPI MI_NAME(InitializeConditionVariable)(
        _Out_ PCONDITION_VARIABLE ConditionVariable
        )
    {
        RtlInitializeConditionVariable(ConditionVariable);
    }
    MI_IAT_SYMBOL(InitializeConditionVariable, 4);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(SleepConditionVariableCS)(
        _Inout_ PCONDITION_VARIABLE ConditionVariable,
        _Inout_ PCRITICAL_SECTION CriticalSection,
        _In_ DWORD Milliseconds
        )
    {
        PAGED_CODE();

        LARGE_INTEGER  Time;
        LARGE_INTEGER* Timeout = nullptr;

        if (Milliseconds != INFINITE) {
            Time.QuadPart = Int32x32To64(Milliseconds, -10000);
            Timeout = &Time;
        }

        const auto Status = RtlSleepConditionVariableCS(ConditionVariable, CriticalSection, Timeout);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MI_IAT_SYMBOL(SleepConditionVariableCS, 12);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(SleepConditionVariableSRW)(
        _Inout_ PCONDITION_VARIABLE ConditionVariable,
        _Inout_ PSRWLOCK SRWLock,
        _In_ DWORD Milliseconds,
        _In_ ULONG Flags
        )
    {
        PAGED_CODE();

        LARGE_INTEGER  Time;
        LARGE_INTEGER* Timeout = nullptr;

        if (Milliseconds != INFINITE) {
            Time.QuadPart = Int32x32To64(Milliseconds, -10000);
            Timeout = &Time;
        }

        const auto Status = RtlSleepConditionVariableSRW(ConditionVariable, SRWLock, Timeout, Flags);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MI_IAT_SYMBOL(SleepConditionVariableSRW, 16);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    VOID WINAPI MI_NAME(WakeConditionVariable)(
        _Inout_ PCONDITION_VARIABLE ConditionVariable
        )
    {
        PAGED_CODE();
        RtlWakeConditionVariable(ConditionVariable);
    }
    MI_IAT_SYMBOL(WakeConditionVariable, 4);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    VOID WINAPI MI_NAME(WakeAllConditionVariable)(
        _Inout_ PCONDITION_VARIABLE ConditionVariable
        )
    {
        PAGED_CODE();
        RtlWakeAllConditionVariable(ConditionVariable);
    }
    MI_IAT_SYMBOL(WakeAllConditionVariable, 4);


}
EXTERN_C_END
