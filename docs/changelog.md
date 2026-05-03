# 变更日志

> [系统架构](./system-architecture.md) ｜ [部署指南](./deployment-guide.md) ｜ [构建配置](./configuration-guide.md)

## v* — 最新

### Feat（新功能）

- 实现 Phase 1-6 kernel32 API thunks，支持 Musa.Runtime 集成
- 新增 CWD（当前工作目录）功能，通过 KPEB 实现
  - `GetCurrentDirectoryW`, `SetCurrentDirectoryW`
  - `ExpandEnvironmentStringsW`
  - `RtlGetCurrentDirectory_U`, `RtlSetCurrentDirectory_U`
- 新增 `GetEnvironmentVariableW` — 回退至 `HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion` 查询 `SystemRoot`

### Fix（修复）

- `ExpandEnvironmentStringsW`：修正 ValLen 查询路径下包含 null 字符的缓冲区偏移计算，修正双重减去 null 终止符的问题
- `GetEnvironmentVariableW`：添加 SystemRoot 注册表回退路径
- `EnvironmentBlockSetup`：添加完整回滚路径，防止部分失败导致的状态不一致
- `\??\UNC\` 前缀长度从 10 修正为 8
- `WaitForSingleObject` / `WaitForMultipleObjects`：返回正确的等待状态
- `GetStartupInfoW`：始终返回空的标准句柄
- PEB 全局指针清理延迟至堆销毁之后
- PEB 全局指针清理延迟至 rundown protection 释放之后
- `RtlMapResourceId`：传播异常状态，添加 PE 解析器 null 检查
- `ProcessEnvironmentBlock`：初始化 ERESOURCE 锁，添加 setup 回滚，修复 teardown 顺序
- `RtlFindResource`：使用 `ULONG_PTR` 数组，添加资源 ID 宏
- `RtlFindAndFormatMessage`：使用 `MusaCoreLiteGetNtdllBase()` 并添加 null 检查
- 回调对象 teardown 中的 TOCTOU 竞态条件
- `MusaCoreThreadNotifyCallbackObject` 在 `ExNotifyCallback` 前添加 null 守卫
- `WideCharToMultiByte` CP_UTF8 修复 PAGE_FAULT
- NLS UTF8 转换 bug 修复及内核模式测试更新
- SAL 注解、`PAGED_CODE` 位置和 CODEANALYSIS 抑制修正

### Docs（文档）

- 更新 AGENTS.md，反映当前项目结构，修复过期内容
- 更新文档为 kernel-mode only 项目
- 添加分层 AGENTS.md 知识库用于代码导航

### Refactor（重构）

- 拆分 `Ntdll.Synchronize` 和 `KernelBase.System` 为更小模块
- 拆分大型 thunk 文件为更小模块
- 移除所有剩余的 `_KERNEL_MODE` 条件编译守卫
- 移除冗余 `_KERNEL_MODE` 守卫，项目为内核专属

### Build（构建）

- 升级 `Mile.Project.Configurations` 至 1.0.1917，`Musa.CoreLite` 至 1.1.0
- 更新 `Musa.CoreLite` 至 1.1.1
- 更新 `Musa.Veil` 1.4.1 → 1.5.0，`Musa.CoreLite` 1.0.1 → 1.0.3
- 统一 vcxproj 和 filters 文件中的路径格式
- 从跟踪中移除 `Publish/Include/Musa.Core.h`
- 移除用户模式支持，仅保留内核模式
- 修复 `BuildAllTargets.cmd` 构建脚本

### Test（测试）

- 扩展测试套件：新增 LibraryLoader、NLS、RealTime、Handle、Debug 模块
- 添加 `TestForDriver` boot-start 驱动重新初始化支持
- 重写 `TestForDriver`，遵循 CoreLite 测试模式
- `RtlDosPathNameToNtPathName_U` 输出内容验证
- `Get/SetCurrentDirectoryW` 和 `ExpandEnvironmentStringsW` 内容验证
- 验证 `ExpandEnvironmentStringsW` 展开内容以 `SystemRoot` 开头

### CI

- CI runner 升级至 `windows-2025`，WDK 自动探测和安装

### Cleanup（清理）

- 移除 `ProcessAppMemoryInfo` 中的 TODO 注释

### Update（依赖更新）

- 分离 `Musa.CoreLite` 为独立依赖
- 更新 `Musa.Veil` 1.3.2
- 更新 `Musa.CoreLite` 1.0.1

## 历史版本

### v0.4.1

- 初始公开发布版本
