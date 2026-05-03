# Musa.Core — 系统架构

## 组件关系图 (Component Relationship)

```mermaid
graph TB
    subgraph Consumer["消费者 — 内核驱动"]
        Driver["Kernel Driver<br/>DriverEntry / DriverUnload"]
    end

    subgraph MusaCore["Musa.Core"]
        PublicAPI["Musa.Core.h<br/>MusaCoreStartup<br/>MusaCoreShutdown"]

        subgraph Thunks["Thunks 层"]
            KB["KernelBase 命名空间<br/>高层 Win32 包装器"]
            NT["Ntdll 命名空间<br/>低层 Rtl* 函数"]
        end

        subgraph SEB["系统环境块 (SEB)"]
            KPEB["KPEB<br/>进程环境块"]
            KTEB["KTEB<br/>线程环境块"]
            FLS["FLS 管理<br/>128/256 槽位"]
        end

        subgraph Utils["Musa::Utils"]
            PE["PEParser<br/>导出枚举 / RVA 解析"]
            Enc["Encode/DecodePointer<br/>安全指针编码"]
            Hash["Fnv1aHash<br/>编译期字符串哈希"]
            Loader["模块加载辅助<br/>GetKnownDllSectionHandle"]
        end

        ZwDef["Musa.Core.def<br/>~490 Zw* 导出"]
    end

    subgraph Deps["依赖"]
        CoreLite["Musa.CoreLite<br/>v1.1.1<br/>Lite 启动/关闭"]
        Veil["Musa.Veil<br/>Veil.h<br/>NT 内部声明"]
    end

    subgraph Kernel["ntoskrnl.exe"]
        NtAPI["Zw*/Nt* 系统调用"]
        ExAPI["Ex* 执行体 API"]
        PsAPI["Ps* 进程/线程 API"]
        IoAPI["Io* I/O 管理器 API"]
    end

    Driver -->|链接 lib| PublicAPI
    PublicAPI --> CoreLite
    PublicAPI -->|初始化| SEB

    KB -->|调用| NT
    KB -->|使用| SEB
    NT -->|调用| NtAPI
    NT -->|调用| ExAPI
    NT -->|调用| PsAPI

    SEB -->|使用| ExAPI
    FLS -->|回调清理| KTEB

    ZwDef -->|直接映射| NtAPI
    Driver -->|直接调用| ZwDef

    Utils -->|PE 解析| Loader
    Utils -->|哈希| NT

    CoreLite --> Veil
    Thunks --> Veil
    SEB --> Veil
```

---

## 数据流图 (Data Flow)

### 启动流程 (Startup Flow)

```mermaid
sequenceDiagram
    participant D as Driver (DriverEntry)
    participant MC as MusaCoreStartup
    participant MCL as MusaCoreLiteStartup
    participant EB as EnvironmentBlockSetup
    participant PEB as ProcessEnvironmentBlockSetup
    participant TEB as ThreadEnvironmentBlockSetup
    participant FLS as FlsCreate
    participant Heap as RtlCreateHeap
    participant TNC as PsSetCreateThreadNotifyRoutineEx

    D->>MC: MusaCoreStartup(DriverObject, RegistryPath, TLSWithThreadNotifyCallback)
    MC->>MC: 设置 MusaCoreUseThreadNotifyCallback 标志

    MC->>MCL: MusaCoreLiteStartup()
    MCL-->>MC: NTSTATUS

    alt Lite 启动失败
        MC-->>D: 返回失败状态
    end

    MC->>EB: EnvironmentBlockSetup(DriverObject, RegistryPath)
    EB->>PEB: ProcessEnvironmentBlockSetup()
    PEB->>PEB: 初始化 KPEB 结构体
    PEB->>PEB: 设置 DriverObject / RegistryPath
    PEB->>Heap: RtlCreateHeap (默认堆)
    PEB->>PEB: 初始化 StandardInput/Output/Error 句柄
    PEB-->>EB: NTSTATUS

    EB->>TEB: ThreadEnvironmentBlockSetup()
    TEB->>TEB: 初始化 TEB 列表和锁
    TEB-->>EB: NTSTATUS

    EB->>FLS: FlsCreate()
    FLS->>FLS: 初始化 RTL_FLS_CONTEXT
    FLS->>FLS: 分配 FLS Bitmap 和回调数组
    FLS-->>EB: void

    alt TLSWithThreadNotifyCallback == TRUE
        EB->>TNC: PsSetCreateThreadNotifyRoutineEx
        TNC-->>EB: NTSTATUS
    end

    EB-->>MC: NTSTATUS
    MC-->>D: STATUS_SUCCESS
```

### 关闭流程 (Shutdown Flow)

```mermaid
sequenceDiagram
    participant D as Driver (DriverUnload)
    participant MC as MusaCoreShutdown
    participant EB as EnvironmentBlockTeardown
    participant TEB as ThreadEnvironmentBlockTeardown
    participant FLS as FlsCleanup
    participant PEB as ProcessEnvironmentBlockTeardown
    participant Heap as Heap 销毁
    participant MCL as MusaCoreLiteShutdown

    D->>MC: MusaCoreShutdown()
    MC->>EB: EnvironmentBlockTeardown()

    EB->>FLS: FlsCleanup()
    FLS->>FLS: 遍历 FLS 槽位，执行回调
    FLS->>FLS: 释放 FlsCallback 数组和 Bitmap
    FLS-->>EB: void

    EB->>TEB: ThreadEnvironmentBlockTeardown()
    TEB->>TEB: 清理线程环境块列表
    TEB-->>EB: NTSTATUS

    EB->>PEB: ProcessEnvironmentBlockTeardown()
    PEB->>Heap: 销毁所有已创建堆
    PEB->>PEB: Null 化全局 KPEB 指针 (heaps 先于指针销毁)
    PEB-->>EB: NTSTATUS

    EB-->>MC: NTSTATUS

    MC->>MCL: MusaCoreLiteShutdown()
    MCL-->>MC: NTSTATUS
    MC-->>D: STATUS_SUCCESS
```

---

## 服务依赖图 (Service Dependency)

```mermaid
graph LR
    subgraph InitServices["初始化服务"]
        Lite["MusaCoreLite<br/>轻量核心"]
        EnvBlock["EnvironmentBlock<br/>环境块总控"]
        PEBSvc["ProcessEnvironmentBlock<br/>进程环境块"]
        TEBSvc["ThreadEnvironmentBlock<br/>线程环境块"]
        FLSSvc["FLS 管理<br/>Fiber Local Storage"]
    end

    subgraph RuntimeServices["运行时服务"]
        HeapSvc["堆管理<br/>ProcessHeaps + DefaultHeap"]
        SyncSvc["同步原语<br/>Event/Mutex/Semaphore/CS/SRW/CV"]
        FileSvc["文件操作<br/>CreateFile/ReadFile/WriteFile"]
        ProcSvc["进程查询<br/>GetCurrentProcess/GetProcessId"]
        ThreadSvc["线程操作<br/>GetCurrentThread/CreateThread"]
        ModSvc["模块加载<br/>GetModuleHandle/GetProcAddress"]
        NLSSvc["本地化<br/>WideCharToMultiByte"]
        TimeSvc["时间服务<br/>GetSystemTime/GetTickCount64"]
    end

    subgraph InfraServices["基础设施"]
        ZwExports["~490 Zw* 导出"]
        SEB["KPEB/KTEB<br/>系统环境块"]
        Utils["Musa::Utils<br/>工具库"]
        VeilLib["Musa.Veil<br/>NT 声明"]
    end

    Lite --> EnvBlock
    EnvBlock --> PEBSvc
    EnvBlock --> TEBSvc
    PEBSvc --> HeapSvc
    TEBSvc --> FLSSvc
    FLSSvc --> SEB

    HeapSvc --> SEB
    SyncSvc --> ZwExports
    SyncSvc --> SEB
    FileSvc --> ZwExports
    ProcSvc --> ZwExports
    ProcSvc --> SEB
    ThreadSvc --> ZwExports
    ThreadSvc --> SEB
    ModSvc --> Utils
    ModSvc --> ZwExports
    NLSSvc --> ZwExports
    TimeSvc --> ZwExports

    SEB --> VeilLib
    ZwExports --> VeilLib
    Utils --> VeilLib
```

---

## SEB 架构详解

### KPEB (Kernel Process Environment Block)

```
KPEB (8 字节对齐)
├── ERESOURCE           Lock           # 读写锁 (APC_LEVEL)
├── EX_RUNDOWN_REF      RundownProtect # 关闭保护
├── PDRIVER_OBJECT      DriverObject   # 驱动对象引用
├── UNICODE_STRING      RegistryPath   # 注册表路径
├── SIZE_T              SizeOfImage    # 镜像大小
├── PVOID               ImageBaseAddress
├── UNICODE_STRING      ImagePathName
├── UNICODE_STRING      ImageBaseName
├── ULONG               NumberOfHeaps
├── ULONG               MaximumNumberOfHeaps
├── PVOID               DefaultHeap    # 默认堆 (RtlCreateHeap)
├── PVOID*              ProcessHeaps   # 堆指针数组
├── ULONG               HardErrorMode
├── HANDLE              StandardInput  # 标准输入句柄
├── HANDLE              StandardOutput # 标准输出句柄
├── HANDLE              StandardError  # 标准错误句柄
├── HANDLE              DefaultStandardInput
├── HANDLE              DefaultStandardOutput
├── HANDLE              DefaultStandardError
├── WCHAR               CurrentDirectory[MAX_PATH]
└── PVOID               WaitOnAddressHashTable[128]
```

大小约束：`ALIGN_DOWN(sizeof(KPEB), 8) < PAGE_SIZE`

### KTEB (Kernel Thread Environment Block)

```
KTEB (8 字节对齐)
├── HANDLE              ThreadId        # 线程 ID
├── HANDLE              ProcessId       # 所属进程 ID
├── struct _KPEB*       ProcessEnvironmentBlock  # 指向 KPEB
├── ULONG               HardErrorMode
├── NTSTATUS            ExceptionCode
├── ULONG               LastErrorValue  # GetLastError 存储
├── NTSTATUS            LastStatusValue # RtlGetLastNTError 存储
└── struct _RTL_FLS_DATA* FlsData       # 线程 FLS 数据
```

### FLS (Fiber Local Storage)

```
RTL_FLS_CONTEXT
├── EX_SPIN_LOCK                Lock
├── PFLS_CALLBACK_FUNCTION*     FlsCallback      # 回调函数指针数组
├── LIST_ENTRY                  FlsListHead      # FLS 数据链表头
├── RTL_BITMAP                  FlsBitmap        # 槽位分配位图
├── ULONG                       FlsBitmapBits[]  # 位图数据
└── ULONG                       FlsHighIndex     # 最高使用索引

RTL_FLS_DATA (per-thread)
├── LIST_ENTRY  Entry           # 链表节点
└── PVOID       Slots[]         # 槽位数组 (x64: 128 槽位)
```

FLS 容量：`RTLP_FLS_MAXIMUM_AVAILABLE = 256 - (sizeof(LIST_ENTRY) / sizeof(PVOID))`
- x64：128 槽位
- x86：254 槽位

---

## Thunk 实现模式

每个 Thunk 函数遵循统一的结构模式：

```
#include "Internal/模块.h"          ← 私有头文件
#pragma alloc_text(PAGE, MUSA_NAME(Fn))   ← PAGE 段注解
EXTERN_C_START                      ← C 链接开始

NTSTATUS MUSA_NAME(Fn)(参数) {      ← SAL 注解的函数签名
    PAGED_CODE();                   ← 分页检查

    // NTSTATUS 内核实现...

    return BaseSetLastNTError(Status);  ← 设置 LastError 并返回
}

MUSA_IAT_SYMBOL(Fn, stack_bytes)    ← IAT 符号注册
EXTERN_C_END                        ← C 链接结束
```

命名约定：
- `MUSA_NAME(name)` → `_Musa_name`（公开符号）
- `MUSA_NAME_PRIVATE(name)` → `_Musa_Private_name`（内部符号）
- x86：`MUSA_IAT_SYMBOL(name, stack)` 生成 `name@stack` 修饰符
- x64/ARM64：直接符号名

---

## 构建依赖链

```mermaid
graph TB
    subgraph Source["源码"]
        Thunks["Thunks/*.cpp"]
        SEBSrc["SEB/*.cpp"]
        UtilsSrc["Utils/*.cpp"]
        CoreEntry["Musa.Core.cpp"]
    end

    subgraph Build["构建"]
        PCH["universal.h (PCH)"]
        Nothing["Musa.Core.Nothing.cpp"]
        StaticLib["Musa.Core.StaticLibraryForDriver → .lib"]
    end

    subgraph Output["输出"]
        PublishLib["Publish/Library/{Config}/{Platform}/*.lib"]
        PublishHeaders["Publish/Include/Musa.Core.h"]
        NuGet["Musa.Core.*.nupkg"]
    end

    subgraph Consumer["消费方"]
        TestDriver["Musa.Core.TestForDriver"]
        ExternalDriver["外部内核驱动"]
    end

    PCH -->|forced-include| Nothing
    Nothing --> StaticLib
    Thunks --> StaticLib
    SEBSrc --> StaticLib
    UtilsSrc --> StaticLib
    CoreEntry --> StaticLib

    StaticLib --> PublishLib
    CoreEntry -->|复制| PublishHeaders

    PublishLib --> NuGet
    PublishHeaders --> NuGet

    PublishLib -->|链接| TestDriver
    PublishLib -->|链接| ExternalDriver
    PublishHeaders -->|包含| TestDriver
    PublishHeaders -->|包含| ExternalDriver
```

## 相关文档

- [项目概述 (PDR)](project-overview-pdr.md) — 目的、设计、路线图
- [代码库摘要](codebase-summary.md) — 完整文件清单和依赖
