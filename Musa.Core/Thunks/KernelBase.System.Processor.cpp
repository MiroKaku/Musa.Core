#include "KernelBase.Private.h"
#include "Internal/KernelBase.System.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(GetLogicalProcessorInformation))
#pragma alloc_text(PAGE, MUSA_NAME(GetLogicalProcessorInformationEx))
#endif

using namespace Musa;

EXTERN_C_START

//
// Processor
//

BOOL WINAPI MUSA_NAME(IsProcessorFeaturePresent)(
    _In_ DWORD ProcessorFeature
)
{
    return RtlIsProcessorFeaturePresent(ProcessorFeature);
}

MUSA_IAT_SYMBOL(IsProcessorFeaturePresent, 4);

DWORD WINAPI MUSA_NAME(GetCurrentProcessorNumber)(
    VOID
)
{
    return RtlGetCurrentProcessorNumber();
}

MUSA_IAT_SYMBOL(GetCurrentProcessorNumber, 0);

VOID WINAPI MUSA_NAME(GetCurrentProcessorNumberEx)(
    _Out_ PPROCESSOR_NUMBER ProcNumber
)
{
    return RtlGetCurrentProcessorNumberEx(ProcNumber);
}

MUSA_IAT_SYMBOL(GetCurrentProcessorNumberEx, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetLogicalProcessorInformation)(
    _Out_writes_bytes_to_opt_(*ReturnedLength, *ReturnedLength) PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer,
    _Inout_ PDWORD ReturnedLength
)
{
    PAGED_CODE();

    if (ReturnedLength == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    NTSTATUS Status = ZwQuerySystemInformation(SystemLogicalProcessorInformation,
        Buffer, *ReturnedLength, ReturnedLength);
    if (Status == STATUS_INFO_LENGTH_MISMATCH) {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    } else {
        return TRUE;
    }
}

MUSA_IAT_SYMBOL(GetLogicalProcessorInformation, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetLogicalProcessorInformationEx)(
    _In_ LOGICAL_PROCESSOR_RELATIONSHIP RelationshipType,
    _Out_writes_bytes_to_opt_(*ReturnedLength, *ReturnedLength) PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX Buffer,
    _Inout_ PDWORD ReturnedLength
)
{
    PAGED_CODE();

    if (ReturnedLength == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    NTSTATUS Status = ZwQuerySystemInformationEx(SystemLogicalProcessorAndGroupInformation,
        &RelationshipType, sizeof(RelationshipType),
        Buffer, *ReturnedLength, ReturnedLength);
    if (Status == STATUS_INFO_LENGTH_MISMATCH) {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    } else {
        return TRUE;
    }
}

MUSA_IAT_SYMBOL(GetLogicalProcessorInformationEx, 12);

WORD WINAPI MUSA_NAME(GetActiveProcessorGroupCount)(VOID)
{
    return KeQueryActiveGroupCount();
}

MUSA_IAT_SYMBOL(GetActiveProcessorGroupCount, 0);

WORD WINAPI MUSA_NAME(GetMaximumProcessorGroupCount)(VOID)
{
    return KeQueryMaximumGroupCount();
}

MUSA_IAT_SYMBOL(GetMaximumProcessorGroupCount, 0);

DWORD WINAPI MUSA_NAME(GetActiveProcessorCount)(
    _In_ WORD GroupNumber
)
{
    return KeQueryActiveProcessorCountEx(GroupNumber);
}

MUSA_IAT_SYMBOL(GetActiveProcessorCount, 4);

DWORD WINAPI MUSA_NAME(GetMaximumProcessorCount)(
    _In_ WORD GroupNumber
)
{
    return KeQueryMaximumProcessorCountEx(GroupNumber);
}

MUSA_IAT_SYMBOL(GetMaximumProcessorCount, 4);


_Success_(return != NULL)
PVOID WINAPI MUSA_NAME(LocateXStateFeature)(
    _In_ PCONTEXT Context,
    _In_ DWORD FeatureId,
    _Out_opt_ PDWORD Length
)
{
    /*
     * ContextFlags architecture bits: CONTEXT_i386   (0x10000)
     *                                  CONTEXT_AMD64  (0x100000)
     *                                  CONTEXT_ARM64  (0x400000)
     *
     * The XSTATE sub-flag (0x40 on i386, 0x40 on AMD64, 0x20 on ARM64)
     * indicates that extended processor state is present in the context.
     */
    ULONG ContextFlags = Context->ContextFlags;
    PCONTEXT_EX ContextEx = nullptr;


    if (ContextFlags & 0x10000) {  // CONTEXT_i386
        // -- Path 1: XSTATE compact context (x86 / WOW64) --
        // Offsets are x86-CONTEXT-specific; sizeof(CONTEXT)/offsetof
        // resolve to the build arch (e.g. AMD64), so raw hex is used.
        ContextEx = reinterpret_cast<PCONTEXT_EX>(
            reinterpret_cast<PUCHAR>(Context) + 0x2CC);
        if ((ContextFlags & 0x10040) != 0x10040)  // CONTEXT_i386 | CONTEXT_XSTATE
            ContextEx = nullptr;

        if (FeatureId == 0) {
            if (Length) *Length = 0xA0;   // X87 save area size
            return reinterpret_cast<PUCHAR>(Context) + 0xCC;
        }
        if (FeatureId == 1) {
            if (Length) *Length = 0x80;   // SSE save area size
            return reinterpret_cast<PUCHAR>(Context) + 0x16C;
        }

    } else if (ContextFlags & 0x100000) {  // CONTEXT_AMD64
        // -- Path 2: AMD64 native context --
        ContextEx = reinterpret_cast<PCONTEXT_EX>(
            reinterpret_cast<PUCHAR>(Context) + sizeof(CONTEXT));
        if ((ContextFlags & 0x100040) != 0x100040)  // CONTEXT_XSTATE (AMD64)
            ContextEx = nullptr;

        if (FeatureId == 0) {
            if (Length) *Length = 0xA0;
            return reinterpret_cast<PUCHAR>(Context) + 0x100;  // &Context->FltSave
        }
        if (FeatureId == 1) {
            if (Length) *Length = 0x100;
            return reinterpret_cast<PUCHAR>(Context) + 0x1A0;  // &Context->Xmm0
        }
    } else {
        // -- Path 3: ARM64 --
        if ((ContextFlags & 0x400020) != 0x400020 ||  // CONTEXT_ARM64_XSTATE
            (ContextFlags & 0x400000) == 0)           // CONTEXT_ARM64
            return nullptr;
        ContextEx = reinterpret_cast<PCONTEXT_EX>(
            reinterpret_cast<PUCHAR>(Context) + 0x390);  // ARM64 CONTEXT XState offset
    }

    if (!ContextEx)
        return nullptr;

    PULONG NtLength = nullptr;
    ULONG NtLengthValue = 0;
    if (Length)
        NtLength = &NtLengthValue;

    PVOID Result = RtlLocateExtendedFeature(
        ContextEx, static_cast<ULONG>(FeatureId), NtLength);

    if (Length && Result)
        *Length = static_cast<DWORD>(NtLengthValue);

    return Result;
}

MUSA_IAT_SYMBOL(LocateXStateFeature, 12);
_VEIL_DECLARE_ALTERNATE_NAME(LocateXStateFeature, _VEIL_DEFINE_IAT_SYMBOL_MAKE_NAME(LocateXStateFeature))

DWORD64 WINAPI MUSA_NAME(GetEnabledXStateFeatures)(VOID)
{
    DWORD64 Features = RtlGetEnabledExtendedFeatures(~0ui64);
    if (Features)
        return Features;
    return 3;  // X87 | SSE always enabled
}

MUSA_IAT_SYMBOL(GetEnabledXStateFeatures, 0);

EXTERN_C_END
