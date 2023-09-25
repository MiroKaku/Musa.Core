#include "thunks/thunks.hpp"


EXTERN_C_START


NTSTATUS NTAPI MI_NAME(RtlFlsAlloc)(
    _In_ PFLS_CALLBACK_FUNCTION Callback,
    _Out_ PULONG FlsIndex
)
{
    PAGED_CODE()

    //
    // Attempt to allocate a fiber local storage index.
    //

    const auto Teb = NtCurrentTeb();
    const auto Peb = Teb->ProcessEnvironmentBlock;

    RtlAcquirePebLock();
    __try {

        *FlsIndex = FLS_OUT_OF_INDEXES;

        //
        // Search for the first free entry in the fiber local storage
        // bitmap.
        //

        const auto Index = RtlFindClearBitsAndSet(&Peb->FlsBitmap, 1, 0);

        //
        // If no fiber local storage is available, then set the last error
        // value and return the distinguished value.
        //

        if (Index == FLS_OUT_OF_INDEXES) {
            return STATUS_NO_MEMORY;
        }

        //
        // If the fiber local storage data structure has not yet been
        // allocated for the current thread, then attempt to allocate
        // it now.
        //

        if (Teb->FlsData == nullptr) {
            Teb->FlsData = static_cast<PMI_FLS_DATA>(RtlAllocateHeap(
                Peb->ProcessHeap, HEAP_ZERO_MEMORY, sizeof(MI_FLS_DATA)));

            //
            // If a fiber local storage data structure was allocated, then
            // insert the allocated data structure in the process local
            // storage data structure list. Otherwise, clear the allocated
            // slot in the bitmap, set the last error value, return the
            // distinguished value.
            //

            if (Teb->FlsData == nullptr) {
                RtlClearBits(&Peb->FlsBitmap, Index, 1);
                return STATUS_NO_MEMORY;
            }

            InsertTailList(&Peb->FlsListHead, &Teb->FlsData->Entry);
        }

        //
        // Initialize the free function callback address and the fiber
        // local storage value.
        //

        Peb->FlsCallback[Index] = Callback;
        Teb->FlsData->Slots[Index] = nullptr;

        //
        // If the allocated index is greater than the highest allocated
        // index, then save the new high.
        //

        if (Index > Peb->FlsHighIndex) {
            Peb->FlsHighIndex = Index;
        }

        *FlsIndex = Index;
    }
    __finally {
        RtlReleasePebLock();
    }

    return STATUS_SUCCESS;
}
MI_IAT_SYMBOL(RtlFlsAlloc, 8);

NTSTATUS NTAPI MI_NAME(RtlFlsFree)(
    _In_ ULONG FlsIndex
)
{
    PAGED_CODE()

    if (FlsIndex >= MI_FLS_MAXIMUM_AVAILABLE) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // If the fiber local storage index is invalid, then return FALSE.
    // Otherwise, invoke the callback function associated with the fiber
    // local storage data entry if necessary, free the specified index,
    // and return TRUE.
    //

    const auto Teb = NtCurrentTeb();
    const auto Peb = Teb->ProcessEnvironmentBlock;

    RtlAcquirePebLock();
    __try {
        if (!RtlAreBitsSet(&Peb->FlsBitmap, FlsIndex, 1)) {
            return STATUS_INVALID_PARAMETER;
        }

        //
        // Clear the allocated slot in the fiber local storage bitmap,
        // enumerate fiber local data structures, and invoke callback
        // routines if necessary.
        //

        RtlClearBits(&Peb->FlsBitmap, FlsIndex, 1);

        const auto Callback = Peb->FlsCallback[FlsIndex];

        for (auto NextEntry = Peb->FlsListHead.Flink;
            NextEntry != &Peb->FlsListHead; NextEntry = NextEntry->Flink) {

            const auto FlsData = CONTAINING_RECORD(NextEntry, MI_FLS_DATA, Entry);
            if ((Callback != nullptr) && (FlsData->Slots[FlsIndex])) {
                (Callback)(FlsData->Slots[FlsIndex]);
            }

            FlsData->Slots[FlsIndex] = nullptr;
        }

        Peb->FlsCallback[FlsIndex] = nullptr;
    }
    __finally {
        RtlReleasePebLock();
    }

    return STATUS_SUCCESS;
}
MI_IAT_SYMBOL(RtlFlsFree, 4);

NTSTATUS WINAPI MI_NAME(RtlFlsGetValue)(
    _In_ ULONG FlsIndex,
    _Out_ PVOID* FlsData
)
{
    PAGED_CODE()

    *FlsData = nullptr;

    if (FlsIndex >= MI_FLS_MAXIMUM_AVAILABLE) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // If the fiber local storage index is invalid or the fiber local storage
    // data structure has not been allocated, then set the last error value
    // and return NULL. Otherwise, return the specified value from fiber local
    // storage data.
    //

    const auto Teb = NtCurrentTeb();
    const auto Peb = Teb->ProcessEnvironmentBlock;

    if (FlsIndex > Peb->FlsHighIndex) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!RtlAreBitsSet(&Peb->FlsBitmap, FlsIndex, 1)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (Teb->FlsData == nullptr) {
        return STATUS_MEMORY_NOT_ALLOCATED;
    }

    *FlsData = Teb->FlsData->Slots[FlsIndex];

    return STATUS_SUCCESS;
}
MI_IAT_SYMBOL(RtlFlsGetValue, 8);

NTSTATUS WINAPI MI_NAME(RtlFlsSetValue)(
    _In_ ULONG FlsIndex,
    _In_ PVOID FlsData
)
{
    PAGED_CODE()

    if (FlsIndex >= MI_FLS_MAXIMUM_AVAILABLE) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // If the fiber local storage index is invalid, then set the last error
    // value and return FALSE. Otherwise, attempt to store the specified
    // value in the associated fiber local storage data.
    //

    const auto Teb = NtCurrentTeb();
    const auto Peb = Teb->ProcessEnvironmentBlock;

    if (FlsIndex > Peb->FlsHighIndex) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!RtlAreBitsSet(&Peb->FlsBitmap, FlsIndex, 1)) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // If the fiber local storage data structure has not yet been
    // allocated for the current thread, then attempt to allocate it
    // now.
    //

    if (Teb->FlsData == nullptr) {
        Teb->FlsData = static_cast<PMI_FLS_DATA>(RtlAllocateHeap(Peb->ProcessHeap,
            HEAP_ZERO_MEMORY, sizeof(MI_FLS_DATA)));

        //
        // If a fiber local storage data structure was allocated, then
        // insert the allocated structure in the process fiber local
        // storage list. Otherwise, clear the allocated slot in the bitmap,
        // set the last error value, return the distinguished value.
        //

        if (Teb->FlsData != nullptr) {
            RtlAcquirePebLock();
            __try {
                InsertTailList(&Peb->FlsListHead, &Teb->FlsData->Entry);
            }
            __finally {
                RtlReleasePebLock();
            }
        }
        else {
            return STATUS_NO_MEMORY;
        }
    }

    //
    // Store the specified value in the fiber local storage data entry and
    // return STATUS_SUCCESS.
    //

    Teb->FlsData->Slots[FlsIndex] = FlsData;

    return STATUS_SUCCESS;
}
MI_IAT_SYMBOL(RtlFlsSetValue, 8);


EXTERN_C_END
