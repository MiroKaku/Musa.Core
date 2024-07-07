#include "KernelBase.Private.h"
#include "KernelBase.Handle.Private.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(CloseHandle))
#pragma alloc_text(PAGE, MUSA_NAME(DuplicateHandle))
#pragma alloc_text(PAGE, MUSA_NAME(CompareObjectHandles))
#pragma alloc_text(PAGE, MUSA_NAME(GetHandleInformation))
#pragma alloc_text(PAGE, MUSA_NAME(SetHandleInformation))
#endif


EXTERN_C_START
namespace Musa
{
    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MUSA_NAME(CloseHandle)(
        _In_ _Post_ptr_invalid_ HANDLE Handle
        )
    {
        PAGED_CODE();

        const auto Status = ZwClose(Handle);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MUSA_IAT_SYMBOL(CloseHandle, 4);

#pragma warning(push)
#pragma warning(disable: 6101)
    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MUSA_NAME(DuplicateHandle)(
        _In_ HANDLE SourceProcessHandle,
        _In_ HANDLE SourceHandle,
        _In_ HANDLE TargetProcessHandle,
        _Outptr_ LPHANDLE TargetHandle,
        _In_ DWORD  DesiredAccess,
        _In_ BOOL   InheritHandle,
        _In_ DWORD  Options
        )
    {
        PAGED_CODE();

        // TODO: Console
        switch (HandleToULong(SourceHandle)) {
            case STD_INPUT_HANDLE:
            case STD_OUTPUT_HANDLE:
            case STD_ERROR_HANDLE:
                BaseSetLastNTError(STATUS_NOT_IMPLEMENTED);
                return FALSE;
            default:
                break;
        }

        ULONG HandleAttributes = 0;

        if (InheritHandle) {
            HandleAttributes |= OBJ_INHERIT;
        }
    #ifdef _KERNEL_MODE
        else if (ObIsKernelHandle(SourceHandle)) {
            HandleAttributes |= OBJ_KERNEL_HANDLE;
        }
    #endif

        const auto Status = ZwDuplicateObject(
            SourceProcessHandle, SourceHandle,
            TargetProcessHandle, TargetHandle,
            DesiredAccess, HandleAttributes, Options);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MUSA_IAT_SYMBOL(DuplicateHandle, 28);
#pragma warning(pop)

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MUSA_NAME(CompareObjectHandles)(
        _In_ HANDLE FirstHandle,
        _In_ HANDLE SecondHandle
        )
    {
        PAGED_CODE();

        const auto Status = ZwCompareObjects(FirstHandle, SecondHandle);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MUSA_IAT_SYMBOL(CompareObjectHandles, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MUSA_NAME(GetHandleInformation)(
        _In_  HANDLE  Handle,
        _Out_ LPDWORD Flags
        )
    {
        PAGED_CODE();

        *Flags = 0;

        // TODO: Console
        switch (HandleToULong(Handle)) {
        case STD_INPUT_HANDLE:
        case STD_OUTPUT_HANDLE:
        case STD_ERROR_HANDLE:
            BaseSetLastNTError(STATUS_NOT_IMPLEMENTED);
            return FALSE;
        default:
            break;
        }

        OBJECT_HANDLE_FLAG_INFORMATION HandleInfo{};
        const auto Status = ZwQueryObject(Handle, ObjectHandleFlagInformation,
            &HandleInfo, sizeof(HandleInfo), nullptr);
        if (NT_SUCCESS(Status)) {
            if (HandleInfo.Inherit) {
                SetFlag(*Flags, HANDLE_FLAG_INHERIT);
            }

            if (HandleInfo.ProtectFromClose) {
                SetFlag(*Flags, HANDLE_FLAG_PROTECT_FROM_CLOSE);
            }

            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MUSA_IAT_SYMBOL(GetHandleInformation, 8);

    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL WINAPI MUSA_NAME(SetHandleInformation)(
        _In_ HANDLE Handle,
        _In_ DWORD  Mask,
        _In_ DWORD  Flags
        )
    {
        PAGED_CODE();

        // TODO: Console
        switch (HandleToULong(Handle)) {
        case STD_INPUT_HANDLE:
        case STD_OUTPUT_HANDLE:
        case STD_ERROR_HANDLE:
            BaseSetLastNTError(STATUS_NOT_IMPLEMENTED);
            return FALSE;
        default:
            break;
        }

        OBJECT_HANDLE_FLAG_INFORMATION HandleInfo{};
        auto Status = ZwQueryObject(Handle, ObjectHandleFlagInformation, 
            &HandleInfo, sizeof(HandleInfo), nullptr);
        if (NT_SUCCESS(Status)) {
            if (BooleanFlagOn(Mask, HANDLE_FLAG_INHERIT)) {
                HandleInfo.Inherit = BooleanFlagOn(Flags, HANDLE_FLAG_INHERIT) ? TRUE : FALSE;
            }

            if (BooleanFlagOn(Mask, HANDLE_FLAG_PROTECT_FROM_CLOSE)) {
                HandleInfo.ProtectFromClose = BooleanFlagOn(Flags , HANDLE_FLAG_PROTECT_FROM_CLOSE) ? TRUE : FALSE;
            }

            Status = ZwSetInformationObject(Handle, ObjectHandleFlagInformation,
                &HandleInfo, sizeof(HandleInfo));
            if (NT_SUCCESS(Status)) {
                return TRUE;
            }
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MUSA_IAT_SYMBOL(SetHandleInformation, 12);

}
EXTERN_C_END
