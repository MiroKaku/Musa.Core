#include "MiCore.Utility.h"
#ifdef _KERNEL_MODE

#include "MiCore.SystemEnvironmentBlock.Private.h"
#include "MiCore.SystemEnvironmentBlock.Thread.h"
#include "Thunks/Ntdll.FiberLocalStorage.Private.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MI_NAME_PRIVATE(SetupThreadEnvironmentBlock))
#pragma alloc_text(PAGE, MI_NAME_PRIVATE(FreeThreadEnvironmentBlock))
#endif


EXTERN_C_START
namespace Mi
{
    extern PDRIVER_OBJECT   MiCoreDriverObject;
    extern PCALLBACK_OBJECT MiCoreThreadNotifyCallbackObject;

    PVOID MiCoreThreadNotifyCallbackHandleForTeb = nullptr;

    EX_SPIN_LOCK  MiCoreTebLock {};
    RTL_AVL_TABLE MiCoreTebTable{};
    LOOKASIDE_ALIGN LOOKASIDE_LIST_EX MiCoreTebPool{};

    extern"C++"
    {
        namespace TebTableRoutines
        {
            RTL_AVL_COMPARE_ROUTINE  Compare;
            RTL_AVL_ALLOCATE_ROUTINE Allocate;
            RTL_AVL_FREE_ROUTINE     Free;

            RTL_GENERIC_COMPARE_RESULTS NTAPI Compare(
                _In_ RTL_AVL_TABLE* Table,
                _In_ PVOID First,
                _In_ PVOID Second
            )
            {
                UNREFERENCED_PARAMETER(Table);

                const auto Entry1 = static_cast<PCKTEB>(First);
                const auto Entry2 = static_cast<PCKTEB>(Second);

                return Entry1->ThreadId == Entry2->ThreadId
                    ? GenericEqual
                    : (Entry1->ThreadId > Entry2->ThreadId ? GenericGreaterThan : GenericLessThan);
            }

            PVOID NTAPI Allocate(
                _In_ RTL_AVL_TABLE* Table,
                _In_ ULONG Size
            )
            {
                UNREFERENCED_PARAMETER(Table);

                const auto Buffer = ExAllocateFromLookasideListEx(&MiCoreTebPool);
                if (Buffer) {
                    RtlSecureZeroMemory(Buffer, Size);
                }
                return Buffer;
            }

            VOID NTAPI Free(
                _In_ RTL_AVL_TABLE* Table,
                _In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Buffer
            )
            {
                UNREFERENCED_PARAMETER(Table);

                return ExFreeToLookasideListEx(&MiCoreTebPool, Buffer);
            }
        }
    }

    static
    VOID MI_NAME_PRIVATE(ThreadNotifyCallbackForTeb)(
        _In_opt_ PVOID  CallbackContext,
        _In_opt_ HANDLE ThreadId,
        _In_opt_ PVOID  Create
    )
    {
        UNREFERENCED_PARAMETER(CallbackContext);

        if (Create) {
            return;
        }

        PFLS_DATA FlsData  = nullptr;
        KIRQL     LockIrql = PASSIVE_LEVEL;

        __try {
            LockIrql = ExAcquireSpinLockExclusive(&MiCoreTebLock);

            KTEB Block{};
            Block.ThreadId = ThreadId;

            if (const auto Teb = static_cast<PKTEB>(RtlLookupElementGenericTableAvl(&MiCoreTebTable, &Block))) {
                FlsData = Teb->FlsData;
                RtlDeleteElementGenericTableAvl(&MiCoreTebTable, Teb);
            }
        } __finally {
            ExReleaseSpinLockExclusive(&MiCoreTebLock, LockIrql);
        }

        if (FlsData == nullptr) {
            return;
        }

        if (LockIrql <= APC_LEVEL) {
            MI_NAME_PRIVATE(RtlRemoveFlsData)(FlsData);
        }
        else {
            (void)RunTaskOnLowIrql(MiCoreDriverObject, [](PVOID FlsData)
            {
                MI_NAME_PRIVATE(RtlRemoveFlsData)(static_cast<PFLS_DATA>(FlsData));
                return STATUS_SUCCESS;
            }, FlsData);
        }
    }

    NTSTATUS MICORE_API MI_NAME_PRIVATE(SetupThreadEnvironmentBlock)(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
        )
    {
        UNREFERENCED_PARAMETER(DriverObject);
        UNREFERENCED_PARAMETER(RegistryPath);

        PAGED_CODE();

        NTSTATUS Status;

        do {
            MiCoreTebLock = 0;

            constexpr auto BlockSize = ROUND_TO_SIZE(sizeof(KTEB) + sizeof(RTL_BALANCED_LINKS), sizeof(void*));
            Status = ExInitializeLookasideListEx(&MiCoreTebPool, nullptr, nullptr,
                NonPagedPool, 0, BlockSize, MI_TAG, 0);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            RtlInitializeGenericTableAvl(&MiCoreTebTable,
                &TebTableRoutines::Compare,
                &TebTableRoutines::Allocate,
                &TebTableRoutines::Free,
                nullptr);

            MiCoreThreadNotifyCallbackHandleForTeb = ExRegisterCallback(MiCoreThreadNotifyCallbackObject,
                MI_NAME_PRIVATE(ThreadNotifyCallbackForTeb), nullptr);
            if (MiCoreThreadNotifyCallbackHandleForTeb == nullptr) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

        } while (false);

        return Status;
    }

    NTSTATUS MICORE_API MI_NAME_PRIVATE(FreeThreadEnvironmentBlock)()
    {
        PAGED_CODE();

        do {
            if (MiCoreThreadNotifyCallbackHandleForTeb) {
                ExUnregisterCallback(MiCoreThreadNotifyCallbackHandleForTeb);
                MiCoreThreadNotifyCallbackHandleForTeb = nullptr;
            }

            for (auto Entry = RtlGetElementGenericTableAvl(&MiCoreTebTable, 0);
                Entry;
                Entry = RtlGetElementGenericTableAvl(&MiCoreTebTable, 0)) {

                RtlDeleteElementGenericTableAvl(&MiCoreTebTable, Entry);
            }

            if (MiCoreTebPool.L.Size) {
                ExDeleteLookasideListEx(&MiCoreTebPool);
            }

            MI_NAME_PRIVATE(RtlRemoveFlsDataAll)();

        } while (false);

        return STATUS_SUCCESS;
    }

    PKTEB MICORE_API MI_NAME_PRIVATE(RtlGetCurrentTeb)()
    {
        PKTEB   Teb = nullptr;

        KIRQL   LockIrql      = PASSIVE_LEVEL;
        BOOLEAN LockExclusive = FALSE;

        __try {
            LockIrql = ExAcquireSpinLockShared(&MiCoreTebLock);

            KTEB  Block{};
            Block.ThreadId  = PsGetCurrentThreadId ();
            Block.ProcessId = PsGetCurrentProcessId();
            Block.ProcessEnvironmentBlock = MI_NAME_PRIVATE(RtlGetCurrentPeb)();

            Teb = static_cast<PKTEB>(RtlLookupElementGenericTableAvl(&MiCoreTebTable, &Block));

        #ifndef MICORE_FLS_USE_THREAD_NOTIFY_CALLBACK
            if (Teb) {
                if (Teb->ProcessId != Block.ProcessId) {
                    const auto Expired = static_cast<PKTEB>(InterlockedExchangePointer(
                        reinterpret_cast<PVOID volatile*>(&Teb), nullptr));
                    const auto FlsData = Expired->FlsData;

                    RtlDeleteElementGenericTableAvl(&MiCoreTebTable, Expired);

                    (void)RunTaskOnLowIrql(MiCoreDriverObject,[](PVOID FlsData)
                    {
                        MI_NAME_PRIVATE(RtlRemoveFlsData)(static_cast<PFLS_DATA>(FlsData));
                        return STATUS_SUCCESS;
                    }, FlsData);
                }
            }
        #endif

            // Lazy-init
            if (Teb == nullptr) {
                if (ExTryConvertSharedSpinLockExclusive(&MiCoreTebLock) == FALSE) {
                    ExReleaseSpinLockShared(&MiCoreTebLock, LockIrql);

                    KeMemoryBarrier();

                    LockIrql = ExAcquireSpinLockExclusive(&MiCoreTebLock);
                }

                LockExclusive = TRUE;

                Teb = static_cast<PKTEB>(RtlInsertElementGenericTableAvl(
                    &MiCoreTebTable, &Block, sizeof(KTEB), nullptr));
            }
        }
        __finally {
            if (LockExclusive) {
                ExReleaseSpinLockExclusive(&MiCoreTebLock, LockIrql);
            }
            else {
                ExReleaseSpinLockShared(&MiCoreTebLock, LockIrql);
            }
        }

        return Teb;
    }

}
EXTERN_C_END


#endif // _KERNEL_MODE
