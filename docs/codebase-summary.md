# Musa.Core — 代码库摘要

## 项目结构

```
Musa.Core/
├── .github/workflows/
│   ├── CI.yaml                 # GitHub Actions CI/CD 配置
│   └── EnableX86AndARM.ps1     # 架构启用脚本
├── Musa.Core/                  # 核心源码
│   ├── Musa.Core.h             # 公共头文件（仅暴露 2 个 API）
│   ├── Musa.Core.cpp           # 入口/出口实现 (MusaCoreStartup/Shutdown)
│   ├── Musa.Core.def           # 模块定义文件 (~490 Zw* 导出)
│   ├── Musa.Core.Nothing.cpp   # PCH 锚点文件（空，触发 universal.h）
│   │
│   ├── System Environment Block (SEB)
│   │   ├── Musa.Core.SystemEnvironmentBlock.h
│   │   ├── Musa.Core.SystemEnvironmentBlock.cpp
│   │   ├── Musa.Core.SystemEnvironmentBlock.Process.h/.cpp
│   │   ├── Musa.Core.SystemEnvironmentBlock.Thread.h/.cpp
│   │   └── Musa.Core.SystemEnvironmentBlock.Fls.h/.cpp
│   │
│   ├── 工具库
│   │   ├── Musa.Utilities.h            # 工具函数声明 (Encode/Decode, Fnv1aHash, PEParser)
│   │   ├── Musa.Utilities.Overlay.cpp  # 工具函数实现
│   │   └── Musa.Utilities.PEParser.h   # PE 解析器声明
│   │
│   └── Thunks/                 # Win32 API Shim 实现
│       ├── KernelBase.*.cpp    # KernelBase 命名空间 (~15 文件)
│       ├── Ntdll.*.cpp         # Ntdll 命名空间 (~14 文件)
│       └── Internal/           # 私有头文件 (~17 .h/.cpp)
│
├── Musa.Core.StaticLibraryForDriver/
│   ├── universal.h             # 强制 PCH 头（包含 Veil.h、配置宏、全局宏）
│   └── *.vcxproj               # 内核静态库项目
│
├── Musa.Core.TestForDriver/    # 内核态测试
│   ├── Musa.Core.TestForDriver.cpp   # DriverEntry + KTEST_EXPECT 测试套件
│   ├── Musa.Core.TestForDriver.inf   # INF 安装文件
│   └── *.vcxproj
│
├── Musa.Core.NuGet/            # NuGet 打包
│   ├── Musa.Core.nuspec        # 包元数据
│   ├── Musa.Core.props         # MSBuild 属性注入
│   ├── Musa.Core.Config.props  # 配置属性
│   └── Musa.Core.Config.targets
│
├── Publish/                    # 构建输出（供消费者和 NuGet 使用）
│   ├── Include/                # 发布的头文件
│   ├── Library/{Config}/{Platform}/   # .lib 文件
│   └── Config/                 # .props/.targets
│
├── Output/                     # 构建产物 (Binaries/, Objects/, .binlog)
├── BuildAllTargets.cmd         # 构建入口脚本
├── BuildAllTargets.proj        # MSBuild 项目文件
├── Directory.Build.props       # MSBuild 全局属性
├── Directory.Build.targets
├── Directory.Packages.Cpp.props   # 集中包管理 (Musa.CoreLite)
├── Musa.Core.slnx              # VS2022 17.10+ XML 解决方案
├── global.json                 # .NET SDK 版本锁定
└── exclusion.txt               # 导出排除列表
```

## 文件清单

### 核心源文件 (Musa.Core/)

| 文件 | 行数 | 说明 |
|---|---|---|
| `Musa.Core.h` | 32 | 公共 API 头：`MusaCoreStartup` / `MusaCoreShutdown` |
| `Musa.Core.cpp` | 77 | 入口/出口：初始化 SEB、Lite 子系统 |
| `Musa.Core.def` | 491 | 模块定义：~490 Zw* 导出映射到 `_Mi_Zw*` |
| `Musa.Core.Nothing.cpp` | — | PCH 锚点（空文件，强制包含 `universal.h`） |

### SEB 系统环境块 (Musa.Core/)

| 文件 | 说明 |
|---|---|
| `Musa.Core.SystemEnvironmentBlock.h` | SEB 总入口：`EnvironmentBlockSetup/Teardown` |
| `Musa.Core.SystemEnvironmentBlock.cpp` | SEB 初始化/清理总控 |
| `Musa.Core.SystemEnvironmentBlock.Process.h` | KPEB 结构体定义 + PEB 锁操作声明 |
| `Musa.Core.SystemEnvironmentBlock.Process.cpp` | KPEB 初始化、堆管理、PEB 锁实现 |
| `Musa.Core.SystemEnvironmentBlock.Thread.h` | KTEB 结构体定义 + TEB 锁操作声明 |
| `Musa.Core.SystemEnvironmentBlock.Thread.cpp` | KTEB 初始化、TEB 锁实现 |
| `Musa.Core.SystemEnvironmentBlock.Fls.h` | FLS 上下文结构体 (`RTL_FLS_DATA`, `RTL_FLS_CONTEXT`) |
| `Musa.Core.SystemEnvironmentBlock.Fls.cpp` | FLS 创建/清理/回调实现 |

### 工具库 (Musa.Core/)

| 文件 | 说明 |
|---|---|
| `Musa.Utilities.h` | `FastEncodePointer` / `FastDecodePointer` / `Fnv1aHash` / `GetLoadedModuleBase` / `GetKnownDllSectionHandle` / `RemapSectionView` / `RunTaskOnLowIrql` |
| `Musa.Utilities.Overlay.cpp` | 工具函数实现 |
| `Musa.Utilities.PEParser.h` | `ImageEnumerateExports` / `ImageRvaToSection` |

### Thunks — KernelBase (Musa.Core/Thunks/)

| 文件 | 实现类别 | 关键 API |
|---|---|---|
| `KernelBase.Debug.cpp` | Debug | `OutputDebugString` |
| `KernelBase.Error.cpp` | Error | `SetLastError`, `GetLastError`, `SetErrorMode` |
| `KernelBase.FiberLocalStorage.cpp` | FLS | `FlsAlloc`, `FlsSetValue`, `FlsGetValue`, `FlsFree` |
| `KernelBase.File.cpp` | File | `CreateFile`, `ReadFile`, `WriteFile`, `GetFileType` |
| `KernelBase.FileVersion.cpp` | FileVersion | `GetFileVersionInfoSize`, `GetFileVersionInfo` |
| `KernelBase.Handle.cpp` | Handle | `DuplicateHandle`, `GetHandleInformation`, `CloseHandle` |
| `KernelBase.Heap.cpp` | Heap | `GetProcessHeap`, `HeapAlloc`, `HeapCreate`, `HeapDestroy`, `LocalAlloc`, `GlobalAlloc` |
| `KernelBase.LibraryLoader.cpp` | LibraryLoader | `GetModuleHandle`, `GetProcAddress`, `LoadLibrary` |
| `KernelBase.NLS.cpp` | NLS | `WideCharToMultiByte`, `MultiByteToWideChar`, `GetACP` |
| `KernelBase.Path.cpp` | — | (Ntdll 命名空间) |
| `KernelBase.PerformanceCounter.cpp` | PerfCounter | `QueryPerformanceFrequency`, `QueryPerformanceCounter` |
| `KernelBase.Private.cpp/h` | Private | 内部辅助函数 |
| `KernelBase.Process.cpp` | Process | `GetCurrentProcess`, `GetProcessId`, `GetProcessTimes` |
| `KernelBase.Process.Information.cpp` | Process.Info | `GetProcessHandleCount`, `ProcessIdToSessionId` |
| `KernelBase.RealTime.cpp` | RealTime | `GetSystemTime`, `GetLocalTime`, `GetTickCount64` |
| `KernelBase.Synchronize.cpp` | Sync | `InitializeCriticalSection`, `EnterCriticalSection`, `LeaveCriticalSection` |
| `KernelBase.Synchronize.Event.cpp` | Sync.Event | `CreateEvent`, `SetEvent`, `ResetEvent`, `WaitForSingleObject` |
| `KernelBase.Synchronize.Mutex.cpp` | Sync.Mutex | `CreateMutex`, `ReleaseMutex` |
| `KernelBase.Synchronize.Semaphore.cpp` | Sync.Semaphore | `CreateSemaphore`, `ReleaseSemaphore` |
| `KernelBase.System.cpp` | System | 系统查询框架 |
| `KernelBase.System.Processor.cpp` | System.Processor | `GetSystemInfo`, `GetNativeSystemInfo` |
| `KernelBase.System.Times.cpp` | System.Times | 系统时间查询 |
| `KernelBase.Thread.cpp` | Thread | `GetCurrentThread`, `GetThreadId`, `GetThreadPriority` |
| `KernelBase.Thread.Create.cpp` | Thread.Create | `CreateThread` |
| `KernelBase.Thread.ProcAttr.cpp` | Thread.ProcAttr | 进程属性相关 |
| `KernelBase.ThreadLocalStorage.cpp` | TLS | `TlsAlloc`, `TlsSetValue`, `TlsGetValue`, `TlsFree` |
| `KernelBase.Utility.cpp` | Utility | `EncodePointer`, `DecodePointer` |

### Thunks — Ntdll (Musa.Core/Thunks/)

| 文件 | 实现类别 | 关键 API |
|---|---|---|
| `Ntdll.Debug.cpp` | Debug | `DbgPrint` 封装 |
| `Ntdll.Error.cpp` | Error | `BaseSetLastNTError` |
| `Ntdll.FiberLocalStorage.cpp` | FLS | 内部 FLS 辅助 |
| `Ntdll.Heap.cpp` | Heap | 通用堆封装 |
| `Ntdll.Heap.Core.cpp` | Heap.Core | 堆核心实现 |
| `Ntdll.Heap.Lock.cpp` | Heap.Lock | 堆锁实现 |
| `Ntdll.Image.cpp` | Image | `LdrGetDllHandle`, `LdrGetProcedureAddress` |
| `Ntdll.LibraryLoader.cpp` | LibraryLoader | 模块加载底层 |
| `Ntdll.LibraryLoader.ProcAddr.cpp` | LibraryLoader.ProcAddr | 地址解析 |
| `Ntdll.Path.cpp` | Path | 路径处理 |
| `Ntdll.PerformanceCounter.cpp` | PerfCounter | 性能计数底层 |
| `Ntdll.Pool.cpp` | Pool | `ExAllocatePool2`, `ExFreePool` |
| `Ntdll.RealTime.cpp` | RealTime | `ZwQuerySystemTime` 封装 |
| `Ntdll.Synchronize.CriticalSection.cpp` | Sync.CS | 临界区底层实现 |
| `Ntdll.Synchronize.ConditionVariable.cpp` | Sync.CV | 条件变量 |
| `Ntdll.Synchronize.SRWLock.cpp` | Sync.SRW | SRW 锁底层 |
| `Ntdll.System.cpp` | System | 系统信息查询 |
| `Ntdll.Thread.cpp` | Thread | 线程底层操作 |
| `Ntdll.Utility.cpp` | Utility | 编码/解码底层 |

### Thunks — Internal 私有头 (Musa.Core/Thunks/Internal/)

| 文件 | 说明 |
|---|---|
| `KernelBase.File.h` | 文件操作内部声明 |
| `KernelBase.FileVersion.h` | 版本信息内部声明 |
| `KernelBase.Handle.h` | 句柄操作内部声明 |
| `KernelBase.LibraryLoader.h` | 模块加载内部声明 |
| `KernelBase.NLS.h` / `KernelBase.NLS.Table.cpp` | NLS 内部声明 + 码表数据 |
| `KernelBase.Process.h` | 进程操作内部声明 |
| `KernelBase.Synchronize.h` | 同步原语内部声明 |
| `KernelBase.System.h` | 系统查询内部声明 |
| `KernelBase.Thread.h` | 线程操作内部声明 |
| `KernelBase.Utility.h` | 工具函数内部声明 |
| `Ntdll.FiberLocalStorage.h` | FLS 内部声明 |
| `Ntdll.Heap.h` | 堆内部声明 |
| `Ntdll.Image.h` | 镜像操作内部声明 |
| `Ntdll.LibraryLoader.h` | 模块加载内部声明 |
| `Ntdll.Path.h` | 路径处理内部声明 |
| `Ntdll.Synchronize.h` | 同步内部声明 |
| `Ntdll.Thread.h` | 线程内部声明 |
| `Ntdll.Utility.h` | 工具内部声明 |

### 构建与打包

| 文件 | 说明 |
|---|---|
| `Musa.Core.StaticLibraryForDriver/universal.h` | 强制 PCH 头：配置宏、Veil.h、全局变量和宏定义 |
| `Musa.Core.StaticLibraryForDriver/*.vcxproj` | 内核静态库 MSBuild 项目 |
| `Musa.Core.TestForDriver/*.cpp/.inf/.vcxproj` | KMDF 测试驱动 |
| `Musa.Core.NuGet/Musa.Core.nuspec` | NuGet 包规范 |
| `Musa.Core.NuGet/Musa.Core.props` | MSBuild 属性（链接器 /INTEGRITYCHECK 强制、头/库路径） |
| `Musa.Core.NuGet/Musa.Core.Config.props/targets` | 配置属性/目标注入 |
| `Directory.Build.props` | 全局 MSBuild 属性（Sources/Publish/Output 路径、SDK 导入） |
| `Directory.Packages.Cpp.props` | 集中包管理：`Musa.CoreLite v1.1.1` |
| `BuildAllTargets.cmd` | 构建入口：初始化 VS 环境 → MSBuild |
| `BuildAllTargets.proj` | MSBuild 元项目，协调所有子项目构建 |
| `Musa.Core.slnx` | VS2022 XML 解决方案格式 |

---

## Key Dependencies (关键依赖)

### 编译时依赖

| 包名 | 版本 | 用途 | 类型 |
|---|---|---|---|
| **Mile.Project.Configurations** | MSBuild SDK (隐式) | MSBuild 构建 SDK，提供 6 种配置 (Debug/Release × x86/x64/ARM64) 的统一构建基础设施 | 编译时 (SDK) |
| **Musa.CoreLite** | 1.1.1 | 轻量级核心子集，暴露 `MusaCoreLiteStartup()`、`MusaCoreLiteShutdown()`、`MusaCoreLiteGetNtdllBase()` 等基础函数 | 运行时 + 编译时 (NuGet) |
| **Musa.Veil** | git submodule / NuGet | 纯头文件 NT 内部 API 声明库 (`Veil.h`)，提供 `Zw*`/`Rtl*` 声明、结构体定义、宏 | 编译时 (Header-only) |

### 依赖说明

- **Mile.Project.Configurations**：通过 `<Import Sdk="Mile.Project.Configurations" Project="Mile.Project.Build.props" />` 引入，是 Mile.Project.Windows 生态的 MSBuild SDK 分发形式。提供跨配置、跨平台的编译管道、输出目录管理和 NuGet 打包基础设施。

- **Musa.CoreLite**：Musa.Core 的轻量级上游依赖，提供最小化的核心启动/关闭能力和 Ntdll 基址获取。版本锁定为 `1.1.1`（`Directory.Packages.Cpp.props`）。NuGet 包中声明为 `<dependency id="Musa.CoreLite" version="1.1.1" />`。

- **Musa.Veil**：header-only 的 NT 内部 API 声明集合。通过 `universal.h` 中的 `#include <Veil.h>` 引入。可作为 git submodule 或 NuGet 包获取。提供 `VEIL_DECLARE_STRUCT`、`VEIL_DECLARE_STRUCT_ALIGN` 等结构体声明宏。

### 开发工具依赖

| 工具 | 用途 |
|---|---|
| Visual Studio 2022 17.10+ | 解决方案格式为 `.slnx`（XML solution），需要 VS2022 17.10 或更高版本 |
| Windows Driver Kit (WDK) | 内核驱动编译，CI 中通过 winget 自动安装匹配版本 |
| NuGet CLI | 包打包与发布 |
| .NET SDK | 通过 `global.json` 锁定版本，用于 `dotnet nuget push` |

---

## 构建系统

### 配置矩阵

| 配置 | 平台 |
|---|---|
| Debug | x86, x64, ARM64 |
| Release | x86, x64, ARM64 |

共 **6 种** 配置组合。

### 构建流程

```bash
# 1. 清理输出目录
rd /s /q Output

# 2. 初始化 VS 环境（如未设置 VSINSTALLDIR）
call InitializeVisualStudioEnvironment.cmd

# 3. MSBuild 构建
MSBuild -binaryLogger:Output\BuildAllTargets.binlog -m BuildAllTargets.proj
```

### CI/CD

- **平台**：GitHub Actions `windows-2025`
- **触发条件**：
  - `push` 到 `main` 分支
  - `push` 到 `v*` tag
  - `pull_request` 到 `main` 分支
- **WDK 安装**：自动检测 SDK 版本，通过 `winget` 安装对应 WDK (`Microsoft.WindowsWDK.10.0.<build>`)
- **NuGet 发布**：`v*` tag push → 提取 tag 内容作为 releaseNotes → `NuGet pack` → `dotnet nuget push` 到 nuget.org
- **GitHub Release**：同时发布 `.zip`（Publish 目录）和 `.nupkg`

### 特殊构建要求

- 内核驱动 **必须** 使用 `/INTEGRITYCHECK` 链接器标志（通过 `Musa.Core.Config.props` 注入）
- PCH 机制：`Musa.Core.Nothing.cpp` 作为空锚点文件，强制包含 `universal.h`
- 模块定义文件 `Musa.Core.def` 控制 ~490 个 Zw* 导出

## 相关文档

- [项目概述 (PDR)](project-overview-pdr.md) — 目的、设计、路线图
- [系统架构](system-architecture.md) — Mermaid 组件关系、数据流、服务依赖图
