#include "KernelBase.Private.h"
#include "KernelBase.LibraryLoader.Private.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(FreeLibrary))
#pragma alloc_text(PAGE, MUSA_NAME(GetModuleFileNameW))
#pragma alloc_text(PAGE, MUSA_NAME(GetModuleHandleW))
#pragma alloc_text(PAGE, MUSA_NAME(GetModuleHandleExW))
#pragma alloc_text(PAGE, MUSA_NAME(GetProcAddress))
#endif

EXTERN_C_START
namespace Musa
{
#if defined(_KERNEL_MODE)
    _IRQL_requires_max_(APC_LEVEL)
    BOOL WINAPI MUSA_NAME(FreeLibrary)(
        _In_ HMODULE DllHandle
    )
    {
        PAGED_CODE();

        // TODO: DATA_FILE

        NTSTATUS Status = LdrUnloadDll(DllHandle);
        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MUSA_IAT_SYMBOL(FreeLibrary, 4);
#endif

#pragma warning(push)
#pragma warning(disable: 6054) // warning C6054: String 'DllFullName' might not be zero-terminated.
    _IRQL_requires_max_(APC_LEVEL)
    _Success_(return != 0)
    _Ret_range_(1, Size)
    DWORD WINAPI MUSA_NAME(GetModuleFileNameW)(
        _In_opt_ HMODULE DllHandle,
        _Out_writes_to_(Size, ((return < Size) ? (return +1) : Size)) LPWSTR DllFullName,
        _In_ DWORD Size
    )
    {
        PAGED_CODE();

        DWORD    Result = 0;
        NTSTATUS Status;

        do {
            if (LDR_IS_RESOURCE(DllHandle)) {
                Status = STATUS_DLL_NOT_FOUND;
                break;
            }

            if (Size > INT16_MAX) {
                Size = INT16_MAX;
            }

            if (Size == 0) {
                Status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            if (DllHandle == nullptr) {
                DllHandle = reinterpret_cast<HMODULE>(RtlCurrentImageBase());
            }

            UNICODE_STRING DllFullName_U{};
            DllFullName_U.Buffer        = DllFullName;
            DllFullName_U.MaximumLength = static_cast<USHORT>((Size - 1) * 2ul);

            Status = LdrGetDllFullName(DllHandle, &DllFullName_U);
            if (Status == STATUS_BUFFER_TOO_SMALL) {
                Result = Size;
            }

        } while (false);

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
        }

        return Result;
    }
    MUSA_IAT_SYMBOL(GetModuleFileNameW, 12);
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 6387)
    _IRQL_requires_max_(APC_LEVEL)
    _When_(ModuleName == NULL, _Ret_notnull_)
    _When_(ModuleName != NULL, _Ret_maybenull_)
    HMODULE WINAPI MUSA_NAME(GetModuleHandleW)(
        _In_opt_ LPCWSTR ModuleName
    )
    {
        PAGED_CODE();

        if (ModuleName == nullptr) {
            return reinterpret_cast<HMODULE>(RtlCurrentImageBase());
        }

        HMODULE DllHandle = nullptr;
        if (GetModuleHandleExW(0, ModuleName, &DllHandle)) {
            return DllHandle;
        }

        return nullptr;
    }
    MUSA_IAT_SYMBOL(GetModuleHandleW, 4);
#pragma warning(pop)

    _IRQL_requires_max_(APC_LEVEL)
    BOOL WINAPI MUSA_NAME(GetModuleHandleExW)(
        _In_ DWORD Flags,
        _In_opt_ LPCWSTR ModuleName,
        _Out_ HMODULE* DllHandle
    )
    {
        PAGED_CODE();

        NTSTATUS Status = STATUS_SUCCESS;

        do {
            if (DllHandle) {
                *DllHandle = nullptr;
            }

            if (Flags & ~(GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS)) {
                Status = STATUS_INVALID_PARAMETER_1;
                break;
            }

            if ((Flags & GET_MODULE_HANDLE_EX_FLAG_PIN) &&
                (Flags & GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT)){
                Status = STATUS_INVALID_PARAMETER_1;
                break;
            }

            if ((ModuleName == nullptr) && ((Flags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS) != 0)) {
                Status = STATUS_INVALID_PARAMETER_1;
                break;
            }

            if (DllHandle == nullptr) {
                Status = STATUS_INVALID_PARAMETER_2;
                break;
            }

            if (ModuleName == nullptr) {
                *DllHandle = reinterpret_cast<HMODULE>(RtlCurrentImageBase());
                break;
            }

            if (Flags & GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS) {
                *DllHandle = static_cast<HMODULE>(RtlPcToFileHeader(
                    const_cast<LPWSTR>(ModuleName), reinterpret_cast<PVOID*>(DllHandle)));
                if (*DllHandle == nullptr) {
                    Status = STATUS_DLL_NOT_FOUND;
                    break;
                }
            }else {
                UNICODE_STRING DllFullName_U{};
                Status = RtlInitUnicodeStringEx(&DllFullName_U, ModuleName);
                if (!NT_SUCCESS(Status)) {
                    break;
                }

                ULONG LdrFlags = 0;
                if (Flags & GET_MODULE_HANDLE_EX_FLAG_PIN) {
                    LdrFlags |= LDR_GET_DLL_HANDLE_EX_PIN;
                }
                if (Flags & GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT) {
                    LdrFlags |= LDR_GET_DLL_HANDLE_EX_UNCHANGED_REFCOUNT;
                }

                Status = LdrGetDllHandleEx(LdrFlags, nullptr, nullptr,
                    &DllFullName_U, reinterpret_cast<PVOID*>(DllHandle));
            }

        } while (false);

        if (NT_SUCCESS(Status)) {
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }
    MUSA_IAT_SYMBOL(GetModuleHandleExW, 12);

    _IRQL_requires_max_(APC_LEVEL)
    FARPROC WINAPI MUSA_NAME(GetProcAddress)(
        _In_ HMODULE Module,
        _In_ LPCSTR ProcedureName
    )
    {
        PAGED_CODE();

        NTSTATUS Status;
        PVOID    ProcedureAddress = nullptr;

        if (reinterpret_cast<uintptr_t>(ProcedureName) > 0xFFFF) {
            STRING ProcedureName_A{};
            RtlInitString(&ProcedureName_A, ProcedureName);

            Status = LdrGetProcedureAddress(Module, &ProcedureName_A, 0,
                &ProcedureAddress);
        }
        else {
            Status = LdrGetProcedureAddress(Module, nullptr, PtrToUlong(ProcedureName),
                &ProcedureAddress);
        }

        if (NT_SUCCESS(Status)) {
            PVOID BaseAddress = nullptr;

            if (Module) {
                if (!LDR_IS_RESOURCE(Module)) {
                    BaseAddress = Module;
                }
            }
            else {
                BaseAddress = RtlCurrentImageBase();
            }

            if (ProcedureAddress == BaseAddress) {
                if (reinterpret_cast<uintptr_t>(ProcedureName) > 0xffff) {
                    Status = STATUS_ENTRYPOINT_NOT_FOUND;
                }
                else {
                    Status = STATUS_ORDINAL_NOT_FOUND;
                }

                ProcedureAddress = nullptr;
                BaseSetLastNTError(Status);
            }
        }

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
        }

        return static_cast<FARPROC>(ProcedureAddress);
    }
    MUSA_IAT_SYMBOL(GetProcAddress, 8);

}
EXTERN_C_END
