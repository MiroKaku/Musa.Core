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
        HANDLE    Thread   = nullptr;
        CLIENT_ID ClientId = { nullptr, ULongToHandle(ThreadId) };

        auto ObjectAttributes = OBJECT_ATTRIBUTES RTL_CONSTANT_OBJECT_ATTRIBUTES(
            static_cast<PCUNICODE_STRING>(nullptr),
            OBJ_KERNEL_HANDLE | (InheritHandle ? OBJ_INHERIT : 0ul));

        const auto Status = ZwOpenThread(&Thread, DesiredAccess, &ObjectAttributes, &ClientId);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return nullptr;
        }

        return Thread;
    }
    MI_IAT_SYMBOL(OpenThread, 12);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(SetThreadPriority)(
        _In_ HANDLE Thread,
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

        const auto Status = ZwSetInformationThread(Thread, ThreadBasePriority,
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
        _In_ HANDLE Thread
        )
    {
        THREAD_BASIC_INFORMATION BasicInfo{};

        const auto Status = ZwQueryInformationThread(Thread, ThreadBasicInformation,
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
        _In_ HANDLE Thread,
        _In_ BOOL   DisablePriorityBoost
        )
    {
        ULONG DisableBoost = DisablePriorityBoost ? 1 : 0;

        const auto Status = ZwSetInformationThread(Thread,ThreadPriorityBoost,
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
        _In_ HANDLE Thread,
        _Out_ PBOOL DisablePriorityBoost
        )
    {
        DWORD DisableBoost = FALSE;

        const auto Status = ZwQueryInformationThread(Thread, ThreadPriorityBoost,
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
        _In_ HANDLE Thread,
        _In_ DWORD  ExitCode
        )
    {
        const auto Status = ZwTerminateThread(Thread, ExitCode);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(TerminateThread, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(GetExitCodeThread)(
        _In_  HANDLE  Thread,
        _Out_ LPDWORD ExitCode
        )
    {
        THREAD_BASIC_INFORMATION BasicInformation{};

        const auto Status = ZwQueryInformationThread(Thread, ThreadBasicInformation,
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
        _In_ HANDLE Thread
        )
    {
        DWORD PreviousSuspendCount = 0;

        const auto Status = ZwSuspendThread(Thread, &PreviousSuspendCount);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return static_cast<DWORD>(-1);
        }

        return PreviousSuspendCount;
    }
    MI_IAT_SYMBOL(SuspendThread, 0);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    DWORD WINAPI MI_NAME(ResumeThread)(
        _In_ HANDLE Thread
        )
    {
        DWORD PreviousSuspendCount = 0;

        const auto Status = ZwResumeThread(Thread, &PreviousSuspendCount);
        if (!NT_SUCCESS(Status)) {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            return static_cast<DWORD>(-1);
        }

        return PreviousSuspendCount;
    }
    MI_IAT_SYMBOL(ResumeThread, 0);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(SetThreadToken)(
        _In_opt_ PHANDLE Thread,
        _In_opt_ HANDLE  Token
        )
    {
        HANDLE ThreadHandle = GetCurrentThread();
        if (Thread) {
            ThreadHandle = *Thread;
        }

        const auto Status = ZwSetInformationThread(ThreadHandle, ThreadImpersonationToken,
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
        _In_ HANDLE Thread
        )
    {
        THREAD_BASIC_INFORMATION BasicInformation{};

        const auto Status = ZwQueryInformationThread(Thread, ThreadBasicInformation,
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
        _In_    HANDLE    Thread,
        _Inout_ LPCONTEXT Context
        )
    {
        const auto Status = ZwGetContextThread(Thread, Context);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(GetThreadContext, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(SetThreadContext)(
        _In_ HANDLE Thread,
        _In_ CONST CONTEXT* Context
        )
    {
        const auto Status = ZwSetContextThread(Thread, const_cast<PCONTEXT>(Context));
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(SetThreadContext, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(FlushInstructionCache)(
        _In_ HANDLE Process,
        _In_reads_bytes_opt_(Size) LPCVOID BaseAddress,
        _In_ SIZE_T Size
        )
    {
        const auto Status = ZwFlushInstructionCache(Process, const_cast<PVOID>(BaseAddress), Size);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(FlushInstructionCache, 12);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(GetThreadTimes)(
        _In_  HANDLE Thread,
        _Out_ LPFILETIME CreationTime,
        _Out_ LPFILETIME ExitTime,
        _Out_ LPFILETIME KernelTime,
        _Out_ LPFILETIME UserTime
        )
    {
        KERNEL_USER_TIMES TimeInfo{};

        const auto Status = ZwQueryInformationThread(Thread, ThreadTimes,
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
        _In_ HANDLE Thread,
        _In_ DWORD  IdealProcessor
        )
    {
        const auto Status = ZwSetInformationThread(Thread, ThreadIdealProcessor,
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
        _In_ HANDLE Thread,
        _In_ PPROCESSOR_NUMBER IdealProcessor,
        _Out_opt_ PPROCESSOR_NUMBER PreviousIdealProcessor
        )
    {
        PROCESSOR_NUMBER Information = *IdealProcessor;

        const auto Status = ZwSetInformationThread(Thread, ThreadIdealProcessorEx,
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
        _In_ HANDLE Thread,
        _In_ PPROCESSOR_NUMBER IdealProcessor
        )
    {
        const auto Status = ZwQueryInformationThread(Thread, ThreadIdealProcessorEx,
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
        _In_ HANDLE Thread,
        _Out_ PBOOL IOIsPending
        )
    {
        ULONG Pending;

        const auto Status = ZwQueryInformationThread(Thread, ThreadIsIoPending,
            &Pending, sizeof(Pending), nullptr);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        *IOIsPending = (Pending ? TRUE : FALSE);

        return TRUE;
    }
    MI_IAT_SYMBOL(GetThreadIOPendingFlag, 8);

    BOOL WINAPI MI_NAME(Template)(
        VOID
        )
    {}
    MI_IAT_SYMBOL(Template, 0);

    BOOL WINAPI MI_NAME(Template)(
        VOID
        )
    {}
    MI_IAT_SYMBOL(Template, 0);

    BOOL WINAPI MI_NAME(Template)(
        VOID
        )
    {}
    MI_IAT_SYMBOL(Template, 0);

    BOOL WINAPI MI_NAME(Template)(
        VOID
        )
    {}
    MI_IAT_SYMBOL(Template, 0);

    BOOL WINAPI MI_NAME(Template)(
        VOID
        )
    {}
    MI_IAT_SYMBOL(Template, 0);

}
EXTERN_C_END
