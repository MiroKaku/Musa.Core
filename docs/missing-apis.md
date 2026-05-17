# Musa.Core 缺失 API 汇总

> 由 Musa.Runtime UCRT 解锁计划触发 · 2026-05-04

## 背景

Musa.Runtime 向 `Musa.Runtime.UCRT.vcxproj` 添加了 112 个基础 SDK UCRT 源文件（lowio/stdio/filesystem/time/env），这些文件引用了一些 Musa.Core 尚未实现 thunk 的 kernel32 API，导致 TestForDriver 链接阶段出现未解析符号。

## 缺失的 kernel32 API

按优先级排列：

### P1 — 阻塞已解锁核心目录

| API | 来源 UCRT 文件 | 说明 |
|---|---|---|
| `GetSystemTimeAsFileTime` | `time.obj` (`time/time.cpp`) | 用于 `timespec_get` 实现；已有 `GetSystemTime`，但此 API 提供更高精度 FILETIME |
| `SetEnvironmentVariableW` | `setenv.obj` (`env/setenv.cpp`) | 环境变量写入；宽字符版本 |
| `GetEnvironmentStringsW` | `get_environment_from_os.obj` (`env/get_environment_from_os.cpp`) | 获取进程环境块；内核模式下可返回空 |
| `FreeEnvironmentStringsW` | `get_environment_from_os.obj` | 释放环境块；伴随上述 API |

### P2 — 控制台 I/O（内核驱动通常不需要）

| API | 来源 UCRT 文件 | 说明 |
|---|---|---|
| `GetConsoleOutputCP` | `write.obj` (`stdio/write.cpp`) | 获取控制台输出代码页 |
| `GetConsoleMode` | `write.obj`, `read.obj` (`stdio/read.cpp`) | 获取控制台模式 |
| `ReadConsoleW` | `read.obj` | 控制台读取 |

### P3 — 日期/时间格式化（依赖 locale）

| API | 来源 UCRT 文件 | 说明 |
|---|---|---|
| `GetDateFormatEx` | `wcsftime.obj` (`time/wcsftime.cpp`) | 日期格式化（需要 locale 数据） |
| `GetTimeFormatEx` | `wcsftime.obj` | 时间格式化（需要 locale 数据） |

## 建议实现方案

### 1. `GetSystemTimeAsFileTime`

```c
// 直接调用 KeQuerySystemTimePrecise 或 KeQuerySystemTime
VOID WINAPI GetSystemTimeAsFileTime(LPFILETIME lpSystemTimeAsFileTime)
{
    LARGE_INTEGER Time;
    KeQuerySystemTime(&Time);  // 或 KeQuerySystemTimePrecise
    lpSystemTimeAsFileTime->dwLowDateTime  = Time.LowPart;
    lpSystemTimeAsFileTime->dwHighDateTime = Time.HighPart;
}
```

### 2. `SetEnvironmentVariableW`

```c
// 内核模式：维护简单的键值表，或返回 STATUS_NOT_SUPPORTED
BOOL WINAPI SetEnvironmentVariableW(LPCWSTR lpName, LPCWSTR lpValue)
{
    // 方案 A：空实现（内核驱动通常不需要环境变量）
    RtlSetLastWin32Error(ERROR_NOT_SUPPORTED);
    return FALSE;
    
    // 方案 B：基于注册表或内部表实现（复杂度高）
}
```

### 3. `GetEnvironmentStringsW` / `FreeEnvironmentStringsW`

```c
// 内核模式无进程环境块，返回空
LPWCH WINAPI GetEnvironmentStringsW()
{
    RtlSetLastWin32Error(ERROR_NOT_SUPPORTED);
    return NULL;
}

BOOL WINAPI FreeEnvironmentStringsW(LPWCH lpszEnvironmentBlock)
{
    UNREFERENCED_PARAMETER(lpszEnvironmentBlock);
    return TRUE;
}
```

### 4. 控制台 API（P2）

建议实现为 stub，返回不支持：

```c
UINT WINAPI GetConsoleOutputCP(void) { return CP_ACP; }
BOOL WINAPI GetConsoleMode(HANDLE h, LPDWORD m) { RtlSetLastWin32Error(ERROR_NOT_SUPPORTED); return FALSE; }
BOOL WINAPI ReadConsoleW(...) { RtlSetLastWin32Error(ERROR_NOT_SUPPORTED); return FALSE; }
```

## 非 Musa.Core 范围的缺失符号

以下符号由 UCRT 内部提供，不属于 kernel32 thunk 范围，应在 Musa.Runtime 的 overlay 中解决：

| 符号 | 类别 | 说明 |
|---|---|---|
| `__acrt_clock_initializer` | CRT 初始化器 | overlay `internal/initialization.cpp` 需添加 |
| `__acrt_timeset_initializer` | CRT 初始化器 | 同上 |
| `__acrt_stdio_initializer` | CRT 初始化器 | 同上 |
| `__acrt_stdio_terminator` | CRT 终止器 | 同上 |
| `__acrt_MultiByteToWideChar` | 字符转换 | overlay `convert/` 需提供内核版实现 |
| `__acrt_WideCharToMultiByte` | 字符转换 | 同上 |
| `__acrt_initialize_multibyte` | 多字节初始化 | 内核模式可空实现 |
| `_wcsnicoll`, `_strnicoll` | 字符串排序 | overlay `string/` 需提供内核版 |
| `_ismbblead`, `_mbsdec` | 多字节函数 | overlay `mbstring/` 需提供内核版 |
| `_mbtowc_internal` | 多字节转换 | 同上 |
| `__crt_mbstring::__mbsrtowcs_utf8` | UTF-8 转换 | 同上 |
| `__acrt_GetSystemTimePreciseAsFileTime` | UCRT 时间 | 可委托给 `GetSystemTimeAsFileTime` thunk |
| `__acrt_AreFileApisANSI` | ANSI API 检测 | 内核模式始终返回 FALSE |
| `___lc_codepage_func` | locale 代码页 | 内核模式返回固定值 |
| `__acrt_GetDateFormatEx` | 日期格式化 | 委托给 `GetDateFormatEx` thunk |
| `__acrt_GetTimeFormatEx` | 时间格式化 | 委托给 `GetTimeFormatEx` thunk |
| `__lc_time_c` | locale 时间数据 | overlay locale/ 需提供 |
| `_query_app_type` | 应用类型检测 | 内核模式返回固定值 |
| `_putwch_nolock` | 宽字符控制台输出 | 属于 `conio/`，内核模式可空实现 |
