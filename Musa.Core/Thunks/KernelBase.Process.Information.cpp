#include "KernelBase.Private.h"
#include "Internal/KernelBase.Process.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(SetProcessInformation))
#pragma alloc_text(PAGE, MUSA_NAME(GetProcessInformation))
#endif

using namespace Musa;

EXTERN_C_START

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SetProcessInformation)(
    _In_ HANDLE                                     ProcessHandle,
    _In_ PROCESS_INFORMATION_CLASS                  ProcessInformationClass,
    _In_reads_bytes_(ProcessInformationSize) LPVOID ProcessInformation,
    _In_ DWORD                                      ProcessInformationSize
)
{
    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    do {
        PROCESSINFOCLASS Class = ProcessPagePriority;

        switch (ProcessInformationClass) {
            case ProcessMemoryPriority: {
                Class = ProcessPagePriority;
                break;
            }
            case ProcessMemoryExhaustionInfo: {
                if (ProcessInformation == nullptr) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                if (ProcessInformationSize != sizeof(PROCESS_MEMORY_EXHAUSTION_INFO)) {
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                    break;
                }

                const auto Information = static_cast<PROCESS_MEMORY_EXHAUSTION_INFO*>(ProcessInformation);

                if (Information->Version != PME_CURRENT_VERSION ||
                    Information->Reserved != 0u ||
                    Information->Type >= PMETypeMax) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                Class = ProcessMemoryExhaustion;
                break;
            }
            case ProcessInPrivateInfo: {
                if (ProcessInformation != nullptr ||
                    ProcessInformationSize != 0) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                Class = ProcessInPrivate;
                break;
            }
            case ProcessPowerThrottling: {
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
            case ProcessTelemetryCoverageInfo: {
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
            case ProcessLeapSecondInfo: {
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
            default: {
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

MUSA_IAT_SYMBOL(SetProcessInformation, 16);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetProcessInformation)(
    _In_ HANDLE                                       ProcessHandle,
    _In_ PROCESS_INFORMATION_CLASS                    ProcessInformationClass,
    _Out_writes_bytes_(ProcessInformationSize) LPVOID ProcessInformation,
    _In_ DWORD                                        ProcessInformationSize
)
{
    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    do {
        PROCESSINFOCLASS Class = ProcessPagePriority;

        switch (ProcessInformationClass) {
            case ProcessMemoryPriority: {
                Class = ProcessPagePriority;
                break;
            }
            case ProcessAppMemoryInfo: {
                // TODO:
                Status = STATUS_NOT_IMPLEMENTED;
                break;
            }
            case ProcessInPrivateInfo: {
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
            case ProcessProtectionLevelInfo: {
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
            case ProcessLeapSecondInfo: {
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
            default: {
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
            auto& ProtectionLevel = static_cast<PROCESS_PROTECTION_LEVEL_INFORMATION*>(ProcessInformation)->
                ProtectionLevel;

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
                case PsProtectedValue(PsProtectedSignerWinTcb, FALSE, PsProtectedTypeProtected):
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

MUSA_IAT_SYMBOL(GetProcessInformation, 16);

EXTERN_C_END
