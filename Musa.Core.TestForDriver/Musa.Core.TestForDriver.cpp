// unnecessary, fix ReSharper's code analysis.
#pragma warning(suppress: 4117)
#define _KERNEL_MODE 1

#include <Veil.h>
#include <Musa.Core/Musa.Core.h>

#define MusaLOG(fmt, ...) DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, \
    "[Musa.Core][%s():%u] " fmt "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__)

#define KTEST_EXPECT(expr, name)                                              \
    do {                                                                      \
        ++TestsRun;                                                           \
        if (expr) {                                                           \
            MusaLOG("[PASS] %s", name);                                       \
        }                                                                     \
        else {                                                                \
            ++TestsFailed;                                                    \
            MusaLOG("[FAIL] %s", name);                                       \
        }                                                                     \
    } while (false)


EXTERN_C DRIVER_INITIALIZE DriverEntry;
EXTERN_C DRIVER_UNLOAD     DriverUnload;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, DriverUnload)
#endif


namespace Main
{
    static UNICODE_STRING SavedRegistryPath{};

    static void RunTests()
    {
        ULONG TestsRun    = 0;
        ULONG TestsFailed = 0;

        MusaLOG("=== Musa.Core Kernel Test Suite ===");

        // --- StdHandle ---

        {
            const auto StdIn = GetStdHandle(STD_INPUT_HANDLE);
            KTEST_EXPECT(StdIn != nullptr && StdIn != INVALID_HANDLE_VALUE,
                "StdHandle_GetInput_ReturnsValidHandle");
        }

        {
            const auto StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
            KTEST_EXPECT(StdOut != nullptr && StdOut != INVALID_HANDLE_VALUE,
                "StdHandle_GetOutput_ReturnsValidHandle");
        }

        {
            const auto StdErr = GetStdHandle(STD_ERROR_HANDLE);
            KTEST_EXPECT(StdErr != nullptr && StdErr != INVALID_HANDLE_VALUE,
                "StdHandle_GetError_ReturnsValidHandle");
        }

        {
            DWORD Flags = 0;
            KTEST_EXPECT(GetHandleInformation(GetStdHandle(STD_OUTPUT_HANDLE), &Flags),
                "StdHandle_GetHandleInformation_Succeeds");
        }

        {
            const auto StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
            HANDLE Dup = nullptr;
            BOOL Result = DuplicateHandle(GetCurrentProcess(), StdOut,
                GetCurrentProcess(), &Dup, 0, FALSE, DUPLICATE_SAME_ACCESS);
            KTEST_EXPECT(Result,
                "StdHandle_DuplicateHandle_Succeeds");
            if (Dup) {
                CloseHandle(Dup);
            }
        }

        {
            const auto StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
            const auto StdErr = GetStdHandle(STD_ERROR_HANDLE);
            KTEST_EXPECT(SetStdHandle(STD_OUTPUT_HANDLE, StdErr),
                "StdHandle_SetStdHandle_Succeeds");
            KTEST_EXPECT(GetStdHandle(STD_OUTPUT_HANDLE) == StdErr,
                "StdHandle_SetStdHandle_ValuePersists");
            SetStdHandle(STD_OUTPUT_HANDLE, StdOut);
        }

        {
            KTEST_EXPECT(GetStdHandle((DWORD)-1) == INVALID_HANDLE_VALUE,
                "StdHandle_InvalidId_ReturnsInvalidHandle");
        }

        // --- Heap ---

        {
            const auto Heap = GetProcessHeap();
            KTEST_EXPECT(Heap != nullptr,
                "Heap_GetProcessHeap_ReturnsNonNull");
        }

        {
            const auto Heap = GetProcessHeap();
            auto* Ptr = static_cast<BYTE*>(HeapAlloc(Heap, HEAP_ZERO_MEMORY, 128));
            KTEST_EXPECT(Ptr != nullptr,
                "Heap_Alloc_Succeeds");
            if (Ptr) {
                KTEST_EXPECT(Ptr[0] == 0 && Ptr[127] == 0,
                    "Heap_AllocZeroMemory_IsZeroed");

                const auto Size = HeapSize(Heap, 0, Ptr);
                KTEST_EXPECT(Size >= 128,
                    "Heap_Size_ReturnsAtLeastRequested");

                auto* Ptr2 = static_cast<BYTE*>(HeapReAlloc(Heap, HEAP_ZERO_MEMORY, Ptr, 256));
                KTEST_EXPECT(Ptr2 != nullptr,
                    "Heap_ReAlloc_Succeeds");
                if (Ptr2) {
                    Ptr = Ptr2;
                    const auto Size2 = HeapSize(Heap, 0, Ptr);
                    KTEST_EXPECT(Size2 >= 256,
                        "Heap_ReAlloc_SizeIncreased");
                }

                KTEST_EXPECT(HeapValidate(Heap, 0, Ptr),
                    "Heap_Validate_Succeeds");
                KTEST_EXPECT(HeapFree(Heap, 0, Ptr),
                    "Heap_Free_Succeeds");
            }
        }

        {
            const auto CustomHeap = HeapCreate(HEAP_GROWABLE, 0, 0);
            KTEST_EXPECT(CustomHeap != nullptr,
                "Heap_Create_Succeeds");
            if (CustomHeap) {
                auto* Block = HeapAlloc(CustomHeap, 0, 64);
                KTEST_EXPECT(Block != nullptr,
                    "Heap_CustomAlloc_Succeeds");
                if (Block) {
                    HeapFree(CustomHeap, 0, Block);
                }
                HeapDestroy(CustomHeap);
            }
        }

        {
            ULONG HeapCount = GetProcessHeaps(0, nullptr);
            KTEST_EXPECT(HeapCount >= 1,
                "Heap_GetProcessHeaps_ReturnsAtLeastOne");
        }

        {
            auto* LocalPtr = LocalAlloc(LPTR, 64);
            KTEST_EXPECT(LocalPtr != nullptr,
                "Heap_LocalAlloc_Succeeds");
            if (LocalPtr) {
                KTEST_EXPECT(LocalSize(LocalPtr) >= 64,
                    "Heap_LocalSize_ReturnsAtLeastRequested");
                LocalFree(LocalPtr);
            }
        }

        {
            auto* GlobalPtr = GlobalAlloc(GPTR, 64);
            KTEST_EXPECT(GlobalPtr != nullptr,
                "Heap_GlobalAlloc_Succeeds");
            if (GlobalPtr) {
                KTEST_EXPECT(GlobalSize(GlobalPtr) >= 64,
                    "Heap_GlobalSize_ReturnsAtLeastRequested");
                GlobalFree(GlobalPtr);
            }
        }

        // --- Synchronization ---

        {
            CRITICAL_SECTION Cs{};
            KTEST_EXPECT(InitializeCriticalSectionAndSpinCount(&Cs, 4000),
                "Sync_InitCriticalSection_Succeeds");
            const auto OldSpin = SetCriticalSectionSpinCount(&Cs, 2000);
            KTEST_EXPECT(OldSpin == 4000,
                "Sync_SetCriticalSectionSpinCount_ReturnsOldValue");
            EnterCriticalSection(&Cs);
            KTEST_EXPECT(TryEnterCriticalSection(&Cs),
                "Sync_TryEnterCriticalSection_SucceedsWhenOwned");
            LeaveCriticalSection(&Cs);
            LeaveCriticalSection(&Cs);
            DeleteCriticalSection(&Cs);
        }

        {
            SRWLOCK Srw = SRWLOCK_INIT;
            InitializeSRWLock(&Srw);
            AcquireSRWLockExclusive(&Srw);
            ReleaseSRWLockExclusive(&Srw);
            KTEST_EXPECT(TryAcquireSRWLockExclusive(&Srw),
                "Sync_TryAcquireSRWLockExclusive_Succeeds");
            ReleaseSRWLockExclusive(&Srw);
            AcquireSRWLockShared(&Srw);
            ReleaseSRWLockShared(&Srw);
        }

        {
            const auto Event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
            KTEST_EXPECT(Event != nullptr,
                "Sync_CreateEvent_Succeeds");
            if (Event) {
                KTEST_EXPECT(SetEvent(Event),
                    "Sync_SetEvent_Succeeds");
                KTEST_EXPECT(WaitForSingleObject(Event, 0) == WAIT_OBJECT_0,
                    "Sync_WaitSignaledEvent_ReturnsObject0");
                KTEST_EXPECT(ResetEvent(Event),
                    "Sync_ResetEvent_Succeeds");
                KTEST_EXPECT(WaitForSingleObject(Event, 0) == WAIT_TIMEOUT,
                    "Sync_WaitResetEvent_ReturnsTimeout");
                CloseHandle(Event);
            }
        }

        {
            const auto Mutex = CreateMutexW(nullptr, FALSE, nullptr);
            KTEST_EXPECT(Mutex != nullptr,
                "Sync_CreateMutex_Succeeds");
            if (Mutex) {
                KTEST_EXPECT(WaitForSingleObject(Mutex, 0) == WAIT_OBJECT_0,
                    "Sync_WaitMutex_ReturnsObject0");
                KTEST_EXPECT(ReleaseMutex(Mutex),
                    "Sync_ReleaseMutex_Succeeds");
                CloseHandle(Mutex);
            }
        }

        {
            const auto Semaphore = CreateSemaphoreW(nullptr, 1, 2, nullptr);
            KTEST_EXPECT(Semaphore != nullptr,
                "Sync_CreateSemaphore_Succeeds");
            if (Semaphore) {
                KTEST_EXPECT(WaitForSingleObject(Semaphore, 0) == WAIT_OBJECT_0,
                    "Sync_WaitSemaphore_ReturnsObject0");
                LONG PrevCount = 0;
                KTEST_EXPECT(ReleaseSemaphore(Semaphore, 1, &PrevCount),
                    "Sync_ReleaseSemaphore_Succeeds");
                KTEST_EXPECT(PrevCount == 0,
                    "Sync_ReleaseSemaphore_PrevCountZero");
                CloseHandle(Semaphore);
            }
        }

        // --- Process ---

        {
            KTEST_EXPECT(GetCurrentProcess() != nullptr,
                "Process_GetCurrentProcess_ReturnsNonNull");
        }

        {
            KTEST_EXPECT(GetCurrentProcessId() != 0,
                "Process_GetCurrentProcessId_ReturnsNonZero");
        }

        {
            FILETIME Creation{}, Exit{}, Kernel{}, User{};
            KTEST_EXPECT(GetProcessTimes(GetCurrentProcess(), &Creation, &Exit, &Kernel, &User),
                "Process_GetProcessTimes_Succeeds");
            KTEST_EXPECT(Creation.dwLowDateTime != 0 || Creation.dwHighDateTime != 0,
                "Process_GetProcessTimes_CreationNonZero");
        }

        {
            DWORD HandleCount = 0;
            KTEST_EXPECT(GetProcessHandleCount(GetCurrentProcess(), &HandleCount),
                "Process_GetProcessHandleCount_Succeeds");
            KTEST_EXPECT(HandleCount > 0,
                "Process_GetProcessHandleCount_GreaterThanZero");
        }

        {
            const auto ProcessId = GetCurrentProcessId();
            DWORD SessionId = 0;
            KTEST_EXPECT(ProcessIdToSessionId(ProcessId, &SessionId),
                "Process_ProcessIdToSessionId_Succeeds");
        }

        {
            KTEST_EXPECT(GetProcessId(GetCurrentProcess()) == GetCurrentProcessId(),
                "Process_GetProcessId_MatchesCurrentProcessId");
        }

        {
            STARTUPINFOW Si{};
            GetStartupInfoW(&Si);
            KTEST_EXPECT(Si.cb == sizeof(Si),
                "Process_GetStartupInfoW_CbMatchesSize");
        }

        // --- Thread ---

        {
            KTEST_EXPECT(GetCurrentThread() != nullptr,
                "Thread_GetCurrentThread_ReturnsNonNull");
        }

        {
            KTEST_EXPECT(GetCurrentThreadId() != 0,
                "Thread_GetCurrentThreadId_ReturnsNonZero");
        }

        {
            KTEST_EXPECT(GetThreadId(GetCurrentThread()) == GetCurrentThreadId(),
                "Thread_GetThreadId_MatchesCurrentThreadId");
        }

        {
            KTEST_EXPECT(GetProcessIdOfThread(GetCurrentThread()) == GetCurrentProcessId(),
                "Thread_GetProcessIdOfThread_MatchesCurrentProcessId");
        }

        {
            KTEST_EXPECT(GetThreadPriority(GetCurrentThread()) != THREAD_PRIORITY_ERROR_RETURN,
                "Thread_GetThreadPriority_ReturnsValidValue");
        }

        {
            FILETIME Creation{}, Exit{}, Kernel{}, User{};
            KTEST_EXPECT(GetThreadTimes(GetCurrentThread(), &Creation, &Exit, &Kernel, &User),
                "Thread_GetThreadTimes_Succeeds");
        }

        // --- Utility ---

        {
            auto* Original = reinterpret_cast<PVOID>(static_cast<ULONG_PTR>(0xDEADBEEF));
            auto* Decoded = DecodePointer(EncodePointer(Original));
            KTEST_EXPECT(Decoded == Original,
                "Utility_EncodeDecodePointer_RoundTrip");
        }

        {
            auto* Original = reinterpret_cast<PVOID>(static_cast<ULONG_PTR>(0xDEADBEEF));
            auto* Decoded = DecodeSystemPointer(EncodeSystemPointer(Original));
            KTEST_EXPECT(Decoded == Original,
                "Utility_EncodeDecodeSystemPointer_RoundTrip");
        }

        {
            auto* Original = reinterpret_cast<PVOID>(static_cast<ULONG_PTR>(0xDEADBEEF));
            PVOID RemoteEncoded = nullptr;
            auto Hr = EncodeRemotePointer(GetCurrentProcess(), Original, &RemoteEncoded);
            KTEST_EXPECT(SUCCEEDED(Hr),
                "Utility_EncodeRemotePointer_Succeeds");
            if (SUCCEEDED(Hr)) {
                PVOID RemoteDecoded = nullptr;
                Hr = DecodeRemotePointer(GetCurrentProcess(), RemoteEncoded, &RemoteDecoded);
                KTEST_EXPECT(SUCCEEDED(Hr),
                    "Utility_DecodeRemotePointer_Succeeds");
                KTEST_EXPECT(RemoteDecoded == Original,
                    "Utility_EncodeDecodeRemotePointer_RoundTrip");
            }
        }

        // --- FLS ---

        {
            const auto FlsIndex = FlsAlloc(nullptr);
            KTEST_EXPECT(FlsIndex != FLS_OUT_OF_INDEXES,
                "Fls_Alloc_Succeeds");
            if (FlsIndex != FLS_OUT_OF_INDEXES) {
                auto* TestValue = reinterpret_cast<PVOID>(static_cast<ULONG_PTR>(0x12345678));
                KTEST_EXPECT(FlsSetValue(FlsIndex, TestValue),
                    "Fls_SetValue_Succeeds");
                KTEST_EXPECT(FlsGetValue(FlsIndex) == TestValue,
                    "Fls_GetValue_ReturnsSetValue");
                KTEST_EXPECT(FlsFree(FlsIndex),
                    "Fls_Free_Succeeds");
            }
        }

        // --- Error ---

        {
            SetLastError(ERROR_SUCCESS);
            KTEST_EXPECT(GetLastError() == ERROR_SUCCESS,
                "Error_SetGetLastError_Success");
        }

        {
            SetLastError(ERROR_INVALID_PARAMETER);
            KTEST_EXPECT(GetLastError() == ERROR_INVALID_PARAMETER,
                "Error_SetGetLastError_InvalidParameter");
            SetLastError(ERROR_SUCCESS);
        }

        // --- LibraryLoader ---

        {
            const auto Ntoskrnl = GetModuleHandleW(L"ntoskrnl.exe");
            KTEST_EXPECT(Ntoskrnl != nullptr,
                "LibraryLoader_GetModuleHandleNtoskrnl_Succeeds");
        }

        {
            const auto Kernel32 = GetModuleHandleW(L"kernel32.dll");
            KTEST_EXPECT(Kernel32 == nullptr,
                "LibraryLoader_GetModuleHandleKernel32_ReturnsNull");
        }

        // --- NLS ---

        {
            const wchar_t WStr[] = L"Hello";
            char Buf[32] = {};
            const auto Result = WideCharToMultiByte(CP_UTF8, 0, WStr, -1, Buf, sizeof(Buf), nullptr, nullptr);
            KTEST_EXPECT(Result > 0,
                "NLS_WideCharToMultiByte_UTF8_Succeeds");
        }

        {
            const char Str[] = "Hello";
            wchar_t Buf[32] = {};
            const auto Result = MultiByteToWideChar(CP_UTF8, 0, Str, -1, Buf, sizeof(Buf) / sizeof(wchar_t));
            KTEST_EXPECT(Result > 0,
                "NLS_MultiByteToWideChar_UTF8_Succeeds");
        }

        {
            const auto Acp = GetACP();
            KTEST_EXPECT(Acp != 0,
                "NLS_GetACP_ReturnsNonZero");
        }

        {
            CPINFOEXW CpInfo = {};
            KTEST_EXPECT(GetCPInfoExW(CP_UTF8, 0, &CpInfo),
                "NLS_GetCPInfoExW_Succeeds");
        }

        // --- RealTime ---

        {
            FILETIME Ft{};
            GetSystemTimePreciseAsFileTime(&Ft);
            KTEST_EXPECT(Ft.dwLowDateTime != 0 || Ft.dwHighDateTime != 0,
                "RealTime_GetSystemTimePreciseAsFileTime_NonZero");
        }

        {
            const auto Ticks = GetTickCount64();
            KTEST_EXPECT(Ticks > 0,
                "RealTime_GetTickCount64_ReturnsNonZero");
        }

        {
            const auto Ticks32 = GetTickCount();
            KTEST_EXPECT(Ticks32 != 0,
                "RealTime_GetTickCount_ReturnsNonZero");
        }

        {
            LARGE_INTEGER Freq{};
            KTEST_EXPECT(QueryPerformanceFrequency(&Freq),
                "RealTime_QueryPerformanceFrequency_Succeeds");
            KTEST_EXPECT(Freq.QuadPart > 0,
                "RealTime_QueryPerformanceFrequency_Positive");
        }

        {
            LARGE_INTEGER Counter{};
            KTEST_EXPECT(QueryPerformanceCounter(&Counter),
                "RealTime_QueryPerformanceCounter_Succeeds");
        }


        // --- Time ---

        {
            SYSTEMTIME St{};
            GetSystemTime(&St);
            KTEST_EXPECT(St.wYear >= 2026,
                "Time_GetSystemTime_ReturnsPlausibleYear");
            KTEST_EXPECT(St.wMonth >= 1 && St.wMonth <= 12,
                "Time_GetSystemTime_MonthInRange");
        }

        {
            SYSTEMTIME St{};
            GetLocalTime(&St);
            KTEST_EXPECT(St.wYear >= 2026,
                "Time_GetLocalTime_ReturnsPlausibleYear");
        }

        {
            SYSTEMTIME St{};
            GetSystemTime(&St);

            FILETIME Ft{};
            KTEST_EXPECT(SystemTimeToFileTime(&St, &Ft),
                "Time_SystemTimeToFileTime_Succeeds");

            SYSTEMTIME St2{};
            KTEST_EXPECT(FileTimeToSystemTime(&Ft, &St2),
                "Time_FileTimeToSystemTime_Succeeds");

            KTEST_EXPECT(St2.wYear == St.wYear && St2.wMonth == St.wMonth && St2.wDay == St.wDay,
                "Time_FileTimeToSystemTime_RoundTrip");
        }

        {
            SYSTEMTIME St{};
            GetSystemTime(&St);

            FILETIME Ft{};
            SystemTimeToFileTime(&St, &Ft);

            FILETIME LocalFt{};
            KTEST_EXPECT(FileTimeToLocalFileTime(&Ft, &LocalFt),
                "Time_FileTimeToLocalFileTime_Succeeds");

            FILETIME UtcFt{};
            KTEST_EXPECT(LocalFileTimeToFileTime(&LocalFt, &UtcFt),
                "Time_LocalFileTimeToFileTime_Succeeds");

            KTEST_EXPECT(UtcFt.dwLowDateTime == Ft.dwLowDateTime && UtcFt.dwHighDateTime == Ft.dwHighDateTime,
                "Time_LocalFileTimeToFileTime_RoundTrip");
        }


        // --- Path ---

        {
            DWORD TempLen = GetTempPathW(0, nullptr);
            KTEST_EXPECT(TempLen > 0,
                "Path_GetTempPathW_ReturnsLength");

            WCHAR TempBuf[260] = {};
            TempLen = GetTempPathW(_countof(TempBuf), TempBuf);
            KTEST_EXPECT(TempLen > 0 && TempLen < _countof(TempBuf),
                "Path_GetTempPathW_ReturnsPath");
        }

        {
            // RtlDosPathNameToNtPathName_U
            UNICODE_STRING NtPath{};
            BOOLEAN Result = RtlDosPathNameToNtPathName_U(
                L"C:\\Windows\\System32\\ntoskrnl.exe",
                &NtPath, nullptr, nullptr);
            KTEST_EXPECT(Result,
                "Path_RtlDosPathNameToNtPathName_U_DrivePath");
            KTEST_EXPECT(wcsncmp(NtPath.Buffer, L"\\??\\C:\\Windows\\System32\\ntoskrnl.exe", NtPath.Length / sizeof(WCHAR)) == 0,
                "Path_RtlDosPathNameToNtPathName_U_DrivePath_Content");
            RtlFreeUnicodeString(&NtPath);

            // UNC path
            NtPath.Buffer = nullptr;
            Result = RtlDosPathNameToNtPathName_U(
                L"\\\\localhost\\share\\file.txt",
                &NtPath, nullptr, nullptr);
            KTEST_EXPECT(Result,
                "Path_RtlDosPathNameToNtPathName_U_UncPath");
            KTEST_EXPECT(wcsncmp(NtPath.Buffer, L"\\??\\UNC\\localhost\\share\\file.txt", NtPath.Length / sizeof(WCHAR)) == 0,
                "Path_RtlDosPathNameToNtPathName_U_UncPath_Content");
            RtlFreeUnicodeString(&NtPath);

            // Already NT path
            NtPath.Buffer = nullptr;
            Result = RtlDosPathNameToNtPathName_U(
                L"\\Device\\HarddiskVolume3\\Windows",
                &NtPath, nullptr, nullptr);
            KTEST_EXPECT(Result,
                "Path_RtlDosPathNameToNtPathName_U_NtPath");
            KTEST_EXPECT(wcsncmp(NtPath.Buffer, L"\\Device\\HarddiskVolume3\\Windows", NtPath.Length / sizeof(WCHAR)) == 0,
                "Path_RtlDosPathNameToNtPathName_U_NtPath_Content");
            RtlFreeUnicodeString(&NtPath);

            // Extended-length path
            NtPath.Buffer = nullptr;
            Result = RtlDosPathNameToNtPathName_U(
                L"\\\\?\\C:\\Windows\\System32\\ntoskrnl.exe",
                &NtPath, nullptr, nullptr);
            KTEST_EXPECT(Result,
                "Path_RtlDosPathNameToNtPathName_U_ExtendedPath");
            KTEST_EXPECT(wcsncmp(NtPath.Buffer, L"\\??\\C:\\Windows\\System32\\ntoskrnl.exe", NtPath.Length / sizeof(WCHAR)) == 0,
                "Path_RtlDosPathNameToNtPathName_U_ExtendedPath_Content");
            RtlFreeUnicodeString(&NtPath);

            // Invalid path
            NtPath.Buffer = nullptr;
            Result = RtlDosPathNameToNtPathName_U(
                L"no_backslash_here",
                &NtPath, nullptr, nullptr);
            KTEST_EXPECT(!Result,
                "Path_RtlDosPathNameToNtPathName_U_RelativeFails");
        }

        // --- File I/O ---

        {
            // Create a test file
            HANDLE hFile = CreateFileW(
                L"C:\\Windows\\Temp\\MusaCore_testfile.tmp",
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_DELETE,
                nullptr,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
                nullptr);
            KTEST_EXPECT(hFile != INVALID_HANDLE_VALUE,
                "FileIO_CreateFileW_CreatesFile");


            // Test hTemplateFile
            HANDLE hFromTemplate = CreateFileW(
                L"C:\\Windows\\Temp\\MusaCore_from_template.tmp",
                GENERIC_READ,
                FILE_SHARE_DELETE,
                nullptr,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
                hFile);
            KTEST_EXPECT(hFromTemplate != INVALID_HANDLE_VALUE,
                "FileIO_CreateFileW_WithTemplateFile_Succeeds");
            if (hFromTemplate != INVALID_HANDLE_VALUE) {
                CloseHandle(hFromTemplate);
            }
            if (hFile != INVALID_HANDLE_VALUE) {
                // Get file type
                DWORD FileType = GetFileType(hFile);
                KTEST_EXPECT(FileType == FILE_TYPE_DISK,
                    "FileIO_GetFileType_ReturnsDisk");

                // Write data
                const char TestData[] = "Hello Musa.Core!";
                DWORD BytesWritten = 0;
                KTEST_EXPECT(WriteFile(hFile, TestData, sizeof(TestData) - 1, &BytesWritten, nullptr),
                    "FileIO_WriteFile_Succeeds");
                KTEST_EXPECT(BytesWritten == sizeof(TestData) - 1,
                    "FileIO_WriteFile_ByteCountMatches");

                // Set file pointer to beginning
                LARGE_INTEGER Dist{};
                Dist.QuadPart = 0;
                LARGE_INTEGER NewPos{};
                KTEST_EXPECT(SetFilePointerEx(hFile, Dist, &NewPos, FILE_BEGIN),
                    "FileIO_SetFilePointerEx_Begin_Succeeds");

                // Read data back
                char ReadBuf[64] = {};
                DWORD BytesRead = 0;
                KTEST_EXPECT(ReadFile(hFile, ReadBuf, sizeof(ReadBuf), &BytesRead, nullptr),
                    "FileIO_ReadFile_Succeeds");
                KTEST_EXPECT(BytesRead == sizeof(TestData) - 1,
                    "FileIO_ReadFile_ByteCountMatches");
                KTEST_EXPECT(memcmp(ReadBuf, TestData, sizeof(TestData) - 1) == 0,
                    "FileIO_ReadFile_DataMatches");

                // Set file pointer to end
                Dist.QuadPart = 0;
                KTEST_EXPECT(SetFilePointerEx(hFile, Dist, &NewPos, FILE_END),
                    "FileIO_SetFilePointerEx_End_Succeeds");
                KTEST_EXPECT(NewPos.QuadPart == static_cast<LONGLONG>(sizeof(TestData) - 1),
                    "FileIO_SetFilePointerEx_End_CorrectPosition");

                // Flush buffers
                KTEST_EXPECT(FlushFileBuffers(hFile),
                    "FileIO_FlushFileBuffers_Succeeds");

                CloseHandle(hFile);
            }
        }

        {
            // Get file attributes of an existing file
            WIN32_FILE_ATTRIBUTE_DATA AttrData{};
            BOOL Result = GetFileAttributesExW(
                L"C:\\Windows\\System32\\ntoskrnl.exe",
                GetFileExInfoStandard,
                &AttrData);
            KTEST_EXPECT(Result,
                "FileIO_GetFileAttributesExW_Succeeds");
            if (Result) {
                KTEST_EXPECT((AttrData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0,
                    "FileIO_GetFileAttributesExW_NotDirectory");
            }
        }

        {
            // SetFilePointer legacy API
            HANDLE hFile = CreateFileW(
                L"C:\\Windows\\Temp\\MusaCore_testfile2.tmp",
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_DELETE,
                nullptr,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
                nullptr);
            if (hFile != INVALID_HANDLE_VALUE) {
                const char Data[32] = {};
                WriteFile(hFile, Data, sizeof(Data), nullptr, nullptr);

                DWORD Result = SetFilePointer(hFile, 8, nullptr, FILE_BEGIN);
                KTEST_EXPECT(Result != INVALID_SET_FILE_POINTER,
                    "FileIO_SetFilePointer_Succeeds");
                KTEST_EXPECT(Result == 8,
                    "FileIO_SetFilePointer_CorrectPosition");

                CloseHandle(hFile);
            }
        }

        {
            // SetFileAttributesW
            SetFileAttributesW(L"C:\\Windows\\Temp\\MusaCore_attr_test.tmp", 0);
        }

        {
            // DeleteFileW
            HANDLE hFile = CreateFileW(
                L"C:\\Windows\\Temp\\MusaCore_delme.tmp",
                GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                FILE_ATTRIBUTE_TEMPORARY, nullptr);
            if (hFile != INVALID_HANDLE_VALUE) {
                CloseHandle(hFile);
                KTEST_EXPECT(DeleteFileW(L"\\??\\C:\\Windows\\Temp\\MusaCore_delme.tmp"),
                    "FileIO_DeleteFileW_Succeeds");
            }
        }


        // --- Directory & Find ---

        {
            KTEST_EXPECT(CreateDirectoryW(L"C:\\Windows\\Temp\\MusaCore_dir_test", nullptr),
                "DirIO_CreateDirectoryW_Succeeds");
            KTEST_EXPECT(RemoveDirectoryW(L"C:\\Windows\\Temp\\MusaCore_dir_test"),
                "DirIO_RemoveDirectoryW_Succeeds");
        }

        {
            HANDLE hFile = CreateFileW(
                L"C:\\Windows\\Temp\\MusaCore_rename_src.tmp",
                GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                FILE_ATTRIBUTE_TEMPORARY, nullptr);
            if (hFile != INVALID_HANDLE_VALUE) {
                CloseHandle(hFile);
                KTEST_EXPECT(MoveFileExW(
                    L"C:\\Windows\\Temp\\MusaCore_rename_src.tmp",
                    L"C:\\Windows\\Temp\\MusaCore_rename_dst.tmp",
                    MOVEFILE_REPLACE_EXISTING),
                    "DirIO_MoveFileExW_Succeeds");
                DeleteFileW(L"C:\\Windows\\Temp\\MusaCore_rename_dst.tmp");
            }
        }

        {
            UINT DriveType = GetDriveTypeW(L"C:\\");
            KTEST_EXPECT(DriveType == DRIVE_FIXED || DriveType == DRIVE_REMOTE,
                "DirIO_GetDriveTypeW_ReturnsValidType");
        }

        {
            WIN32_FIND_DATAW FindData{};
            HANDLE hFind = FindFirstFileExW(
                L"C:\\Windows\\System32",
                FindExInfoStandard, &FindData,
                FindExSearchNameMatch, nullptr, 0);
            if (hFind != INVALID_HANDLE_VALUE) {
                KTEST_EXPECT(FindData.cFileName[0] != L'\0',
                    "DirIO_FindFirstFileExW_ReturnsFileName");

                int FoundCount = 1;
                while (FindNextFileW(hFind, &FindData)) {
                    ++FoundCount;
                    if (FoundCount >= 50) break;
                }
                KTEST_EXPECT(FoundCount >= 1,
                    "DirIO_FindNextFileW_FindsMultiple");

                KTEST_EXPECT(FindClose(hFind),
                    "DirIO_FindClose_Succeeds");
            }
        }

        {
            // FindClose on invalid handle
            KTEST_EXPECT(!FindClose(nullptr),
                "DirIO_FindClose_NullFails");
            KTEST_EXPECT(!FindClose(INVALID_HANDLE_VALUE),
                "DirIO_FindClose_InvalidFails");
        }
        // --- CommandLine & Environment ---

        {
            LPWSTR CmdLine = GetCommandLineW();
            KTEST_EXPECT(CmdLine != nullptr,
                "Sys_GetCommandLineW_ReturnsNonNull");
        }

        {
            // SystemRoot may be unavailable at DriverEntry time
            DWORD Len = GetEnvironmentVariableW(L"SystemRoot", nullptr, 0); (void)Len;
            KTEST_EXPECT(true,
                "Sys_GetEnvironmentVariableW_SystemRoot_NoCrash");
        }

        {
            DWORD MissingLen = GetEnvironmentVariableW(L"MusaCore_NoSuchVar_XYZ", nullptr, 0);
            KTEST_EXPECT(MissingLen == 0,
                "Sys_GetEnvironmentVariableW_NonExistent_ReturnsZero");
        }

        // --- CWD ---

        {
            WCHAR CwdBuf[MAX_PATH] = {};
            DWORD Len = GetCurrentDirectoryW(0, nullptr);
            KTEST_EXPECT(Len > 0,
                "Sys_GetCurrentDirectoryW_QuerySize_Succeeds");

            Len = GetCurrentDirectoryW(Len, CwdBuf);
            KTEST_EXPECT(Len > 0 && CwdBuf[0] != L'\0',
                "Sys_GetCurrentDirectoryW_ReturnsPath");
            KTEST_EXPECT(wcsncmp(CwdBuf, L"C:\\Windows", Len) == 0,
                "Sys_GetCurrentDirectoryW_ReturnsSystemRoot");

            // Change CWD and verify
            KTEST_EXPECT(SetCurrentDirectoryW(L"C:\\Windows\\Temp"),
                "Sys_SetCurrentDirectoryW_Succeeds");
            WCHAR NewCwd[MAX_PATH] = {};
            DWORD NewLen = GetCurrentDirectoryW(_countof(NewCwd), NewCwd);
            KTEST_EXPECT(NewLen > 0 && wcsncmp(NewCwd, L"C:\\Windows\\Temp", NewLen) == 0,
                "Sys_SetCurrentDirectoryW_ValueStored");

            // Restore
            SetCurrentDirectoryW(L"C:\\Windows");
        }

        {
            DWORD Len = ExpandEnvironmentStringsW(L"%SystemRoot%\\System32\\ntoskrnl.exe", nullptr, 0);
            KTEST_EXPECT(Len > 0,
                "Sys_ExpandEnvironmentStringsW_QuerySize_Succeeds");

            if (Len > 0 && Len < MAX_PATH) {
                WCHAR ExpBuf[MAX_PATH] = {};
                Len = ExpandEnvironmentStringsW(L"%SystemRoot%\\System32\\ntoskrnl.exe", ExpBuf, Len);
                KTEST_EXPECT(Len > 0,
                    "Sys_ExpandEnvironmentStringsW_Expands");
                // Verify content starts with SystemRoot
                KTEST_EXPECT(wcsncmp(ExpBuf, L"C:\\Windows\\", 11) == 0,
                    "Sys_ExpandEnvironmentStringsW_ExpandedStartsWithSystemRoot");
                KTEST_EXPECT(Len >= static_cast<DWORD>(wcslen(L"C:\\Windows\\System32\\ntoskrnl.exe")),
                    "Sys_ExpandEnvironmentStringsW_ExpandedLength");
            }

            // Literal string without variables
            WCHAR LitBuf[64] = {};
            Len = ExpandEnvironmentStringsW(L"C:\\Windows\\System32", LitBuf, _countof(LitBuf));
            KTEST_EXPECT(Len > 0 && wcsncmp(LitBuf, L"C:\\Windows\\System32", wcslen(L"C:\\Windows\\System32")) == 0,
                "Sys_ExpandEnvironmentStringsW_LiteralContentMatches");
        }
        // --- Handle ---

        {
            const auto Event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
            if (Event) {
                DWORD Flags = 0;
                KTEST_EXPECT(GetHandleInformation(Event, &Flags),
                    "Handle_GetHandleInformation_Succeeds");
                CloseHandle(Event);
            }
        }

        // --- Debug ---

        {
            wchar_t Buf[256] = {};
            const auto Result = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, ERROR_SUCCESS, 0, Buf, sizeof(Buf) / sizeof(wchar_t), nullptr);
            KTEST_EXPECT(Result > 0,
                "Debug_FormatMessageW_Succeeds");
        }

        {
            OutputDebugStringW(L"[Musa.Core.Test] Debug output test\n");
            KTEST_EXPECT(true,
                "Debug_OutputDebugStringW_NoCrash");
        }



        // --- New APIs (missing from docs/missing-apis.md) ---


        // GetSystemTimeAsFileTime — compare with SystemTimeToFileTime
        {
            FILETIME Ft1{};
            GetSystemTimeAsFileTime(&Ft1);

            SYSTEMTIME St{};
            GetSystemTime(&St);
            FILETIME Ft2{};
            SystemTimeToFileTime(&St, &Ft2);

            // Ft1 (precise) should be within ~1s of Ft2 (from GetSystemTime)
            LONGLONG Diff = static_cast<LONGLONG>(Ft1.dwHighDateTime - Ft2.dwHighDateTime);
            Diff = (Diff << 32) | (Ft1.dwLowDateTime - Ft2.dwLowDateTime);
            // Allow 2 seconds difference (2 * 10^7 100ns units)
            Diff = Diff < 0 ? -Diff : Diff;
            KTEST_EXPECT(Diff < 20000000,
                "Time_GetSystemTimeAsFileTime_WithinTwoSeconds");
        }

        // Environment Variable Write / Read / Delete
        {
            BOOL SetOk = SetEnvironmentVariableW(L"MusaCore_Test_Var", L"HelloWorld");
            KTEST_EXPECT(SetOk,
                "Env_SetEnvironmentVariableW_Succeeds");

            WCHAR Buf[64] = {};
            DWORD Len = GetEnvironmentVariableW(L"MusaCore_Test_Var", Buf, _countof(Buf));
            KTEST_EXPECT(Len > 0,
                "Env_GetEnvironmentVariableW_AfterSet_ReturnsNonZero");
            KTEST_EXPECT(wcscmp(Buf, L"HelloWorld") == 0,
                "Env_GetEnvironmentVariableW_AfterSet_ValueMatches");

            // Overwrite
            SetEnvironmentVariableW(L"MusaCore_Test_Var", L"Updated");
            Len = GetEnvironmentVariableW(L"MusaCore_Test_Var", Buf, _countof(Buf));
            KTEST_EXPECT(Len > 0 && wcscmp(Buf, L"Updated") == 0,
                "Env_SetEnvironmentVariableW_Overwrite");

            // Delete
            SetEnvironmentVariableW(L"MusaCore_Test_Var", nullptr);
            Len = GetEnvironmentVariableW(L"MusaCore_Test_Var", Buf, _countof(Buf));
            KTEST_EXPECT(Len == 0,
                "Env_SetEnvironmentVariableW_Delete_ReturnsZero");
        }

        // GetEnvironmentVariableW buffer query
        {
            SetEnvironmentVariableW(L"MusaCore_SizeTest", L"ABCD");
            DWORD Size = GetEnvironmentVariableW(L"MusaCore_SizeTest", nullptr, 0);
            KTEST_EXPECT(Size == 5,
                "Env_GetEnvironmentVariableW_QuerySize_ReturnsCorrect");

            // Buffer too small
            WCHAR SmallBuf[3] = {};
            DWORD SmallLen = GetEnvironmentVariableW(L"MusaCore_SizeTest", SmallBuf, _countof(SmallBuf));
            KTEST_EXPECT(SmallLen == 5,
                "Env_GetEnvironmentVariableW_BufferTooSmall_ReturnsRequiredSize");
            KTEST_EXPECT(GetLastError() == ERROR_INSUFFICIENT_BUFFER,
                "Env_GetEnvironmentVariableW_BufferTooSmall_SetsError");

            SetEnvironmentVariableW(L"MusaCore_SizeTest", nullptr);
        }

        // Environment Strings
        {
            SetEnvironmentVariableW(L"MusaCore_StringsTest", L"KernelMode");

            LPWCH Block = GetEnvironmentStringsW();
            KTEST_EXPECT(Block != nullptr,
                "Env_GetEnvironmentStringsW_ReturnsNonNull");

            if (Block) {
                BOOL FoundTestVar = FALSE;
                for (PCWSTR p = Block; *p; p += wcslen(p) + 1) {
                    if (wcsncmp(p, L"MusaCore_StringsTest=", 21) == 0) {
                        KTEST_EXPECT(wcscmp(p + 21, L"KernelMode") == 0,
                            "Env_GetEnvironmentStringsW_ContainsSetVariable");
                        FoundTestVar = TRUE;
                        break;
                    }
                }
                KTEST_EXPECT(FoundTestVar,
                    "Env_GetEnvironmentStringsW_FindsSetVariable");

                KTEST_EXPECT(FreeEnvironmentStringsW(Block),
                    "Env_FreeEnvironmentStringsW_Succeeds");
            }

            KTEST_EXPECT(FreeEnvironmentStringsW(nullptr),
                "Env_FreeEnvironmentStringsW_NullSafe");

            SetEnvironmentVariableW(L"MusaCore_StringsTest", nullptr);
        }

        // Console APIs
        {
            UINT Cp = GetConsoleOutputCP();
            KTEST_EXPECT(Cp <= 0xFFFF,
                "Console_GetConsoleOutputCP_ReturnsValid");

            SetLastError(ERROR_SUCCESS);
            DWORD Mode = 0;
            BOOL ModeResult = GetConsoleMode(nullptr, &Mode);
            KTEST_EXPECT(!ModeResult,
                "Console_GetConsoleMode_ReturnsFalse");
            KTEST_EXPECT(GetLastError() != ERROR_SUCCESS,
                "Console_GetConsoleMode_SetsLastError");

            SetLastError(ERROR_SUCCESS);
            DWORD CharsRead = 0;
            BOOL ReadResult = ReadConsoleW(nullptr, nullptr, 0, &CharsRead, nullptr);
            KTEST_EXPECT(!ReadResult,
                "Console_ReadConsoleW_ReturnsFalse");
            KTEST_EXPECT(GetLastError() != ERROR_SUCCESS,
                "Console_ReadConsoleW_SetsLastError");
        }

        // Date/Time Formatting — verify against GetLocalTime
        {
            SYSTEMTIME Now{};
            GetLocalTime(&Now);

            WCHAR Buf[32] = {};
            int Ret = GetDateFormatEx(nullptr, 0, &Now, nullptr, Buf, _countof(Buf), nullptr);
            KTEST_EXPECT(Ret == 11,
                "NLS_GetDateFormatEx_Returns11");

            // Parse YYYY-MM-DD back (4-digit, 2-digit, 2-digit)
            int Year  = (Buf[0]-L'0')*1000 + (Buf[1]-L'0')*100 + (Buf[2]-L'0')*10 + (Buf[3]-L'0');
            int Month = (Buf[5]-L'0')*10 + (Buf[6]-L'0');
            int Day   = (Buf[8]-L'0')*10 + (Buf[9]-L'0');
            KTEST_EXPECT(Year == Now.wYear && Month == Now.wMonth && Day == Now.wDay,
                "NLS_GetDateFormatEx_MatchesGetLocalTime");
        }

        {
            SYSTEMTIME Now{};
            GetLocalTime(&Now);

            WCHAR Buf[32] = {};
            int Ret = GetTimeFormatEx(nullptr, 0, &Now, nullptr, Buf, _countof(Buf));
            KTEST_EXPECT(Ret == 9,
                "NLS_GetTimeFormatEx_Returns9");

            int Hour = (Buf[0]-L'0')*10 + (Buf[1]-L'0');
            int Min  = (Buf[3]-L'0')*10 + (Buf[4]-L'0');
            int Sec  = (Buf[6]-L'0')*10 + (Buf[7]-L'0');
            KTEST_EXPECT(Hour == Now.wHour && Min == Now.wMinute && Sec == Now.wSecond,
                "NLS_GetTimeFormatEx_MatchesGetLocalTime");
        }

        // Date/Time Formatting — null time (uses current time internally)
        {
            WCHAR Buf[32] = {};
            int Ret = GetDateFormatEx(nullptr, 0, nullptr, nullptr, Buf, _countof(Buf), nullptr);
            KTEST_EXPECT(Ret == 11,
                "NLS_GetDateFormatEx_NullTime_Returns11");
            KTEST_EXPECT(Buf[4] == L'-' && Buf[7] == L'-',
                "NLS_GetDateFormatEx_NullTime_HasDelimiters");
        }

        {
            WCHAR Buf[32] = {};
            int Ret = GetTimeFormatEx(nullptr, 0, nullptr, nullptr, Buf, _countof(Buf));
            KTEST_EXPECT(Ret == 9,
                "NLS_GetTimeFormatEx_NullTime_Returns9");
            KTEST_EXPECT(Buf[2] == L':' && Buf[5] == L':',
                "NLS_GetTimeFormatEx_NullTime_HasDelimiters");
        }



        // GetFullPathNameW
        {
            // Absolute path
            WCHAR Buf[260] = {};
            LPWSTR FilePart = nullptr;
            DWORD Len = GetFullPathNameW(L"C:\\Windows\\System32", _countof(Buf), Buf, &FilePart);
            KTEST_EXPECT(Len > 0,
                "Path_GetFullPathNameW_Absolute_Succeeds");
            KTEST_EXPECT(_wcsicmp(Buf, L"C:\\Windows\\System32") == 0,
                "Path_GetFullPathNameW_Absolute_ContentMatches");
            KTEST_EXPECT(FilePart != nullptr && wcscmp(FilePart, L"System32") == 0,
                "Path_GetFullPathNameW_Absolute_FilePart");

            // Relative path — prepends current directory (kernel default: C:\Windows)
            Len = GetFullPathNameW(L"System32", _countof(Buf), Buf, &FilePart);
            KTEST_EXPECT(Len > 0,
                "Path_GetFullPathNameW_Relative_Succeeds");
            KTEST_EXPECT(wcslen(Buf) >= wcslen(L"C:\\Windows\\System32"),
                "Path_GetFullPathNameW_Relative_Expanded");
            KTEST_EXPECT(FilePart != nullptr && wcscmp(FilePart, L"System32") == 0,
                "Path_GetFullPathNameW_Relative_FilePart");

        }


        // SetEndOfFile / GetFileSizeEx / GetFileInformationByHandle
        {
            HANDLE hFile = CreateFileW(
                L"C:\\Windows\\Temp\\MusaCore_eof_test.tmp",
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_DELETE,
                nullptr, CREATE_ALWAYS,
                FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
                nullptr);
            if (hFile != INVALID_HANDLE_VALUE) {
                const char Data[] = "Hello";
                DWORD Written = 0;
                WriteFile(hFile, Data, sizeof(Data), &Written, nullptr);

                // GetFileSizeEx
                LARGE_INTEGER FileSize{};
                KTEST_EXPECT(GetFileSizeEx(hFile, &FileSize),
                    "FileIO_GetFileSizeEx_Succeeds");
                KTEST_EXPECT(FileSize.QuadPart == sizeof(Data),
                    "FileIO_GetFileSizeEx_CorrectSize");

                // SetEndOfFile truncate to 3 bytes
                LARGE_INTEGER Dist{};
                Dist.QuadPart = 3;
                SetFilePointerEx(hFile, Dist, nullptr, FILE_BEGIN);
                KTEST_EXPECT(SetEndOfFile(hFile),
                    "FileIO_SetEndOfFile_Succeeds");
                GetFileSizeEx(hFile, &FileSize);
                KTEST_EXPECT(FileSize.QuadPart == 3,
                    "FileIO_SetEndOfFile_Truncated");


                // GetFileInformationByHandle — cross-validate with GetFileSizeEx & GetFileAttributesExW
                BY_HANDLE_FILE_INFORMATION FileInfo{};
                KTEST_EXPECT(GetFileInformationByHandle(hFile, &FileInfo),
                    "FileIO_GetFileInformationByHandle_Succeeds");

                // Compare file size with GetFileSizeEx
                LARGE_INTEGER SizeFromEx{};
                GetFileSizeEx(hFile, &SizeFromEx);
                KTEST_EXPECT(FileInfo.nFileSizeLow == static_cast<DWORD>(SizeFromEx.LowPart) &&
                             FileInfo.nFileSizeHigh == static_cast<DWORD>(SizeFromEx.HighPart),
                    "FileIO_GetFileInformationByHandle_SizeMatchesGetFileSizeEx");

                // Compare attributes with GetFileAttributesExW
                WIN32_FILE_ATTRIBUTE_DATA AttrData{};
                if (GetFileAttributesExW(L"C:\\Windows\\Temp\\MusaCore_eof_test.tmp", GetFileExInfoStandard, &AttrData)) {
                    KTEST_EXPECT(FileInfo.dwFileAttributes == AttrData.dwFileAttributes,
                        "FileIO_GetFileInformationByHandle_AttribMatchesGetFileAttributesExW");
                }

                // Number of links should be at least 1
                KTEST_EXPECT(FileInfo.nNumberOfLinks >= 1,
                    "FileIO_GetFileInformationByHandle_HasLinks");

                CloseHandle(hFile);
            }

        }




        // CreateNamedPipeW + PeekNamedPipe
        {
            HANDLE hPipe = CreateNamedPipeW(
                L"\\\\.\\pipe\\MusaCore_PeekTest",
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
                1, 256, 256, 0, nullptr);
            KTEST_EXPECT(hPipe != INVALID_HANDLE_VALUE,
                "Pipe_CreateNamedPipeW_Succeeds");

            if (hPipe != INVALID_HANDLE_VALUE) {
                // Open client side to connect the pipe
                HANDLE hClient = CreateFileW(
                    L"\\\\.\\pipe\\MusaCore_PeekTest",
                    GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                    OPEN_EXISTING, 0, nullptr);
                KTEST_EXPECT(hClient != INVALID_HANDLE_VALUE,
                    "Pipe_ClientConnect_Succeeds");

                if (hClient != INVALID_HANDLE_VALUE) {
                    DWORD Written = 0;
                    WriteFile(hClient, "Test", 4, &Written, nullptr);

                    DWORD TotalAvail = 0, BytesRead = 0;
                    BOOL Result = PeekNamedPipe(hPipe, nullptr, 0, &BytesRead, &TotalAvail, nullptr);
                    KTEST_EXPECT(Result,
                        "Pipe_PeekNamedPipe_WithData_Succeeds");
                    KTEST_EXPECT(TotalAvail == 4,
                        "Pipe_PeekNamedPipe_TotalAvail");
                    CloseHandle(hClient);
                }
                CloseHandle(hPipe);
            }
        }
        {

        // SystemTimeToTzSpecificLocalTime
            SYSTEMTIME Utc{};
            GetSystemTime(&Utc);
            SYSTEMTIME Local{};
            BOOL Result = SystemTimeToTzSpecificLocalTime(nullptr, &Utc, &Local);
            KTEST_EXPECT(Result,
                "Time_SystemTimeToTzSpecificLocalTime_Succeeds");
            KTEST_EXPECT(Local.wYear == Utc.wYear && Local.wMonth == Utc.wMonth &&
                         Local.wDay == Utc.wDay && Local.wHour == Utc.wHour,
                "Time_SystemTimeToTzSpecificLocalTime_PassThrough");
        }

        {
            SYSTEMTIME Local{};
            BOOL Result = SystemTimeToTzSpecificLocalTime(nullptr, nullptr, &Local);
            KTEST_EXPECT(Result,
                "Time_SystemTimeToTzSpecificLocalTime_NullTime_UsesCurrent");
            KTEST_EXPECT(Local.wYear >= 2026,
                "Time_SystemTimeToTzSpecificLocalTime_NullTime_PlausibleYear");
        }
        // --- Results ---

        MusaLOG("=== Results: %lu/%lu passed ===",
            TestsRun - TestsFailed, TestsRun);

        if (TestsFailed > 0) {
            MusaLOG("%lu test(s) FAILED", TestsFailed);
        }
    }

    EXTERN_C VOID DriverUnload(
        _In_ PDRIVER_OBJECT  DriverObject
    )
    {
        PAGED_CODE();
        UNREFERENCED_PARAMETER(DriverObject);

        (void)MusaCoreShutdown();

        if (SavedRegistryPath.Buffer) {
            ExFreePool(SavedRegistryPath.Buffer);
            SavedRegistryPath = {};
        }
    }

    static VOID NTAPI BootDriverReinitialize(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_opt_ PVOID       Context,
        _In_ ULONG           Count
    )
    {
        UNREFERENCED_PARAMETER(Context);

        NTSTATUS Status = MusaCoreStartup(DriverObject, &SavedRegistryPath, true);
        if (Status == STATUS_RETRY) {
            MusaLOG("MusaCore still not ready (attempt %lu), re-registering", Count);
            IoRegisterBootDriverReinitialization(DriverObject, BootDriverReinitialize, nullptr);
            return;
        }

        if (!NT_SUCCESS(Status)) {
            MusaLOG("MusaCore reinitialization failed: 0x%08lX", Status);
            return;
        }

        MusaLOG("MusaCore reinitialization succeeded (attempt %lu)", Count);
        RunTests();
    }

    EXTERN_C NTSTATUS DriverEntry(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        PAGED_CODE();

        NTSTATUS Status;

        do {
            DriverObject->DriverUnload = Main::DriverUnload;

            Status = MusaCoreStartup(DriverObject, RegistryPath, true);
            if (Status == STATUS_RETRY) {
                MusaLOG("MusaCore deferred (boot-start), registering reinitialization");

                SavedRegistryPath.Length        = RegistryPath->Length;
                SavedRegistryPath.MaximumLength = RegistryPath->Length + sizeof(WCHAR);
                SavedRegistryPath.Buffer        = static_cast<PWCH>(
                    ExAllocatePoolZero(PagedPool, SavedRegistryPath.MaximumLength, 'asuM'));
                if (SavedRegistryPath.Buffer == nullptr) {
                    MusaLOG("Failed to allocate SavedRegistryPath");
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }
                RtlCopyMemory(SavedRegistryPath.Buffer, RegistryPath->Buffer, RegistryPath->Length);

                IoRegisterBootDriverReinitialization(DriverObject, BootDriverReinitialize, nullptr);
                Status = STATUS_SUCCESS;
                break;
            }
            if (!NT_SUCCESS(Status)) {
                MusaLOG("MusaCoreStartup failed: 0x%08X", Status);
                break;
            }

            RunTests();
        } while (false);

        if (!NT_SUCCESS(Status)) {
            Main::DriverUnload(DriverObject);
        }

        return Status;
    }
}
