#include "thunks/thunks.hpp"



namespace Mi::Thunk
{
    extern RTL_BITMAP FlsBitmap;
    extern PVOID*     FlsCallback;
    extern LIST_ENTRY FlsListHead;
    extern FAST_MUTEX FlsListLock;
}

EXTERN_C_START


NTSTATUS NTAPI MI_NAME(RtlFlsAlloc)(
    _In_ PFLS_CALLBACK_FUNCTION Callback,
    _Out_ PULONG FlsIndex
)
{
    *FlsIndex = FLS_OUT_OF_INDEXES;

    const auto ThreadBlock = Mi::Thunk::GetThreadBlock();
    if (ThreadBlock == nullptr) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NTSTATUS Status = STATUS_SUCCESS;

    ExAcquireFastMutex(&Mi::Thunk::FlsListLock);
    do {
        //
        // Search for the first free entry in the fiber local storage
        // bitmap.
        //

        const ULONG Index = RtlFindClearBitsAndSet(&Mi::Thunk::FlsBitmap, 1, 0);

        //
        // If no fiber local storage is available, then set the last error
        // value and return the distinguished value.
        //

        if (Index == FLS_OUT_OF_INDEXES) {
            return STATUS_NO_MEMORY;
        }

        //
        // A fiber local storage index was successfully allocated.
        //
        // If a callback function array has not yet been allocated,
        // then allocate it now.
        //

        if (Mi::Thunk::FlsCallback == nullptr) {
            Mi::Thunk::FlsCallback = static_cast<PVOID*>(ExAllocatePoolZero(NonPagedPool,
                MI_FLS_MAXIMUM_AVAILABLE * sizeof(PVOID), MI_TAG));

            //
            // If a callback function array was not allocated, then clear
            // the allocated slot in the bitmap, set the last error value,
            // and return the distinguished value.
            //

            if (Mi::Thunk::FlsCallback == nullptr) {
                RtlClearBits(&Mi::Thunk::FlsBitmap, Index, 1);
                Status = STATUS_NO_MEMORY;
                break;
            }
        }

        //
        // If the fiber local storage data structure has not yet been
        // allocated for the current thread, then attempt to allocate
        // it now.
        //

        if (ThreadBlock->FlsData == nullptr) {
            ThreadBlock->FlsData = static_cast<PMI_FLS_DATA>(ExAllocatePoolZero(NonPagedPool,
                sizeof(MI_FLS_DATA), MI_TAG));

            //
            // If a fiber local storage data structure was allocated, then
            // insert the allocated data structure in the process local
            // storage data structure list. Otherwise, clear the allocated
            // slot in the bitmap, set the last error value, return the
            // distuiguished value.
            //

            if (ThreadBlock->FlsData != nullptr) {
                InsertTailList(&Mi::Thunk::FlsListHead, &ThreadBlock->FlsData->Entry);
            }
            else {
                RtlClearBits(&Mi::Thunk::FlsBitmap, Index, 1);
                Status = STATUS_NO_MEMORY;
                break;
            }
        }

        //
        // Initialize the free function callback address and the fiber
        // local storage value.
        //

        Mi::Thunk::FlsCallback[Index] = Callback;
        ThreadBlock->FlsData->Slots[Index] = nullptr;

        *FlsIndex = Index;

    } while (false);
    ExReleaseFastMutex(&Mi::Thunk::FlsListLock);

    return Status;
}
MI_IAT_SYMBOL(RtlFlsAlloc, 8);

NTSTATUS NTAPI MI_NAME(RtlFlsFree)(
    _In_ ULONG FlsIndex
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    if (FlsIndex >= MI_FLS_MAXIMUM_AVAILABLE) {
        return STATUS_INVALID_PARAMETER;
    }

    ExAcquireFastMutex(&Mi::Thunk::FlsListLock);
    __try {
        if (!RtlAreBitsSet(&Mi::Thunk::FlsBitmap, FlsIndex, 1)) {
            Status = STATUS_INVALID_PARAMETER;
            __leave;
        }

        //
        // Clear the allocated slot in the fiber local storage bitmap,
        // enumerate fiber local data structures, and invoke callback
        // routines if necessary.
        //

        RtlClearBits(&Mi::Thunk::FlsBitmap, FlsIndex, 1);

        const auto Callback = static_cast<PFLS_CALLBACK_FUNCTION>(Mi::Thunk::FlsCallback[FlsIndex]);
        for (auto NextEntry = Mi::Thunk::FlsListHead.Flink;
            NextEntry != &Mi::Thunk::FlsListHead; NextEntry = NextEntry->Flink) {

            const auto FlsData = CONTAINING_RECORD(NextEntry, MI_FLS_DATA, Entry);
            if ((Callback != nullptr) && (FlsData->Slots[FlsIndex])) {
                (Callback)(FlsData->Slots[FlsIndex]);
            }
            FlsData->Slots[FlsIndex] = nullptr;
        }

        Mi::Thunk::FlsCallback[FlsIndex] = nullptr;

    }
    __finally {
        ExReleaseFastMutex(&Mi::Thunk::FlsListLock);
    }

    return Status;
}
MI_IAT_SYMBOL(RtlFlsFree, 4);

NTSTATUS WINAPI MI_NAME(RtlFlsGetValue)(
    _In_ ULONG FlsIndex,
    _Out_ PVOID* FlsData
)
{
    *FlsData = nullptr;

    if (FlsIndex >= MI_FLS_MAXIMUM_AVAILABLE) {
        return STATUS_INVALID_PARAMETER;
    }

    const auto ThreadBlock = Mi::Thunk::GetThreadBlock();
    if (ThreadBlock == nullptr) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (ThreadBlock->FlsData == nullptr) {
        return STATUS_MEMORY_NOT_ALLOCATED;
    }

    if (!RtlAreBitsSet(&Mi::Thunk::FlsBitmap, FlsIndex, 1)) {
        return STATUS_INVALID_PARAMETER;
    }

    *FlsData = ThreadBlock->FlsData->Slots[FlsIndex];

    return STATUS_SUCCESS;
}
MI_IAT_SYMBOL(RtlFlsGetValue, 8);

NTSTATUS WINAPI MI_NAME(RtlFlsSetValue)(
    _In_ ULONG FlsIndex,
    _In_ PVOID FlsData
)
{
    if (FlsIndex >= MI_FLS_MAXIMUM_AVAILABLE) {
        return STATUS_INVALID_PARAMETER;
    }

    const auto ThreadBlock = Mi::Thunk::GetThreadBlock();
    if (ThreadBlock == nullptr) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!RtlAreBitsSet(&Mi::Thunk::FlsBitmap, FlsIndex, 1)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (ThreadBlock->FlsData == nullptr) {
        ThreadBlock->FlsData = static_cast<PMI_FLS_DATA>(ExAllocatePoolZero(NonPagedPool,
            sizeof(MI_FLS_DATA), MI_TAG));

        //
        // If a fiber local storage data structure was allocated, then
        // insert the allocated structure in the process fiber local
        // storage list. Otherwise, clear the allocated slot in the bitmap,
        // set the last error value, return the distuiguished value.
        //

        if (ThreadBlock->FlsData != nullptr) {
            ExAcquireFastMutex(&Mi::Thunk::FlsListLock);
            __try {
                InsertTailList(&Mi::Thunk::FlsListHead, &ThreadBlock->FlsData->Entry);
            }
            __finally {
                ExReleaseFastMutex(&Mi::Thunk::FlsListLock);
            }
        }
        else {
            return STATUS_NO_MEMORY;
        }
    }

    ThreadBlock->FlsData->Slots[FlsIndex] = FlsData;

    return STATUS_SUCCESS;
}
MI_IAT_SYMBOL(RtlFlsSetValue, 8);


EXTERN_C_END
