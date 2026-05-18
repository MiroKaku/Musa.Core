EXTERN_C_START

_IRQL_requires_max_(PASSIVE_LEVEL) BOOLEAN WINAPI MUSA_NAME(RtlIsProcessorFeaturePresent)(
    _In_ ULONG ProcessorFeature
)
{
    return ExIsProcessorFeaturePresent(ProcessorFeature);
}

MUSA_IAT_SYMBOL(RtlIsProcessorFeaturePresent, 4);

PVOID NTAPI MUSA_NAME(RtlLocateExtendedFeature)(
    _In_ PCONTEXT_EX ContextEx,
    _In_ ULONG       FeatureId,
    _Out_opt_ PULONG Length
)
{
    return RtlLocateExtendedFeature2(
        ContextEx, FeatureId, SharedUserData->XState, Length);
}

MUSA_IAT_SYMBOL(RtlLocateExtendedFeature, 12);

PVOID NTAPI MUSA_NAME(RtlLocateExtendedFeature2)(
    _In_ PCONTEXT_EX          ContextEx,
    _In_ ULONG                FeatureId,
    _In_ XSTATE_CONFIGURATION XStateConfig,
    _Out_opt_ PULONG          Length
)
{
    // FeatureId 2..63; 0 (x87) and 1 (SSE) are in Legacy chunk
    if (FeatureId < 2 || FeatureId > 63) {
        return nullptr;
    }

    const ULONG64 FeatureBit = 1ui64 << FeatureId;


    // (EnabledFeatures | EnabledUserVisibleSupervisorFeatures) & bit
    const ULONG64 EnabledFeatures = XStateConfig.EnabledFeatures;
    const ULONG64 SupervisorFeatures = XStateConfig.EnabledUserVisibleSupervisorFeatures;
    if (((EnabledFeatures | SupervisorFeatures) & FeatureBit) == 0) {
        return nullptr;
    }


    // ControlFlags: only bits 0-2 allowed (OptimizedSave / CompactionEnabled / ExtendedFeatureDisable)
    const ULONG ControlFlags = XStateConfig.ControlFlags;
    if ((ControlFlags & ~7u) != 0) {
        return nullptr;
    }

    // Locate XState chunk: (PCHAR)ContextEx + XState.Offset + bounds check
    const LONG  XStateOffset = ContextEx->XState.Offset;
    const ULONG AllOffset    = static_cast<ULONG>(ContextEx->All.Offset);
    if (AllOffset > static_cast<ULONG>(XStateOffset) ||
        AllOffset + ContextEx->All.Length <
            static_cast<ULONG>(XStateOffset) + ContextEx->XState.Length) {
        return nullptr;
    }

    PCHAR XStateChunk = reinterpret_cast<PCHAR>(ContextEx) + XStateOffset;
    if (!XStateChunk) {
        return nullptr;
    }

    // Output length: AllFeatures[id] for compacted, Features[id].Size otherwise
    if (Length) {
        if (ControlFlags & 2) {  // CompactionEnabled
            *Length = XStateConfig.AllFeatures[FeatureId];
        } else {
            *Length = XStateConfig.Features[FeatureId].Size;
        }
    }

    // Non-compacted: XStateChunk + Features[id].Offset - sizeof(XSAVE_FORMAT)
    if ((ControlFlags & 2) == 0) {
        return XStateChunk + XStateConfig.Features[FeatureId].Offset - sizeof(XSAVE_FORMAT);
    }

    // ── Compacted path (XSAVEC) ──
    const ULONG64 CompactionMask =
        reinterpret_cast<const XSAVE_AREA_HEADER*>(XStateChunk)->CompactionMask;
    if ((FeatureBit & CompactionMask) == 0) {
        return nullptr;
    }

    const ULONG64 AlignedFeatures = XStateConfig.AlignedFeatures;
    ULONG BaseOffset = sizeof(XSAVE_FORMAT) + sizeof(XSAVE_AREA_HEADER);
    static constexpr ULONG XStateAlignMinusOne = 63;  // 64-byte XSAVE alignment


    if (FeatureId > 2) {
        ULONG64 Bit   = 1ui64 << 2;  // features start at 2 (0=X87, 1=SSE in Legacy)
        ULONG   Count = FeatureId - 2;
        auto*   AllF  = &XStateConfig.AllFeatures[2];

        do {
            if (Bit & CompactionMask) {
                if (Bit & AlignedFeatures) {
                    BaseOffset = (BaseOffset + XStateAlignMinusOne) & ~XStateAlignMinusOne;
                }
                BaseOffset += *AllF;
            }
            Bit = _rotl64(Bit, 1);
            ++AllF;
        } while (--Count);
    }

    if (FeatureBit & AlignedFeatures) {
        BaseOffset = (BaseOffset + XStateAlignMinusOne) & ~XStateAlignMinusOne;
    }

    return XStateChunk + BaseOffset - sizeof(XSAVE_FORMAT);
}

MUSA_IAT_SYMBOL(RtlLocateExtendedFeature2, 16);


ULONG64 NTAPI MUSA_NAME(RtlGetEnabledExtendedFeatures)(
    _In_ ULONG64 FeatureMask
)
{
    const ULONG64 EnabledFeatures = SharedUserData->XState.EnabledFeatures;
    const ULONG64 EnabledUserVisibleSupervisorFeatures = SharedUserData->XState.EnabledUserVisibleSupervisorFeatures;
    const ULONG64 Result = (EnabledFeatures | EnabledUserVisibleSupervisorFeatures) & FeatureMask;

#if defined(_AMD64_)
    constexpr auto Machine = IMAGE_FILE_MACHINE_AMD64;
#elif defined(_ARM64_)
    constexpr auto Machine = IMAGE_FILE_MACHINE_ARM64;
#elif defined(_X86_)
    constexpr auto Machine = IMAGE_FILE_MACHINE_I386;
#endif

    ULONG ArchFlag = 0;
    switch (Machine) {
    case IMAGE_FILE_MACHINE_I386:
        ArchFlag = 0x10000;
        break;
    case IMAGE_FILE_MACHINE_ARMNT:
        ArchFlag = 0x200000;
        break;
    case IMAGE_FILE_MACHINE_AMD64:
        ArchFlag = 0x100000;
        break;
    case IMAGE_FILE_MACHINE_ARM64:
        ArchFlag = 0x400000;
        break;
    default:
        break;
    }

    if (ArchFlag & 0x10000)        // I386
        return Result & 0x40000000000001FFui64;
    if (ArchFlag & 0x100000)       // AMD64
        return Result & 0x4000000000060DFFui64;
    if (ArchFlag & 0x400000)       // ARM64
        return Result & 0x3Cui64;

    return 0;
}

MUSA_IAT_SYMBOL(RtlGetEnabledExtendedFeatures, 8);

ULONG WINAPI MUSA_NAME(RtlGetCurrentProcessorNumber)(VOID)
{
    return KeGetCurrentProcessorNumber();
}

MUSA_IAT_SYMBOL(RtlGetCurrentProcessorNumber, 0);

VOID WINAPI MUSA_NAME(RtlGetCurrentProcessorNumberEx)(_Out_ PPROCESSOR_NUMBER ProcessorNumber)
{
    (void)KeGetCurrentProcessorNumberEx(ProcessorNumber);
}

MUSA_IAT_SYMBOL(RtlGetCurrentProcessorNumberEx, 4);

EXTERN_C_END
