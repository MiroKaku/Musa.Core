# [Musa.Core](https://github.com/MiroKaku/Musa.Core)

[![Actions Status](https://github.com/MiroKaku/Musa.Core/workflows/build/badge.svg)](https://github.com/MiroKaku/Musa.Core/actions)
[![nuget](https://img.shields.io/nuget/v/Musa.Core)](https://www.nuget.org/packages/Musa.Core/)
[![LICENSE](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/MiroKaku/Musa.Core/blob/main/LICENSE)
![Visual Studio](https://img.shields.io/badge/Visual%20Studio-2022-purple.svg)
![Windows](https://img.shields.io/badge/Windows-10+-orange.svg)
![Platform](https://img.shields.io/badge/Windows-X64%7CARM64-%23FFBCD9)

* [English](https://github.com/MiroKaku/Musa.Core/blob/main/README.md)

## 简介

> **Warning**
> 
> Musa.Core 在测试阶段中 ...

Musa.Core 是 [Musa.Runtime](https://github.com/MiroKaku/Musa.Runtime) (原 [ucxxrt](https://github.com/MiroKaku/ucxxrt)) 的底层API实现的衍生物。

主要是用 ntdll/ntoskrnl 实现 Kernel32、Advapi32 等API，它包括用户模式和内核模式两种。

推荐与 [Musa.Veil](https://github.com/MiroKaku/Veil) 一起食用。

## 使用方法

右键单击该项目并选择“管理 NuGet 包”，然后搜索`Musa.Core`并选择适合你的版本，最后单击“安装”。

或者

如果你的项目模板用的是 [Mile.Project.Windows](https://github.com/ProjectMile/Mile.Project.Windows)，那么可以直接在你的 `.vcxproj` 文件里面添加下面代码：

```XML
  <ItemGroup>
    <PackageReference Include="Musa.Core">
      <!-- 期望的版本 -->
      <Version>0.1.0</Version>
    </PackageReference>
  </ItemGroup>
```

### 仅头文件模式

在你的 `.vcxproj` 文件里面添加下面代码：

```XML
  <PropertyGroup>
    <MusaCoreOnlyHeader>true</MusaCoreOnlyHeader>
  </PropertyGroup>
```

这个模式不会自动引入lib文件。

## 特性

- [x] 可以直接使用所有的当前系统支持的 Zw 例程。
    ```C
    NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
    {

        UNREFERENCED_PARAMETER(DriverObject);
        UNREFERENCED_PARAMETER(RegistryPath);

        NTSTATUS Status;

        do {
            DriverObject->DriverUnload = DriverUnload;

            Status = MusaCoreStartup(DriverObject, RegistryPath);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            LARGE_INTEGER SystemTime{};
            Status = ZwQuerySystemTime(&SystemTime);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = RtlSystemTimeToLocalTime(&SystemTime, &SystemTime);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            TIME_FIELDS Time{};
            RtlTimeToTimeFields(&SystemTime, &Time);

            MusaLOG("Loading time is %04d/%02d/%02d %02d:%02d:%02d",
                Time.Year, Time.Month, Time.Day,
                Time.Hour, Time.Minute, Time.Second);

        } while (false);

        if (!NT_SUCCESS(Status)) {
            DriverUnload(DriverObject);
        }

        return Status;
    }
    ```

- [x] 支持部分 RuntimeLibrary(Rtl) 系列的 APIs
- [x] 支持部分 KernelBase 模块的 APIs
- [ ] 支持部分 Advapi32 模块的的 APIs

## 进度
查看 [Project](https://github.com/users/MiroKaku/projects/1/views/1)

## 鸣谢

> [IntelliJ IDEA](https://zh.wikipedia.org/zh-hans/IntelliJ_IDEA) 是一个在各个方面都最大程度地提高开发人员的生产力的 IDE。

特别感谢 [JetBrains](https://www.jetbrains.com/?from=meesong) 为开源项目提供免费的 [Resharper C++](https://www.jetbrains.com/resharper-cpp/?from=meesong) 等 IDE 的授权

[<img src="https://resources.jetbrains.com/storage/products/company/brand/logos/ReSharperCPP_icon.png" alt="ReSharper C++ logo." width=200>](https://www.jetbrains.com/?from=meesong)

## 感谢 & 参考
* 感谢：Zw 例程获取方案由 @[xiaobfly](https://github.com/xiaobfly) 提供。
* 参考：[systeminformer](https://github.com/winsiderss/systeminformer)/phnt
* 参考：[Windows_OS_Internals_Curriculum_Resource_Kit-ACADEMIC](https://github.com/MeeSong/Windows_OS_Internals_Curriculum_Resource_Kit-ACADEMIC)
