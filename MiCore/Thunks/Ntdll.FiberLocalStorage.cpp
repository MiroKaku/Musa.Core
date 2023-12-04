#ifdef _KERNEL_MODE

#include "Ntdll.FiberLocalStorage.Private.h"
#include "MiCore/MiCore.SystemEnvironmentBlock.Private.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MI_NAME_PRIVATE(RtlRemoveFlsData))
#pragma alloc_text(PAGE, MI_NAME_PRIVATE(RtlRemoveFlsDataAll))
#endif


EXTERN_C_START
namespace Mi
{
    _IRQL_requires_max_(APC_LEVEL)
    VOID NTAPI MI_NAME_PRIVATE(RtlRemoveFlsData)(_In_ PFLS_DATA FlsData)
    {
        PAGED_CODE();

        const auto Peb = MI_NAME_PRIVATE(RtlGetCurrentPeb)();

        __try {
            MI_NAME_PRIVATE(RtlAcquirePebLockExclusive)();

            RemoveEntryList(&FlsData->Entry);

            for (auto Idx = 0ul; Idx < Peb->FlsHighIndex; ++Idx) {
                if (RtlAreBitsSet(&Peb->FlsBitmap, Idx, 1)) {

                    if (const auto Callback = Peb->FlsCallback[Idx]) {
                        if (FlsData->Slots[Idx]) {
                            Callback(FlsData->Slots[Idx]);
                        }
                    }

                    FlsData->Slots[Idx] = nullptr;
                }
            }

            ExFreePoolWithTag(FlsData, MI_TAG);
        }
        __finally {
            MI_NAME_PRIVATE(RtlReleasePebLockExclusive)();
        }
    }

    _IRQL_requires_max_(APC_LEVEL)
    VOID NTAPI MI_NAME_PRIVATE(RtlRemoveFlsDataAll)()
    {
        PAGED_CODE();

        const auto Peb = MI_NAME_PRIVATE(RtlGetCurrentPeb)();

        __try {
            MI_NAME_PRIVATE(RtlAcquirePebLockExclusive)();

            if (!IsListEmpty(&Peb->FlsListHead)) {

                for (auto Entry = RemoveHeadList(&Peb->FlsListHead);
                    Entry != &Peb->FlsListHead;
                    Entry = RemoveHeadList(&Peb->FlsListHead)) {

                    const auto FlsData = CONTAINING_RECORD(Entry, FLS_DATA, Entry);

                    for (auto Idx = 0ul; Idx < Peb->FlsHighIndex; ++Idx) {
                        if (RtlAreBitsSet(&Peb->FlsBitmap, Idx, 1)) {

                            if (const auto Callback = Peb->FlsCallback[Idx]) {
                                if (FlsData->Slots[Idx]) {
                                    Callback(FlsData->Slots[Idx]);
                                }
                            }

                            FlsData->Slots[Idx] = nullptr;
                        }
                    }

                    ExFreePoolWithTag(Entry, MI_TAG);
                }
            }
        }
        __finally {
            MI_NAME_PRIVATE(RtlReleasePebLockExclusive)();
        }
    }


}
EXTERN_C_END


#endif // _KERNEL_MODE
