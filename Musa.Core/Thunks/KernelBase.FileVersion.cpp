#include "KernelBase.Private.h"
#include "KernelBase.FileVersion.Private.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(GetFileVersionInfoW))
#pragma alloc_text(PAGE, MUSA_NAME(GetFileVersionInfoSizeW))
#endif

EXTERN_C_START
namespace Musa
{
#if defined(_KERNEL_MODE)

    /* Read version info into buffer */
    _IRQL_requires_max_(PASSIVE_LEVEL)
    BOOL APIENTRY MUSA_NAME(GetFileVersionInfoW)(
        _In_                LPCWSTR FileName,   /* Filename of version stamped file */
        _Reserved_          DWORD Handle,       /* Information from GetFileVersionSize */
        _In_                DWORD Length,       /* Length of buffer for info */
        _Out_writes_bytes_(Length) LPVOID Data   /* Buffer to place the data structure */
    )
    {
        PAGED_CODE();
        UNREFERENCED_PARAMETER(Handle);

        NTSTATUS Status;
        PVOID    DllHandle = nullptr;

        do {
            Status = LdrLoadDataFile(FileName, &DllHandle);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            HRSRC ResBase = nullptr;

            Status = RtlFindResource(&ResBase, DllHandle,
                MAKEINTRESOURCE(VS_VERSION_INFO), VS_FILE_INFO, 0);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            PVOID ResBuff = nullptr;
            ULONG ResSize = 0ul;

            Status = RtlLoadResource(&ResBuff, &ResSize, ResBase, DllHandle);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            const auto VerInfoLength = static_cast<PVS_VERSIONINFO>(ResBuff)->TotalLength;

            if (Length < VerInfoLength) {
                Status = STATUS_BUFFER_OVERFLOW;
            }

            __try {
                RtlCopyMemory(Data, ResBuff, min(Length, VerInfoLength));
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
            }

        } while (false);

        if (DllHandle) {
            (void)LdrUnloadDataFile(DllHandle);
        }

        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
        }

        if (!NT_ERROR(Status)) {
            return TRUE;
        }

        return FALSE;
    }
    MUSA_IAT_SYMBOL(GetFileVersionInfoW, 16);
    
    /* Returns size of version info in bytes */
    _IRQL_requires_max_(PASSIVE_LEVEL)
    DWORD APIENTRY MUSA_NAME(GetFileVersionInfoSizeW)(
        _In_      LPCWSTR FileName,  /* Filename of version stamped file */
        _Out_opt_ LPDWORD Handle     /* Information for use by GetFileVersionInfo */
    )
    {
        PAGED_CODE();

        NTSTATUS Status;
        DWORD    Result    = 0;
        PVOID    DllHandle = nullptr;

        do {
            if (Handle) {
                *Handle = 0;
            }

            Status = LdrLoadDataFile(FileName, &DllHandle);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            HRSRC ResBase = nullptr;

            Status = RtlFindResource(&ResBase, DllHandle,
                MAKEINTRESOURCE(VS_VERSION_INFO), VS_FILE_INFO, 0);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            PVOID ResBuff = nullptr;
            ULONG ResSize = 0ul;

            Status = RtlLoadResource(&ResBuff, &ResSize, ResBase, DllHandle);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Result = static_cast<PVS_VERSIONINFO>(ResBuff)->TotalLength;

        } while (false);

        if(DllHandle) {
            (void)LdrUnloadDataFile(DllHandle);
        }

        if (NT_SUCCESS(Status)) {
            return Result;
        }

        BaseSetLastNTError(Status);
        return 0;
    }
    MUSA_IAT_SYMBOL(GetFileVersionInfoSizeW, 8);

#pragma warning(push)
#pragma warning(disable: 6305)  // Potential mismatch between sizeof and countof quantities
    static
    PVS_VERSIONINFO_BLOCK APIENTRY MUSA_NAME_PRIVATE(VerFindNextBlock)(
        _In_ PVS_VERSIONINFO_BLOCK Block,
        _In_ LPCWSTR SubBlock,
        _In_ SIZE_T  Length
    )
    {
        if (Block == nullptr) {
            return nullptr;
        }

        auto Child = __VS_VERSIONINFO_ChildrenFirst(Block);

        while ((LPBYTE)Child < (LPBYTE)Block + Block->TotalLength) {
            if (!_wcsnicmp(Child->Key, SubBlock, Length) && !Child->Key[Length]) {
                return  const_cast<PVS_VERSIONINFO_BLOCK>(Child);
            }

            if (!Child->TotalLength) {
                return nullptr;
            }

            Child = __VS_VERSIONINFO_ChildrenNext(Child);
        }

        return nullptr;
    }

    BOOL APIENTRY MUSA_NAME(VerQueryValueW)(
        _In_ LPCVOID Block,
        _In_ LPCWSTR SubBlock,
        _Outptr_result_buffer_(_Inexpressible_("buffer can be PWSTR or DWORD*")) LPVOID* Buffer,
        _Out_ PUINT Length
    )
    {
        while (*SubBlock) {

            LPCWSTR NextSlash;

            /* Find next path component */
            for (NextSlash = SubBlock; *NextSlash; NextSlash++) {
                if (*NextSlash == '\\') {
                    break;
                }
            }

            /* Skip empty components */
            if (NextSlash == SubBlock) {
                SubBlock++;
                continue;
            }

            /* We have a non-empty component: search info for key */
            Block = MUSA_NAME_PRIVATE(VerFindNextBlock)(static_cast<PVS_VERSIONINFO_BLOCK>(const_cast<PVOID>(Block)),
                SubBlock, NextSlash - SubBlock);
            if (!Block) {
                BaseSetLastNTError(STATUS_RESOURCE_NAME_NOT_FOUND);
                return FALSE;
            }

            /* Skip path component */
            SubBlock = NextSlash;
        }

        /* Return value */
        *Buffer = __VS_VERSIONINFO_ValueBuffer(Block);
        *Length = __VS_VERSIONINFO_ValueLength(Block);

        return TRUE;
    }
    MUSA_IAT_SYMBOL(VerQueryValueW, 16);
#pragma warning(pop)


#endif

}
EXTERN_C_END
