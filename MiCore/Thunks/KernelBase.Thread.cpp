#include "KernelBase.Private.h"


EXTERN_C_START
namespace Mi
{
    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(SwitchToThread)(
        VOID
        )
    {
        if (ZwYieldExecution() != STATUS_NO_YIELD_PERFORMED) {
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(SwitchToThread, 0);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    HANDLE WINAPI MI_NAME(CreateThread)(
        _In_opt_ LPSECURITY_ATTRIBUTES ThreadAttributes,
        _In_ SIZE_T StackSize,
        _In_ LPTHREAD_START_ROUTINE StartAddress,
        _In_opt_ __drv_aliasesMem LPVOID Parameter,
        _In_ DWORD CreationFlags,
        _Out_opt_ LPDWORD ThreadId
        )
    {
        return CreateRemoteThreadEx(GetCurrentProcess(), ThreadAttributes, StackSize,
            StartAddress, Parameter, CreationFlags & (STACK_SIZE_PARAM_IS_A_RESERVATION | CREATE_SUSPENDED),
            nullptr, ThreadId);
    }
    MI_IAT_SYMBOL(CreateThread, 24);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    HANDLE WINAPI MI_NAME(CreateRemoteThread)(
        _In_ HANDLE Process,
        _In_opt_ LPSECURITY_ATTRIBUTES ThreadAttributes,
        _In_ SIZE_T StackSize,
        _In_ LPTHREAD_START_ROUTINE StartAddress,
        _In_opt_ LPVOID Parameter,
        _In_ DWORD CreationFlags,
        _Out_opt_ LPDWORD ThreadId
        )
    {
        return CreateRemoteThreadEx(Process, ThreadAttributes, StackSize,
            StartAddress, Parameter, CreationFlags & (STACK_SIZE_PARAM_IS_A_RESERVATION | CREATE_SUSPENDED),
            nullptr, ThreadId);
    }
    MI_IAT_SYMBOL(CreateRemoteThread, 28);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    HANDLE WINAPI MI_NAME(CreateRemoteThreadEx)(
        _In_ HANDLE Process,
        _In_opt_ LPSECURITY_ATTRIBUTES ThreadAttributes,
        _In_ SIZE_T StackSize,
        _In_ LPTHREAD_START_ROUTINE StartAddress,
        _In_opt_ LPVOID Parameter,
        _In_ DWORD CreationFlags,
        _In_opt_ LPPROC_THREAD_ATTRIBUTE_LIST AttributeList,
        _Out_opt_ LPDWORD ThreadId
        )
    {
        // TODO:

        NtCreateThreadEx













    }
    MI_IAT_SYMBOL(CreateRemoteThreadEx, 32);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    VOID WINAPI MI_NAME(ExitThread)(
        _In_ DWORD ExitCode
        )
    {
        return RtlExitUserThread(ExitCode);
    }
    MI_IAT_SYMBOL(ExitThread, 4);

    HANDLE WINAPI MI_NAME(GetCurrentThread)(
        VOID
        )
    {
        return ZwCurrentThread();
    }
    MI_IAT_SYMBOL(GetCurrentThread, 0);

    DWORD WINAPI MI_NAME(GetCurrentThreadId)(
        VOID
        )
    {
        return HandleToULong(ZwCurrentThreadId());
    }
    MI_IAT_SYMBOL(GetCurrentThreadId, 0);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    HANDLE WINAPI MI_NAME(OpenThread)(
        _In_ DWORD DesiredAccess,
        _In_ BOOL  InheritHandle,
        _In_ DWORD ThreadId
        )
    {
        HANDLE    Handle   = nullptr;
        CLIENT_ID ClientId = { nullptr, ULongToHandle(ThreadId) };

        auto ObjectAttributes = OBJECT_ATTRIBUTES RTL_CONSTANT_OBJECT_ATTRIBUTES(
            static_cast<PCUNICODE_STRING>(nullptr),
            OBJ_KERNEL_HANDLE | (InheritHandle ? OBJ_INHERIT : 0ul));

        const auto Status = ZwOpenThread(&Handle, DesiredAccess, &ObjectAttributes, &ClientId);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return nullptr;
        }

        return Handle;
    }
    MI_IAT_SYMBOL(OpenThread, 12);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(SetThreadPriority)(
        _In_ HANDLE ThreadHandle,
        _In_ int    Priority
        )
    {
        //
        // saturation is indicated by calling with a value of 16 or -16
        //

        if (Priority == THREAD_PRIORITY_TIME_CRITICAL) {
            Priority = ((HIGH_PRIORITY + 1) / 2);
        }
        else if (Priority == THREAD_PRIORITY_IDLE) {
            Priority = -((HIGH_PRIORITY + 1) / 2);
        }

        const auto Status = ZwSetInformationThread(ThreadHandle, ThreadBasePriority,
            &Priority, sizeof(Priority));
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(SetThreadPriority, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    int WINAPI MI_NAME(GetThreadPriority)(
        _In_ HANDLE ThreadHandle
        )
    {
        THREAD_BASIC_INFORMATION BasicInfo{};

        const auto Status = ZwQueryInformationThread(ThreadHandle, ThreadBasicInformation,
            &BasicInfo, sizeof(BasicInfo), nullptr);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return THREAD_PRIORITY_ERROR_RETURN;
        }

        auto Priority = static_cast<int>(BasicInfo.BasePriority);
        if (Priority == ((HIGH_PRIORITY + 1) / 2)) {
            Priority = THREAD_PRIORITY_TIME_CRITICAL;
        }
        else if (Priority == -((HIGH_PRIORITY + 1) / 2)) {
            Priority = THREAD_PRIORITY_IDLE;
        }
        return Priority;
    }
    MI_IAT_SYMBOL(GetThreadPriority, 4);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(SetThreadPriorityBoost)(
        _In_ HANDLE ThreadHandle,
        _In_ BOOL   DisablePriorityBoost
        )
    {
        ULONG DisableBoost = DisablePriorityBoost ? 1 : 0;

        const auto Status = ZwSetInformationThread(ThreadHandle,ThreadPriorityBoost,
            &DisableBoost, sizeof(DisableBoost));
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(SetThreadPriorityBoost, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(GetThreadPriorityBoost)(
        _In_ HANDLE ThreadHandle,
        _Out_ PBOOL DisablePriorityBoost
        )
    {
        DWORD DisableBoost = FALSE;

        const auto Status = ZwQueryInformationThread(ThreadHandle, ThreadPriorityBoost,
            &DisableBoost, sizeof(DisableBoost), nullptr);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return FALSE;
        }

        *DisablePriorityBoost = DisableBoost;
        return TRUE;
    }
    MI_IAT_SYMBOL(GetThreadPriorityBoost, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(TerminateThread)(
        _In_ HANDLE ThreadHandle,
        _In_ DWORD  ExitCode
        )
    {
        const auto Status = ZwTerminateThread(ThreadHandle, ExitCode);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(TerminateThread, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(GetExitCodeThread)(
        _In_  HANDLE  ThreadHandle,
        _Out_ LPDWORD ExitCode
        )
    {
        THREAD_BASIC_INFORMATION BasicInformation{};

        const auto Status = ZwQueryInformationThread(ThreadHandle, ThreadBasicInformation,
            &BasicInformation, sizeof(BasicInformation), nullptr);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return FALSE;
        }

        *ExitCode = BasicInformation.ExitStatus;
        return TRUE;
    }
    MI_IAT_SYMBOL(GetExitCodeThread, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    DWORD WINAPI MI_NAME(SuspendThread)(
        _In_ HANDLE ThreadHandle
        )
    {
        DWORD PreviousSuspendCount = 0;

        const auto Status = ZwSuspendThread(ThreadHandle, &PreviousSuspendCount);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return static_cast<DWORD>(-1);
        }

        return PreviousSuspendCount;
    }
    MI_IAT_SYMBOL(SuspendThread, 0);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    DWORD WINAPI MI_NAME(ResumeThread)(
        _In_ HANDLE ThreadHandle
        )
    {
        DWORD PreviousSuspendCount = 0;

        const auto Status = ZwResumeThread(ThreadHandle, &PreviousSuspendCount);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return static_cast<DWORD>(-1);
        }

        return PreviousSuspendCount;
    }
    MI_IAT_SYMBOL(ResumeThread, 0);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(SetThreadToken)(
        _In_opt_ PHANDLE ThreadHandle,
        _In_opt_ HANDLE  Token
        )
    {
        HANDLE Handle = GetCurrentThread();
        if (ThreadHandle) {
            Handle = *ThreadHandle;
        }

        const auto Status = ZwSetInformationThread(Handle, ThreadImpersonationToken,
            &Token, sizeof(Token));
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(SetThreadToken, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(OpenThreadToken)(
        _In_ HANDLE ThreadHandle,
        _In_ DWORD DesiredAccess,
        _In_ BOOL OpenAsSelf,
        _Outptr_ PHANDLE TokenHandle
        )
    {
        const auto Status = ZwOpenThreadToken(ThreadHandle, DesiredAccess, OpenAsSelf, TokenHandle);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(OpenThreadToken, 16);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    DWORD WINAPI MI_NAME(GetThreadId)(
        _In_ HANDLE ThreadHandle
        )
    {
        THREAD_BASIC_INFORMATION BasicInformation{};

        const auto Status = ZwQueryInformationThread(ThreadHandle, ThreadBasicInformation,
            &BasicInformation, sizeof(BasicInformation), nullptr);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return 0;
        }

        return HandleToULong(BasicInformation.ClientId.UniqueThread);
    }
    MI_IAT_SYMBOL(GetThreadId, 4);

    _Success_(return != FALSE)
    BOOL WINAPI MI_NAME(InitializeProcThreadAttributeList)(
        _Out_writes_bytes_to_opt_(*lpSize, *lpSize) LPPROC_THREAD_ATTRIBUTE_LIST AttributeList,
        _In_ DWORD AttributeCount,
        _Reserved_ DWORD Flags,
        _When_(AttributeList == nullptr, _Out_) _When_(AttributeList != nullptr, _Inout_) PSIZE_T Size
        )
    {
        if (Flags) {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER_3);
            return FALSE;
        }

        if (AttributeCount > 30 /* _countof(PROC_THREAD_ATTRIBUTE_NUM) */ ) {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER_2);
            return FALSE;
        }

        BOOL Result;

        const auto TotalSize = sizeof(PROC_THREAD_ATTRIBUTE_LIST) + sizeof(PROC_THREAD_ATTRIBUTE) * AttributeCount;
        if (AttributeList && *Size >= TotalSize) {

            *AttributeList = {0, AttributeCount};
            Result = TRUE;
        }
        else {
            BaseSetLastNTError(STATUS_BUFFER_TOO_SMALL);
            Result = FALSE;
        }

        *Size = TotalSize;

        return Result;
    }
    MI_IAT_SYMBOL(InitializeProcThreadAttributeList, 16);

    VOID WINAPI MI_NAME(DeleteProcThreadAttributeList)(
        _Inout_ LPPROC_THREAD_ATTRIBUTE_LIST AttributeList
        )
    {
        return;
    }
    MI_IAT_SYMBOL(DeleteProcThreadAttributeList, 4);

    _IRQL_requires_max_(APC_LEVEL)
    VOID WINAPI MI_NAME(GetCurrentThreadStackLimits)(
        _Out_ PULONG_PTR LowLimit,
        _Out_ PULONG_PTR HighLimit
        )
    {
    #ifdef _KERNEL_MODE
        IoGetStackLimits(LowLimit, HighLimit);
    #else
        const auto Teb = ZwCurrentTeb();
        *LowLimit  = reinterpret_cast<ULONG_PTR>(Teb->DeallocationStack);
        *HighLimit = reinterpret_cast<ULONG_PTR>(Teb->NtTib.StackBase);
    #endif
    }
    MI_IAT_SYMBOL(GetCurrentThreadStackLimits, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(GetThreadContext)(
        _In_    HANDLE    ThreadHandle,
        _Inout_ LPCONTEXT Context
        )
    {
        const auto Status = ZwGetContextThread(ThreadHandle, Context);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(GetThreadContext, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(SetThreadContext)(
        _In_ HANDLE ThreadHandle,
        _In_ CONST CONTEXT* Context
        )
    {
        const auto Status = ZwSetContextThread(ThreadHandle, const_cast<PCONTEXT>(Context));
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(SetThreadContext, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(GetThreadTimes)(
        _In_  HANDLE ThreadHandle,
        _Out_ LPFILETIME CreationTime,
        _Out_ LPFILETIME ExitTime,
        _Out_ LPFILETIME KernelTime,
        _Out_ LPFILETIME UserTime
        )
    {
        KERNEL_USER_TIMES TimeInfo{};

        const auto Status = ZwQueryInformationThread(ThreadHandle, ThreadTimes,
            &TimeInfo, sizeof(TimeInfo), nullptr);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        *CreationTime = *reinterpret_cast<LPFILETIME>(&TimeInfo.CreateTime);
        *ExitTime     = *reinterpret_cast<LPFILETIME>(&TimeInfo.ExitTime);
        *KernelTime   = *reinterpret_cast<LPFILETIME>(&TimeInfo.KernelTime);
        *UserTime     = *reinterpret_cast<LPFILETIME>(&TimeInfo.UserTime);

        return TRUE;
    }
    MI_IAT_SYMBOL(GetThreadTimes, 20);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    DWORD WINAPI MI_NAME(SetThreadIdealProcessor)(
        _In_ HANDLE ThreadHandle,
        _In_ DWORD  IdealProcessor
        )
    {
        const auto Status = ZwSetInformationThread(ThreadHandle, ThreadIdealProcessor,
            &IdealProcessor, sizeof(IdealProcessor));
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return static_cast<DWORD>(-1);
        }

        return static_cast<ULONG>(Status);
    }
    MI_IAT_SYMBOL(SetThreadIdealProcessor, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(SetThreadIdealProcessorEx)(
        _In_ HANDLE ThreadHandle,
        _In_ PPROCESSOR_NUMBER IdealProcessor,
        _Out_opt_ PPROCESSOR_NUMBER PreviousIdealProcessor
        )
    {
        PROCESSOR_NUMBER Information = *IdealProcessor;

        const auto Status = ZwSetInformationThread(ThreadHandle, ThreadIdealProcessorEx,
            &Information, sizeof(Information));
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        if (PreviousIdealProcessor) {
            *PreviousIdealProcessor = Information;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(SetThreadIdealProcessorEx, 12);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(GetThreadIdealProcessorEx)(
        _In_ HANDLE ThreadHandle,
        _In_ PPROCESSOR_NUMBER IdealProcessor
        )
    {
        const auto Status = ZwQueryInformationThread(ThreadHandle, ThreadIdealProcessorEx,
            IdealProcessor, sizeof(*IdealProcessor), nullptr);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(GetThreadIdealProcessorEx, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(GetThreadIOPendingFlag)(
        _In_ HANDLE ThreadHandle,
        _Out_ PBOOL IOIsPending
        )
    {
        ULONG Pending;

        const auto Status = ZwQueryInformationThread(ThreadHandle, ThreadIsIoPending,
            &Pending, sizeof(Pending), nullptr);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        *IOIsPending = (Pending ? TRUE : FALSE);

        return TRUE;
    }
    MI_IAT_SYMBOL(GetThreadIOPendingFlag, 8);

    BOOL WINAPI MI_NAME(GetThreadInformation)(
        _In_ HANDLE ThreadHandle,
        _In_ THREAD_INFORMATION_CLASS ThreadInformationClass,
        _Out_writes_bytes_(ThreadInformationSize) LPVOID ThreadInformation,
        _In_ DWORD ThreadInformationSize
        )
    {
        NTSTATUS Status = STATUS_SUCCESS;

        do {

            THREADINFOCLASS Class = ThreadPagePriority;

            switch (ThreadInformationClass) {
            case ThreadMemoryPriority:
                Class = ThreadPagePriority;
                break;
            case ThreadAbsoluteCpuPriority:
                Class = ThreadActualBasePriority;
                break;
            case ThreadDynamicCodePolicy:
                Class = ThreadDynamicCodePolicyInfo;
                break;
            case ThreadPowerThrottling:
            {
                if (ThreadInformationSize != sizeof(THREAD_POWER_THROTTLING_STATE)) {
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                    break;
                }

                const auto PowerThrottling = static_cast<THREAD_POWER_THROTTLING_STATE*>(ThreadInformation);
                if (PowerThrottling->Version != THREAD_POWER_THROTTLING_CURRENT_VERSION) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                Class = ThreadPowerThrottlingState;
                break;
            }
            default:
                break;
            }

            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = ZwQueryInformationThread(ThreadHandle, Class,
                ThreadInformation, ThreadInformationSize, nullptr);

        } while (false);

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(GetThreadInformation, 0);

    BOOL WINAPI MI_NAME(SetThreadInformation)(
        _In_ HANDLE ThreadHandle,
        _In_ THREAD_INFORMATION_CLASS ThreadInformationClass,
        _In_reads_bytes_(ThreadInformationSize) LPVOID ThreadInformation,
        _In_ DWORD ThreadInformationSize
        )
    {
        NTSTATUS Status = STATUS_SUCCESS;

        do {
            THREADINFOCLASS Class = ThreadPagePriority;

            switch (ThreadInformationClass) {
            case ThreadMemoryPriority:
                Class = ThreadPagePriority;
                break;
            case ThreadAbsoluteCpuPriority:
                Class = ThreadActualBasePriority;
                break;
            case ThreadDynamicCodePolicy:
                Class = ThreadDynamicCodePolicyInfo;
                break;
            case ThreadPowerThrottling:
            {
                if (ThreadInformationSize < sizeof(THREAD_POWER_THROTTLING_STATE)) {
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                    break;
                }

                const auto PowerThrottling = static_cast<THREAD_POWER_THROTTLING_STATE*>(ThreadInformation);

                if (PowerThrottling->Version != THREAD_POWER_THROTTLING_CURRENT_VERSION) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                if (PowerThrottling->ControlMask & ~THREAD_POWER_THROTTLING_VALID_FLAGS) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                if (~PowerThrottling->ControlMask & PowerThrottling->StateMask) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                Class = ThreadPowerThrottlingState;
                break;
            }
            default:
                break;
            }

            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = ZwSetInformationThread(ThreadHandle, Class, ThreadInformation, ThreadInformationSize);

        } while (false);

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(SetThreadInformation, 0);

    HRESULT WINAPI MI_NAME(SetThreadDescription)(
        _In_ HANDLE ThreadHandle,
        _In_ PCWSTR ThreadDescription
        )
    {
        THREAD_NAME_INFORMATION Information{};
        auto Status = RtlInitUnicodeStringEx(&Information.ThreadName, ThreadDescription);
        if (NT_SUCCESS(Status)) {
            Status = ZwSetInformationThread(ThreadHandle, ThreadNameInformation,
                &Information, sizeof(Information));
        }

        return HRESULT_FROM_NT(Status);
    }
    MI_IAT_SYMBOL(SetThreadDescription, 0);

    HRESULT WINAPI MI_NAME(GetThreadDescription)(
        _In_ HANDLE ThreadHandle,
        _Outptr_result_z_ PWSTR* ThreadDescription
        )
    {
        NTSTATUS Status;

        do {
            PTHREAD_NAME_INFORMATION Information;
            ULONG Length = sizeof(THREAD_NAME_INFORMATION) + 128;

            for (;;Length += sizeof(UNICODE_NULL)) {

                Information = static_cast<PTHREAD_NAME_INFORMATION>(
                    RtlAllocateHeap(GetProcessHeap(), 0, Length));
                if (Information == nullptr) {
                    Status = STATUS_NO_MEMORY;
                    break;
                }

                Status = ZwQueryInformationThread(ThreadHandle, ThreadNameInformation,
                    Information, Length, &Length);
                if (Status != STATUS_INFO_LENGTH_MISMATCH &&
                    Status != STATUS_BUFFER_TOO_SMALL     &&
                    Status != STATUS_BUFFER_OVERFLOW) {

                    break;
                }

                RtlFreeHeap(GetProcessHeap(), 0, Information);
            }

            if (!NT_SUCCESS(Status)) {
                RtlFreeHeap(GetProcessHeap(), 0, Information);
                break;
            }

            Length = Information->ThreadName.Length;
            RtlMoveMemory(Information, Information->ThreadName.Buffer, Information->ThreadName.Length);

            *ThreadDescription = reinterpret_cast<PWSTR>(Information);
            *ThreadDescription[Length >> 1] = UNICODE_NULL;

        } while (false);

        return HRESULT_FROM_NT(Status);
    }
    MI_IAT_SYMBOL(GetThreadDescription, 0);

}
EXTERN_C_END
