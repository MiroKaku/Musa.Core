#include "miutil.hpp"


namespace Mi::Util
{
    NTSTATUS ImageEnumerateExports(
        _In_ void* BaseOfImage,
        _In_ bool(CALLBACK *Callback)(uint32_t Ordinal, const char* Name, const void* Address, void* Context),
        _In_opt_ void* Context
    )
    {
        ULONG DataEntrySize   = 0;
        const void* DataEntry = RtlImageDirectoryEntryToData(BaseOfImage, TRUE,
            IMAGE_DIRECTORY_ENTRY_EXPORT, &DataEntrySize);
        if (DataEntry == nullptr) {
            return STATUS_INVALID_IMAGE_FORMAT;
        }

        const auto ExportEntry    = static_cast<PCIMAGE_EXPORT_DIRECTORY>(DataEntry);
        const auto AddressOfNames = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(BaseOfImage) + ExportEntry->AddressOfNames);
        const auto AddressOfFuncs = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(BaseOfImage) + ExportEntry->AddressOfFunctions);
        const auto AddressOfOrdis = reinterpret_cast<uint16_t*>(static_cast<uint8_t*>(BaseOfImage) + ExportEntry->AddressOfNameOrdinals);

        for (uint32_t Ordinal = 0; Ordinal < ExportEntry->NumberOfFunctions; ++Ordinal) {
            if (0 == AddressOfFuncs[Ordinal]) {
                continue;
            }

            const char*    ExportName    = nullptr;
            const uint32_t ExportOrdinal = Ordinal + ExportEntry->Base;
            const void*    ExportAddress = static_cast<void*>(static_cast<uint8_t*>(BaseOfImage) + AddressOfFuncs[Ordinal]);

            for (uint32_t Idx = 0u; Idx <= ExportEntry->NumberOfNames; ++Idx) {
                if (Ordinal == AddressOfOrdis[Idx]) {
                    ExportName = reinterpret_cast<const char*>(static_cast<uint8_t*>(BaseOfImage) + AddressOfNames[Idx]);
                    break;
                }
            }

            if (Callback(ExportOrdinal, ExportName, ExportAddress, Context)) {
                break;
            }
        }

        return STATUS_SUCCESS;
    }

    int64_t GetUniqueIdViaClientId(
        _In_ CLIENT_ID ClientId
    )
    {
        const auto High = reinterpret_cast<SIZE_T>(ClientId.UniqueProcess) >> 2;
        const auto Low  = reinterpret_cast<SIZE_T>(ClientId.UniqueThread) >> 2;

        return
            (static_cast<int64_t>(High & ~static_cast<uint32_t>(0)) << 32) |
            (static_cast<int64_t>(Low & ~static_cast<uint32_t>(0)));
    }

    //NTSTATUS QueueWorkItem(
    //    _In_opt_ PLARGE_INTEGER WaitTime,
    //    _In_ void(CALLBACK* Callback)(void*),
    //    _In_opt_ void* Context
    //)
    //{
    //    typedef struct _MI_WORK_ITEM
    //    {
    //        void(CALLBACK* Callback)(PVOID);
    //        PVOID   Context;

    //        KEVENT  WaitEvent;
    //        UINT8   IoWorkItem[ANYSIZE_ARRAY];

    //    } MI_WORK_ITEM, * PMI_WORK_ITEM;

    //    NTSTATUS Status;

    //    do {
    //        const auto WorkItem = static_cast<PMI_WORK_ITEM>(ExAllocatePoolZero(NonPagedPool,
    //            sizeof(MI_WORK_ITEM) + IoSizeofWorkItem(), MI_TAG));
    //        if (WorkItem == nullptr) {
    //            Status = STATUS_INSUFFICIENT_RESOURCES;
    //            break;
    //        }

    //        WorkItem->Callback  = Callback;
    //        WorkItem->Context   = Context;
    //        KeInitializeEvent(&WorkItem->WaitEvent, NotificationEvent, FALSE);
    //        IoInitializeWorkItem(MI_DRIVER_OBJECT, reinterpret_cast<PIO_WORKITEM>(WorkItem->IoWorkItem));

    //        IoQueueWorkItemEx(reinterpret_cast<PIO_WORKITEM>(WorkItem->IoWorkItem), [](PVOID, PVOID Context, PIO_WORKITEM)
    //        {
    //            const auto WorkItem = static_cast<PMI_WORK_ITEM>(Context);
    //            WorkItem->Callback(WorkItem->Context);

    //            KeSetEvent(&WorkItem->WaitEvent, IO_NO_INCREMENT, FALSE);
    //            ExFreePoolWithTag(WorkItem, MI_TAG);

    //        }, DelayedWorkQueue, WorkItem);

    //        Status = KeWaitForSingleObject(&WorkItem->WaitEvent, Executive, KernelMode, FALSE, WaitTime);

    //    } while (false);

    //    return Status;
    //}
}
