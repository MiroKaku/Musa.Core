#ifdef _KERNEL_MODE

#include "MiCore.Utility.h"
#include "MiCore.SystemEnvironmentBlock.Private.h"
#include "MiCore.SystemEnvironmentBlock.Thread.h"


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
    _IRQL_requires_max_(DISPATCH_LEVEL)
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

        PRTL_FLS_DATA FlsData = nullptr;

        const auto LockIrql = MI_NAME_PRIVATE(RtlAcquireTebLockExclusive)();
        __try {
            KTEB Block{};
            Block.ThreadId = ThreadId;

            const auto Teb = static_cast<PKTEB>(RtlLookupElementGenericTableAvl(&MiCoreTebTable, &Block));
            if (Teb) {
                FlsData = Teb->FlsData;
                RtlDeleteElementGenericTableAvl(&MiCoreTebTable, Teb);
            }

        } __finally {
            MI_NAME_PRIVATE(RtlReleaseTebLockExclusive)(LockIrql);
        }

        if (FlsData == nullptr) {
            return;
        }

        if (LockIrql <= APC_LEVEL) {
            RtlProcessFlsData(FlsData,
                RTLP_FLS_DATA_CLEANUP_RUN_CALLBACKS | RTLP_FLS_DATA_CLEANUP_DEALLOCATE);
        }
        else {
            (void)RunTaskOnLowIrql(MiCoreDriverObject, [](PVOID FlsData)
            {
                RtlProcessFlsData(FlsData,
                    RTLP_FLS_DATA_CLEANUP_RUN_CALLBACKS | RTLP_FLS_DATA_CLEANUP_DEALLOCATE);

                return STATUS_SUCCESS;
            }, FlsData);
        }
    }

    _Must_inspect_result_
    _IRQL_requires_max_(APC_LEVEL)
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

    _Must_inspect_result_
    _IRQL_requires_max_(APC_LEVEL)
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

        } while (false);

        return STATUS_SUCCESS;
    }

    _IRQL_requires_max_(DISPATCH_LEVEL)
    PKTEB MICORE_API MI_NAME_PRIVATE(RtlGetCurrentTeb)()
    {
        PKTEB   Teb      = nullptr;
    #ifndef MICORE_FLS_USE_THREAD_NOTIFY_CALLBACK
        BOOLEAN Expired  = FALSE;
    #endif

        KTEB  Block{};
        Block.ThreadId  = PsGetCurrentThreadId();
        Block.ProcessId = PsGetCurrentProcessId();
        Block.ProcessEnvironmentBlock = MI_NAME_PRIVATE(RtlGetCurrentPeb)();

        // Lookup already exists

        KIRQL LockIrql = MI_NAME_PRIVATE(RtlAcquireTebLockShared)();
        __try {
            Teb = static_cast<PKTEB>(RtlLookupElementGenericTableAvl(&MiCoreTebTable, &Block));
            if (Teb) {
            #ifndef MICORE_FLS_USE_THREAD_NOTIFY_CALLBACK
                if (Teb->ProcessId != Block.ProcessId) {
                    Expired = true;
                    __leave;
                }
            #endif

                return Teb;
            }
        }
        __finally {
            MI_NAME_PRIVATE(RtlReleaseTebLockShared)(LockIrql);
        }

    #ifndef MICORE_FLS_USE_THREAD_NOTIFY_CALLBACK
        if (Expired) {
            ExNotifyCallback(MiCoreThreadNotifyCallbackHandleForTeb,
                Block.ThreadId, nullptr /* false */);
        }
    #endif

        // Lazy-init - Insert new item

        LockIrql = MI_NAME_PRIVATE(RtlAcquireTebLockExclusive)();
        __try {
            Teb = static_cast<PKTEB>(RtlInsertElementGenericTableAvl(
                &MiCoreTebTable, &Block, sizeof(KTEB), nullptr));
        }
        __finally {
            MI_NAME_PRIVATE(RtlReleaseTebLockExclusive)(LockIrql);
        }

        return Teb;
    }

    _IRQL_saves_
    _IRQL_raises_(DISPATCH_LEVEL)
    KIRQL MICORE_API MI_NAME_PRIVATE(RtlAcquireTebLockExclusive)()
    {
        return ExAcquireSpinLockExclusive(&MiCoreTebLock);
    }

    _IRQL_requires_(DISPATCH_LEVEL)
    VOID  MICORE_API MI_NAME_PRIVATE(RtlReleaseTebLockExclusive)(_In_ _IRQL_restores_ KIRQL Irql)
    {
        return ExReleaseSpinLockExclusive(&MiCoreTebLock, Irql);
    }

    _IRQL_saves_
    _IRQL_raises_(DISPATCH_LEVEL)
    KIRQL MICORE_API MI_NAME_PRIVATE(RtlAcquireTebLockShared)()
    {
        return ExAcquireSpinLockShared(&MiCoreTebLock);
    }

    _IRQL_requires_(DISPATCH_LEVEL)
    VOID  MICORE_API MI_NAME_PRIVATE(RtlReleaseTebLockShared)(_In_ _IRQL_restores_ KIRQL Irql)
    {
        return ExReleaseSpinLockShared(&MiCoreTebLock, Irql);
    }


}
EXTERN_C_END


#endif // _KERNEL_MODE
