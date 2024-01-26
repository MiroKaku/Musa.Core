#include "KernelBase.Private.h"


EXTERN_C_START
namespace Mi
{
    BOOL WINAPI MI_NAME(GetProcessTimes)(
        _In_  HANDLE     ProcessHandle,
        _Out_ LPFILETIME CreationTime,
        _Out_ LPFILETIME ExitTime,
        _Out_ LPFILETIME KernelTime,
        _Out_ LPFILETIME UserTime
        )
    {
        KERNEL_USER_TIMES TimeInfo{};

        const auto Status = ZwQueryInformationProcess(ProcessHandle, ProcessTimes,
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
        _In_ HANDLE ProcessHandle,
        _In_ UINT   ExitCode
        )
    {
        const auto Status = ZwTerminateProcess(ProcessHandle, ExitCode);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MI_IAT_SYMBOL(TerminateProcess, 8);

    BOOL WINAPI MI_NAME(GetExitCodeProcess)(
        _In_  HANDLE  ProcessHandle,
        _Out_ LPDWORD ExitCode
        )
    {
        PROCESS_BASIC_INFORMATION BasicInformation{};

        const auto Status = ZwQueryInformationProcess(ProcessHandle, ProcessBasicInformation,
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
        HANDLE Handle;

        if (ProcessId == 0 || ProcessId == GetCurrentProcessId()) {
            Handle = GetCurrentProcess();
        }
        else {
            auto ClientId         = CLIENT_ID { ULongToHandle(ProcessId) };
            auto ObjectAttributes = OBJECT_ATTRIBUTES RTL_CONSTANT_OBJECT_ATTRIBUTES(
                static_cast<PCUNICODE_STRING>(nullptr), OBJ_KERNEL_HANDLE);

            const auto Status = ZwOpenProcess(&Handle, PROCESS_QUERY_LIMITED_INFORMATION,
                &ObjectAttributes, &ClientId);
            if (!NT_SUCCESS(Status)) {
                BaseSetLastNTError(Status);
                return 0;
            }
        }

        SECTION_IMAGE_INFORMATION ProcessInformation{};
        const auto Status = ZwQueryInformationProcess(Handle, ProcessImageInformation,
            &ProcessInformation, sizeof(ProcessInformation), nullptr);

        if (Handle != GetCurrentProcess()) {
            CloseHandle(Handle);
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
        _In_ HANDLE ProcessHandle,
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

        const auto Status = ZwSetInformationProcess(ProcessHandle, ProcessPriorityClass,
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
        _In_ HANDLE ProcessHandle
        )
    {
        PROCESS_PRIORITY_CLASS PriorityInformation{};

        const auto Status = ZwQueryInformationProcess(ProcessHandle, ProcessPriorityClass,
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

        if (ProcessId == GetCurrentProcessId()) {
            *SessionId = ZwCurrentPeb()->SessionId;
            return TRUE;
        }

        NTSTATUS Status;
        HANDLE   Handle = nullptr;

        do {
            auto ClientId         = CLIENT_ID { ULongToHandle(ProcessId) };
            auto ObjectAttributes = OBJECT_ATTRIBUTES RTL_CONSTANT_OBJECT_ATTRIBUTES(
                static_cast<PCUNICODE_STRING>(nullptr), OBJ_KERNEL_HANDLE);

            Status = ZwOpenProcess(&Handle, PROCESS_QUERY_LIMITED_INFORMATION,
                &ObjectAttributes, &ClientId);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            PROCESS_SESSION_INFORMATION SessionInformation{};
            Status = ZwQueryInformationProcess(Handle, ProcessSessionInformation,
                &SessionInformation, sizeof(SessionInformation), nullptr);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            *SessionId = SessionInformation.SessionId;
        } while (false);

        if (Handle) {
            (void)ZwClose(Handle);
        }

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(ProcessIdToSessionId, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    DWORD WINAPI MI_NAME(GetProcessId)(
        _In_ HANDLE ProcessHandle
        )
    {
        PROCESS_BASIC_INFORMATION BasicInformation{};

        const auto Status = ZwQueryInformationProcess(ProcessHandle, ProcessBasicInformation,
            &BasicInformation, sizeof(BasicInformation), nullptr);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return 0;
        }

        return HandleToULong(BasicInformation.UniqueProcessId);
    }
    MI_IAT_SYMBOL(GetProcessId, 4);

    _IRQL_requires_max_(PASSIVE_LEVEL)
        BOOL WINAPI MI_NAME(FlushInstructionCache)(
            _In_ HANDLE ProcessHandle,
            _In_reads_bytes_opt_(Size) LPCVOID BaseAddress,
            _In_ SIZE_T Size
            )
    {
        const auto Status = ZwFlushInstructionCache(ProcessHandle, const_cast<PVOID>(BaseAddress), Size);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(FlushInstructionCache, 12);

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
        HANDLE    Handle   = nullptr;
        CLIENT_ID ClientId = { ULongToHandle(ProcessId) };

        auto ObjectAttributes = OBJECT_ATTRIBUTES RTL_CONSTANT_OBJECT_ATTRIBUTES(
            static_cast<PCUNICODE_STRING>(nullptr),
            OBJ_KERNEL_HANDLE | (InheritHandle ? OBJ_INHERIT : 0ul));

        const auto Status = ZwOpenProcess(&Handle, DesiredAccess, &ObjectAttributes, &ClientId);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return nullptr;
        }

        return Handle;
    }
    MI_IAT_SYMBOL(OpenProcess, 12);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MI_NAME(GetProcessHandleCount)(
        _In_  HANDLE ProcessHandle,
        _Out_ PDWORD HandleCount
        )
    {
        const auto Status = ZwQueryInformationProcess(ProcessHandle, ProcessHandleCount,
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
        _In_  HANDLE ProcessHandle,
        _Out_ PBOOL  DisablePriorityBoost
        )
    {
        DWORD DisableBoost;

        const auto Status = ZwQueryInformationProcess(ProcessHandle, ProcessPriorityBoost,
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
        _In_ HANDLE ProcessHandle,
        _In_ BOOL   DisablePriorityBoost
        )
    {
        ULONG DisableBoost = DisablePriorityBoost ? TRUE : FALSE;

        const auto Status = ZwSetInformationProcess(ProcessHandle, ProcessPriorityBoost,
            &DisableBoost, sizeof(DisableBoost));
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(SetProcessPriorityBoost, 8);

    BOOL WINAPI MI_NAME(IsProcessCritical)(
        _In_ HANDLE ProcessHandle,
        _Out_ PBOOL Critical
        )
    {
        ULONG Information = 0;
        const auto Status = ZwQueryInformationProcess(ProcessHandle, ProcessBreakOnTermination,
            &Information, sizeof(Information), nullptr);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        *Critical = !!Information;
        return TRUE;
    }
    MI_IAT_SYMBOL(IsProcessCritical, 0);

    BOOL WINAPI MI_NAME(SetProcessInformation)(
        _In_ HANDLE ProcessHandle,
        _In_ PROCESS_INFORMATION_CLASS ProcessInformationClass,
        _In_reads_bytes_(ProcessInformationSize) LPVOID ProcessInformation,
        _In_ DWORD ProcessInformationSize
        )
    {
        NTSTATUS Status = STATUS_SUCCESS;

        do {
            PROCESSINFOCLASS Class = ProcessPagePriority;

            switch (ProcessInformationClass) {
            case ProcessMemoryPriority:
            {
                Class = ProcessPagePriority;
                break;
            }
            case ProcessMemoryExhaustionInfo:
            {
                if (ProcessInformation == nullptr) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                if (ProcessInformationSize != sizeof(PROCESS_MEMORY_EXHAUSTION_INFO)) {
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                    break;
                }

                const auto Information = static_cast<PROCESS_MEMORY_EXHAUSTION_INFO*>(ProcessInformation);

                if (Information->Version  != PME_CURRENT_VERSION ||
                    Information->Reserved != 0u ||
                    Information->Type     >= PMETypeMax) {

                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                Class = ProcessMemoryExhaustion;
                break;
            }
            case ProcessInPrivateInfo:
            {
                if (ProcessInformation     != nullptr ||
                    ProcessInformationSize != 0) {

                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                Class = ProcessInPrivate;
                break;
            }
            case ProcessPowerThrottling:
            {
                if (ProcessInformation == nullptr) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                if (ProcessInformationSize != sizeof(PROCESS_POWER_THROTTLING_STATE)) {
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                    break;
                }

                Class = ProcessPowerThrottlingState;
                break;
            }
            case ProcessTelemetryCoverageInfo:
            {
                if (ProcessInformation == nullptr) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                if (ProcessInformationSize != sizeof(TELEMETRY_COVERAGE_POINT)) {
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                    break;
                }

                Class = ProcessTelemetryCoverage;
                break;
            }
            case ProcessLeapSecondInfo:
            {
                if (ProcessInformation == nullptr) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                if (ProcessInformationSize != sizeof(PROCESS_LEAP_SECOND_INFO)) {
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                    break;
                }

                const auto Information = static_cast<PROCESS_LEAP_SECOND_INFO*>(ProcessInformation);

                if (Information->Flags & ~PROCESS_LEAP_SECOND_INFO_VALID_FLAGS) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                Class = ProcessLeapSecondInformation;
                break;
            }
            default:
            {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }
            }

            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = ZwSetInformationProcess(ProcessHandle, Class,
                ProcessInformation, ProcessInformationSize);

        } while (false);

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(SetProcessInformation, 0);

    BOOL WINAPI MI_NAME(GetProcessInformation)(
        _In_ HANDLE ProcessHandle,
        _In_ PROCESS_INFORMATION_CLASS ProcessInformationClass,
        _Out_writes_bytes_(ProcessInformationSize) LPVOID ProcessInformation,
        _In_ DWORD ProcessInformationSize
        )
    {
        NTSTATUS Status = STATUS_SUCCESS;

        do {
            PROCESSINFOCLASS Class = ProcessPagePriority;

            switch (ProcessInformationClass) {
            case ProcessMemoryPriority:
            {
                Class = ProcessPagePriority;
                break;
            }
            case ProcessAppMemoryInfo:
            {
                // TODO:
                Status = STATUS_NOT_IMPLEMENTED;
                break;
            }
            case ProcessInPrivateInfo:
            {
                if (ProcessInformation == nullptr) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                if (ProcessInformationSize != sizeof(BOOLEAN)) {
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                    break;
                }

                Class = ProcessInPrivate;
                break;
            }
            case ProcessProtectionLevelInfo:
            {
                if (ProcessInformation == nullptr) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                if (ProcessInformationSize != sizeof(PROCESS_PROTECTION_LEVEL_INFORMATION)) {
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                    break;
                }

                Class = ProcessProtectionInformation;
                break;
            }
            case ProcessLeapSecondInfo:
            {
                if (ProcessInformation == nullptr) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                if (ProcessInformationSize != sizeof(PROCESS_LEAP_SECOND_INFO)) {
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                    break;
                }

                Class = ProcessLeapSecondInformation;
                break;
            }
            default:
            {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }
            }

            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = ZwQueryInformationProcess(ProcessHandle, Class,
                ProcessInformation, ProcessInformationSize, nullptr);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            if (ProcessInformationClass == ProcessProtectionLevelInfo) {
                auto& ProtectionLevel = static_cast<PROCESS_PROTECTION_LEVEL_INFORMATION*>(ProcessInformation)->ProtectionLevel;

                switch (ProtectionLevel) {
                case PsProtectedValue(PsProtectedSignerNone, FALSE, PsProtectedTypeNone):
                    ProtectionLevel = PROTECTION_LEVEL_NONE;
                    break;
                case PsProtectedValue(PsProtectedSignerWinTcb, FALSE, PsProtectedTypeProtectedLight):
                    ProtectionLevel = PROTECTION_LEVEL_WINTCB_LIGHT;
                    break;
                case PsProtectedValue(PsProtectedSignerWindows, FALSE, PsProtectedTypeProtected):
                    ProtectionLevel = PROTECTION_LEVEL_WINDOWS;
                    break;
                case PsProtectedValue(PsProtectedSignerWindows, FALSE, PsProtectedTypeProtectedLight):
                    ProtectionLevel = PROTECTION_LEVEL_WINDOWS_LIGHT;
                    break;
                case PsProtectedValue(PsProtectedSignerAntimalware, FALSE, PsProtectedTypeProtectedLight):
                    ProtectionLevel = PROTECTION_LEVEL_ANTIMALWARE_LIGHT;
                    break;
                case PsProtectedValue(PsProtectedSignerLsa, FALSE, PsProtectedTypeProtectedLight):
                    ProtectionLevel = PROTECTION_LEVEL_LSA_LIGHT;
                    break;
                case PsProtectedValue(PsProtectedSignerWinTcb,    FALSE, PsProtectedTypeProtected):
                case PsProtectedValue(PsProtectedSignerWinSystem, FALSE, PsProtectedTypeProtected):
                    ProtectionLevel = PROTECTION_LEVEL_WINTCB;
                    break;
                case PsProtectedValue(PsProtectedSignerCodeGen, FALSE, PsProtectedTypeProtectedLight):
                    ProtectionLevel = PROTECTION_LEVEL_CODEGEN_LIGHT;
                    break;
                case PsProtectedValue(PsProtectedSignerAuthenticode, FALSE, PsProtectedTypeProtected):
                    ProtectionLevel = PROTECTION_LEVEL_AUTHENTICODE;
                    break;
                case PsProtectedValue(PsProtectedSignerApp, FALSE, PsProtectedTypeProtectedLight):
                    ProtectionLevel = PROTECTION_LEVEL_PPL_APP;
                    break;
                default:
                    Status = STATUS_NOT_SUPPORTED;
                    break;
                }
            }

        } while (false);

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        return TRUE;
    }
    MI_IAT_SYMBOL(GetProcessInformation, 0);

}
EXTERN_C_END
