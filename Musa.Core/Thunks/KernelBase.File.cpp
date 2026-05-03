// Phase 2: CreateFileW, SetFilePointerEx, GetFileAttributesExW, DeleteFileW, SetFileAttributesW
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
#pragma alloc_text(PAGE, MUSA_NAME(SetFilePointerEx))
#pragma alloc_text(PAGE, MUSA_NAME(SetFilePointer))
#pragma alloc_text(PAGE, MUSA_NAME(GetFileAttributesExW))
#pragma alloc_text(PAGE, MUSA_NAME(DeleteFileW))
#pragma alloc_text(PAGE, MUSA_NAME(SetFileAttributesW))
#pragma alloc_text(PAGE, MUSA_NAME(GetTempPathW))
#pragma alloc_text(PAGE, MUSA_NAME(CreateDirectoryW))
#pragma alloc_text(PAGE, MUSA_NAME(RemoveDirectoryW))
#pragma alloc_text(PAGE, MUSA_NAME(MoveFileExW))
#pragma alloc_text(PAGE, MUSA_NAME(GetDriveTypeW))
#pragma alloc_text(PAGE, MUSA_NAME(FindFirstFileExW))
#pragma alloc_text(PAGE, MUSA_NAME(FindClose))
#endif

EXTERN_C_START


// Resolve standard I/O pseudo-handles from PEB
static HANDLE MusaCoreResolveStdHandle(HANDLE hFile)
{
    switch (HandleToULong(hFile)) {
        case STD_INPUT_HANDLE:
        case STD_OUTPUT_HANDLE:
        case STD_ERROR_HANDLE: {
            const auto Peb = static_cast<Musa::Core::KPEB*>(MUSA_NAME_PRIVATE(RtlGetCurrentPeb)());
            if (Peb == nullptr) {
                return INVALID_HANDLE_VALUE;
            }
            switch (HandleToULong(hFile)) {
                case STD_INPUT_HANDLE:  hFile = Peb->StandardInput;  break;
                case STD_OUTPUT_HANDLE: hFile = Peb->StandardOutput; break;
                case STD_ERROR_HANDLE:  hFile = Peb->StandardError;  break;
            }
            return hFile;
        }
    }
    return hFile;
}

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

    hFile = MusaCoreResolveStdHandle(hFile);
    if (hFile == INVALID_HANDLE_VALUE) {
        return FILE_TYPE_UNKNOWN;
    }

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
    hFile = MusaCoreResolveStdHandle(hFile);
    if (hFile == INVALID_HANDLE_VALUE) {
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
    hFile = MusaCoreResolveStdHandle(hFile);
    if (hFile == INVALID_HANDLE_VALUE) {
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
    hFile = MusaCoreResolveStdHandle(hFile);
    if (hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

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
    // Kernel-mode: default to synchronous unless OVERLAPPED is requested
    if (!(dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED)) {
        CreateOptions |= FILE_SYNCHRONOUS_IO_NONALERT;
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
    FILE_BASIC_INFORMATION BasicInfo;
    Status = ZwQueryAttributesFile(&ObjAttrs, &BasicInfo);
    RtlFreeUnicodeString(&NtPath);

    if (NT_SUCCESS(Status)) {
        WIN32_FILE_ATTRIBUTE_DATA* OutData = static_cast<WIN32_FILE_ATTRIBUTE_DATA*>(lpFileInformation);
        OutData->dwFileAttributes = BasicInfo.FileAttributes;
        OutData->ftCreationTime = reinterpret_cast<FILETIME&>(BasicInfo.CreationTime);
        OutData->ftLastAccessTime = reinterpret_cast<FILETIME&>(BasicInfo.LastAccessTime);
        OutData->ftLastWriteTime = reinterpret_cast<FILETIME&>(BasicInfo.LastWriteTime);
        OutData->nFileSizeHigh = 0;
        OutData->nFileSizeLow = 0;
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


_IRQL_requires_max_(PASSIVE_LEVEL)
DWORD WINAPI MUSA_NAME(GetTempPathW)(
    _In_ DWORD nBufferLength,
    _Out_writes_to_opt_(nBufferLength, return + 1) LPWSTR lpBuffer
)
{
    PAGED_CODE();

    // Try TMP first, then TEMP, then default
    static LPCWSTR Fallback = L"C:\\Windows\\Temp\\";
    LPCWSTR Candidates[] = { L"TMP", L"TEMP" };

    for (const auto& Name : Candidates) {
        DWORD Len = GetEnvironmentVariableW(Name, nullptr, 0);
        if (Len > 0 && Len <= nBufferLength) {
            Len = GetEnvironmentVariableW(Name, lpBuffer, nBufferLength);
            if (Len > 0) {
                return Len;
            }
        }
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
    RtlStringCchCopyW(lpBuffer, nBufferLength, Fallback);
    return FallbackLen - 1;
}

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

    RtlFreeUnicodeString(&NtPath);
    return DRIVE_UNKNOWN;
}

MUSA_IAT_SYMBOL(GetDriveTypeW, 4);

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

    if (lpFileName == nullptr || lpFindFileData == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    // Only support standard info level
    if (fInfoLevelId != FindExInfoStandard || fSearchOp > FindExSearchLimitToDirectories) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    // Build search path with wildcard
    WCHAR SearchPath[MAX_PATH];
    size_t Len = wcslen(lpFileName);
    if (Len + 4 > MAX_PATH) {
        BaseSetLastNTError(STATUS_NAME_TOO_LONG);
        return INVALID_HANDLE_VALUE;
    }
    wcscpy_s(SearchPath, lpFileName);
    // Append * if no wildcard present
    if (Len > 0 && SearchPath[Len - 1] != L'*' && SearchPath[Len - 1] != L'\\') {
        wcscat_s(SearchPath, L"\\*");
    } else if (Len > 0 && SearchPath[Len - 1] == L'\\') {
        wcscat_s(SearchPath, L"*");
    }

    UNICODE_STRING NtPath{};
    NTSTATUS Status = RtlDosPathNameToNtPathName_U_WithStatus(SearchPath, &NtPath, nullptr, nullptr);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
    }

    // Strip the wildcard suffix to get the directory path
    // Open the parent directory
    OBJECT_ATTRIBUTES ObjAttrs;
    InitializeObjectAttributes(&ObjAttrs, &NtPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr);

    HANDLE DirHandle = nullptr;
    IO_STATUS_BLOCK Iosb{};
    ULONG CreateOptions = FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT;
    if (dwAdditionalFlags & FIND_FIRST_EX_LARGE_FETCH)
        CreateOptions |= FILE_OPEN_FOR_BACKUP_INTENT;

    Status = ZwOpenFile(&DirHandle, FILE_LIST_DIRECTORY | SYNCHRONIZE, &ObjAttrs, &Iosb,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, CreateOptions);
    RtlFreeUnicodeString(&NtPath);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
    }

    // Use ZwQueryDirectoryFile to get the first matching file
    FILE_BOTH_DIR_INFORMATION DirInfo{};
    Status = ZwQueryDirectoryFile(DirHandle, nullptr, nullptr, nullptr, &Iosb,
        &DirInfo, sizeof(DirInfo), FileBothDirectoryInformation, TRUE,
        nullptr, TRUE);

    if (!NT_SUCCESS(Status) && Status != STATUS_NO_MORE_FILES) {
        ZwClose(DirHandle);
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
    }

    // Convert to WIN32_FIND_DATAW
    LPWIN32_FIND_DATAW FindData = static_cast<LPWIN32_FIND_DATAW>(lpFindFileData);
    RtlZeroMemory(FindData, sizeof(WIN32_FIND_DATAW));

    if (Status == STATUS_NO_MORE_FILES) {
        BaseSetLastNTError(STATUS_NO_MORE_FILES);
        return DirHandle; // Return handle even with no matches for FindClose
    }

    FindData->dwFileAttributes = DirInfo.FileAttributes;
    FindData->ftCreationTime = reinterpret_cast<FILETIME&>(DirInfo.CreationTime);
    FindData->ftLastAccessTime = reinterpret_cast<FILETIME&>(DirInfo.LastAccessTime);
    FindData->ftLastWriteTime = reinterpret_cast<FILETIME&>(DirInfo.LastWriteTime);
    FindData->nFileSizeHigh = DirInfo.EndOfFile.HighPart;
    FindData->nFileSizeLow = DirInfo.EndOfFile.LowPart;

    size_t NameLen = DirInfo.FileNameLength / sizeof(WCHAR);
    if (NameLen >= MAX_PATH) NameLen = MAX_PATH - 1;
    memcpy(FindData->cFileName, DirInfo.FileName, NameLen * sizeof(WCHAR));
    FindData->cFileName[NameLen] = L'\0';
    FindData->cAlternateFileName[0] = L'\0';

    return DirHandle;
}

MUSA_IAT_SYMBOL(FindFirstFileExW, 24);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(FindNextFileW)(
    _In_ HANDLE hFindFile,
    _Out_ LPWIN32_FIND_DATAW lpFindFileData
)
{
    if (hFindFile == INVALID_HANDLE_VALUE || lpFindFileData == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    FILE_BOTH_DIR_INFORMATION DirInfo{};
    IO_STATUS_BLOCK Iosb{};
    NTSTATUS Status = ZwQueryDirectoryFile(hFindFile, nullptr, nullptr, nullptr, &Iosb,
        &DirInfo, sizeof(DirInfo), FileBothDirectoryInformation, TRUE,
        nullptr, FALSE);

    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    RtlZeroMemory(lpFindFileData, sizeof(WIN32_FIND_DATAW));
    lpFindFileData->dwFileAttributes = DirInfo.FileAttributes;
    lpFindFileData->ftCreationTime = reinterpret_cast<FILETIME&>(DirInfo.CreationTime);
    lpFindFileData->ftLastAccessTime = reinterpret_cast<FILETIME&>(DirInfo.LastAccessTime);
    lpFindFileData->ftLastWriteTime = reinterpret_cast<FILETIME&>(DirInfo.LastWriteTime);
    lpFindFileData->nFileSizeHigh = DirInfo.EndOfFile.HighPart;
    lpFindFileData->nFileSizeLow = DirInfo.EndOfFile.LowPart;

    size_t NameLen = DirInfo.FileNameLength / sizeof(WCHAR);
    if (NameLen >= MAX_PATH) NameLen = MAX_PATH - 1;
    memcpy(lpFindFileData->cFileName, DirInfo.FileName, NameLen * sizeof(WCHAR));
    lpFindFileData->cFileName[NameLen] = L'\0';
    lpFindFileData->cAlternateFileName[0] = L'\0';

    return TRUE;
}

MUSA_IAT_SYMBOL(FindNextFileW, 8);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOL WINAPI MUSA_NAME(FindClose)(
    _Inout_ HANDLE hFindFile
)
{
    if (hFindFile == INVALID_HANDLE_VALUE || hFindFile == nullptr) {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
    }

    NTSTATUS Status = ZwClose(hFindFile);
    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

MUSA_IAT_SYMBOL(FindClose, 4);
EXTERN_C_END
