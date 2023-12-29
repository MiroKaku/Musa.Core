#include "KernelBase.Private.h"


EXTERN_C_START
namespace Mi
{
    BOOL WINAPI MI_NAME(GetProcessTimes)(
        _In_  HANDLE     Process,
        _Out_ LPFILETIME CreationTime,
        _Out_ LPFILETIME ExitTime,
        _Out_ LPFILETIME KernelTime,
        _Out_ LPFILETIME UserTime
        )
    {
        KERNEL_USER_TIMES TimeInfo{};

        const auto Status = ZwQueryInformationProcess(Process, ProcessTimes,
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
    MI_IAT_SYMBOL(GetProcessTimes, 20);

    HANDLE WINAPI MI_NAME(GetCurrentProcess)(
        VOID
        )
    {
        return ZwCurrentProcess();
    }
    MI_IAT_SYMBOL(GetCurrentProcess, 0);

    DWORD WINAPI MI_NAME(GetCurrentProcessId)(
        VOID
        )
    {
        return HandleToULong(ZwCurrentProcessId());
    }
    MI_IAT_SYMBOL(GetCurrentProcessId, 0);

    VOID WINAPI MI_NAME(ExitProcess)(
        _In_ UINT ExitCode
        )
    {
        __fastfail(ExitCode);
    }
    MI_IAT_SYMBOL(ExitProcess, 4);

    BOOL WINAPI MI_NAME(TerminateProcess)(
        _In_ HANDLE Process,
        _In_ UINT   ExitCode
        )
    {
        const auto Status = ZwTerminateProcess(Process, ExitCode);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MI_IAT_SYMBOL(TerminateProcess, 8);

    BOOL WINAPI MI_NAME(GetExitCodeProcess)(
        _In_  HANDLE  Process,
        _Out_ LPDWORD ExitCode
        )
    {
        PROCESS_BASIC_INFORMATION BasicInformation{};

        const auto Status = ZwQueryInformationProcess(Process, ProcessBasicInformation,
            &BasicInformation, sizeof(BasicInformation), nullptr);
        if (NT_SUCCESS(Status)) {
            *ExitCode = BasicInformation.ExitStatus;
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MI_IAT_SYMBOL(GetExitCodeProcess, 8);

    DWORD WINAPI MI_NAME(GetProcessVersion)(
        _In_ DWORD ProcessId
        )
    {
        HANDLE ProcessHandle;

        if (ProcessId == 0 || ProcessId == GetCurrentProcessId()) {
            ProcessHandle = GetCurrentProcess();
        }
        else {
            auto ClientId         = CLIENT_ID { ULongToHandle(ProcessId) };
            auto ObjectAttributes = OBJECT_ATTRIBUTES RTL_CONSTANT_OBJECT_ATTRIBUTES(
                static_cast<PCUNICODE_STRING>(nullptr), OBJ_KERNEL_HANDLE);

            const auto Status = ZwOpenProcess(&ProcessHandle, PROCESS_QUERY_LIMITED_INFORMATION,
                &ObjectAttributes, &ClientId);
            if (!NT_SUCCESS(Status)) {
                BaseSetLastNTError(Status);
                return 0;
            }
        }

        SECTION_IMAGE_INFORMATION ProcessInformation{};
        const auto Status = ZwQueryInformationProcess(ProcessHandle, ProcessImageInformation,
            &ProcessInformation, sizeof(ProcessInformation), nullptr);

        if (ProcessHandle != GetCurrentProcess()) {
            CloseHandle(ProcessHandle);
        }

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return 0;
        }

        return ProcessInformation.SubSystemVersion;
    }
    MI_IAT_SYMBOL(GetProcessVersion, 4);

    VOID WINAPI MI_NAME(GetStartupInfoW)(
        _Out_ LPSTARTUPINFOW StartupInfo
        )
    {
    #ifdef _KERNEL_MODE
        const auto ProcessParameters = MI_NAME_PRIVATE(RtlGetCurrentPeb)();

        * StartupInfo = { sizeof(*StartupInfo) };

        if (StartupInfo->dwFlags & (STARTF_USESTDHANDLES | STARTF_USEHOTKEY | STARTF_HASSHELLDATA)) {
            StartupInfo->hStdInput  = ProcessParameters->StandardInput;
            StartupInfo->hStdOutput = ProcessParameters->StandardOutput;
            StartupInfo->hStdError  = ProcessParameters->StandardError;
        }
    #else
        const auto ProcessParameters = NtCurrentPeb()->ProcessParameters;

        StartupInfo->cb              = sizeof(*StartupInfo);
        StartupInfo->lpReserved      = ProcessParameters->ShellInfo.Buffer;
        StartupInfo->lpDesktop       = ProcessParameters->DesktopInfo.Buffer;
        StartupInfo->lpTitle         = ProcessParameters->WindowTitle.Buffer;
        StartupInfo->dwX             = ProcessParameters->StartingX;
        StartupInfo->dwY             = ProcessParameters->StartingY;
        StartupInfo->dwXSize         = ProcessParameters->CountX;
        StartupInfo->dwYSize         = ProcessParameters->CountY;
        StartupInfo->dwXCountChars   = ProcessParameters->CountCharsX;
        StartupInfo->dwYCountChars   = ProcessParameters->CountCharsY;
        StartupInfo->dwFillAttribute = ProcessParameters->FillAttribute;
        StartupInfo->dwFlags         = ProcessParameters->WindowFlags;
        StartupInfo->wShowWindow     = static_cast<WORD>(ProcessParameters->ShowWindowFlags);
        StartupInfo->cbReserved2     = ProcessParameters->RuntimeData.Length;
        StartupInfo->lpReserved2     = reinterpret_cast<LPBYTE>(ProcessParameters->RuntimeData.Buffer);

        if (StartupInfo->dwFlags & (STARTF_USESTDHANDLES | STARTF_USEHOTKEY | STARTF_HASSHELLDATA)) {
            StartupInfo->hStdInput  = ProcessParameters->StandardInput;
            StartupInfo->hStdOutput = ProcessParameters->StandardOutput;
            StartupInfo->hStdError  = ProcessParameters->StandardError;
        }
    #endif
    }
    MI_IAT_SYMBOL(GetStartupInfoW, 4);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(OpenProcessToken)(
        _In_ HANDLE ProcessHandle,
        _In_ DWORD DesiredAccess,
        _Outptr_ PHANDLE TokenHandle
        )
    {
        const auto Status = ZwOpenProcessToken(ProcessHandle, DesiredAccess, TokenHandle);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(OpenProcessToken, 12);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(SetPriorityClass)(
        _In_ HANDLE Process,
        _In_ DWORD  PriorityClass
        )
    {
        PVOID State = nullptr;

        UCHAR Priority;
        if (PriorityClass & IDLE_PRIORITY_CLASS) {
            Priority = PROCESS_PRIORITY_CLASS_IDLE;
        }
        else if (PriorityClass & BELOW_NORMAL_PRIORITY_CLASS) {
            Priority = PROCESS_PRIORITY_CLASS_BELOW_NORMAL;
        }
        else if (PriorityClass & NORMAL_PRIORITY_CLASS) {
            Priority = PROCESS_PRIORITY_CLASS_NORMAL;
        }
        else if (PriorityClass & ABOVE_NORMAL_PRIORITY_CLASS) {
            Priority = PROCESS_PRIORITY_CLASS_ABOVE_NORMAL;
        }
        else if (PriorityClass & HIGH_PRIORITY_CLASS) {
            Priority = PROCESS_PRIORITY_CLASS_HIGH;
        }
        else if (PriorityClass & REALTIME_PRIORITY_CLASS) {
            State = BaseIsRealtimeAllowed(TRUE, FALSE);
            if (State) {
                Priority = PROCESS_PRIORITY_CLASS_REALTIME;
            }
            else {
                Priority = PROCESS_PRIORITY_CLASS_HIGH;
            }
        }
        else {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return FALSE;
        }

        PROCESS_PRIORITY_CLASS PriorityInformation = { FALSE, Priority };

        const auto Status = ZwSetInformationProcess(Process, ProcessPriorityClass,
            &PriorityInformation, sizeof(PriorityInformation));

        if (State) {
            RtlReleasePrivilege(State);
        }

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(SetPriorityClass, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(GetPriorityClass)(
        _In_ HANDLE Process
        )
    {
        PROCESS_PRIORITY_CLASS PriorityInformation{};

        const auto Status = ZwQueryInformationProcess(Process, ProcessPriorityClass,
            &PriorityInformation, sizeof(PriorityInformation), nullptr);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        switch (PriorityInformation.PriorityClass) {
            case PROCESS_PRIORITY_CLASS_IDLE:
                return IDLE_PRIORITY_CLASS;

            case PROCESS_PRIORITY_CLASS_NORMAL:
                return NORMAL_PRIORITY_CLASS;

            case PROCESS_PRIORITY_CLASS_HIGH:
                return HIGH_PRIORITY_CLASS;

            case PROCESS_PRIORITY_CLASS_REALTIME:
                return REALTIME_PRIORITY_CLASS;

            case PROCESS_PRIORITY_CLASS_BELOW_NORMAL:
                return BELOW_NORMAL_PRIORITY_CLASS;

            case PROCESS_PRIORITY_CLASS_ABOVE_NORMAL:
                return ABOVE_NORMAL_PRIORITY_CLASS;

            default:
                return NORMAL_PRIORITY_CLASS;
        }
    }
    MI_IAT_SYMBOL(GetPriorityClass, 4);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(ProcessIdToSessionId)(
        _In_  DWORD  ProcessId,
        _Out_ DWORD* SessionId
        )
    {
        if (SessionId == nullptr) {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return FALSE;
        }

    #ifdef _KERNEL_MODE
        // TODO:
        PsGetProcessSessionId();
    #else

        if (ProcessId == GetCurrentProcessId()) {
            *SessionId = ZwCurrentPeb()->SessionId;
            return TRUE;
        }

        NTSTATUS Status;
        HANDLE   ProcessHandle = nullptr;

        do {
            auto ClientId         = CLIENT_ID { ULongToHandle(ProcessId) };
            auto ObjectAttributes = OBJECT_ATTRIBUTES RTL_CONSTANT_OBJECT_ATTRIBUTES(
                static_cast<PCUNICODE_STRING>(nullptr), OBJ_KERNEL_HANDLE);

            Status = ZwOpenProcess(&ProcessHandle, PROCESS_QUERY_LIMITED_INFORMATION,
                &ObjectAttributes, &ClientId);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            PROCESS_SESSION_INFORMATION SessionInformation{};
            Status = ZwQueryInformationProcess(ProcessHandle, ProcessSessionInformation,
                &SessionInformation, sizeof(SessionInformation), nullptr);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            *SessionId = SessionInformation.SessionId;
        } while (false);

        if (ProcessHandle) {
            (void)ZwClose(ProcessHandle);
        }

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    #endif
    }
    MI_IAT_SYMBOL(ProcessIdToSessionId, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    DWORD WINAPI MI_NAME(GetProcessId)(
        _In_ HANDLE Process
        )
    {
        PROCESS_BASIC_INFORMATION BasicInformation{};

        const auto Status = ZwQueryInformationProcess(Process, ProcessBasicInformation,
            &BasicInformation, sizeof(BasicInformation), nullptr);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return 0;
        }

        return HandleToULong(BasicInformation.UniqueProcessId);
    }
    MI_IAT_SYMBOL(GetProcessId, 4);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    VOID WINAPI MI_NAME(FlushProcessWriteBuffers)(
        VOID
        )
    {
        const auto Status = ZwFlushProcessWriteBuffers();
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
        }
    }
    MI_IAT_SYMBOL(FlushProcessWriteBuffers, 0);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    DWORD WINAPI MI_NAME(GetProcessIdOfThread)(
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

        return HandleToULong(BasicInformation.ClientId.UniqueProcess);
    }
    MI_IAT_SYMBOL(GetProcessIdOfThread, 4);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    HANDLE WINAPI MI_NAME(OpenProcess)(
        _In_ DWORD DesiredAccess,
        _In_ BOOL  InheritHandle,
        _In_ DWORD ProcessId
        )
    {
        HANDLE    Process  = nullptr;
        CLIENT_ID ClientId = { ULongToHandle(ProcessId) };

        auto ObjectAttributes = OBJECT_ATTRIBUTES RTL_CONSTANT_OBJECT_ATTRIBUTES(
            static_cast<PCUNICODE_STRING>(nullptr),
            OBJ_KERNEL_HANDLE | (InheritHandle ? OBJ_INHERIT : 0ul));

        const auto Status = ZwOpenProcess(&Process, DesiredAccess, &ObjectAttributes, &ClientId);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return nullptr;
        }

        return Process;
    }
    MI_IAT_SYMBOL(OpenProcess, 12);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(GetProcessHandleCount)(
        _In_  HANDLE Process,
        _Out_ PDWORD HandleCount
        )
    {
        const auto Status = ZwQueryInformationProcess(Process, ProcessHandleCount,
            HandleCount, sizeof(*HandleCount), nullptr);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(GetProcessHandleCount, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(GetProcessPriorityBoost)(
        _In_  HANDLE Process,
        _Out_ PBOOL  DisablePriorityBoost
        )
    {
        DWORD DisableBoost;

        const auto Status = ZwQueryInformationProcess(Process, ProcessPriorityBoost,
            &DisableBoost, sizeof(DisableBoost), nullptr);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        *DisablePriorityBoost = DisableBoost;

        return TRUE;
    }
    MI_IAT_SYMBOL(GetProcessPriorityBoost, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(SetProcessPriorityBoost)(
        _In_ HANDLE Process,
        _In_ BOOL   DisablePriorityBoost
        )
    {
        ULONG DisableBoost = DisablePriorityBoost ? TRUE : FALSE;

        const auto Status = ZwSetInformationProcess(Process, ProcessPriorityBoost,
            &DisableBoost, sizeof(DisableBoost));
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(SetProcessPriorityBoost, 8);

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
