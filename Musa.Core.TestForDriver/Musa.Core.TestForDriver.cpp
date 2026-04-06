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
