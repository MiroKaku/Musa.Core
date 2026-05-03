# API Reference — Musa.Core

本文档提供 Musa.Core 内核模式驱动的公开 API、系统环境块接口、ZwRoutine 导出与 Win32 thunk 分类目录。

相关文档：
- [系统架构](system-architecture.md)
- [代码规范](code-standards.md)
- [测试指南](testing-guide.md)

## 1. 公共驱动 API

Musa.Core 提供两个公共入口点，定义于 `Musa.Core.h`：

### 1.1 MusaCoreStartup

```cpp
NTSTATUS MUSA_API MusaCoreStartup(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath,
    _In_ BOOL            TLSWithThreadNotifyCallback
);
```

| 参数 | 说明 |
|---|---|
| `DriverObject` | 驱动对象指针 |
| `RegistryPath` | 驱动注册表路径 |
| `TLSWithThreadNotifyCallback` | 是否启用线程通知回调以支持 TLS/FLS |

**返回值：** `STATUS_SUCCESS` 或失败 NTSTATUS

**初始化序列：**
1. 调用 `MusaCoreLiteStartup()`（依赖 Musa.CoreLite v1.1.1）
2. 调用 `EnvironmentBlockSetup()` 创建内核态 PEB/TEB 伪造环境
3. 通过 `MusaLOG` 记录结果

### 1.2 MusaCoreShutdown

```cpp
NTSTATUS MUSA_API MusaCoreShutdown();
```

**拆卸序列：** `EnvironmentBlockTeardown()` → `MusaCoreLiteShutdown()`

## 2. 系统环境块 API

内核态 PEB/TEB 操作接口，定义于 `Musa.Core.SystemEnvironmentBlock.*.h`。

### 2.1 进程环境

| 函数 | 说明 |
|---|---|
| `RtlGetCurrentPeb()` | 获取当前进程的伪造 PEB 指针 |
| `RtlAcquirePebLockExclusive()` | 获取 PEB 排他锁 |
| `RtlAcquirePebLockShared()` | 获取 PEB 共享锁 |
| `RtlReleasePebLockExclusive()` | 释放 PEB 排他锁 |
| `RtlReleasePebLockShared()` | 释放 PEB 共享锁 |

### 2.2 线程环境

| 函数 | 说明 |
|---|---|
| `RtlGetCurrentTeb()` | 获取当前线程的 TEB 指针（延迟创建） |
| `RtlAcquireTebLockExclusive()` | 获取 TEB 排他锁 |
| `RtlAcquireTebLockShared()` | 获取 TEB 共享锁 |
| `RtlReleaseTebLockExclusive()` | 释放 TEB 排他锁 |
| `RtlReleaseTebLockShared()` | 释放 TEB 共享锁 |

**注意：** TEB 通过 AVL 表 + Lookaside 池管理，按需懒加载创建。

### 2.3 FLS（Fiber Local Storage）

| 函数 | 说明 |
|---|---|
| `FlsCreate()` | 初始化 FLS 子系统 |
| `FlsCleanup()` | 清理当前线程 FLS 数据 |
| `FlsDataCleanup()` | 清理进程级 FLS 数据 |

## 3. ZwRoutine 导出

`Musa.Core.def` 定义了约 490 个 `_Mi_Zw*` 前缀导出，通过 `MakeImageOverlay` 在初始化时重定向导入表。

### 3.1 内存管理

| 导出 | 说明 |
|---|---|
| `ZwAllocateVirtualMemory` | 分配虚拟内存 |
| `ZwFreeVirtualMemory` | 释放虚拟内存 |
| `ZwProtectVirtualMemory` | 修改内存保护 |
| `ZwQueryVirtualMemory` | 查询内存信息 |
| `ZwMapViewOfSection` | 映射段视图 |
| `ZwUnmapViewOfSection` | 解除段映射 |

### 3.2 进程管理

| 导出 | 说明 |
|---|---|
| `ZwCreateProcess` / `ZwCreateProcessEx` | 创建进程 |
| `ZwOpenProcess` | 打开进程句柄 |
| `ZwTerminateProcess` | 终止进程 |
| `ZwQueryInformationProcess` | 查询进程信息 |
| `ZwSetInformationProcess` | 设置进程信息 |

### 3.3 线程管理

| 导出 | 说明 |
|---|---|
| `ZwCreateThread` / `ZwCreateThreadEx` | 创建线程 |
| `ZwOpenThread` | 打开线程句柄 |
| `ZwTerminateThread` | 终止线程 |
| `ZwQueryInformationThread` | 查询线程信息 |
| `ZwSetInformationThread` | 设置线程信息 |
| `ZwSuspendThread` | 挂起线程 |
| `ZwResumeThread` | 恢复线程 |
| `ZwGetContextThread` | 获取线程上下文 |
| `ZwSetContextThread` | 设置线程上下文 |

### 3.4 文件 I/O

| 导出 | 说明 |
|---|---|
| `ZwCreateFile` | 创建/打开文件 |
| `ZwOpenFile` | 打开文件 |
| `ZwReadFile` | 读取文件 |
| `ZwWriteFile` | 写入文件 |
| `ZwFlushBuffersFile` | 刷新文件缓冲区 |
| `ZwDeleteFile` | 删除文件 |
| `ZwQueryInformationFile` | 查询文件信息 |
| `ZwSetInformationFile` | 设置文件信息 |

### 3.5 同步对象

| 导出 | 说明 |
|---|---|
| `ZwCreateEvent` / `ZwOpenEvent` | 事件对象 |
| `ZwCreateSemaphore` / `ZwOpenSemaphore` | 信号量 |
| `ZwCreateMutant` / `ZwOpenMutant` | 互斥体 |
| `ZwCreateTimer` / `ZwOpenTimer` | 定时器 |
| `ZwSetEvent` | 设置事件 |
| `ZwResetEvent` | 重置事件 |
| `ZwPulseEvent` | 脉冲事件 |
| `ZwReleaseSemaphore` | 释放信号量 |
| `ZwReleaseMutant` | 释放互斥体 |
| `ZwWaitForSingleObject` | 单对象等待 |
| `ZwWaitForMultipleObjects` | 多对象等待 |

### 3.6 注册表

| 导出 | 说明 |
|---|---|
| `ZwCreateKey` | 创建注册表项 |
| `ZwOpenKey` | 打开注册表项 |
| `ZwDeleteKey` | 删除注册表项 |
| `ZwQueryValueKey` | 查询注册表值 |
| `ZwSetValueKey` | 设置注册表值 |
| `ZwEnumerateKey` | 枚举注册表子项 |
| `ZwEnumerateValueKey` | 枚举注册表值 |

### 3.7 Token

| 导出 | 说明 |
|---|---|
| `ZwOpenProcessToken` | 打开进程 Token |
| `ZwOpenThreadToken` | 打开线程 Token |
| `ZwDuplicateToken` | 复制 Token |
| `ZwQueryInformationToken` | 查询 Token 信息 |
| `ZwSetInformationToken` | 设置 Token 信息 |
| `ZwAdjustPrivilegesToken` | 调整 Token 权限 |

### 3.8 ALPC

| 导出 | 说明 |
|---|---|
| `ZwAcceptConnectPort` | 接受 ALPC 连接 |
| `ZwConnectPort` | 发起 ALPC 连接 |
| `ZwSendWaitReceivePort` | ALPC 发送/接收 |

### 3.9 段与定时器

| 导出 | 说明 |
|---|---|
| `ZwCreateSection` | 创建段对象 |
| `ZwOpenSection` | 打开段对象 |
| `ZwExtendSection` | 扩展段 |
| `ZwSetTimer` | 设置定时器 |
| `ZwCancelTimer` | 取消定时器 |
| `ZwQueryTimer` | 查询定时器 |

### 3.10 其他

| 类别 | 说明 |
|---|---|
| 系统信息 | `ZwQuerySystemInformation` 等 |
| 电源管理 | `ZwPowerInformation` 等 |
| 驱动加载 | 驱动对象操作 |
| Debug 对象 | 调试对象管理 |
| Job 对象 | 作业对象管理 |
| KeyedEvent | 内核 KeyedEvent |
| WNF | Windows Notification Facility |

## 4. Win32 Thunk — KernelBase

按功能类别组织的 KernelBase 层 Win32 API thunk 实现。

### 4.1 进程（Process）

| 函数 | 文件 | 说明 |
|---|---|---|
| `OpenProcess` | `KernelBase.Process.cpp` | 打开进程 |
| `TerminateProcess` | `KernelBase.Process.cpp` | 终止进程 |
| `GetProcessTimes` | `KernelBase.Process.cpp` | 获取进程时间 |
| `GetCurrentProcessId` | `KernelBase.Process.cpp` | 当前进程 ID |
| `GetProcessId` | `KernelBase.Process.cpp` | 获取进程 ID |
| `GetProcessHandleCount` | `KernelBase.Process.cpp` | 句柄计数 |
| `GetProcessInformation` | `KernelBase.Process.cpp` | 进程信息（`ProcessAppMemoryInfo` 返回 `STATUS_NOT_IMPLEMENTED`） |

### 4.2 线程（Thread）

| 函数 | 文件 | 说明 |
|---|---|---|
| `CreateThread` | `KernelBase.Thread.Create.cpp` | 创建线程 |
| `ExitThread` | `KernelBase.Thread.Create.cpp` | 退出线程 |
| `OpenThread` | `KernelBase.Thread.Create.cpp` | 打开线程 |
| `GetCurrentThreadId` | `KernelBase.Thread.cpp` | 当前线程 ID |
| `GetThreadId` | `KernelBase.Thread.cpp` | 获取线程 ID |
| `SetThreadPriority` | `KernelBase.Thread.ProcAttr.cpp` | 设置优先级 |
| `GetThreadPriority` | `KernelBase.Thread.ProcAttr.cpp` | 获取优先级 |
| `GetThreadTimes` | `KernelBase.Thread.cpp` | 获取线程时间 |
| `GetThreadContext` | `KernelBase.Thread.cpp` | 获取线程上下文 |

### 4.3 堆（Heap）

| 函数 | 文件 | 说明 |
|---|---|---|
| `HeapAlloc` | `KernelBase.Heap.cpp` | 堆分配 |
| `HeapFree` | `KernelBase.Heap.cpp` | 堆释放 |
| `HeapReAlloc` | `KernelBase.Heap.cpp` | 堆重新分配 |
| `HeapCreate` | `KernelBase.Heap.cpp` | 创建堆 |
| `HeapDestroy` | `KernelBase.Heap.cpp` | 销毁堆 |
| `HeapValidate` | `KernelBase.Heap.cpp` | 验证堆 |
| `LocalAlloc` / `LocalFree` / `LocalReAlloc` | `KernelBase.Heap.cpp` | Local 堆（`LMEM_MOVEABLE` 不支持） |
| `GlobalAlloc` / `GlobalFree` / `GlobalReAlloc` | `KernelBase.Heap.cpp` | Global 堆 |

### 4.4 文件 I/O（File）

| 函数 | 文件 | 说明 |
|---|---|---|
| `CreateFileW` | `KernelBase.File.cpp` | 创建/打开文件 |
| `ReadFile` | `KernelBase.File.cpp` | 读取 |
| `WriteFile` | `KernelBase.File.cpp` | 写入 |
| `GetFileType` | `KernelBase.File.cpp` | 获取文件类型 |
| `SetFilePointer` | `KernelBase.File.cpp` | 设置文件指针 |
| `FlushFileBuffers` | `KernelBase.File.cpp` | 刷新缓冲区 |
| `DeleteFileW` | `KernelBase.File.cpp` | 删除文件 |
| `GetFileAttributesExW` | `KernelBase.File.cpp` | 获取文件属性 |

### 4.5 目录（Directory）

| 函数 | 文件 | 说明 |
|---|---|---|
| `CreateDirectoryW` | `KernelBase.File.cpp` | 创建目录 |
| `RemoveDirectoryW` | `KernelBase.File.cpp` | 删除目录 |
| `MoveFileW` | `KernelBase.File.cpp` | 移动文件/目录 |
| `GetDriveTypeW` | `KernelBase.File.cpp` | 获取驱动器类型 |
| `FindFirstFileW` / `FindNextFileW` / `FindClose` | `KernelBase.File.cpp` | 目录枚举 |

### 4.6 同步（Synchronization）

| 函数 | 文件 | 说明 |
|---|---|---|
| `InitializeCriticalSection` | `KernelBase.Synchronize.cpp` | 初始化临界区 |
| `EnterCriticalSection` | `KernelBase.Synchronize.cpp` | 进入临界区 |
| `LeaveCriticalSection` | `KernelBase.Synchronize.cpp` | 离开临界区 |
| `DeleteCriticalSection` | `KernelBase.Synchronize.cpp` | 删除临界区 |
| `CreateEventW` | `Synchronize.Event.cpp` | 创建事件 |
| `SetEvent` / `ResetEvent` / `PulseEvent` | `Synchronize.Event.cpp` | 事件操作 |
| `CreateMutexW` | `Synchronize.Mutex.cpp` | 创建互斥体 |
| `ReleaseMutex` | `Synchronize.Mutex.cpp` | 释放互斥体 |
| `CreateSemaphoreW` | `Synchronize.Semaphore.cpp` | 创建信号量 |
| `ReleaseSemaphore` | `Synchronize.Semaphore.cpp` | 释放信号量 |
| `WaitForSingleObject` | `KernelBase.Synchronize.cpp` | 单对象等待 |
| `WaitForMultipleObjects` | `KernelBase.Synchronize.cpp` | 多对象等待 |

### 4.7 模块加载（LibraryLoader）

| 函数 | 文件 | 说明 |
|---|---|---|
| `LoadLibraryExA` | `KernelBase.LibraryLoader.cpp` | 加载库 |
| `FreeLibrary` | `KernelBase.LibraryLoader.cpp` | 释放库 |
| `GetModuleHandleW` | `KernelBase.LibraryLoader.cpp` | 获取模块句柄 |
| `GetProcAddress` | `KernelBase.LibraryLoader.cpp` | 获取函数地址 |

### 4.8 NLS（National Language Support）

| 函数 | 文件 | 说明 |
|---|---|---|
| `WideCharToMultiByte` | `KernelBase.NLS.cpp` | 宽字符转多字节 |
| `MultiByteToWideChar` | `KernelBase.NLS.cpp` | 多字节转宽字符 |
| `GetACP` | `KernelBase.NLS.cpp` | 获取 ANSI 代码页 |
| `GetCPInfoExW` | `KernelBase.NLS.cpp` | 获取代码页信息 |

### 4.9 错误处理

| 函数 | 文件 | 说明 |
|---|---|---|
| `SetLastError` | `KernelBase.Error.cpp` | 设置 last-error |
| `GetLastError` | `KernelBase.Error.cpp` | 获取 last-error |

### 4.10 其他类别

| 类别 | 文件 | 关键函数 |
|---|---|---|
| Debug | `KernelBase.Debug.cpp` | `OutputDebugStringW`, `FormatMessageW` |
| Handle | `KernelBase.Handle.cpp` | `GetHandleInformation` |
| RealTime | `KernelBase.RealTime.cpp` | `GetTickCount64`, `QueryPerformanceCounter`, `GetSystemTimePreciseAsFileTime` |
| System | `KernelBase.System.cpp` | `GetSystemInfo`, `GetSystemTime`, `GetLocalTime` |
| Path | `KernelBase.File.cpp` | `GetTempPathW`, `RtlDosPathNameToNtPathName_U` |
| FLS | `KernelBase.FiberLocalStorage.cpp` | `FlsAlloc`, `FlsFree`, `FlsGetValue`, `FlsSetValue` |
| TLS | `KernelBase.ThreadLocalStorage.cpp` | `TlsAlloc`, `TlsFree`, `TlsGetValue`, `TlsSetValue` |
| Utility | `KernelBase.Utility.cpp` | `EncodePointer`, `DecodePointer` |
| FileVersion | `KernelBase.FileVersion.cpp` | 版本资源 API |
| PerformanceCounter | `KernelBase.PerformanceCounter.cpp` | 性能计数器 |

## 5. Win32 Thunk — Ntdll

Ntdll 层 Rtl* 函数实现，提供更底层的基础设施。

| 类别 | 文件 | 关键函数 |
|---|---|---|
| 堆（Core） | `Ntdll.Heap.Core.cpp` | `RtlAllocateHeap`, `RtlFreeHeap`, `RtlCreateHeap`, `RtlDestroyHeap`, `RtlWalkHeap`（stub） |
| 堆（Lock） | `Ntdll.Heap.Lock.cpp` | 堆锁操作 |
| 同步（CriticalSection） | `Ntdll.Synchronize.CriticalSection.cpp` | `RtlInitializeCriticalSection`, `RtlDeleteCriticalSection`, `RtlEnterCriticalSection`, `RtlLeaveCriticalSection` |
| 同步（SRWLock） | `Ntdll.Synchronize.SRWLock.cpp` | `RtlAcquireSRWLockExclusive/Shared`, `RtlReleaseSRWLockExclusive/Shared` |
| 同步（ConditionVariable） | `Ntdll.Synchronize.ConditionVariable.cpp` | `RtlInitializeConditionVariable`, `RtlWakeConditionVariable`, `RtlSleepConditionVariableCS/SRW` |
| 线程 | `Ntdll.Thread.cpp` | 线程操作 |
| 系统 | `Ntdll.System.cpp` | 系统查询 |
| RealTime | `Ntdll.RealTime.cpp` | 高精度时间 |
| PerformanceCounter | `Ntdll.PerformanceCounter.cpp` | 性能计数器 |
| 池 | `Ntdll.Pool.cpp` | 池内存管理 |
| Image | `Ntdll.Image.cpp` | PE 镜像加载 |
| LibraryLoader | `Ntdll.LibraryLoader.cpp` | 模块加载 |
| LibraryLoader（ProcAddr） | `Ntdll.LibraryLoader.ProcAddr.cpp` | 函数地址解析 |
| Path | `Ntdll.Path.cpp` | 路径操作 |
| Error | `Ntdll.Error.cpp` | 错误处理 |
| FiberLocalStorage | `Ntdll.FiberLocalStorage.cpp` | FLS |
| Debug | `Ntdll.Debug.cpp` | 调试输出 |
| Utility | `Ntdll.Utility.cpp` | 工具函数 |
