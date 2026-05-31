// Iteration 1-2: 2026-05-02 -- GetFileType, ReadFile, WriteFile, FlushFileBuffers
// Build: PASS (x64/ARM64, Debug/Release) -- 0 errors
#include "Musa.Core/Musa.Core.SystemEnvironmentBlock.Process.h"
#include "KernelBase.Private.h"
#include "Internal/KernelBase.File.h"
#include "Internal/KernelBase.System.h"
#include "Internal/Ntdll.Path.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(GetFileType))
#pragma alloc_text(PAGE, MUSA_NAME(CreateFileW))
#pragma alloc_text(PAGE, MUSA_NAME(GetFileAttributesExW))
#pragma alloc_text(PAGE, MUSA_NAME(DeleteFileW))
#pragma alloc_text(PAGE, MUSA_NAME(SetFileAttributesW))
#pragma alloc_text(PAGE, MUSA_NAME(GetTempPathW))
#pragma alloc_text(PAGE, MUSA_NAME(CreateDirectoryW))
#pragma alloc_text(PAGE, MUSA_NAME(RemoveDirectoryW))
#pragma alloc_text(PAGE, MUSA_NAME(MoveFileExW))

#pragma alloc_text(PAGE, MUSA_NAME(GetFullPathNameW))

#pragma alloc_text(PAGE, MUSA_NAME(PeekNamedPipe))

#pragma alloc_text(PAGE, MUSA_NAME(CreateNamedPipeW))

#pragma alloc_text(PAGE, MUSA_NAME(SetEndOfFile))
#pragma alloc_text(PAGE, MUSA_NAME(GetFileTime))
#pragma alloc_text(PAGE, MUSA_NAME(GetFileSizeEx))
#pragma alloc_text(PAGE, MUSA_NAME(GetFileInformationByHandle))
#pragma alloc_text(PAGE, MUSA_NAME(GetDriveTypeW))
#pragma alloc_text(PAGE, MUSA_NAME(FindFirstFileExW))
#pragma alloc_text(PAGE, MUSA_NAME(SetFileTime))
#pragma alloc_text(PAGE, MUSA_NAME(FindClose))

#pragma alloc_text(PAGE, MUSA_NAME(GetFileAttributesW))
#pragma alloc_text(PAGE, MUSA_NAME(CreateFile2))
#pragma alloc_text(PAGE, MUSA_NAME(GetFileInformationByHandleEx))
#pragma alloc_text(PAGE, MUSA_NAME(SetFileInformationByHandle))
#pragma alloc_text(PAGE, MUSA_NAME(CreateSymbolicLinkW))
#pragma alloc_text(PAGE, MUSA_NAME(GetFinalPathNameByHandleW))
#pragma alloc_text(PAGE, MUSA_NAME(GetDiskFreeSpaceExW))
#pragma alloc_text(PAGE, MUSA_NAME(CopyFile2))
#pragma alloc_text(PAGE, MUSA_NAME(FindFirstFileW))
#pragma alloc_text(PAGE, MUSA_NAME(DeviceIoControl))
#pragma alloc_text(PAGE, MUSA_NAME(CreateHardLinkW))
#pragma alloc_text(PAGE, MUSA_NAME(CreateDirectoryExW))
#endif


EXTERN_C_START
/**
 * GetFileType - Determines the type of the specified file.
 *
 * @param hFile  Handle to the file.
 * @return       FILE_TYPE_DISK, FILE_TYPE_CHAR, FILE_TYPE_PIPE, or FILE_TYPE_UNKNOWN.
 *
 * Maps to NtQueryVolumeInformationFile(FileFsDeviceInformation) -- matches kernelbase.dll.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetFileType)(
    _In_ HANDLE hFile
)
{
    PAGED_CODE();


    FILE_FS_DEVICE_INFORMATION FsInfo{};
    IO_STATUS_BLOCK Iosb{};
    NTSTATUS Status = ZwQueryVolumeInformationFile(
        hFile, &Iosb, &FsInfo, sizeof(FsInfo), FileFsDeviceInformation);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FILE_TYPE_UNKNOWN;
    }

    switch (FsInfo.DeviceType) {
        // FILE_TYPE_DISK
        case FILE_DEVICE_CD_ROM:
        case FILE_DEVICE_CONTROLLER:
        case FILE_DEVICE_DATALINK:
        case FILE_DEVICE_DFS:
        case FILE_DEVICE_DISK:
        case FILE_DEVICE_DISK_FILE_SYSTEM:
        case FILE_DEVICE_DVD:
        case FILE_DEVICE_SCANNER:
            return FILE_TYPE_DISK;
        // FILE_TYPE_PIPE
        case FILE_DEVICE_NAMED_PIPE:
            return FILE_TYPE_PIPE;
        // FILE_TYPE_CHAR
        case FILE_DEVICE_FILE_SYSTEM:
        case FILE_DEVICE_INPORT_PORT:
        case FILE_DEVICE_NETWORK:
        case FILE_DEVICE_NETWORK_BROWSER:
        case FILE_DEVICE_NULL:
        case FILE_DEVICE_PARALLEL_PORT:
        case FILE_DEVICE_PHYSICAL_NETCARD:
        case FILE_DEVICE_PRINTER:
        case FILE_DEVICE_SCREEN:
        case FILE_DEVICE_VIRTUAL_DISK:
            return FILE_TYPE_CHAR;
        default:
            RtlSetLastWin32Error(0);
            return FILE_TYPE_UNKNOWN;
    }
}

MUSA_IAT_SYMBOL(GetFileType, 4);

/**
 * ReadFile - Reads data from a file.
 *
 * @param hFile               Handle to the file.
 * @param lpBuffer            Buffer to receive the read data.
 * @param nNumberOfBytesToRead Number of bytes to read.
 * @param lpNumberOfBytesRead  Number of bytes read (optional).
 * @param lpOverlapped         Overlapped structure (optional, not supported in kernel mode).
 * @return                     TRUE on success, FALSE on failure.
 *
 * Maps to ZwReadFile.
 */
_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MUSA_NAME(ReadFile)(
    _In_ HANDLE       hFile,
    _Out_writes_bytes_to_opt_(nNumberOfBytesToRead, *lpNumberOfBytesRead) __out_data_source(FILE) LPVOID lpBuffer,
    _In_ DWORD        nNumberOfBytesToRead,
    _Out_opt_ LPDWORD lpNumberOfBytesRead,
    _Inout_opt_ LPOVERLAPPED lpOverlapped
)
{
    if (lpBuffer == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    if (lpOverlapped) {
        // Overlapped I/O path
        lpOverlapped->Internal = STATUS_PENDING;

        HANDLE hEvent = lpOverlapped->hEvent;
        PVOID ApcContext = ((reinterpret_cast<ULONG_PTR>(hEvent) & 1) == 0) ? lpOverlapped : nullptr;

        LARGE_INTEGER Offset;
        Offset.LowPart = lpOverlapped->Offset;
        Offset.HighPart = static_cast<LONG>(lpOverlapped->OffsetHigh);

        NTSTATUS Status = ZwReadFile(
            hFile, hEvent, nullptr, ApcContext,
            reinterpret_cast<PIO_STATUS_BLOCK>(lpOverlapped),
            lpBuffer, nNumberOfBytesToRead, &Offset, nullptr);

        if (Status != STATUS_PENDING && (Status & 0xC0000000) != 0xC0000000) {
            if (lpNumberOfBytesRead) {
                *lpNumberOfBytesRead = static_cast<DWORD>(lpOverlapped->InternalHigh);
            }
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }

    // Synchronous I/O path
    if (lpNumberOfBytesRead) {
        *lpNumberOfBytesRead = 0;
    }

    if (lpBuffer == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    IO_STATUS_BLOCK Iosb;
    Iosb.Status = STATUS_PENDING;
    Iosb.Information = 0;
    NTSTATUS Status = ZwReadFile(
        hFile, nullptr, nullptr, nullptr, &Iosb,
        lpBuffer, nNumberOfBytesToRead, nullptr, nullptr);

    if (Status == STATUS_PENDING) {
        Status = ZwWaitForSingleObject(hFile, FALSE, nullptr);
        if (NT_SUCCESS(Status)) {
            Status = Iosb.Status;
        }
    }

    if (NT_SUCCESS(Status)) {
        if (lpNumberOfBytesRead) {
            *lpNumberOfBytesRead = static_cast<DWORD>(Iosb.Information);
        }
        return TRUE;
    }

    // STATUS_END_OF_FILE is not an error for ReadFile
    if (Status == STATUS_END_OF_FILE) {
        return TRUE;
    }

    // Warning status codes (0x80000000 range) are not errors
    if ((Status & 0xC0000000) == 0x80000000) {
        if (lpNumberOfBytesRead) {
            *lpNumberOfBytesRead = static_cast<DWORD>(Iosb.Information);
        }
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(ReadFile, 20);

/**
 * WriteFile - Writes data to a file.
 *
 * @param hFile                Handle to the file.
 * @param lpBuffer             Buffer containing the data to write.
 * @param nNumberOfBytesToWrite Number of bytes to write.
 * @param lpNumberOfBytesWritten Number of bytes written (optional).
 * @param lpOverlapped         Overlapped structure (optional, not supported in kernel mode).
 * @return                     TRUE on success, FALSE on failure.
 *
 * Maps to ZwWriteFile.
 */
_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MUSA_NAME(WriteFile)(
    _In_ HANDLE       hFile,
    _In_reads_bytes_(nNumberOfBytesToWrite) LPCVOID lpBuffer,
    _In_ DWORD        nNumberOfBytesToWrite,
    _Out_opt_ LPDWORD lpNumberOfBytesWritten,
    _Inout_opt_ LPOVERLAPPED lpOverlapped
)
{

    if (lpOverlapped) {
        // Overlapped I/O path
        lpOverlapped->Internal = STATUS_PENDING;

        HANDLE hEvent = lpOverlapped->hEvent;
        PVOID ApcContext = ((reinterpret_cast<ULONG_PTR>(hEvent) & 1) == 0) ? lpOverlapped : nullptr;

        LARGE_INTEGER Offset;
        Offset.LowPart = lpOverlapped->Offset;
        Offset.HighPart = static_cast<LONG>(lpOverlapped->OffsetHigh);

        NTSTATUS Status = ZwWriteFile(
            hFile, hEvent, nullptr, ApcContext,
            reinterpret_cast<PIO_STATUS_BLOCK>(lpOverlapped),
            const_cast<LPVOID>(lpBuffer), nNumberOfBytesToWrite, &Offset, nullptr);

        if (Status != STATUS_PENDING && (Status & 0xC0000000) != 0xC0000000) {
            if (lpNumberOfBytesWritten) {
                *lpNumberOfBytesWritten = static_cast<DWORD>(lpOverlapped->InternalHigh);
            }
            return TRUE;
        }

        BaseSetLastNTError(Status);
        return FALSE;
    }

    // Synchronous I/O path
    if (lpNumberOfBytesWritten) {
        *lpNumberOfBytesWritten = 0;
    }

    if (lpBuffer == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    IO_STATUS_BLOCK Iosb;
    Iosb.Status = STATUS_PENDING;
    Iosb.Information = 0;
    NTSTATUS Status = ZwWriteFile(
        hFile, nullptr, nullptr, nullptr, &Iosb,
        const_cast<LPVOID>(lpBuffer), nNumberOfBytesToWrite, nullptr, nullptr);

    if (Status == STATUS_PENDING) {
        Status = ZwWaitForSingleObject(hFile, FALSE, nullptr);
        if (NT_SUCCESS(Status)) {
            Status = Iosb.Status;
        }
    }

    if (NT_SUCCESS(Status)) {
        if (lpNumberOfBytesWritten) {
            *lpNumberOfBytesWritten = static_cast<DWORD>(Iosb.Information);
        }
        return TRUE;
    }

    // Warning status codes (0x80000000 range) are not errors
    if ((Status & 0xC0000000) == 0x80000000) {
        if (lpNumberOfBytesWritten) {
            *lpNumberOfBytesWritten = static_cast<DWORD>(Iosb.Information);
        }
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(WriteFile, 20);

/**
 * FlushFileBuffers - Flushes the buffers of a file and causes all buffered data
 * to be written to the file.
 *
 * @param hFile  Handle to the file.
 * @return       TRUE on success, FALSE on failure.
 *
 * Maps to ZwFlushBuffersFile.
 */
_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MUSA_NAME(FlushFileBuffers)(
    _In_ HANDLE hFile
)
{

    IO_STATUS_BLOCK Iosb{};
    NTSTATUS Status = ZwFlushBuffersFile(hFile, &Iosb);
    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(FlushFileBuffers, 4);

/**
 * CreateFileW - Creates or opens a file or I/O device.
 * Maps to ZwCreateFile with path conversion via RtlDosPathNameToNtPathName_U.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(CreateFileW)(
    _In_ LPCWSTR lpFileName,
    _In_ DWORD dwDesiredAccess,
    _In_ DWORD dwShareMode,
    _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    _In_ DWORD dwCreationDisposition,
    _In_ DWORD dwFlagsAndAttributes,
    _In_opt_ HANDLE hTemplateFile
)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(lpSecurityAttributes);

    if (lpFileName == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    UNICODE_STRING NtPath{};
    NTSTATUS Status = RtlDosPathNameToNtPathName_U_WithStatus(lpFileName, &NtPath, nullptr, nullptr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
    }

    OBJECT_ATTRIBUTES ObjAttrs;
    InitializeObjectAttributes(&ObjAttrs, &NtPath,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        nullptr, nullptr);

    ULONG CreateDisposition;
    switch (dwCreationDisposition) {
        case CREATE_NEW:
            CreateDisposition = FILE_CREATE;
            break;
        case CREATE_ALWAYS:
            CreateDisposition = FILE_OVERWRITE_IF;
            break;
        case OPEN_EXISTING:
            CreateDisposition = FILE_OPEN;
            break;
        case OPEN_ALWAYS:
            CreateDisposition = FILE_OPEN_IF;
            break;
        case TRUNCATE_EXISTING:
            CreateDisposition = FILE_OVERWRITE;
            break;
        default:
            RtlFreeUnicodeString(&NtPath);
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
    }

    ULONG FileAttributes = dwFlagsAndAttributes & FILE_ATTRIBUTE_VALID_FLAGS;
    ULONG CreateOptions = 0;

    if (dwFlagsAndAttributes & FILE_FLAG_WRITE_THROUGH)
        CreateOptions |= FILE_WRITE_THROUGH;
    // Kernel-mode: default to synchronous unless OVERLAPPED is requested.
    // ZwCreateFile requires SYNCHRONIZE in DesiredAccess when using
    // FILE_SYNCHRONOUS_IO_*; user-mode kernelbase!CreateFileW does this
    // translation implicitly, so we must do it explicitly here.
    if (!(dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED)) {
        CreateOptions |= FILE_SYNCHRONOUS_IO_NONALERT;
        dwDesiredAccess |= SYNCHRONIZE;
    }
    if (dwFlagsAndAttributes & FILE_FLAG_NO_BUFFERING) {
        CreateOptions |= FILE_NO_INTERMEDIATE_BUFFERING;
    }
    if (dwFlagsAndAttributes & FILE_FLAG_RANDOM_ACCESS)
        CreateOptions |= FILE_RANDOM_ACCESS;
    if (dwFlagsAndAttributes & FILE_FLAG_SEQUENTIAL_SCAN)
        CreateOptions |= FILE_SEQUENTIAL_ONLY;
    if (dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE)
        CreateOptions |= FILE_DELETE_ON_CLOSE;
    if (dwFlagsAndAttributes & FILE_FLAG_BACKUP_SEMANTICS)
        CreateOptions |= FILE_OPEN_FOR_BACKUP_INTENT;
    if (dwFlagsAndAttributes & FILE_FLAG_OPEN_REPARSE_POINT)
        CreateOptions |= FILE_OPEN_REPARSE_POINT;

    if (dwFlagsAndAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        CreateOptions |= FILE_DIRECTORY_FILE;
        if (dwDesiredAccess == 0)
            dwDesiredAccess = FILE_READ_ATTRIBUTES | FILE_READ_DATA;
    } else {
        CreateOptions |= FILE_NON_DIRECTORY_FILE;
    }

    // Handle template file: inherit attributes if no specific ones requested
    IO_STATUS_BLOCK Iosb{};
    if (hTemplateFile != nullptr && hTemplateFile != INVALID_HANDLE_VALUE) {
        FILE_BASIC_INFORMATION TemplateInfo{};
        NTSTATUS TplStatus = ZwQueryInformationFile(hTemplateFile, &Iosb,
            &TemplateInfo, sizeof(TemplateInfo), FileBasicInformation);
        if (NT_SUCCESS(TplStatus)) {
            // Remove FILE_ATTRIBUTE_NORMAL (0x80) pattern - user didn't specify specifics
            if ((FileAttributes & ~FILE_ATTRIBUTE_NORMAL) == 0) {
                FileAttributes |= (TemplateInfo.FileAttributes & ~FILE_ATTRIBUTE_NORMAL);
            }
        }
    }

    IO_STATUS_BLOCK Iosb2{};
    HANDLE FileHandle = nullptr;
    Status = ZwCreateFile(
        &FileHandle,
        dwDesiredAccess,
        &ObjAttrs,
        &Iosb2,
        nullptr,
        FileAttributes,
        dwShareMode,
        CreateDisposition,
        CreateOptions,
        nullptr,
        0);

    RtlFreeUnicodeString(&NtPath);

    if (NT_SUCCESS(Status)) {
        return FileHandle;
    }

    BaseSetLastNTError(Status);
    return INVALID_HANDLE_VALUE;
}

MUSA_IAT_SYMBOL(CreateFileW, 28);

/**
 * CreateFile2 - Creates or opens a file or I/O device with extended parameters.
 *
 * Maps to ZwCreateFile, extracting dwFileAttributes and dwFileFlags from
 * CREATEFILE2_EXTENDED_PARAMETERS, merging them into the flags/attributes.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(CreateFile2)(
    _In_ LPCWSTR lpFileName,
    _In_ DWORD dwDesiredAccess,
    _In_ DWORD dwShareMode,
    _In_ DWORD dwCreationDisposition,
    _In_opt_ LPCREATEFILE2_EXTENDED_PARAMETERS pCreateExParams
)
{
    PAGED_CODE();
    DWORD dwFlagsAndAttributes = 0;
    LPSECURITY_ATTRIBUTES lpSecurityAttributes = nullptr;
    HANDLE hTemplateFile = nullptr;
    if (pCreateExParams) {
        if (pCreateExParams->dwSize < sizeof(CREATEFILE2_EXTENDED_PARAMETERS)) {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
        }
        dwFlagsAndAttributes = pCreateExParams->dwFileAttributes | pCreateExParams->dwFileFlags;
        lpSecurityAttributes = pCreateExParams->lpSecurityAttributes;
        hTemplateFile = pCreateExParams->hTemplateFile;
    }

    return CreateFileW(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);
}
MUSA_IAT_SYMBOL(CreateFile2, 20);

/**
 * SetFilePointerEx - Moves the file pointer of the specified file.
 *
 * Maps to ZwSetInformationFile(FilePositionInformation).
 */
_IRQL_requires_max_(APC_LEVEL)
BOOL WINAPI MUSA_NAME(SetFilePointerEx)(
    _In_ HANDLE hFile,
    _In_ LARGE_INTEGER liDistanceToMove,
    _Out_opt_ PLARGE_INTEGER lpNewFilePointer,
    _In_ DWORD dwMoveMethod
)
{
    if (dwMoveMethod > FILE_END) {
        RtlSetLastWin32Error(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    FILE_POSITION_INFORMATION PosInfo;
    FILE_STANDARD_INFORMATION StandardInfo;
    IO_STATUS_BLOCK Iosb{};
    LARGE_INTEGER NewPosition;
    NTSTATUS Status = STATUS_SUCCESS;

    if (dwMoveMethod == FILE_BEGIN) {
        NewPosition.QuadPart = liDistanceToMove.QuadPart;
    } else if (dwMoveMethod == FILE_CURRENT) {
        Status = ZwQueryInformationFile(hFile, &Iosb, &PosInfo, sizeof(PosInfo), FilePositionInformation);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }
        NewPosition.QuadPart = PosInfo.CurrentByteOffset.QuadPart + liDistanceToMove.QuadPart;
    } else { // FILE_END
        Status = ZwQueryInformationFile(hFile, &Iosb, &StandardInfo, sizeof(StandardInfo), FileStandardInformation);
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }
        NewPosition.QuadPart = StandardInfo.EndOfFile.QuadPart + liDistanceToMove.QuadPart;
    }

    if (NewPosition.QuadPart < 0) {
        RtlSetLastWin32Error(ERROR_NEGATIVE_SEEK);
        return FALSE;
    }

    Status = ZwSetInformationFile(hFile, &Iosb, &NewPosition.QuadPart, sizeof(NewPosition.QuadPart), FilePositionInformation);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (lpNewFilePointer) {
        lpNewFilePointer->QuadPart = NewPosition.QuadPart;
    }
    return TRUE;
}

MUSA_IAT_SYMBOL(SetFilePointerEx, 16);

/**
 * SetFilePointer - Legacy 32-bit version of SetFilePointerEx.
 */
_IRQL_requires_max_(APC_LEVEL)
DWORD WINAPI MUSA_NAME(SetFilePointer)(
    _In_ HANDLE hFile,
    _In_ LONG lDistanceToMove,
    _Inout_opt_ PLONG lpDistanceToMoveHigh,
    _In_ DWORD dwMoveMethod
)
{
    LARGE_INTEGER Distance;
    Distance.LowPart = lDistanceToMove;
    Distance.HighPart = lpDistanceToMoveHigh ? *lpDistanceToMoveHigh : 0;

    LARGE_INTEGER NewPosition;
    BOOL Result = MUSA_NAME(SetFilePointerEx)(hFile, Distance, &NewPosition, dwMoveMethod);

    if (Result) {
        if (lpDistanceToMoveHigh) {
            *lpDistanceToMoveHigh = NewPosition.HighPart;
        }
        return NewPosition.LowPart;
    }

    return INVALID_SET_FILE_POINTER;
}

MUSA_IAT_SYMBOL(SetFilePointer, 16);

/**
 * GetFileAttributesExW - Retrieves file or directory attributes.
 *
 * Maps to ZwQueryAttributesFile.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetFileAttributesExW)(
    _In_ LPCWSTR lpFileName,
    _In_ GET_FILEEX_INFO_LEVELS fInfoLevelId,
    _Out_ LPVOID lpFileInformation
)
{
    PAGED_CODE();

    if (lpFileName == nullptr || lpFileInformation == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    if (fInfoLevelId != GetFileExInfoStandard) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    UNICODE_STRING NtPath{};
    NTSTATUS Status = RtlDosPathNameToNtPathName_U_WithStatus(lpFileName, &NtPath, nullptr, nullptr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    OBJECT_ATTRIBUTES ObjAttrs;
    InitializeObjectAttributes(&ObjAttrs, &NtPath, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
    FILE_NETWORK_OPEN_INFORMATION NetInfo{};
    Status = ZwQueryFullAttributesFile(&ObjAttrs, &NetInfo);
    RtlFreeUnicodeString(&NtPath);

    if (NT_SUCCESS(Status)) {
        WIN32_FILE_ATTRIBUTE_DATA* OutData = static_cast<WIN32_FILE_ATTRIBUTE_DATA*>(lpFileInformation);
        OutData->dwFileAttributes = NetInfo.FileAttributes;
        OutData->ftCreationTime = reinterpret_cast<FILETIME&>(NetInfo.CreationTime);
        OutData->ftLastAccessTime = reinterpret_cast<FILETIME&>(NetInfo.LastAccessTime);
        OutData->ftLastWriteTime = reinterpret_cast<FILETIME&>(NetInfo.LastWriteTime);
        OutData->nFileSizeHigh = NetInfo.EndOfFile.HighPart;
        OutData->nFileSizeLow = NetInfo.EndOfFile.LowPart;
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(GetFileAttributesExW, 12);

/**
 * DeleteFileW - Deletes an existing file.
 *
 * Maps to ZwDeleteFile.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(DeleteFileW)(
    _In_ LPCWSTR lpFileName
)
{
    PAGED_CODE();

    if (lpFileName == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    UNICODE_STRING NtPath{};
    NTSTATUS Status = RtlDosPathNameToNtPathName_U_WithStatus(lpFileName, &NtPath, nullptr, nullptr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    OBJECT_ATTRIBUTES ObjAttrs;
    InitializeObjectAttributes(&ObjAttrs, &NtPath,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        nullptr, nullptr);
    Status = ZwDeleteFile(&ObjAttrs);
    RtlFreeUnicodeString(&NtPath);

    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

/**
 * GetFileAttributesW - Retrieves file system attributes for a specified file or directory.
 *
 * Maps to ZwQueryAttributesFile.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetFileAttributesW)(
    _In_ LPCWSTR lpFileName
)
{
    PAGED_CODE();

    if (lpFileName == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return INVALID_FILE_ATTRIBUTES;
    }

    UNICODE_STRING NtPath{};
    NTSTATUS Status = RtlDosPathNameToNtPathName_U_WithStatus(lpFileName, &NtPath, nullptr, nullptr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return INVALID_FILE_ATTRIBUTES;
    }
    OBJECT_ATTRIBUTES ObjAttrs;
    InitializeObjectAttributes(&ObjAttrs, &NtPath, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
    FILE_BASIC_INFORMATION BasicInfo{};
    Status = ZwQueryAttributesFile(&ObjAttrs, &BasicInfo);
    RtlFreeUnicodeString(&NtPath);
    if (NT_SUCCESS(Status)) {
        return BasicInfo.FileAttributes;
    }
    BaseSetLastNTError(Status);
    return INVALID_FILE_ATTRIBUTES;
}
MUSA_IAT_SYMBOL(GetFileAttributesW, 4);

MUSA_IAT_SYMBOL(DeleteFileW, 4);

/**
 * SetFileAttributesW - Sets file attributes.
 *
 * Maps to ZwOpenFile + ZwSetInformationFile(FileBasicInformation).
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SetFileAttributesW)(
    _In_ LPCWSTR lpFileName,
    _In_ DWORD dwFileAttributes
)
{
    PAGED_CODE();

    if (lpFileName == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    UNICODE_STRING NtPath{};
    NTSTATUS Status = RtlDosPathNameToNtPathName_U_WithStatus(lpFileName, &NtPath, nullptr, nullptr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    OBJECT_ATTRIBUTES ObjAttrs;
    InitializeObjectAttributes(&ObjAttrs, &NtPath,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        nullptr, nullptr);

    HANDLE FileHandle = nullptr;
    IO_STATUS_BLOCK Iosb{};
    Status = ZwOpenFile(&FileHandle, FILE_WRITE_ATTRIBUTES | SYNCHRONIZE, &ObjAttrs, &Iosb,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN_FOR_BACKUP_INTENT);
    RtlFreeUnicodeString(&NtPath);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    FILE_BASIC_INFORMATION BasicInfo;
    BasicInfo.FileAttributes = dwFileAttributes;
    BasicInfo.CreationTime.QuadPart = 0;
    BasicInfo.LastAccessTime.QuadPart = 0;
    BasicInfo.LastWriteTime.QuadPart = 0;
    BasicInfo.ChangeTime.QuadPart = 0;
    Status = ZwSetInformationFile(FileHandle, &Iosb, &BasicInfo, sizeof(BasicInfo), FileBasicInformation);
    ZwClose(FileHandle);

    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(SetFileAttributesW, 8);


#pragma warning(push)
#pragma warning(disable: 6385)
_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetTempPathW)(
    _In_ DWORD nBufferLength,
    _Out_writes_to_opt_(nBufferLength, return + 1) LPWSTR lpBuffer
)
{
    PAGED_CODE();

    // Try TMP first, then TEMP, then fallback. Expand env vars
    // because they may contain %SystemRoot% tokens.
    static LPCWSTR Fallback = L"C:\\Windows\\Temp\\";
    LPCWSTR Candidates[] = { L"TMP", L"TEMP" };

    for (const auto& Name : Candidates) {
        DWORD Len = GetEnvironmentVariableW(Name, nullptr, 0);
        if (Len == 0 || Len > nBufferLength) continue;
        Len = GetEnvironmentVariableW(Name, lpBuffer, nBufferLength);
        if (Len == 0) continue;
        // Expand %SystemRoot% etc.
        wchar_t Expanded[261];
        DWORD ExpLen = ExpandEnvironmentStringsW(lpBuffer, Expanded, 261);
        if (ExpLen == 0 || ExpLen > nBufferLength) continue;
        wcscpy_s(lpBuffer, nBufferLength, Expanded);
        return ExpLen - 1;
    }

    // Fallback to default
    DWORD FallbackLen = static_cast<DWORD>(wcslen(Fallback)) + 1;
    if (lpBuffer == nullptr || nBufferLength == 0) {
        return FallbackLen;
    }
    if (FallbackLen > nBufferLength) {
        BaseSetLastNTError(STATUS_BUFFER_TOO_SMALL);
        return FallbackLen;
    }
#pragma warning(suppress: 6385)
    RtlStringCchCopyW(lpBuffer, nBufferLength, Fallback);
    return FallbackLen - 1;
}
#pragma warning(pop)

MUSA_IAT_SYMBOL(GetTempPathW, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(CreateDirectoryW)(
    _In_ LPCWSTR lpPathName,
    _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(lpSecurityAttributes);

    if (lpPathName == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    UNICODE_STRING NtPath{};
    NTSTATUS Status = RtlDosPathNameToNtPathName_U_WithStatus(lpPathName, &NtPath, nullptr, nullptr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    OBJECT_ATTRIBUTES ObjAttrs;
    InitializeObjectAttributes(&ObjAttrs, &NtPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);

    HANDLE DirHandle = nullptr;
    IO_STATUS_BLOCK Iosb{};
    Status = ZwCreateFile(&DirHandle, FILE_LIST_DIRECTORY | SYNCHRONIZE, &ObjAttrs, &Iosb,
        nullptr, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_CREATE, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
        nullptr, 0);
    RtlFreeUnicodeString(&NtPath);

    if (NT_SUCCESS(Status)) {
        ZwClose(DirHandle);
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(CreateDirectoryW, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(RemoveDirectoryW)(
    _In_ LPCWSTR lpPathName
)
{
    PAGED_CODE();

    if (lpPathName == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    UNICODE_STRING NtPath{};
    NTSTATUS Status = RtlDosPathNameToNtPathName_U_WithStatus(lpPathName, &NtPath, nullptr, nullptr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    OBJECT_ATTRIBUTES ObjAttrs;
    InitializeObjectAttributes(&ObjAttrs, &NtPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);

    HANDLE DirHandle = nullptr;
    IO_STATUS_BLOCK Iosb{};
    Status = ZwOpenFile(&DirHandle, DELETE | SYNCHRONIZE, &ObjAttrs, &Iosb,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT);
    RtlFreeUnicodeString(&NtPath);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    FILE_DISPOSITION_INFORMATION_EX DispInfo;
    DispInfo.Flags = FILE_DISPOSITION_DELETE | FILE_DISPOSITION_POSIX_SEMANTICS;
    Status = ZwSetInformationFile(DirHandle, &Iosb, &DispInfo, sizeof(DispInfo), FileDispositionInformationEx);
    ZwClose(DirHandle);

    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(RemoveDirectoryW, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(MoveFileExW)(
    _In_ LPCWSTR lpExistingFileName,
    _In_ LPCWSTR lpNewFileName,
    _In_ DWORD dwFlags
)
{
    PAGED_CODE();

    if (lpExistingFileName == nullptr || lpNewFileName == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    UNICODE_STRING NtPath{};
    NTSTATUS Status = RtlDosPathNameToNtPathName_U_WithStatus(lpExistingFileName, &NtPath, nullptr, nullptr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    OBJECT_ATTRIBUTES ObjAttrs;
    InitializeObjectAttributes(&ObjAttrs, &NtPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);

    HANDLE FileHandle = nullptr;
    IO_STATUS_BLOCK Iosb{};
    Status = ZwOpenFile(&FileHandle, DELETE | SYNCHRONIZE, &ObjAttrs, &Iosb,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN_FOR_BACKUP_INTENT);
    RtlFreeUnicodeString(&NtPath);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    // Build the rename target path
    UNICODE_STRING NewNtPath{};
    Status = RtlDosPathNameToNtPathName_U_WithStatus(lpNewFileName, &NewNtPath, nullptr, nullptr);
    if (!NT_SUCCESS(Status)) {
        ZwClose(FileHandle);
        BaseSetLastNTError(Status);
        return FALSE;
    }

    FILE_RENAME_INFORMATION* RenameInfo = static_cast<FILE_RENAME_INFORMATION*>(
        RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY,
            sizeof(FILE_RENAME_INFORMATION) + NewNtPath.Length));
    if (RenameInfo == nullptr) {
        RtlFreeUnicodeString(&NewNtPath);
        ZwClose(FileHandle);
        BaseSetLastNTError(STATUS_INSUFFICIENT_RESOURCES);
        return FALSE;
    }

    RenameInfo->ReplaceIfExists = (dwFlags & MOVEFILE_REPLACE_EXISTING) != 0;
    RenameInfo->RootDirectory = nullptr;
    RenameInfo->FileNameLength = NewNtPath.Length;
    memcpy(RenameInfo->FileName, NewNtPath.Buffer, NewNtPath.Length);

    Status = ZwSetInformationFile(FileHandle, &Iosb, RenameInfo,
        sizeof(FILE_RENAME_INFORMATION) + NewNtPath.Length, FileRenameInformation);

    RtlFreeHeap(RtlProcessHeap(), 0, RenameInfo);
    RtlFreeUnicodeString(&NewNtPath);
    ZwClose(FileHandle);

    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(MoveFileExW, 12);

_IRQL_requires_max_(PASSIVE_LEVEL)
UINT WINAPI MUSA_NAME(GetDriveTypeW)(
    _In_opt_ LPCWSTR lpRootPathName
)
{
    PAGED_CODE();

    if (lpRootPathName == nullptr || lpRootPathName[0] == L'\0') {
        // No path specified - return DRIVE_UNKNOWN
        return DRIVE_UNKNOWN;
    }

    // Build a root path for querying the volume
    WCHAR RootPath[MAX_PATH + 1];
    if (lpRootPathName[1] == L':') {
        // Drive letter path (C:)
        RootPath[0] = lpRootPathName[0];
        RootPath[1] = L':';
        RootPath[2] = L'\\';
        RootPath[3] = L'\0';
    } else if (lpRootPathName[0] == L'\\' && lpRootPathName[1] == L'\\') {
        // UNC path
        wcscpy_s(RootPath, lpRootPathName);
    } else {
        wcscpy_s(RootPath, lpRootPathName);
    }

    UNICODE_STRING NtPath{};
    NTSTATUS Status = RtlDosPathNameToNtPathName_U_WithStatus(RootPath, &NtPath, nullptr, nullptr);
    if (!NT_SUCCESS(Status)) {
        return DRIVE_UNKNOWN;
    }

    // Open the volume root
    OBJECT_ATTRIBUTES ObjAttrs;
    InitializeObjectAttributes(&ObjAttrs, &NtPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);

    HANDLE VolHandle = nullptr;
    IO_STATUS_BLOCK Iosb{};
    Status = ZwCreateFile(&VolHandle, FILE_READ_ATTRIBUTES | SYNCHRONIZE, &ObjAttrs, &Iosb,
        nullptr, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN, FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT, nullptr, 0);

    if (NT_SUCCESS(Status)) {
        // Query filesystem device type
        FILE_FS_DEVICE_INFORMATION FsInfo{};
        Status = ZwQueryVolumeInformationFile(VolHandle, &Iosb, &FsInfo, sizeof(FsInfo), FileFsDeviceInformation);
        ZwClose(VolHandle);
        RtlFreeUnicodeString(&NtPath);

        if (NT_SUCCESS(Status)) {
            switch (FsInfo.DeviceType) {
                case FILE_DEVICE_CD_ROM:
                case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
                    return DRIVE_CDROM;
                case FILE_DEVICE_DISK:
                case FILE_DEVICE_DISK_FILE_SYSTEM:
                case FILE_DEVICE_DVD:
                    return DRIVE_FIXED;
                case FILE_DEVICE_NETWORK_FILE_SYSTEM:
                    return DRIVE_REMOTE;
                default:
                    return DRIVE_FIXED;
            }
        }
    }

    _Analysis_assume_(NtPath.Buffer != nullptr);
#pragma warning(suppress: 6001)
    RtlFreeUnicodeString(&NtPath);
    return DRIVE_UNKNOWN;
}

MUSA_IAT_SYMBOL(GetDriveTypeW, 4);

// Find context -- allocated by FindFirstFileExW, freed by FindClose.
// Returned as a pseudo-handle so FindNextFileW can retrieve search state.
struct FIND_CONTEXT {
    HANDLE  DirHandle;            // NT directory handle from ZwOpenFile
    WCHAR   Pattern[MAX_PATH];    // wildcard pattern (e.g. "*.txt" or "*")
    DWORD   InfoLevel;            // FINDEX_INFO_LEVELS
    DWORD   SearchOp;             // FINDEX_SEARCH_OPS
    DWORD   AdditionalFlags;
};

// Simple wildcard matching for ? and * (case-insensitive).
static BOOL MatchWildcard(_In_z_ PCWSTR Pattern, _In_z_ PCWSTR Name) {
    while (*Pattern) {
        if (*Pattern == L'*') {
            Pattern++;
            if (*Pattern == L'\0') return TRUE;
            while (*Name) {
                if (MatchWildcard(Pattern, Name)) return TRUE;
                Name++;
            }
            return FALSE;
        }
        if (*Pattern == L'?') {
            if (*Name == L'\0') return FALSE;
            Pattern++;
            Name++;
            continue;
        }
        // Case-insensitive char match
        WCHAR pc = *Pattern;
        WCHAR nc = *Name;
        if (pc >= L'A' && pc <= L'Z') pc += (L'a' - L'A');
        if (nc >= L'A' && nc <= L'Z') nc += (L'a' - L'A');
        if (pc != nc) return FALSE;
        Pattern++;
        Name++;
    }
    return *Name == L'\0';
}

// Fill a WIN32_FIND_DATAW from a FILE_BOTH_DIR_INFORMATION entry.
static void FillFindData(_Out_ LPWIN32_FIND_DATAW Fd, _In_ const FILE_BOTH_DIR_INFORMATION* Info) {
    RtlZeroMemory(Fd, sizeof(WIN32_FIND_DATAW));
    Fd->dwFileAttributes = Info->FileAttributes;
    Fd->ftCreationTime   = reinterpret_cast<const FILETIME&>(Info->CreationTime);
    Fd->ftLastAccessTime = reinterpret_cast<const FILETIME&>(Info->LastAccessTime);
    Fd->ftLastWriteTime  = reinterpret_cast<const FILETIME&>(Info->LastWriteTime);
    Fd->nFileSizeHigh    = Info->EndOfFile.HighPart;
    Fd->nFileSizeLow     = Info->EndOfFile.LowPart;
    size_t nameLen = Info->FileNameLength / sizeof(WCHAR);
    if (nameLen >= MAX_PATH) nameLen = MAX_PATH - 1;
    memcpy(Fd->cFileName, Info->FileName, nameLen * sizeof(WCHAR));
    Fd->cFileName[nameLen] = L'\0';
    Fd->cAlternateFileName[0] = L'\0';
}

// Query the next directory entry that matches the context's criteria.
// Returns TRUE and fills FindData on match, FALSE on end or error.
static BOOL QueryNextMatch(_Inout_ FIND_CONTEXT* Ctx, _Out_ LPWIN32_FIND_DATAW FindData) {
    // Buffer for NT query -- these structs are variable-length
    UCHAR Buffer[sizeof(FILE_BOTH_DIR_INFORMATION) + MAX_PATH * sizeof(WCHAR)];

    for (;;) {
        IO_STATUS_BLOCK Iosb{};
        NTSTATUS Status = ZwQueryDirectoryFile(Ctx->DirHandle, nullptr, nullptr, nullptr, &Iosb,
            Buffer, sizeof(Buffer), FileBothDirectoryInformation, TRUE, nullptr, FALSE);

        if (Status == STATUS_NO_MORE_FILES) {
            BaseSetLastNTError(STATUS_NO_MORE_FILES);
            return FALSE;
        }
        if (!NT_SUCCESS(Status)) {
            BaseSetLastNTError(Status);
            return FALSE;
        }

        auto* Info = reinterpret_cast<FILE_BOTH_DIR_INFORMATION*>(Buffer);

        // Skip "." and ".." entries
        if (Info->FileNameLength == 2 && Info->FileName[0] == L'.') continue;
        if (Info->FileNameLength == 4 && Info->FileName[0] == L'.' && Info->FileName[1] == L'.') continue;

        // Apply search op filter
        if (Ctx->SearchOp == FindExSearchLimitToDirectories) {
            if (!(Info->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
        }

        // Apply wildcard pattern filter
        // Build a null-terminated copy of the filename for matching
        WCHAR Name[MAX_PATH];
        size_t nameLen = Info->FileNameLength / sizeof(WCHAR);
        if (nameLen >= MAX_PATH) nameLen = MAX_PATH - 1;
        memcpy(Name, Info->FileName, nameLen * sizeof(WCHAR));
        Name[nameLen] = L'\0';

        if (!MatchWildcard(Ctx->Pattern, Name)) continue;

        FillFindData(FindData, Info);
        return TRUE;
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(FindFirstFileExW)(
    _In_ LPCWSTR lpFileName,
    _In_ FINDEX_INFO_LEVELS fInfoLevelId,
    _Out_ LPVOID lpFindFileData,
    _In_ FINDEX_SEARCH_OPS fSearchOp,
    _Reserved_ LPVOID lpSearchFilter,
    _In_ DWORD dwAdditionalFlags
)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(lpSearchFilter);

    if (!lpFileName || !lpFindFileData) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    // Parse path into directory + wildcard pattern
    WCHAR DirPath[MAX_PATH];
    WCHAR Pattern[MAX_PATH] = { L'\0' };

    wcscpy_s(DirPath, lpFileName);
    PWCHAR LastSlash = wcsrchr(DirPath, L'\\');
    PWCHAR LastColon = wcsrchr(DirPath, L':');

    if (LastSlash) {
        PWCHAR Wild = LastSlash + 1;
        while (*Wild && *Wild != L'*' && *Wild != L'?') Wild++;
        if (!*Wild) Wild = nullptr;
        if (Wild) {
            // Path contains wildcard -- extract pattern
            wcscpy_s(Pattern, LastSlash + 1);
            *LastSlash = L'\0';
        } else {
            // No wildcard after slash -- treat as directory with \*
            wcscat_s(DirPath, L"\\*");
            wcscpy_s(Pattern, L"*");
        }
    } else if (LastColon && LastColon[1] == L'\0') {
        // Bare drive letter "X:"
        wcscat_s(DirPath, L"\\*");
        wcscpy_s(Pattern, L"*");
    } else {
        // Path without backslash -- append \* (e.g. relative dir)
        wcscat_s(DirPath, L"\\*");
        wcscpy_s(Pattern, L"*");
    }

    if (Pattern[0] == L'\0') wcscpy_s(Pattern, L"*");


    // Open directory via CreateFileW thunk
    // Use Win32 flags (CreateFileW translates to NT CreateOptions internally)
    DWORD DirFlags = FILE_ATTRIBUTE_DIRECTORY;
    if (dwAdditionalFlags & FIND_FIRST_EX_LARGE_FETCH)
        DirFlags |= FILE_FLAG_BACKUP_SEMANTICS;

    HANDLE DirHandle = CreateFileW(
        DirPath,
        FILE_LIST_DIRECTORY | SYNCHRONIZE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        DirFlags,
        nullptr);

    if (DirHandle == INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }

    // Allocate find context
    auto* Ctx = static_cast<FIND_CONTEXT*>(LocalAlloc(LMEM_ZEROINIT, sizeof(FIND_CONTEXT)));
    if (!Ctx) {
        CloseHandle(DirHandle);
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return INVALID_HANDLE_VALUE;
    }

    Ctx->DirHandle       = DirHandle;
    Ctx->InfoLevel       = fInfoLevelId;
    Ctx->SearchOp        = fSearchOp;
    Ctx->AdditionalFlags = dwAdditionalFlags;
    wcscpy_s(Ctx->Pattern, Pattern);

    // Find first match
    LPWIN32_FIND_DATAW FindData = static_cast<LPWIN32_FIND_DATAW>(lpFindFileData);
    if (!QueryNextMatch(Ctx, FindData)) {
        return reinterpret_cast<HANDLE>(Ctx);
    }

    return reinterpret_cast<HANDLE>(Ctx);
}

MUSA_IAT_SYMBOL(FindFirstFileExW, 24);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(FindNextFileW)(
    _In_ HANDLE hFindFile,
    _Out_ LPWIN32_FIND_DATAW lpFindFileData
)
{
    if (!hFindFile || hFindFile == INVALID_HANDLE_VALUE || !lpFindFileData) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    auto* Ctx = reinterpret_cast<FIND_CONTEXT*>(hFindFile);
    return QueryNextMatch(Ctx, lpFindFileData);
}

MUSA_IAT_SYMBOL(FindNextFileW, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(FindClose)(
    _Inout_ HANDLE hFindFile
)
{
    PAGED_CODE();
    if (!hFindFile || hFindFile == INVALID_HANDLE_VALUE) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
    }

    auto* Ctx = reinterpret_cast<FIND_CONTEXT*>(hFindFile);
    CloseHandle(Ctx->DirHandle);
    LocalFree(Ctx);
    return TRUE;
}

MUSA_IAT_SYMBOL(FindClose, 4);


_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetFullPathNameW)(
    _In_ LPCWSTR lpFileName,
    _In_ DWORD nBufferLength,
    _Out_writes_to_opt_(nBufferLength, return + 1) LPWSTR lpBuffer,
    _Out_opt_ LPWSTR* lpFilePart
)
{
    PAGED_CODE();

    RTL_PATH_TYPE PathType = RtlPathTypeUnknown;
    NTSTATUS Status = MUSA_NAME(RtlGetFullPathName_UEx)(
        const_cast<PWSTR>(lpFileName),
        nBufferLength * sizeof(WCHAR),
        lpBuffer,
        lpFilePart,
        &PathType);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return 0;
    }

#pragma warning(suppress: 6387)
    return static_cast<DWORD>(wcslen(lpBuffer));
}

MUSA_IAT_SYMBOL(GetFullPathNameW, 20);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SetEndOfFile)(
    _In_ HANDLE hFile
)
{
    PAGED_CODE();

    IO_STATUS_BLOCK IoStatusBlock{};
    FILE_POSITION_INFORMATION PosInfo{};
    NTSTATUS Status = ZwQueryInformationFile(hFile, &IoStatusBlock, &PosInfo, sizeof(PosInfo), FilePositionInformation);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    FILE_END_OF_FILE_INFORMATION EofInfo{};
    EofInfo.EndOfFile = PosInfo.CurrentByteOffset;
    Status = ZwSetInformationFile(hFile, &IoStatusBlock, &EofInfo, sizeof(EofInfo), FileEndOfFileInformation);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    FILE_ALLOCATION_INFORMATION AllocInfo{};
    AllocInfo.AllocationSize = PosInfo.CurrentByteOffset;
    ZwSetInformationFile(hFile, &IoStatusBlock, &AllocInfo, sizeof(AllocInfo), FileAllocationInformation);
    return TRUE;
}

MUSA_IAT_SYMBOL(SetEndOfFile, 4);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetFileSizeEx)(
    _In_ HANDLE hFile,
    _Out_ PLARGE_INTEGER lpFileSize
)
{
    PAGED_CODE();

    IO_STATUS_BLOCK IoStatusBlock{};
    __declspec(align(8)) UCHAR StdBuf[sizeof(FILE_STANDARD_INFORMATION)]{};
    NTSTATUS Status = ZwQueryInformationFile(hFile, &IoStatusBlock, StdBuf, sizeof(StdBuf), FileStandardInformation);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    *lpFileSize = reinterpret_cast<PFILE_STANDARD_INFORMATION>(StdBuf)->EndOfFile;
    return TRUE;
}

MUSA_IAT_SYMBOL(GetFileSizeEx, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetFileInformationByHandle)(
    _In_ HANDLE hFile,
    _Out_ LPBY_HANDLE_FILE_INFORMATION lpFileInformation
)
{
    PAGED_CODE();


    IO_STATUS_BLOCK IoStatusBlock{};

    // Mimic kernel32: fixed stack buffer for volume info (0x18, 8-byte aligned)
    __declspec(align(8)) UCHAR VolBuf[0x18]{};
    NTSTATUS Status = ZwQueryVolumeInformationFile(hFile, &IoStatusBlock, VolBuf, 0x18, FileFsVolumeInformation);

    DWORD VolSerial = NT_SUCCESS(Status) ? reinterpret_cast<PFILE_FS_VOLUME_INFORMATION>(VolBuf)->VolumeSerialNumber : 0;

    __declspec(align(8)) UCHAR FileBuf[0x68]{};
    Status = ZwQueryInformationFile(hFile, &IoStatusBlock, FileBuf, 0x68, FileAllInformation);
    if (!NT_SUCCESS(Status) && Status != STATUS_BUFFER_OVERFLOW) {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    PFILE_ALL_INFORMATION FileInfo = reinterpret_cast<PFILE_ALL_INFORMATION>(FileBuf);
    lpFileInformation->dwFileAttributes = FileInfo->BasicInformation.FileAttributes;
    lpFileInformation->ftCreationTime   = *reinterpret_cast<const FILETIME*>(&FileInfo->BasicInformation.CreationTime);
    lpFileInformation->ftLastAccessTime = *reinterpret_cast<const FILETIME*>(&FileInfo->BasicInformation.LastAccessTime);
    lpFileInformation->ftLastWriteTime  = *reinterpret_cast<const FILETIME*>(&FileInfo->BasicInformation.LastWriteTime);
    lpFileInformation->dwVolumeSerialNumber = VolSerial;
    lpFileInformation->nFileSizeHigh    = FileInfo->StandardInformation.EndOfFile.HighPart;
    lpFileInformation->nFileSizeLow     = FileInfo->StandardInformation.EndOfFile.LowPart;
    lpFileInformation->nNumberOfLinks   = FileInfo->StandardInformation.NumberOfLinks;
    lpFileInformation->nFileIndexHigh   = FileInfo->InternalInformation.IndexNumber.HighPart;
    lpFileInformation->nFileIndexLow    = FileInfo->InternalInformation.IndexNumber.LowPart;
    return TRUE;

}

MUSA_IAT_SYMBOL(GetFileInformationByHandle, 8);


_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(PeekNamedPipe)(
    _In_ HANDLE hNamedPipe,
    _Out_writes_bytes_to_opt_(nBufferSize, *lpBytesRead) LPVOID lpBuffer,
    _In_ DWORD nBufferSize,
    _Out_opt_ LPDWORD lpBytesRead,
    _Out_opt_ LPDWORD lpTotalBytesAvail,
    _Out_opt_ LPDWORD lpBytesLeftThisMessage
)
{
    PAGED_CODE();

    ULONG OutLen = nBufferSize + 16;
    auto OutBuf = static_cast<PBYTE>(RtlAllocateHeap(RtlProcessHeap(), 0, OutLen));
    if (!OutBuf) {
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return FALSE;
    }

    OBJECT_ATTRIBUTES ObjAttr = RTL_CONSTANT_OBJECT_ATTRIBUTES(nullptr, OBJ_KERNEL_HANDLE);
    HANDLE EventHandle = nullptr;
    NTSTATUS Status = ZwCreateEvent(&EventHandle, EVENT_ALL_ACCESS, &ObjAttr, NotificationEvent, FALSE);
    if (!NT_SUCCESS(Status)) {
        RtlFreeHeap(RtlProcessHeap(), 0, OutBuf);
        BaseSetLastNTError(Status);
        return FALSE;
    }

    IO_STATUS_BLOCK IoStatusBlock{};
    Status = ZwFsControlFile(hNamedPipe, EventHandle, nullptr, nullptr,
        &IoStatusBlock, 0x11400C, nullptr, 0, OutBuf, OutLen);

    if (Status == STATUS_PENDING) {
        Status = ZwWaitForSingleObject(EventHandle, FALSE, nullptr);
        if (NT_SUCCESS(Status))
            Status = IoStatusBlock.Status;
    }

    if (NT_SUCCESS(Status) || Status == STATUS_BUFFER_OVERFLOW) {
        Status = STATUS_SUCCESS;
        DWORD DataLen = static_cast<DWORD>(IoStatusBlock.Information) - 16;
        DWORD TotalAvail = *reinterpret_cast<PDWORD>(OutBuf + 4);
        DWORD LeftMsg = *reinterpret_cast<PDWORD>(OutBuf + 12);

        if (DataLen > nBufferSize) DataLen = nBufferSize;

        if (lpBytesRead) *lpBytesRead = DataLen;
        if (lpTotalBytesAvail) *lpTotalBytesAvail = TotalAvail;
        if (lpBytesLeftThisMessage) *lpBytesLeftThisMessage = LeftMsg - DataLen;
        if (lpBuffer && DataLen > 0)
            memcpy(lpBuffer, OutBuf + 16, DataLen);

    }

    RtlFreeHeap(RtlProcessHeap(), 0, OutBuf);
    ZwClose(EventHandle);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}

MUSA_IAT_SYMBOL(PeekNamedPipe, 24);


_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(CreateNamedPipeW)(
    _In_ LPCWSTR lpName,
    _In_ DWORD dwOpenMode,
    _In_ DWORD dwPipeMode,
    _In_ DWORD nMaxInstances,
    _In_ DWORD nOutBufferSize,
    _In_ DWORD nInBufferSize,
    _In_ DWORD nDefaultTimeOut,
    _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
)
{
    PAGED_CODE();

    if (nMaxInstances < 1 || nMaxInstances > PIPE_UNLIMITED_INSTANCES)
        nMaxInstances = PIPE_UNLIMITED_INSTANCES;

    UNICODE_STRING NtPath{};
    if (!MUSA_NAME(RtlDosPathNameToNtPathName_U)(lpName, &NtPath, nullptr, nullptr)) {
        RtlSetLastWin32Error(ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
    }

    OBJECT_ATTRIBUTES ObjAttr = RTL_CONSTANT_OBJECT_ATTRIBUTES(&NtPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE);
    if (lpSecurityAttributes) {
        ObjAttr.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
        if (lpSecurityAttributes->bInheritHandle) ObjAttr.Attributes |= OBJ_INHERIT;
    }

    ACCESS_MASK DesiredAccess; ULONG ShareAccess;
    switch (dwOpenMode & 3) {
        case PIPE_ACCESS_INBOUND:
            DesiredAccess = FILE_GENERIC_READ; ShareAccess = FILE_SHARE_READ; break;
        case PIPE_ACCESS_OUTBOUND:
            DesiredAccess = FILE_GENERIC_WRITE; ShareAccess = FILE_SHARE_WRITE; break;
        case PIPE_ACCESS_DUPLEX:
            DesiredAccess = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
            ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE; break;
        default:
            RtlFreeUnicodeString(&NtPath);
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
    }

    ULONG CreateOptions = 0;
    if (dwOpenMode & FILE_FLAG_WRITE_THROUGH) CreateOptions |= FILE_WRITE_THROUGH;
    if (dwOpenMode & FILE_FLAG_OVERLAPPED) CreateOptions |= FILE_OPEN_FOR_BACKUP_INTENT;

    ULONG PipeType = (dwPipeMode & PIPE_TYPE_MESSAGE) ? 1 : 0;
    ULONG ReadMode = (dwPipeMode & PIPE_READMODE_MESSAGE) ? 1 : 0;
    ULONG CompletionMode = (dwPipeMode & PIPE_NOWAIT) ? 1 : 0;

    LARGE_INTEGER DefaultTimeOut{};
    DefaultTimeOut.QuadPart = nDefaultTimeOut ? -10000LL * nDefaultTimeOut : -500000;

    HANDLE PipeHandle = nullptr;
    IO_STATUS_BLOCK IoStatusBlock{};
    NTSTATUS Status = ZwCreateNamedPipeFile(&PipeHandle, DesiredAccess, &ObjAttr,
        &IoStatusBlock, ShareAccess, FILE_CREATE, CreateOptions,
        PipeType, ReadMode, CompletionMode, nMaxInstances,
        nInBufferSize, nOutBufferSize, &DefaultTimeOut);

    RtlFreeUnicodeString(&NtPath);

    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_INSTANCE_NOT_AVAILABLE || Status == STATUS_PIPE_NOT_AVAILABLE)
            Status = STATUS_NO_SUCH_DEVICE;
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
    }

    RtlSetLastWin32Error(IoStatusBlock.Information == FILE_CREATED
        ? ERROR_ALREADY_EXISTS : ERROR_SUCCESS);
    return PipeHandle;
}

MUSA_IAT_SYMBOL(CreateNamedPipeW, 32);


/**
 * SetFileTime - Sets the date and time that the specified file was created,
 * last accessed, or last modified.
 *
 * @param hFile            Handle to the file.
 * @param lpCreationTime   File creation time (optional, NULL to leave unchanged).
 * @param lpLastAccessTime File last access time (optional, NULL to leave unchanged).
 * @param lpLastWriteTime  File last write time (optional, NULL to leave unchanged).
 * @return                 TRUE on success, FALSE on failure.
 *
 * Maps to ZwSetInformationFile(FileBasicInformation) -- matches kernelbase.dll.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SetFileTime)(
    _In_ HANDLE                 hFile,
    _In_opt_ const FILETIME*    lpCreationTime,
    _In_opt_ const FILETIME*    lpLastAccessTime,
    _In_opt_ const FILETIME*    lpLastWriteTime
)
{
    PAGED_CODE();
    FILE_BASIC_INFORMATION FileInfo{};
    IO_STATUS_BLOCK Iosb{};


    if (lpCreationTime)
        RtlCopyMemory(&FileInfo.CreationTime, lpCreationTime, sizeof(FILETIME));
    if (lpLastAccessTime)
        RtlCopyMemory(&FileInfo.LastAccessTime, lpLastAccessTime, sizeof(FILETIME));
    if (lpLastWriteTime)
        RtlCopyMemory(&FileInfo.LastWriteTime, lpLastWriteTime, sizeof(FILETIME));

    NTSTATUS Status = ZwSetInformationFile(
        hFile, &Iosb, &FileInfo, sizeof(FileInfo), FileBasicInformation);

    if (NT_SUCCESS(Status))
        return TRUE;

    BaseSetLastNTError(Status);
    return FALSE;
}


/**
 * Retrieves the date and time that a file was created, last accessed, and
 * last modified.
 *
 * @param hFile            Handle to the file.
 * @param lpCreationTime   Receives creation time (optional, NULL to ignore).
 * @param lpLastAccessTime Receives last access time (optional, NULL to ignore).
 * @param lpLastWriteTime  Receives last write time (optional, NULL to ignore).
 * @return                 TRUE on success, FALSE on failure.
 *
 * Maps to ZwQueryInformationFile(FileBasicInformation).
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetFileTime)(
    _In_ HANDLE             hFile,
    _Out_opt_ LPFILETIME    lpCreationTime,
    _Out_opt_ LPFILETIME    lpLastAccessTime,
    _Out_opt_ LPFILETIME    lpLastWriteTime
)
{
    PAGED_CODE();

    FILE_BASIC_INFORMATION FileInfo{};
    IO_STATUS_BLOCK Iosb{};

    NTSTATUS Status = ZwQueryInformationFile(
        hFile, &Iosb, &FileInfo, sizeof(FileInfo), FileBasicInformation);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (lpCreationTime)
        RtlCopyMemory(lpCreationTime, &FileInfo.CreationTime, sizeof(FILETIME));
    if (lpLastAccessTime)
        RtlCopyMemory(lpLastAccessTime, &FileInfo.LastAccessTime, sizeof(FILETIME));
    if (lpLastWriteTime)
        RtlCopyMemory(lpLastWriteTime, &FileInfo.LastWriteTime, sizeof(FILETIME));
    return TRUE;
}

MUSA_IAT_SYMBOL(GetFileTime, 16);

MUSA_IAT_SYMBOL(SetFileTime, 16);

/**
 * GetFileInformationByHandleEx - Retrieves file information for a specified file.
 *
 * Maps to ZwQueryInformationFile, ZwQueryDirectoryFile, or ZwQueryVolumeInformationFile
 * depending on the FileInformationClass.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetFileInformationByHandleEx)(
    _In_ HANDLE hFile,
    _In_ FILE_INFO_BY_HANDLE_CLASS FileInformationClass,
    _Out_writes_bytes_to_opt_(dwBufferSize, dwBufferSize) LPVOID lpFileInformation,
    _In_ DWORD dwBufferSize
)
{
    PAGED_CODE();
    if (!lpFileInformation || !dwBufferSize) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    FILE_INFORMATION_CLASS NtClass;
    DWORD MinSize;
    BOOLEAN IsDirectoryQuery = FALSE;
    BOOLEAN RestartScan = FALSE;
    switch (FileInformationClass) {
    case FileBasicInfo:
        NtClass = FileBasicInformation;
        MinSize = sizeof(FILE_BASIC_INFORMATION);
        break;
    case FileStandardInfo:
        NtClass = FileStandardInformation;
        MinSize = sizeof(FILE_STANDARD_INFORMATION);
        break;
    case FileNameInfo:
        NtClass = FileNameInformation;
        MinSize = sizeof(FILE_NAME_INFORMATION);
        break;
    case FileStreamInfo:
        NtClass = FileStreamInformation;
        MinSize = sizeof(FILE_STREAM_INFORMATION);
        break;
    case FileCompressionInfo:
        NtClass = FileCompressionInformation;
        MinSize = sizeof(FILE_COMPRESSION_INFORMATION);
        break;
    case FileAttributeTagInfo:
        NtClass = FileAttributeTagInformation;
        MinSize = sizeof(FILE_ATTRIBUTE_TAG_INFORMATION);
        break;
    case FileRemoteProtocolInfo:
        NtClass = FileRemoteProtocolInformation;
        MinSize = sizeof(FILE_REMOTE_PROTOCOL_INFORMATION);
        break;
    case FileNormalizedNameInfo:
        NtClass = FileNormalizedNameInformation;
        MinSize = sizeof(FILE_NAME_INFORMATION);
        break;
    case FileIdBothDirectoryInfo:
        NtClass = FileIdBothDirectoryInformation;
        MinSize = sizeof(FILE_ID_BOTH_DIR_INFORMATION);
        IsDirectoryQuery = TRUE;
        break;
    case FileIdBothDirectoryRestartInfo:
        NtClass = FileIdBothDirectoryInformation;
        MinSize = sizeof(FILE_ID_BOTH_DIR_INFORMATION);
        IsDirectoryQuery = TRUE;
        RestartScan = TRUE;
        break;
    case FileFullDirectoryInfo:
        NtClass = FileFullDirectoryInformation;
        MinSize = sizeof(FILE_FULL_DIR_INFORMATION);
        IsDirectoryQuery = TRUE;
        break;
    case FileFullDirectoryRestartInfo:
        NtClass = FileFullDirectoryInformation;
        MinSize = sizeof(FILE_FULL_DIR_INFORMATION);
        IsDirectoryQuery = TRUE;
        RestartScan = TRUE;
        break;
    case FileIdInfo:
        // FILE_ID_INFO <-> FILE_ID_INFORMATION (identical layout: VolumeSerialNumber + FileId128)
        NtClass = FileIdInformation;
        MinSize = sizeof(FILE_ID_INFORMATION);
        break;
    case FileIdExtdDirectoryInfo:
        NtClass = FileIdBothDirectoryInformation;
        MinSize = sizeof(FILE_ID_BOTH_DIR_INFORMATION);
        IsDirectoryQuery = TRUE;
        RestartScan = TRUE;
        break;
    case FileDispositionInfoEx:
        {
            IO_STATUS_BLOCK Iosb{};
            NTSTATUS VolStatus = ZwQueryVolumeInformationFile(
                hFile, &Iosb, lpFileInformation, dwBufferSize, FileFsVolumeInformation);
            if (!NT_SUCCESS(VolStatus)) {
                BaseSetLastNTError(VolStatus);
                return FALSE;
            }
            return TRUE;
        }
    default:
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    if (dwBufferSize < MinSize) {
        BaseSetLastNTError(STATUS_BUFFER_TOO_SMALL);
        return FALSE;
    }
    IO_STATUS_BLOCK Iosb{};
    NTSTATUS Status;
    if (IsDirectoryQuery) {
        Status = ZwQueryDirectoryFile(
            hFile, nullptr, nullptr, nullptr, &Iosb,
            lpFileInformation, dwBufferSize, NtClass,
            FALSE, nullptr, RestartScan);
        if (Status == STATUS_NO_MORE_FILES) {
            Status = ZwWaitForSingleObject(hFile, FALSE, nullptr);
            if (NT_SUCCESS(Status))
                Status = Iosb.Status;
        }
    } else {
        Status = ZwQueryInformationFile(
            hFile, &Iosb, lpFileInformation, dwBufferSize, NtClass);
    }
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    if (FileInformationClass == FileStreamInfo && !Iosb.Information) {
        BaseSetLastNTError(STATUS_BUFFER_OVERFLOW);
        return FALSE;
    }
    return TRUE;
}
MUSA_IAT_SYMBOL(GetFileInformationByHandleEx, 16);
/**
 * SetFileInformationByHandle - Sets file information for a specified file.
 *
 * Maps to ZwSetInformationFile. FileRenameInfo is not supported in kernel mode.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(SetFileInformationByHandle)(
    _In_ HANDLE hFile,
    _In_ FILE_INFO_BY_HANDLE_CLASS FileInformationClass,
    _In_reads_bytes_(dwBufferSize) LPVOID lpFileInformation,
    _In_ DWORD dwBufferSize
)
{
    PAGED_CODE();
    if (!lpFileInformation || !dwBufferSize) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    FILE_INFORMATION_CLASS NtClass;
    DWORD MinSize;
    switch (FileInformationClass) {
    case FileBasicInfo:
        NtClass = FileBasicInformation;
        MinSize = sizeof(FILE_BASIC_INFORMATION);
        break;
    case FileDispositionInfo:
        NtClass = FileDispositionInformation;
        MinSize = sizeof(FILE_DISPOSITION_INFORMATION);
        break;
    case FileAllocationInfo:
        NtClass = FileAllocationInformation;
        MinSize = sizeof(FILE_ALLOCATION_INFORMATION);
        break;
    case FileEndOfFileInfo:
        NtClass = FileEndOfFileInformation;
        MinSize = sizeof(FILE_END_OF_FILE_INFORMATION);
        break;
    case FileIoPriorityHintInfo:
        NtClass = FileIoPriorityHintInformation;
        MinSize = sizeof(FILE_IO_PRIORITY_HINT_INFORMATION);
        if (static_cast<ULONG>(static_cast<PFILE_IO_PRIORITY_HINT_INFORMATION>(lpFileInformation)->PriorityHint) >= static_cast<ULONG>(MaximumIoPriorityHintType)) {
            BaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return FALSE;
        }
        break;
    case FileRenameInfo:
    case FileRenameInfoEx:
        BaseSetLastNTError(STATUS_NOT_SUPPORTED);
        return FALSE;
    default:
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    if (dwBufferSize < MinSize) {
        BaseSetLastNTError(STATUS_BUFFER_TOO_SMALL);
        return FALSE;
    }
    IO_STATUS_BLOCK Iosb{};
    NTSTATUS Status = ZwSetInformationFile(
        hFile, &Iosb, lpFileInformation, dwBufferSize, NtClass);
    if (NT_SUCCESS(Status))
        return TRUE;
    BaseSetLastNTError(Status);
    return FALSE;
}
MUSA_IAT_SYMBOL(SetFileInformationByHandle, 16);

/**
 * CreateSymbolicLinkW - Creates a symbolic link.
 *
 * Maps to ZwCreateFile + ZwFsControlFile(FSCTL_SET_REPARSE_POINT).
 * Privilege acquisition is not needed in kernel mode.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN WINAPI MUSA_NAME(CreateSymbolicLinkW)(
    _In_ LPCWSTR lpSymlinkFileName,
    _In_ LPCWSTR lpTargetFileName,
    _In_ DWORD dwFlags
)
{
    PAGED_CODE();
    if (!lpSymlinkFileName || !lpTargetFileName) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    if ((dwFlags & ~(SYMBOLIC_LINK_FLAG_DIRECTORY | SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE)) != 0) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }
    // Build the substitute name (NT path) for the target
    UNICODE_STRING NtTarget{};
    WCHAR* SubstituteName = nullptr;
    USHORT SubstituteNameLen = 0;
    NTSTATUS Status = RtlDosPathNameToNtPathName_U_WithStatus(lpTargetFileName, &NtTarget, nullptr, nullptr);
    if (NT_SUCCESS(Status)) {
        SubstituteName = NtTarget.Buffer;
        SubstituteNameLen = NtTarget.Length;
    }
    // Compute reparse buffer size
    USHORT TargetLen = (USHORT)(wcslen(lpTargetFileName) * sizeof(WCHAR));
    USHORT SubNameBytes = SubstituteNameLen;
    ULONG BufSize = sizeof(REPARSE_DATA_BUFFER) + SubNameBytes + TargetLen + (2 * sizeof(WCHAR));
    auto ReparseBuf = static_cast<PREPARSE_DATA_BUFFER>(RtlAllocateHeap(RtlProcessHeap(), 0, BufSize));
    if (!ReparseBuf) {
        if (NtTarget.Buffer) RtlFreeUnicodeString(&NtTarget);
        BaseSetLastNTError(STATUS_NO_MEMORY);
        return FALSE;
    }
    RtlZeroMemory(ReparseBuf, BufSize);
    ReparseBuf->ReparseTag = IO_REPARSE_TAG_SYMLINK;
    ReparseBuf->ReparseDataLength = (USHORT)(BufSize - sizeof(ULONG) - 2 * sizeof(USHORT));
    PWCHAR PathBuf = ReparseBuf->SymbolicLinkReparseBuffer.PathBuffer;
    ReparseBuf->SymbolicLinkReparseBuffer.SubstituteNameOffset = 0;
    ReparseBuf->SymbolicLinkReparseBuffer.SubstituteNameLength = SubNameBytes;
    if (SubstituteName && SubNameBytes)
        RtlCopyMemory(PathBuf, SubstituteName, SubNameBytes);
    ReparseBuf->SymbolicLinkReparseBuffer.PrintNameOffset = SubNameBytes + sizeof(WCHAR);
    ReparseBuf->SymbolicLinkReparseBuffer.PrintNameLength = TargetLen;
    RtlCopyMemory((PUCHAR)PathBuf + SubNameBytes + sizeof(WCHAR), lpTargetFileName, TargetLen);
    if (NtTarget.Buffer)
        RtlFreeUnicodeString(&NtTarget);
    // Convert symlink name to NT path
    UNICODE_STRING NtLink{};
    Status = RtlDosPathNameToNtPathName_U_WithStatus(lpSymlinkFileName, &NtLink, nullptr, nullptr);
    if (!NT_SUCCESS(Status)) {
        RtlFreeHeap(RtlProcessHeap(), 0, ReparseBuf);
        BaseSetLastNTError(Status);
        return FALSE;
    }
    OBJECT_ATTRIBUTES ObjAttr;
    InitializeObjectAttributes(&ObjAttr, &NtLink,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        nullptr, nullptr);
    IO_STATUS_BLOCK Iosb{};
    HANDLE FileHandle = nullptr;
    ULONG CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT
                        | FILE_OPEN_FOR_BACKUP_INTENT
                        | FILE_OPEN_REPARSE_POINT;
    if (dwFlags & SYMBOLIC_LINK_FLAG_DIRECTORY)
        CreateOptions |= FILE_DIRECTORY_FILE;
    else
        CreateOptions |= FILE_NON_DIRECTORY_FILE;
    Status = ZwCreateFile(
        &FileHandle,
        GENERIC_WRITE | DELETE | SYNCHRONIZE,
        &ObjAttr,
        &Iosb,
        nullptr,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_CREATE,
        CreateOptions,
        nullptr,
        0);
    RtlFreeUnicodeString(&NtLink);
    if (!NT_SUCCESS(Status)) {
        RtlFreeHeap(RtlProcessHeap(), 0, ReparseBuf);
        BaseSetLastNTError(Status);
        return FALSE;
    }
    Status = ZwFsControlFile(
        FileHandle,
        nullptr,
        nullptr,
        nullptr,
        &Iosb,
        FSCTL_SET_REPARSE_POINT,
        ReparseBuf,
        BufSize,
        nullptr,
        0);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
#pragma push_macro("DeleteFile")
#undef DeleteFile
        FILE_DISPOSITION_INFORMATION Disposition{};
        Disposition.DeleteFile = TRUE;
        ZwSetInformationFile(FileHandle, &Iosb, &Disposition, sizeof(Disposition), FileDispositionInformation);
#pragma pop_macro("DeleteFile")
    }
    ZwClose(FileHandle);
    RtlFreeHeap(RtlProcessHeap(), 0, ReparseBuf);
    return NT_SUCCESS(Status);
}
MUSA_IAT_SYMBOL(CreateSymbolicLinkW, 12);
/**
 * GetFinalPathNameByHandleW - Retrieves the final path for a file handle.
 *
 * Maps to ZwQueryObject(ObjectNameInformation) + ZwQueryInformationFile(FileNameInformation).
 * Only VOLUME_NAME_NT is fully supported; VOLUME_NAME_DOS and VOLUME_NAME_GUID
 * require volume mount-point enumeration not available in kernel mode.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetFinalPathNameByHandleW)(
    _In_ HANDLE hFile,
    _Out_writes_to_opt_(cchFilePath, return + 1) LPWSTR lpszFilePath,
    _In_ DWORD cchFilePath,
    _In_ DWORD dwFlags
)
{
    PAGED_CODE();
    if (hFile == INVALID_HANDLE_VALUE || hFile == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return 0;
    }
    DWORD VolFlags = dwFlags & (FILE_NAME_NORMALIZED | VOLUME_NAME_DOS | VOLUME_NAME_GUID | VOLUME_NAME_NT);
    // Only VOLUME_NAME_NT (flag 0 or explicit) is supported
    if (VolFlags != 0 && VolFlags != VOLUME_NAME_NT) {
        BaseSetLastNTError(STATUS_NOT_SUPPORTED);
        return 0;
    }
    // Phase 1: Query object name
    ULONG ObjBufSize = 512;
    PVOID ObjBuf = nullptr;
    NTSTATUS Status;
    ULONG ReturnLength;
    for (;;) {
        if (ObjBuf)
            RtlFreeHeap(RtlProcessHeap(), 0, ObjBuf);
        ObjBuf = RtlAllocateHeap(RtlProcessHeap(), 0, ObjBufSize);
        if (!ObjBuf) {
            BaseSetLastNTError(STATUS_NO_MEMORY);
            return 0;
        }
        Status = ZwQueryObject(hFile, ObjectNameInformation, ObjBuf, ObjBufSize, &ReturnLength);
        if (Status != STATUS_INFO_LENGTH_MISMATCH)
            break;
        ObjBufSize = ReturnLength + 64;
    }
    if (!NT_SUCCESS(Status)) {
        if (ObjBuf) RtlFreeHeap(RtlProcessHeap(), 0, ObjBuf);
        BaseSetLastNTError(Status);
        return 0;
    }
    auto ObjNameInfo = static_cast<POBJECT_NAME_INFORMATION>(ObjBuf);
    UNICODE_STRING ObjName = ObjNameInfo->Name;
    // Phase 2: Query file name information
    ULONG FileNameBufSize = 512;
    PVOID FileNameBuf = nullptr;
    IO_STATUS_BLOCK Iosb{};
    for (;;) {
        if (FileNameBuf)
            RtlFreeHeap(RtlProcessHeap(), 0, FileNameBuf);
        FileNameBuf = RtlAllocateHeap(RtlProcessHeap(), 0, FileNameBufSize);
        if (!FileNameBuf) {
            RtlFreeHeap(RtlProcessHeap(), 0, ObjBuf);
            BaseSetLastNTError(STATUS_NO_MEMORY);
            return 0;
        }
        Status = ZwQueryInformationFile(hFile, &Iosb, FileNameBuf, FileNameBufSize, FileNameInformation);
        if (Status != STATUS_BUFFER_OVERFLOW)
            break;
        FileNameBufSize += 512;
    }
    if (!NT_SUCCESS(Status) && Status != STATUS_BUFFER_OVERFLOW) {
        RtlFreeHeap(RtlProcessHeap(), 0, FileNameBuf);
        RtlFreeHeap(RtlProcessHeap(), 0, ObjBuf);
        BaseSetLastNTError(Status);
        return 0;
    }
    auto FileNameInfo = static_cast<PFILE_NAME_INFORMATION>(FileNameBuf);
    USHORT FileNameLen = static_cast<USHORT>(FileNameInfo->FileNameLength);
    PWCHAR FileName = FileNameInfo->FileName;
    // Strip the file name portion from the object name to get the device/volume path
    SIZE_T ObjNameChars = ObjName.Length / sizeof(WCHAR);
    SIZE_T FileNameChars = FileNameLen / sizeof(WCHAR);
    SIZE_T VolPathChars = 0;
    if (ObjNameChars > FileNameChars) {
        VolPathChars = ObjNameChars - FileNameChars;
    }
    DWORD RequiredChars = (DWORD)(VolPathChars + FileNameChars + 1);
    DWORD Result = RequiredChars;
    if (RequiredChars <= cchFilePath && lpszFilePath) {
        // Copy volume/device path portion
        if (VolPathChars)
            RtlCopyMemory(lpszFilePath, ObjName.Buffer, VolPathChars * sizeof(WCHAR));
        // Copy file name portion
        RtlCopyMemory(lpszFilePath + VolPathChars, FileName, FileNameLen);
        lpszFilePath[RequiredChars - 1] = L'\0';
    } else if (cchFilePath > 0 && lpszFilePath) {
        *lpszFilePath = L'\0';
    }
    RtlFreeHeap(RtlProcessHeap(), 0, FileNameBuf);
    RtlFreeHeap(RtlProcessHeap(), 0, ObjBuf);
    return Result;
}
MUSA_IAT_SYMBOL(GetFinalPathNameByHandleW, 16);

/**
 * GetDiskFreeSpaceExW - Retrieves information about the amount of space available on a disk volume.
 *
 * Maps to ZwOpenFile + ZwQueryVolumeInformationFile(FileFsFullSizeInformation/FileFsSizeInformation).
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(GetDiskFreeSpaceExW)(
    _In_opt_ LPCWSTR lpDirectoryName,
    _Out_opt_ PULARGE_INTEGER lpFreeBytesAvailableToCaller,
    _Out_opt_ PULARGE_INTEGER lpTotalNumberOfBytes,
    _Out_opt_ PULARGE_INTEGER lpTotalNumberOfFreeBytes
)
{
    PAGED_CODE();

    PCWSTR PathName = lpDirectoryName ? lpDirectoryName : L"C:\\";

    UNICODE_STRING NtPathName{};
    if (!RtlDosPathNameToNtPathName_U(PathName, &NtPathName, nullptr, nullptr)) {
        RtlSetLastWin32Error(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }

    PWSTR Buffer = NtPathName.Buffer;
    OBJECT_ATTRIBUTES ObjAttr;
    InitializeObjectAttributes(&ObjAttr, &NtPathName, OBJ_CASE_INSENSITIVE, nullptr, nullptr);

    HANDLE FileHandle = nullptr;
    IO_STATUS_BLOCK Iosb{};
    NTSTATUS Status = ZwOpenFile(
        &FileHandle,
        FILE_READ_ATTRIBUTES | SYNCHRONIZE,
        &ObjAttr,
        &Iosb,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        if (RtlNtStatusToDosError(Status) == ERROR_FILE_NOT_FOUND)
            RtlSetLastWin32Error(ERROR_PATH_NOT_FOUND);
        RtlFreeHeap(RtlProcessHeap(), 0, Buffer);
        return FALSE;
    }

    RtlFreeHeap(RtlProcessHeap(), 0, Buffer);

    // Try FileFsFullSizeInformation first (provides per-caller free bytes)
    FILE_FS_FULL_SIZE_INFORMATION FullSizeInfo{};
    Status = ZwQueryVolumeInformationFile(FileHandle, &Iosb, &FullSizeInfo,
        sizeof(FullSizeInfo), FileFsFullSizeInformation);


    if (NT_SUCCESS(Status) && lpTotalNumberOfFreeBytes) {
        ZwClose(FileHandle);
        ULONGLONG Factor = (ULONGLONG)FullSizeInfo.SectorsPerAllocationUnit * FullSizeInfo.BytesPerSector;
        if (lpFreeBytesAvailableToCaller)
            lpFreeBytesAvailableToCaller->QuadPart = FullSizeInfo.CallerAvailableAllocationUnits.QuadPart * Factor;
        if (lpTotalNumberOfBytes)
            lpTotalNumberOfBytes->QuadPart = FullSizeInfo.TotalAllocationUnits.QuadPart * Factor;
        if (lpTotalNumberOfFreeBytes)
            lpTotalNumberOfFreeBytes->QuadPart = FullSizeInfo.ActualAvailableAllocationUnits.QuadPart * Factor;
        return TRUE;
    }

    // Fall back to FileFsSizeInformation
    FILE_FS_SIZE_INFORMATION SizeInfo{};
    Status = ZwQueryVolumeInformationFile(FileHandle, &Iosb, &SizeInfo,
        sizeof(SizeInfo), FileFsSizeInformation);
    ZwClose(FileHandle);

    if (NT_SUCCESS(Status)) {
        ULONGLONG Factor = (ULONGLONG)SizeInfo.SectorsPerAllocationUnit * SizeInfo.BytesPerSector;
        ULONGLONG FreeBytes = SizeInfo.AvailableAllocationUnits.QuadPart * Factor;
        ULONGLONG TotalBytes = SizeInfo.TotalAllocationUnits.QuadPart * Factor;
        if (lpFreeBytesAvailableToCaller)
            lpFreeBytesAvailableToCaller->QuadPart = FreeBytes;
        if (lpTotalNumberOfBytes)
            lpTotalNumberOfBytes->QuadPart = TotalBytes;
        if (lpTotalNumberOfFreeBytes)
            lpTotalNumberOfFreeBytes->QuadPart = FreeBytes;
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}
MUSA_IAT_SYMBOL(GetDiskFreeSpaceExW, 16);

/**
 * CopyFile2 - Copies an existing file to a new file with extended parameters.
 *
 * Uses CreateFileW, ReadFile, WriteFile, CloseHandle.
 * Progress callback and transaction support are not implemented in kernel mode.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
HRESULT WINAPI MUSA_NAME(CopyFile2)(
    _In_ LPCWSTR pwszExistingFileName,
    _In_ LPCWSTR pwszNewFileName,
    _In_opt_ COPYFILE2_EXTENDED_PARAMETERS *pExtendedParameters
)
{
    PAGED_CODE();

    DWORD dwCopyFlags = 0;
    BOOL *pfCancel = nullptr;

    if (pExtendedParameters) {
        if (pExtendedParameters->dwSize < sizeof(COPYFILE2_EXTENDED_PARAMETERS))
            return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        dwCopyFlags = pExtendedParameters->dwCopyFlags;
        pfCancel = pExtendedParameters->pfCancel;
    }

    if (!pwszExistingFileName || !pwszNewFileName)
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    if (dwCopyFlags & 0x40000000)
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    // Open source file
    DWORD SrcFlags = FILE_ATTRIBUTE_NORMAL;
    if (dwCopyFlags & COPY_FILE_NO_BUFFERING)
        SrcFlags |= FILE_FLAG_NO_BUFFERING;

    HANDLE hSource = CreateFileW(pwszExistingFileName,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING,
        SrcFlags, nullptr);

    if (hSource == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32(GetLastError());

    // Open/create destination file
    DWORD CreateDisposition = (dwCopyFlags & COPY_FILE_FAIL_IF_EXISTS)
        ? CREATE_NEW : CREATE_ALWAYS;
    DWORD DstFlags = FILE_ATTRIBUTE_NORMAL;
    if (dwCopyFlags & COPY_FILE_NO_BUFFERING)
        DstFlags |= FILE_FLAG_NO_BUFFERING;

    HANDLE hDest = CreateFileW(pwszNewFileName,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, CreateDisposition,
        DstFlags, nullptr);

    if (hDest == INVALID_HANDLE_VALUE) {
        DWORD Err = GetLastError();
        CloseHandle(hSource);
        return HRESULT_FROM_WIN32(Err);
    }

    // Copy loop
    const DWORD ChunkSize = 64 * 1024;
    PVOID CopyBuf = RtlAllocateHeap(RtlProcessHeap(), 0, ChunkSize);
    if (!CopyBuf) {
        CloseHandle(hSource);
        CloseHandle(hDest);
        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }

    HRESULT HResult = S_OK;
    while (TRUE) {
        if (pfCancel && *pfCancel) {
            HResult = HRESULT_FROM_WIN32(ERROR_REQUEST_ABORTED);
            break;
        }

        DWORD BytesRead = 0;
        if (!ReadFile(hSource, CopyBuf, ChunkSize, &BytesRead, nullptr)) {
            HResult = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        if (BytesRead == 0)
            break;

        DWORD BytesWritten = 0;
        if (!WriteFile(hDest, CopyBuf, BytesRead, &BytesWritten, nullptr) || BytesWritten != BytesRead) {
            HResult = HRESULT_FROM_WIN32(GetLastError());
            break;
        }
    }
    RtlFreeHeap(RtlProcessHeap(), 0, CopyBuf);


    CloseHandle(hSource);
    if (FAILED(HResult)) {
        CloseHandle(hDest);
        DeleteFileW(pwszNewFileName);
        return HResult;
    }

    CloseHandle(hDest);
    return S_OK;
}
MUSA_IAT_SYMBOL(CopyFile2, 12);

/**
 * FindFirstFileW - Searches a directory for a file or subdirectory.
 *
 * Wraps FindFirstFileExW with standard search parameters.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
HANDLE WINAPI MUSA_NAME(FindFirstFileW)(
    _In_ LPCWSTR lpFileName,
    _Out_ LPWIN32_FIND_DATAW lpFindFileData
)
{
    PAGED_CODE();
    return FindFirstFileExW(lpFileName, FindExInfoStandard, lpFindFileData,
        FindExSearchNameMatch, nullptr, 0);
}
MUSA_IAT_SYMBOL(FindFirstFileW, 8);

/**
 * DeviceIoControl - Sends a control code directly to a specified device driver.
 *
 * Maps to NtDeviceIoControlFile or NtFsControlFile depending on device type.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(DeviceIoControl)(
    _In_ HANDLE hDevice,
    _In_ DWORD dwIoControlCode,
    _In_reads_bytes_opt_(nInBufferSize) LPVOID lpInBuffer,
    _In_ DWORD nInBufferSize,
    _Out_writes_bytes_opt_(nOutBufferSize) LPVOID lpOutBuffer,
    _In_ DWORD nOutBufferSize,
    _Out_opt_ LPDWORD lpBytesReturned,
    _Inout_opt_ LPOVERLAPPED lpOverlapped
)
{
    PAGED_CODE();

    BOOL IsFsControl = (HIWORD(dwIoControlCode) == FILE_DEVICE_FILE_SYSTEM);
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    if (lpOverlapped) {
        lpOverlapped->Internal = STATUS_PENDING;
        HANDLE hEvent = lpOverlapped->hEvent;
        PVOID ApcContext = (!((ULONG_PTR)hEvent & 1)) ? lpOverlapped : nullptr;

        if (IsFsControl) {
            Status = ZwFsControlFile(hDevice, hEvent, nullptr, ApcContext,
                reinterpret_cast<PIO_STATUS_BLOCK>(lpOverlapped),
                dwIoControlCode, lpInBuffer, nInBufferSize,
                lpOutBuffer, nOutBufferSize);
        } else {
            Status = ZwDeviceIoControlFile(hDevice, hEvent, nullptr, ApcContext,
                reinterpret_cast<PIO_STATUS_BLOCK>(lpOverlapped),
                dwIoControlCode, lpInBuffer, nInBufferSize,
                lpOutBuffer, nOutBufferSize);
        }

        if (!NT_ERROR(Status) && lpBytesReturned)
            *lpBytesReturned = static_cast<DWORD>(lpOverlapped->InternalHigh);

        if (NT_SUCCESS(Status) && Status != STATUS_PENDING)
            return TRUE;

        BaseSetLastNTError(Status);
        return FALSE;
    }

    // Synchronous -- no overlapped
    IO_STATUS_BLOCK Iosb{};

    if (IsFsControl) {
        Status = ZwFsControlFile(hDevice, nullptr, nullptr, nullptr,
            &Iosb, dwIoControlCode, lpInBuffer, nInBufferSize,
            lpOutBuffer, nOutBufferSize);
    } else {
        Status = ZwDeviceIoControlFile(hDevice, nullptr, nullptr, nullptr,
            &Iosb, dwIoControlCode, lpInBuffer, nInBufferSize,
            lpOutBuffer, nOutBufferSize);
    }

    if (Status == STATUS_PENDING) {
        Status = ZwWaitForSingleObject(hDevice, FALSE, nullptr);
        if (NT_SUCCESS(Status))
            Status = Iosb.Status;
    }

    if (NT_SUCCESS(Status)) {
        if (lpBytesReturned)
            *lpBytesReturned = static_cast<DWORD>(Iosb.Information);
        return TRUE;
    }

    if (!NT_ERROR(Status) && lpBytesReturned)
        *lpBytesReturned = static_cast<DWORD>(Iosb.Information);
    BaseSetLastNTError(Status);
    return FALSE;
}
MUSA_IAT_SYMBOL(DeviceIoControl, 32);

/**
 * CreateHardLinkW - Creates a hard link between an existing file and a new file.
 *
 * Maps to NtOpenFile + NtSetInformationFile(FileLinkInformation).
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(CreateHardLinkW)(
    _In_ LPCWSTR lpFileName,
    _In_ LPCWSTR lpExistingFileName,
    _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
)
{
    PAGED_CODE();

    if (!lpFileName || !lpExistingFileName) {
        RtlSetLastWin32Error(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    // Convert existing file to NT path
    UNICODE_STRING NtExisting{};
    if (!RtlDosPathNameToNtPathName_U(lpExistingFileName, &NtExisting, nullptr, nullptr)) {
        RtlSetLastWin32Error(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }

    OBJECT_ATTRIBUTES ObjAttr;
    InitializeObjectAttributes(&ObjAttr, &NtExisting, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
    if (lpSecurityAttributes)
        ObjAttr.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;

    HANDLE FileHandle = nullptr;
    IO_STATUS_BLOCK Iosb{};
    NTSTATUS Status = ZwOpenFile(&FileHandle, FILE_READ_ATTRIBUTES | SYNCHRONIZE,
        &ObjAttr, &Iosb, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        RtlFreeHeap(RtlProcessHeap(), 0, NtExisting.Buffer);
        return FALSE;
    }

    // Convert new link name to NT path
    UNICODE_STRING NtLink{};
    if (!RtlDosPathNameToNtPathName_U(lpFileName, &NtLink, nullptr, nullptr)) {
        ZwClose(FileHandle);
        RtlFreeHeap(RtlProcessHeap(), 0, NtExisting.Buffer);
        RtlSetLastWin32Error(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }

    // Build FILE_LINK_INFORMATION
    ULONG LinkInfoSize = sizeof(FILE_LINK_INFORMATION) + NtLink.Length;
    auto LinkInfo = static_cast<PFILE_LINK_INFORMATION>(
        RtlAllocateHeap(RtlProcessHeap(), 0, LinkInfoSize));
    if (!LinkInfo) {
        ZwClose(FileHandle);
        RtlFreeHeap(RtlProcessHeap(), 0, NtExisting.Buffer);
        RtlFreeHeap(RtlProcessHeap(), 0, NtLink.Buffer);
        RtlSetLastWin32Error(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    LinkInfo->ReplaceIfExists = FALSE;
    LinkInfo->RootDirectory = nullptr;
    LinkInfo->FileNameLength = NtLink.Length;
    RtlCopyMemory(LinkInfo->FileName, NtLink.Buffer, NtLink.Length);

    Status = ZwSetInformationFile(FileHandle, &Iosb, LinkInfo, LinkInfoSize, FileLinkInformation);

    RtlFreeHeap(RtlProcessHeap(), 0, LinkInfo);
    RtlFreeHeap(RtlProcessHeap(), 0, NtLink.Buffer);
    RtlFreeHeap(RtlProcessHeap(), 0, NtExisting.Buffer);
    ZwClose(FileHandle);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}
MUSA_IAT_SYMBOL(CreateHardLinkW, 12);

/**
 * CreateDirectoryExW - Creates a new directory with attributes copied from a template.
 *
 * Simplified kernel-mode implementation: copies basic attributes from template.
 * Extended attributes, alternate streams, and reparse points are not copied.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(CreateDirectoryExW)(
    _In_ LPCWSTR lpTemplateDirectory,
    _In_ LPCWSTR lpNewDirectory,
    _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes
)
{
    PAGED_CODE();

    // Create the new directory first
    if (!MUSA_NAME(CreateDirectoryW)(lpNewDirectory, lpSecurityAttributes))
        return FALSE;

    // If template is null or same as new, we're done
    if (!lpTemplateDirectory || _wcsicmp(lpTemplateDirectory, lpNewDirectory) == 0)
        return TRUE;

    // Open template to copy attributes
    UNICODE_STRING NtTemplate{};
    if (!RtlDosPathNameToNtPathName_U(lpTemplateDirectory, &NtTemplate, nullptr, nullptr))
        return TRUE;  // New directory already created, template failure is non-fatal

    OBJECT_ATTRIBUTES ObjAttr;
    InitializeObjectAttributes(&ObjAttr, &NtTemplate, OBJ_CASE_INSENSITIVE, nullptr, nullptr);

    HANDLE TemplateHandle = nullptr;
    IO_STATUS_BLOCK Iosb{};
    NTSTATUS Status = ZwOpenFile(&TemplateHandle,
        FILE_READ_ATTRIBUTES | SYNCHRONIZE,
        &ObjAttr, &Iosb,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | FILE_DIRECTORY_FILE);

    PWSTR TemplateBuf = NtTemplate.Buffer;

    if (!NT_SUCCESS(Status)) {
        RtlFreeHeap(RtlProcessHeap(), 0, TemplateBuf);
        return TRUE;  // Non-fatal: directory already created
    }

    // Query template attributes
    FILE_BASIC_INFORMATION BasicInfo{};
    Status = ZwQueryInformationFile(TemplateHandle, &Iosb, &BasicInfo,
        sizeof(BasicInfo), FileBasicInformation);

    if (NT_SUCCESS(Status)) {
        RtlFreeHeap(RtlProcessHeap(), 0, TemplateBuf);

        // Apply template attributes to new directory
        UNICODE_STRING NtNew{};
        if (RtlDosPathNameToNtPathName_U(lpNewDirectory, &NtNew, nullptr, nullptr)) {
            HANDLE NewHandle = nullptr;
            OBJECT_ATTRIBUTES NewAttr;
            InitializeObjectAttributes(&NewAttr, &NtNew, OBJ_CASE_INSENSITIVE, nullptr, nullptr);

            Status = ZwOpenFile(&NewHandle,
                FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                &NewAttr, &Iosb,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT | FILE_DIRECTORY_FILE);

            if (NT_SUCCESS(Status)) {
                ZwSetInformationFile(NewHandle, &Iosb, &BasicInfo,
                    sizeof(BasicInfo), FileBasicInformation);
                ZwClose(NewHandle);
            }

            RtlFreeHeap(RtlProcessHeap(), 0, NtNew.Buffer);
        }
    } else {
        RtlFreeHeap(RtlProcessHeap(), 0, TemplateBuf);
    }

    ZwClose(TemplateHandle);
    return TRUE;
}
MUSA_IAT_SYMBOL(CreateDirectoryExW, 12);

EXTERN_C_END
