# Musa.Core — 项目概述 (Purpose, Design, Roadmap)

## Purpose (项目目的)

Musa.Core 是一个**内核模式 (kernel-mode)** Win32 API 重实现项目，通过在 `ntoskrnl.exe` 之上提供 Kernel32、Advapi32 等 DLL 的等价实现，使内核态驱动程序能够直接调用标准 Win32 API。

### 核心动机

- **消除内核态 API 鸿沟**：Windows 内核驱动通常只能通过 NT 原生 API (`Zw*`/`Nt*`) 交互，大量上层 Win32 语义不可用。Musa.Core 填补了这一空白。
- **统一编程模型**：开发者可使用熟悉的 Win32 函数（如 `CreateFile`、`GetModuleHandle`、`HeapAlloc`、`CreateEvent` 等）编写内核代码，降低认知负担。
- **源自 Musa.Runtime 生态**：本项目衍生自 [Musa.Runtime](https://github.com/MiroKaku/Musa.Runtime)（前身为 ucxxrt），是该底层 API 实现体系的扩展。

### 适用场景

| 场景 | 说明 |
|---|---|
| 内核驱动开发 | 在 KMDF/文件系统驱动/过滤驱动中直接使用 Win32 API |
| 安全/反作弊 | 需要内核态文件操作、进程查询、同步原语 |
| 系统监控 | 进程/线程枚举、性能计数器、实时时钟查询 |
| 中间件扩展 | 在内核层复用已有的 Win32 代码模式 |

### 非适用场景

- **用户态进程**：Musa.Core 是纯内核态库，不可链接到用户态 EXE/DLL
- **替代 ntoskrnl**：本项目不是操作系统替代品，而是 ntoskrnl 的上层封装
- **完整 Win32 兼容层**：仅覆盖常用 API 子集，非全量兼容

### 项目状态

- **许可证**：MIT
- **状态**：Beta（beta 测试阶段）
- **作者**：MiroKaku / MeeSong
- **源码**：[github.com/MiroKaku/Musa.Core](https://github.com/MiroKaku/Musa.Core)
- **NuGet**：[Musa.Core](https://www.nuget.org/packages/Musa.Core/)

---

## Design (设计)

### 架构总览

```
┌─────────────────────────────────────────────────┐
│              内核驱动程序 (Consumer)              │
│    CreateFile / HeapAlloc / CreateEvent / ...    │
└──────────────────────┬──────────────────────────┘
                       │ 链接 Musa.Core.lib
┌──────────────────────▼──────────────────────────┐
│              Musa.Core (Thunks 层)                │
│  ┌─────────────┐  ┌─────────────┐               │
│  │ KernelBase  │  │   Ntdll     │               │
│  │ (高层 Win32) │  │ (低层 Rtl*) │               │
│  └──────┬──────┘  └──────┬──────┘               │
│         └───────┬────────┘                      │
│     Musa::Core::KPEB / KTEB (SEB)               │
└─────────────────┼───────────────────────────────┘
                  │
┌─────────────────▼───────────────────────────────┐
│           Musa.CoreLite (依赖)                   │
│   MusaCoreLiteStartup / GetNtdllBase()          │
└─────────────────┬───────────────────────────────┘
                  │
┌─────────────────▼───────────────────────────────┐
│         Musa.Veil (NT 内部头)                    │
│            Veil.h + Zw* 声明                     │
└─────────────────┬───────────────────────────────┘
                  │
┌─────────────────▼───────────────────────────────┐
│              ntoskrnl.exe                        │
│        真正的内核系统调用                        │
└─────────────────────────────────────────────────┘
```

### 关键设计决策

1. **双 DLL 命名空间**：Thunk 实现按 `KernelBase`（高层 Win32 包装器）和 `Ntdll`（低层 `Rtl*` 函数）分属两个命名空间，保持与 Windows 原生的语义分层一致。
2. **SEB（系统环境块）**：内核态 PEB/TEB 等价物，通过 `KPEB` 和 `KTEB` 结构体实现，提供进程/线程级别的运行时环境。
3. **静态库分发**：通过 `Musa.Core.StaticLibraryForDriver` 项目编译为静态库，消费方直接链接 `.lib` 文件。
4. **Module Definition 导出**：~490 个 `Zw*` 例程通过 `.def` 文件以 `ZwXxx=_Mi_ZwXxx` 格式导出，使驱动可直接调用系统原生 API。

### 详细设计文档

详见 [系统架构](system-architecture.md) 和 [代码库摘要](codebase-summary.md)。

---

## Roadmap (路线图)

### 当前已实现 (Beta)

| 类别 | 状态 | 关键 API 示例 |
|---|---|---|
| ZwRoutines | ✅ 全量 | `ZwCreateFile`, `ZwQuerySystemInformation`, `ZwWaitForSingleObject`, ~490 个导出 |
| Rtl* (Ntdll) | ✅ 部分 | `RtlGetCurrentPeb`, `RtlAcquirePebLock*`, `RtlTimeToTimeFields` |
| Heap (堆) | ✅ | `GetProcessHeap`, `HeapAlloc`, `HeapCreate`, `HeapDestroy`, `LocalAlloc`, `GlobalAlloc` |
| Synchronization (同步) | ✅ | `CreateEvent`, `CreateMutex`, `CreateSemaphore`, `InitializeCriticalSection`, `SRWLock`, `ConditionVariable` |
| Process (进程) | ✅ 部分 | `GetCurrentProcess`, `GetProcessId`, `GetProcessTimes`, `ProcessIdToSessionId` |
| Thread (线程) | ✅ 部分 | `GetCurrentThread`, `GetThreadId`, `CreateThread`, `GetThreadTimes` |
| Fiber Local Storage (FLS) | ✅ | `FlsAlloc`, `FlsSetValue`, `FlsGetValue`, `FlsFree` |
| Thread Local Storage (TLS) | ✅ | `TlsAlloc`, `TlsSetValue`, `TlsGetValue`, `TlsFree` |
| File (文件) | ✅ 部分 | `CreateFile`, `ReadFile`, `WriteFile`, `GetFileType` |
| NLS (本地化) | ✅ 部分 | `WideCharToMultiByte`, `MultiByteToWideChar`, `GetACP`, `GetCPInfoExW` |
| RealTime (时间) | ✅ | `GetSystemTime`, `GetLocalTime`, `GetTickCount64`, `QueryPerformanceCounter` |
| LibraryLoader (模块加载) | ✅ 部分 | `GetModuleHandle`, `GetProcAddress`, `LoadLibrary` |
| Handle (句柄) | ✅ | `DuplicateHandle`, `GetHandleInformation`, `CloseHandle` |
| FileVersion (版本信息) | ✅ | `GetFileVersionInfoSize`, `GetFileVersionInfo` |
| PerformanceCounter (性能计数) | ✅ | `QueryPerformanceFrequency`, `QueryPerformanceCounter` |
| Pool (池内存) | ✅ | `ExAllocatePool2`, `ExFreePool` 封装 |
| Utility (工具) | ✅ | `EncodePointer`, `DecodePointer`, `Fnv1aHash` |

### 近期计划

| 特性 | 优先级 | 说明 |
|---|---|---|
| Advapi32 API 支持 | 高 | 注册表操作、服务控制管理器、安全描述符相关 API |
| Process 完整实现 | 中 | `OpenProcess`, `GetProcessInformation`, 进程环境块查询 |
| Thread 完整实现 | 中 | `SetThreadPriority`, `SuspendThread`, `ResumeThread` 完整语义 |
| File 完整实现 | 中 | `SetFileInformationByHandle`, 重叠 I/O 支持 |
| 路径处理 | 低 | `GetFullPathName`, `PathCanonicalize` 等路径规范化 API |

### 长期规划

| 特性 | 说明 |
|---|---|
| ARM64EC 支持 | 扩展 ARM64EC 架构的 thunk 实现 |
| 更多 Advapi32 | 加密 API、事件日志、访问控制列表 |
| 性能优化 | 减少 thunk 层开销，优化 SEB 锁竞争 |
| 测试覆盖扩展 | 增加内核态自动化测试场景 |

### 里程碑

- **v0.x (Beta)**：核心子系统可用，ZwRoutines 全量导出，常用 Win32 API 覆盖
- **v1.0 (Stable)**：API 覆盖度达到实用级别，测试覆盖核心场景
- **v2.0+**：Advapi32 完善，更多内核态场景适配
