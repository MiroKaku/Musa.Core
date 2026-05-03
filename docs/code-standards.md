# Code Standards — Musa.Core

本文档描述 Musa.Core 内核模式驱动的目录结构、编码规范与 thunk 实现约定。

相关文档：
- [系统架构](system-architecture.md)
- [API 参考](api-reference.md)
- [测试指南](testing-guide.md)

## 1. 项目结构

```
Musa.Core/
├── Musa.Core.h                          # 公共头：MusaCoreStartup / MusaCoreShutdown
├── Musa.Core.cpp                        # 驱动初始化序列
├── Musa.Core.def                        # 模块定义文件，~490 个 _Mi_Zw* 导出
├── Musa.Core.SystemEnvironmentBlock.h/.cpp   # PEB/TEB/SEB 内核态伪造
├── Musa.Core.SystemEnvironmentBlock.*.h/.cpp # Process / Thread / FLS 分块
├── Musa.Utilities.h                     # 安全 cookie、栈对齐
├── Musa.Utilities.PEParser.h            # PE 解析模板库
├── Musa.Utilities.Overlay.cpp           # IAT 覆盖：MakeImageOverlay
└── Thunks/
    ├── Internal/                        # 私有头，每 DLL 按类别分
    │   ├── KernelBase.{Category}.h      # 17 个私有头
    │   └── Ntdll.{Category}.h
    ├── KernelBase.Private.h             # 私有共享：BaseSetLastNTError 等
    ├── {DLL}.{Category}.cpp             # thunk 实现主体
    └── {DLL}.{Category}.{Sub}.cpp       # 大类别拆分子文件
```

### 1.1 文件命名约定

Thunk 文件按 `{DLL}.{Category}.cpp` 格式组织：

| 文件模式 | 示例 | 职责 |
|---|---|---|
| `KernelBase.{Category}.cpp` | `KernelBase.Process.cpp` | Win32 API thunk |
| `Ntdll.{Category}.cpp` | `Ntdll.Heap.cpp` | NTDLL Rtl* 层实现 |
| `{DLL}.{Category}.{Sub}.cpp` | `KernelBase.Thread.Create.cpp` | 大类别拆分 |

详细规则参见 [Thunks/AGENTS.md](../Musa.Core/Thunks/AGENTS.md)。

## 2. 编码风格

### 2.1 .editorconfig 强制执行

| 文件类型 | 编码 | 换行 | 缩进 |
|---|---|---|---|
| `.cpp` / `.h` | UTF-8 BOM | CRLF | 4 空格 |
| `.rc` / `.inf` | UTF-16LE | — | — |
| `.bat` | UTF-16LE | CRLF | — |
| `.sh` | UTF-8 | LF | — |

注释使用 Doxygen 风格：`/** */`（`vc_generate_documentation_comments = doxygen_slash_star`）。

### 2.2 命名约定

| 类别 | 宏 | 展开形式 | 示例 |
|---|---|---|---|
| 公共符号 | `MUSA_NAME(name)` | `_Musa_##name` | `MUSA_NAME(OpenProcess)` → `_Musa_OpenProcess` |
| 私有符号 | `MUSA_NAME_PRIVATE(name)` | `_Musa_Private_##name` | `MUSA_NAME_PRIVATE(EnvironmentBlockSetup)` |
| IAT 导出 | `MUSA_IAT_SYMBOL(name, stack)` | x86 使用 `_name@stack` 修饰 | `MUSA_IAT_SYMBOL(CreateFileW, 28)` |
| 调用约定 | `MUSA_API` | `__stdcall` | — |

### 2.3 核心宏参考

```cpp
#define MUSA_API              __stdcall
#define MUSA_NAME(name)       _Musa_##name        // via VEIL
#define MUSA_NAME_PRIVATE(n)  _Musa_Private_##name
#define MusaLOG(fmt, ...)     DbgPrintEx(...)     // DEBUG 构建，Release 无操作
#define MUSA_TAG              '-iM-'              // 内核池标签
```

`BaseSetLastNTError` 在 `KernelBase.Private.h` 中映射为：
```cpp
#define BaseSetLastNTError RtlSetLastWin32ErrorAndNtStatusFromNtStatus
```

## 3. Thunk 实现模式

每个 thunk **必须** 遵循以下结构：

```cpp
#include "Internal/KernelBase.Category.h"    // 或等价 Ntdll 头

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(SomeFunction))
#endif

EXTERN_C_START

_Success_(return)
BOOL MUSA_API MUSA_NAME(SomeFunction)(
    _In_ DWORD Param
) {
    PAGED_CODE();

    NTSTATUS Status = /* 使用 Zw*/Rtl* API 的内核实现 */;
    BaseSetLastNTError(Status);
    return NT_SUCCESS(Status);
}

MUSA_IAT_SYMBOL(SomeFunction, 4);

EXTERN_C_END
```

### 3.1 关键步骤（遗漏任何一步即产生缺陷）

1. `MUSA_NAME(Fn)` 包装函数名 → `_Musa_Fn`
2. `MUSA_API` = `__stdcall` 调用约定
3. `#pragma alloc_text(PAGE, ...)` — 内核构建 **必须** 指定段放置。所有启动函数从 INIT 移至 PAGE 以支持 boot-start 驱动重新初始化
4. `MUSA_IAT_SYMBOL(Fn, N)` 在文件底部 — 注册 IAT 钩子。`N` 为栈字节数（仅 x86 有效，x64/ARM64 填 0）
5. 使用 `BaseSetLastNTError(Status)` 转换 NTSTATUS → Win32 last-error
6. 返回 `NT_SUCCESS(Status)`

### 3.2 分页代码约定

- `#pragma alloc_text(PAGE, ...)` 与 `PAGED_CODE()` 配对使用
- 所有可分页函数 **必须** 运行在 `IRQL <= APC_LEVEL`
- 启动初始化函数使用 PAGE 段而非 INIT，支持 boot-start 驱动的 `BootDriverReinitialize` 模式

### 3.3 错误处理

- 全程使用 NTSTATUS
- `BaseSetLastNTError(Status)` 通过 `RtlNtStatusToDosError` 映射到 Win32 错误
- 启动函数使用 `do/while(false)` 模式进行清理，避免 goto
- 拆卸时使用 `InterlockedExchangePointer` 线程安全置空

## 4. 反模式（禁止行为）

| 反模式 | 后果 | 正确做法 |
|---|---|---|
| 使用 `as-style` 转换 | 编译失败或未定义行为 | 使用 C 风格转换和 `reinterpret_cast` |
| 新增 `.cpp` 未更新 `.vcxproj` | 文件不会被编译 | 同步更新对应 vcxproj |
| 实现 thunk 后遗漏 `MUSA_IAT_SYMBOL` | API 不会被 IAT 钩住，调用者走原始路径 | 每个 thunk 末尾必须注册 |
| 内核 thunk 省略 `#pragma alloc_text` | 链接或分页检查失败 | 每个内核 thunk 必须包含 |
| 假设堆 API 在内核态与用户态相同 | 内存损坏 | 内核使用 `RtlAllocateHeap` 配合特殊池设置 |
| 测试内核代码未使用 `/INTEGRITYCHECK` | 驱动加载失败 | 链接器必须带此标志 |
| 创建新文件未更新 `.vcxproj` | 编译遗漏 | 新增文件后立即更新项目文件 |

## 5. 内部头组织

`Thunks/Internal/` 包含 17 个私有头 + 1 个实现文件（`{DLL}.{Category}.h`），为 thunk 提供共享辅助函数。

示例：
- `KernelBase.Fiber.h` — FLS 辅助
- `KernelBase.Heap.h` — 堆操作辅助
- `KernelBase.Synchronize.h` — 同步对象辅助
- `Ntdll.Image.h` — PE 镜像加载辅助
- `Ntdll.Heap.h` — Rtl 堆辅助

这些头 **不得** 在 `Thunks/` 目录外被包含。

## 6. 类别拆分模式

大型类别拆分为子文件，主文件 include 子文件：

```
KernelBase.Synchronize.cpp
├── Synchronize.Event.cpp      # CreateEventW, SetEvent, ResetEvent...
├── Synchronize.Mutex.cpp      # CreateMutexW, ReleaseMutex...
└── Synchronize.Semaphore.cpp  # CreateSemaphoreW, ReleaseSemaphore...

KernelBase.Thread.cpp
├── Thread.Create.cpp           # CreateThread, ExitThread
└── Thread.ProcAttr.cpp         # SetThreadPriority, GetThreadPriority
```

## 7. 已知不完整项

- `GetStartupInfoW` 内核态：PEB 访问未完成
- `GetProcessInformation(ProcessAppMemoryInfo)`：返回 `STATUS_NOT_IMPLEMENTED`
- `RtlWalkHeap`：stub，返回 `STATUS_NOT_IMPLEMENTED`
- `LocalAlloc/LocalReAlloc`：不支持 `LMEM_MOVEABLE` 标志
- `Ntdll.LibraryLoader.ProcAddr.cpp`：回调字段赋值为 TODO
- `Ntdll.LibraryLoader.cpp`：模块卸载逻辑不完整

详见 [AGENTS.md](../Musa.Core/AGENTS.md) KNOWN ISSUES。
