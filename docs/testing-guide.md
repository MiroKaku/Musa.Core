# Testing Guide — Musa.Core

本文档描述 Musa.Core 内核模式驱动的测试策略、构建、部署、运行与输出捕获方法。

相关文档：
- [系统架构](system-architecture.md)
- [代码规范](code-standards.md)
- [API 参考](api-reference.md)

## 1. 测试框架

Musa.Core 使用 `Musa.Core.TestForDriver/` 目录下的 KMDF 内核驱动测试套件。

### 1.1 架构

- 单一编译单元（~986 行）
- `DriverEntry` 模式 — 作为独立内核驱动加载
- 使用 `KTEST_EXPECT` 宏进行断言计数（pass/fail 统计通过 `DbgPrintEx` 输出）
- 测试生命周期由 `MusaCoreStartup` / `MusaCoreShutdown` 管理

### 1.2 KTEST_EXPECT 宏

```cpp
KTEST_EXPECT(condition);
```

每次调用递增内部 pass/fail 计数器。驱动卸载时打印最终汇总。

## 2. 测试覆盖范围

### 2.1 已覆盖类别

| 类别 | 测试项 |
|---|---|
| StdHandle | Get/Set/Duplicate/GetHandleInfo |
| Heap | Alloc/Free/ReAlloc/Validate/Local/Global |
| 同步 | CRITICAL_SECTION, SRWLock, Event, Mutex, Semaphore |
| 进程 | ID/Times/HandleCount/SessionId/StartupInfo |
| 线程 | ID/Priority/Times |
| 工具 | Encode/DecodePointer 往返测试 |
| FLS | Alloc/Free/GetValue/SetValue |
| 错误 | Set/GetLastError |
| 模块加载 | GetModuleHandle |
| NLS | WideChar/MultiByte 转换, GetACP, GetCPInfoExW |
| 高精度时间 | PreciseFileTime, TickCount, QPC |
| 时间 | System/Local, SystemTimeToFileTime 往返 |
| 路径 | GetTempPath, RtlDosPathNameToNtPathName_U（drive/UNC/NT/extended/relative 变体） |
| 文件 I/O | CreateFile/Read/Write/SetPointer/Flush/Delete/GetFileAttributesExW |
| 目录 | Create/Remove/Move/GetDriveType/FindFirst/Next/Close |
| 系统 | GetCommandLine, GetEnvironmentVariable, Get/SetCurrentDirectory, ExpandEnvironmentStrings |
| 句柄 | GetHandleInformation |
| 调试 | FormatMessage, OutputDebugString |

### 2.2 已知未覆盖/不完整项

| 项目 | 状态 |
|---|---|
| `GetStartupInfoW` 内核态 | PEB 访问未完成，TODO |
| `GetProcessInformation(ProcessAppMemoryInfo)` | 返回 `STATUS_NOT_IMPLEMENTED` |
| `RtlWalkHeap` | stub，返回 `STATUS_NOT_IMPLEMENTED` |
| `LocalAlloc/LocalReAlloc` | `LMEM_MOVEABLE` 标志不支持 |
| `Ntdll.LibraryLoader.ProcAddr.cpp` | 回调字段赋值 TODO |
| `Ntdll.LibraryLoader.cpp` | 模块卸载逻辑不完整 |

## 3. 构建

### 3.1 测试驱动构建

```
MSBuild Musa.Core.TestForDriver.vcxproj /p:Configuration=Debug /p:Platform=x64
```

### 3.2 链接器标志

内核模式测试驱动 **必须** 使用 `/INTEGRITYCHECK` 链接器标志，否则驱动无法加载：

```xml
<Link>
  <AdditionalOptions>/INTEGRITYCHECK %(AdditionalOptions)</AdditionalOptions>
</Link>
```

### 3.3 主驱动构建

```
MSBuild Musa.Core.vcxproj /p:Configuration=Debug /p:Platform=x64
```

## 4. 部署与安装

### 4.1 INF 文件配置

测试驱动使用 INF 文件安装，关键配置：

- **类：** `AntiVirus`
- **启动类型：** `SERVICE_DEMAND_START`
- **安装方式：** `DefaultInstall` 节

### 4.2 安装步骤

1. 编译测试驱动生成 `.sys` 和 `.inf`
2. 以管理员权限执行安装：

```cmd
pnputil -i -a Musa.Core.TestForDriver.inf
```

或使用 `DefaultInstall`：

```cmd
rundll32 setupapi,InstallHinfSection DefaultInstall 132 Musa.Core.TestForDriver.inf
```

### 4.3 启动驱动

```cmd
sc start Musa.Core.TestForDriver
```

或

```cmd
net start Musa.Core.TestForDriver
```

### 4.4 停止与卸载

```cmd
sc stop Musa.Core.TestForDriver
sc delete Musa.Core.TestForDriver
```

## 5. 运行与输出捕获

### 5.1 输出机制

所有测试输出通过 `DbgPrintEx` 发送到内核调试器。测试框架使用 `KTEST_EXPECT` 宏记录 pass/fail 计数。

### 5.2 捕获工具

#### WinDbg

连接内核调试器后，在 WinDbg 中过滤 Musa 相关输出：

```
ed kd_default_mask 8
```

或使用 `!dbgprint` 查看缓冲输出。

#### DbgView（推荐）

1. 下载并运行 [Sysinternals DbgView](https://docs.microsoft.com/sysinternals/downloads/debugview)
2. 以管理员权限运行
3. 启用 **Capture Kernel** 和 **Enable Verbose Kernel Output**
4. 启动测试驱动，DbgView 实时显示 `DbgPrintEx` 输出

### 5.3 预期输出格式

```
[Musa.Test] PASS: TestName - description
[Musa.Test] FAIL: TestName - description
...
[Musa.Test] === Test Summary ===
[Musa.Test] Total: N, Passed: N, Failed: N
```

## 6. Boot-Start 驱动重新初始化

Musa.Core 支持 boot-start 驱动的延迟启动模式：

```
BootDriverReinitialize → MusaCoreStartup → EnvironmentBlockSetup
```

测试此路径时：
1. 将驱动启动类型设为 `SERVICE_BOOT_START`
2. 验证 `BootDriverReinitialize` 回调正确触发
3. 确认 PAGE 段函数（非 INIT 段）在重新初始化后可正常调用

## 7. 添加新测试

### 7.1 步骤

1. 在 `Musa.Core.TestForDriver/` 主测试文件中添加测试用例
2. 使用 `KTEST_EXPECT(condition)` 包裹断言
3. 确保测试在 `DriverEntry` 执行流中被调用
4. 测试应在 `MusaCoreStartup` 成功返回后执行，在 `MusaCoreShutdown` 前完成

### 7.2 测试模式示例

```cpp
VOID TestMyFeature()
{
    // 设置
    HANDLE hObj = CreateMyObject(...);
    KTEST_EXPECT(hObj != NULL);

    // 验证行为
    BOOL result = OperateOnObject(hObj);
    KTEST_EXPECT(result == TRUE);

    // 清理
    if (hObj) CloseMyObject(hObj);
}
```

## 8. 调试建议

### 8.1 驱动加载失败

- 检查 `/INTEGRITYCHECK` 标志是否已添加到链接器
- 确认签名（测试环境可使用测试签名 + `bcdedit /set testsigning on`）
- 查看事件查看器中的驱动加载错误

### 8.2 测试无输出

- 确认 DbgView 已启用 **Capture Kernel**
- 检查 `DbgPrintEx` 组件 ID 和级别设置
- 验证 `MusaCoreStartup` 是否返回 `STATUS_SUCCESS`

### 8.3 内核崩溃

- 连接 WinDbg 获取崩溃转储
- 检查 `PAGED_CODE()` 违规（在高于 `APC_LEVEL` 的 IRQL 调用分页函数）
- 验证池标签 `'-iM-'` 相关内存操作
