#include "Musa.Core.SystemEnvironmentBlock.Fls.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MUSA_NAME_PRIVATE(FlsCreate))
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(FlsCleanup))
#pragma alloc_text(PAGE, MUSA_NAME_PRIVATE(FlsDataCleanup))
#endif

EXTERN_C_START

#if defined(_KERNEL_MODE)

using namespace Musa;
using namespace Musa::Core;

namespace Musa::Core
{
    RTL_FLS_CONTEXT RtlFlsContext{};
}

_IRQL_requires_max_(APC_LEVEL)
VOID NTAPI MUSA_NAME_PRIVATE(FlsCreate)()
{
    PAGED_CODE();

    RtlFlsContext.Lock = 0;
    InitializeListHead(&RtlFlsContext.FlsListHead);
    RtlInitializeBitMap(&RtlFlsContext.FlsBitmap,
        RtlFlsContext.FlsBitmapBits, RTLP_FLS_MAXIMUM_AVAILABLE);
}

_IRQL_requires_max_(APC_LEVEL)
VOID NTAPI MUSA_NAME_PRIVATE(FlsCleanup)()
{
    PAGED_CODE();

    do {
        PLIST_ENTRY Entry = nullptr;

        #pragma warning(suppress: 28150)
        const auto LockIrql = ExAcquireSpinLockExclusive(&RtlFlsContext.Lock);
        __try {
            if (!IsListEmpty(&RtlFlsContext.FlsListHead)) {
                Entry = RemoveHeadList(&RtlFlsContext.FlsListHead);
            }
        } __finally {
            ExReleaseSpinLockExclusive(&RtlFlsContext.Lock, LockIrql);
        }

        if (Entry == nullptr) {
            break;
        }

        RtlProcessFlsData(CONTAINING_RECORD(Entry, RTL_FLS_DATA, Entry),
            RTL_FLS_DATA_CLEANUP_PER_SLOT | RTL_FLS_DATA_CLEANUP_DEALLOCATE);
    } while (true);

    const auto FlsCallback = static_cast<PFLS_CALLBACK_FUNCTION*>(InterlockedCompareExchangePointer(
        reinterpret_cast<PVOID volatile*>(&RtlFlsContext.FlsCallback), nullptr, (PVOID)(RtlFlsContext.FlsCallback)));
    if (FlsCallback) {
        RtlFreeHeap(RtlProcessHeap(), HEAP_NO_SERIALIZE, (PVOID)FlsCallback);
    }
}

_IRQL_requires_max_(APC_LEVEL)
VOID NTAPI MUSA_NAME_PRIVATE(FlsDataCleanup)(
    _In_ PVOID FlsData,
    _In_ ULONG Flags
)
{
    PAGED_CODE();

    auto Data = static_cast<PRTL_FLS_DATA>(FlsData);
    if (Data == nullptr) {
        return;
    }

    if (Flags & RTL_FLS_DATA_CLEANUP_PER_SLOT) {
        const size_t HighIndex = InterlockedCompareExchange(
            reinterpret_cast<LONG volatile*>(&RtlFlsContext.FlsHighIndex), 0, 0);

        const auto FlsCallback = static_cast<PFLS_CALLBACK_FUNCTION*>(InterlockedCompareExchangePointer(
            reinterpret_cast<PVOID volatile*>(&RtlFlsContext.FlsCallback), nullptr, nullptr));

        if (FlsCallback) {
            for (size_t Idx = 0; Idx < HighIndex; Idx++) {
                if (const auto Callback = FlsCallback[Idx]) {
                    if (Data->Slots[Idx]) {
                        Callback(Data->Slots[Idx]);
                    }
                }
                Data->Slots[Idx] = nullptr;
            }
        }

        #pragma warning(suppress: 28150)
        const auto LockIrql = ExAcquireSpinLockExclusive(&RtlFlsContext.Lock);
        __try {
            RemoveEntryListUnsafe(&Data->Entry);
        } __finally {
            ExReleaseSpinLockExclusive(&RtlFlsContext.Lock, LockIrql);
        }
    }

    if (Flags & RTL_FLS_DATA_CLEANUP_DEALLOCATE) {
        RtlFreeHeap(RtlProcessHeap(), 0, FlsData);
    }
}

#endif // defined(_KERNEL_MODE)

EXTERN_C_END
