#include "Musa.Core/Musa.Core.SystemEnvironmentBlock.Process.h"
#include "KernelBase.Private.h"
#include "Internal/KernelBase.Process.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(GetProcessTimes))
#pragma alloc_text(PAGE, MUSA_NAME(TerminateProcess))
#pragma alloc_text(PAGE, MUSA_NAME(GetExitCodeProcess))
#pragma alloc_text(PAGE, MUSA_NAME(GetProcessVersion))
#pragma alloc_text(PAGE, MUSA_NAME(OpenProcessToken))
#pragma alloc_text(PAGE, MUSA_NAME(SetPriorityClass))
#pragma alloc_text(PAGE, MUSA_NAME(GetPriorityClass))
#pragma alloc_text(PAGE, MUSA_NAME(ProcessIdToSessionId))
#pragma alloc_text(PAGE, MUSA_NAME(GetProcessId))
#pragma alloc_text(PAGE, MUSA_NAME(FlushInstructionCache))
#pragma alloc_text(PAGE, MUSA_NAME(FlushProcessWriteBuffers))
#pragma alloc_text(PAGE, MUSA_NAME(GetProcessIdOfThread))
#pragma alloc_text(PAGE, MUSA_NAME(OpenProcess))
#pragma alloc_text(PAGE, MUSA_NAME(GetProcessHandleCount))
#pragma alloc_text(PAGE, MUSA_NAME(GetProcessPriorityBoost))
#pragma alloc_text(PAGE, MUSA_NAME(SetProcessPriorityBoost))
#pragma alloc_text(PAGE, MUSA_NAME(IsProcessCritical))
#endif

using namespace Musa;

EXTERN_C_START

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetProcessTimes)(
    _In_ HANDLE      ProcessHandle,
    _Out_ LPFILETIME CreationTime,
    _Out_ LPFILETIME ExitTime,
    _Out_ LPFILETIME KernelTime,
    _Out_ LPFILETIME UserTime
)
{
    PAGED_CODE();

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

MUSA_IAT_SYMBOL(GetProcessTimes, 20);

HANDLE WINAPI MUSA_NAME(GetCurrentProcess)(
    VOID
)
{
    return ZwCurrentProcess();
}

MUSA_IAT_SYMBOL(GetCurrentProcess, 0);

DWORD WINAPI MUSA_NAME(GetCurrentProcessId)(
    VOID
)
{
    return HandleToULong(ZwCurrentProcessId());
}

MUSA_IAT_SYMBOL(GetCurrentProcessId, 0);

VOID WINAPI MUSA_NAME(ExitProcess)(
    _In_ UINT ExitCode
)
{
    __fastfail(ExitCode);
}

MUSA_IAT_SYMBOL(ExitProcess, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(TerminateProcess)(
    _In_ HANDLE ProcessHandle,
    _In_ UINT   ExitCode
)
{
    PAGED_CODE();

    if (ProcessHandle == GetCurrentProcess()) {
        ExitProcess(ExitCode);
    }

    const auto Status = ZwTerminateProcess(ProcessHandle, ExitCode);
    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(TerminateProcess, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetExitCodeProcess)(
    _In_ HANDLE   ProcessHandle,
    _Out_ LPDWORD ExitCode
)
{
    PAGED_CODE();

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

MUSA_IAT_SYMBOL(GetExitCodeProcess, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetProcessVersion)(
    _In_ DWORD ProcessId
)
{
    PAGED_CODE();

    HANDLE Handle;

    if (ProcessId == 0 || ProcessId == GetCurrentProcessId()) {
        Handle = GetCurrentProcess();
    } else {
        auto ClientId         = CLIENT_ID{ULongToHandle(ProcessId)};
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
    const auto                Status = ZwQueryInformationProcess(Handle, ProcessImageInformation,
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

MUSA_IAT_SYMBOL(GetProcessVersion, 4);

VOID WINAPI MUSA_NAME(GetStartupInfoW)(
    _Out_ LPSTARTUPINFOW StartupInfo
)
{
    RtlZeroMemory(StartupInfo, sizeof(*StartupInfo));
    StartupInfo->cb = sizeof(*StartupInfo);

    const auto Peb = static_cast<Musa::Core::KPEB*>(MUSA_NAME_PRIVATE(RtlGetCurrentPeb)());
    if (Peb) {
        StartupInfo->hStdInput  = Peb->StandardInput;
        StartupInfo->hStdOutput = Peb->StandardOutput;
        StartupInfo->hStdError  = Peb->StandardError;
        StartupInfo->dwFlags    = STARTF_USESTDHANDLES;
    }
}

MUSA_IAT_SYMBOL(GetStartupInfoW, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(OpenProcessToken)(
    _In_ HANDLE      ProcessHandle,
    _In_ DWORD       DesiredAccess,
    _Outptr_ PHANDLE TokenHandle
)
{
    PAGED_CODE();

    const auto Status = ZwOpenProcessToken(ProcessHandle, DesiredAccess, TokenHandle);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

MUSA_IAT_SYMBOL(OpenProcessToken, 12);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SetPriorityClass)(
    _In_ HANDLE ProcessHandle,
    _In_ DWORD  PriorityClass
)
{
    PAGED_CODE();

    UCHAR Priority;
    if (PriorityClass & IDLE_PRIORITY_CLASS) {
        Priority = PROCESS_PRIORITY_CLASS_IDLE;
    } else if (PriorityClass & BELOW_NORMAL_PRIORITY_CLASS) {
        Priority = PROCESS_PRIORITY_CLASS_BELOW_NORMAL;
    } else if (PriorityClass & NORMAL_PRIORITY_CLASS) {
        Priority = PROCESS_PRIORITY_CLASS_NORMAL;
    } else if (PriorityClass & ABOVE_NORMAL_PRIORITY_CLASS) {
        Priority = PROCESS_PRIORITY_CLASS_ABOVE_NORMAL;
    } else if (PriorityClass & HIGH_PRIORITY_CLASS) {
        Priority = PROCESS_PRIORITY_CLASS_HIGH;
    } else if (PriorityClass & REALTIME_PRIORITY_CLASS) {
        Priority = PROCESS_PRIORITY_CLASS_REALTIME;
    } else {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    PROCESS_PRIORITY_CLASS PriorityInformation = {FALSE, Priority};

    const auto Status = ZwSetInformationProcess(ProcessHandle, ProcessPriorityClass,
        &PriorityInformation, sizeof(PriorityInformation));

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

MUSA_IAT_SYMBOL(SetPriorityClass, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetPriorityClass)(
    _In_ HANDLE ProcessHandle
)
{
    PAGED_CODE();

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

MUSA_IAT_SYMBOL(GetPriorityClass, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(ProcessIdToSessionId)(
    _In_ DWORD   ProcessId,
    _Out_ DWORD* SessionId
)
{
    PAGED_CODE();

    if (SessionId == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    if (ProcessId == GetCurrentProcessId()) {
        *SessionId = PsGetCurrentProcessSessionId();
        return TRUE;
    }

    NTSTATUS Status;
    HANDLE   Handle = nullptr;

    do {
        auto ClientId         = CLIENT_ID{ULongToHandle(ProcessId)};
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

MUSA_IAT_SYMBOL(ProcessIdToSessionId, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetProcessId)(
    _In_ HANDLE ProcessHandle
)
{
    PAGED_CODE();

    PROCESS_BASIC_INFORMATION BasicInformation{};

    const auto Status = ZwQueryInformationProcess(ProcessHandle, ProcessBasicInformation,
        &BasicInformation, sizeof(BasicInformation), nullptr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return 0;
    }

    return HandleToULong((HANDLE)BasicInformation.UniqueProcessId);
}

MUSA_IAT_SYMBOL(GetProcessId, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(FlushInstructionCache)(
    _In_ HANDLE                        ProcessHandle,
    _In_reads_bytes_opt_(Size) LPCVOID BaseAddress,
    _In_ SIZE_T                        Size
)
{
    PAGED_CODE();

    const auto Status = ZwFlushInstructionCache(ProcessHandle, const_cast<PVOID>(BaseAddress), Size);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

MUSA_IAT_SYMBOL(FlushInstructionCache, 12);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID WINAPI MUSA_NAME(FlushProcessWriteBuffers)(VOID)
{
    PAGED_CODE();

    const auto Status = ZwFlushProcessWriteBuffers();
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
    }
}

MUSA_IAT_SYMBOL(FlushProcessWriteBuffers, 0);

_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetProcessIdOfThread)(
    _In_ HANDLE Thread
)
{
    PAGED_CODE();

    THREAD_BASIC_INFORMATION BasicInformation{};

    const auto Status = ZwQueryInformationThread(Thread, ThreadBasicInformation,
        &BasicInformation, sizeof(BasicInformation), nullptr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return 0;
    }

    return HandleToULong(BasicInformation.ClientId.UniqueProcess);
}

MUSA_IAT_SYMBOL(GetProcessIdOfThread, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(OpenProcess)(
    _In_ DWORD DesiredAccess,
    _In_ BOOL  InheritHandle,
    _In_ DWORD ProcessId
)
{
    PAGED_CODE();

    HANDLE    Handle   = nullptr;
    CLIENT_ID ClientId = {ULongToHandle(ProcessId)};

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

MUSA_IAT_SYMBOL(OpenProcess, 12);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetProcessHandleCount)(
    _In_ HANDLE  ProcessHandle,
    _Out_ PDWORD HandleCount
)
{
    PAGED_CODE();

    const auto Status = ZwQueryInformationProcess(ProcessHandle, ProcessHandleCount,
        HandleCount, sizeof(*HandleCount), nullptr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

MUSA_IAT_SYMBOL(GetProcessHandleCount, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetProcessPriorityBoost)(
    _In_ HANDLE ProcessHandle,
    _Out_ PBOOL DisablePriorityBoost
)
{
    PAGED_CODE();

    DWORD DisableBoost;

    const auto Status = ZwQueryInformationProcess(ProcessHandle, ProcessPriorityBoost,
        &DisableBoost, sizeof(DisableBoost), nullptr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    *DisablePriorityBoost = (BOOL)DisableBoost;

    return TRUE;
}

MUSA_IAT_SYMBOL(GetProcessPriorityBoost, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SetProcessPriorityBoost)(
    _In_ HANDLE ProcessHandle,
    _In_ BOOL   DisablePriorityBoost
)
{
    PAGED_CODE();

    ULONG DisableBoost = DisablePriorityBoost ? TRUE : FALSE;

    const auto Status = ZwSetInformationProcess(ProcessHandle, ProcessPriorityBoost,
        &DisableBoost, sizeof(DisableBoost));
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

MUSA_IAT_SYMBOL(SetProcessPriorityBoost, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(IsProcessCritical)(
    _In_ HANDLE ProcessHandle,
    _Out_ PBOOL Critical
)
{
    PAGED_CODE();

    ULONG      Information = 0;
    const auto Status      = ZwQueryInformationProcess(ProcessHandle, ProcessBreakOnTermination,
        &Information, sizeof(Information), nullptr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    *Critical = !!Information;
    return TRUE;
}

MUSA_IAT_SYMBOL(IsProcessCritical, 0);

EXTERN_C_END
