# [Musa.Core](https://github.com/MiroKaku/Musa.Core)

[![Actions Status](https://github.com/MiroKaku/Musa.Core/workflows/build/badge.svg)](https://github.com/MiroKaku/Musa.Core/actions)
[![nuget](https://img.shields.io/nuget/v/Musa.Core)](https://www.nuget.org/packages/Musa.Core/)
[![LICENSE](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/MiroKaku/Musa.Core/blob/main/LICENSE)
![Visual Studio](https://img.shields.io/badge/Visual%20Studio-2022-purple.svg)
![Windows](https://img.shields.io/badge/Windows-10+-orange.svg)
![Platform](https://img.shields.io/badge/Windows-X86%7CX64%7CARM64-%23FFBCD9)

* [简体中文](https://github.com/MiroKaku/Musa.Core/blob/main/README.zh-CN.md)

## Introduction

> **Warning**
> 
> Musa.Core is in beta testing...

Musa.Core is a derivative of the underlying API implementation of [Musa.Runtime](https://github.com/MiroKaku/Musa.Runtime) (formerly [ucxxrt](https://github.com/MiroKaku/ucxxrt)).

Use ntdll/ntoskrnl to implement Kernel32, Advapi32 and other APIs. It includes user-mode and kernel-mode.

Recommended for use with [Musa.Veil](https://github.com/MiroKaku/Veil).

## How to use

Right click on the project, select "Manage NuGet Packages".
Search for `Musa.Core`, choose the version that suits you, and then click "Install".

Or

If your project template uses [Mile.Project.Windows](https://github.com/ProjectMile/Mile.Project.Windows), you can add the following code directly to your `.vcxproj` file:

```XML
  <ItemGroup>
    <PackageReference Include="Musa.Core">
      <!-- Expected version -->
      <Version>0.1.0</Version>
    </PackageReference>
  </ItemGroup>
```

### Header-only mode

Add the following code to your `.vcxproj` file:

```XML
  <PropertyGroup>
    <MusaCoreOnlyHeader>true</MusaCoreOnlyHeader>
  </PropertyGroup>
```

This mode will not automatically import lib files.

## Feature

- [x] All ZwRoutines supported by the current system can be used directly.
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

- [x] Support part of RtlXxxx API.
- [x] Support part of KernelBase API.
- [ ] Support part of Advapi32 API.

## Progress
See [Project](https://github.com/users/MiroKaku/projects/1/views/1)

## Acknowledgements

Thanks to [JetBrains](https://www.jetbrains.com/?from=meesong) for providing free licenses such as [Resharper C++](https://www.jetbrains.com/resharper-cpp/?from=meesong) for my open-source projects.

[<img src="https://resources.jetbrains.com/storage/products/company/brand/logos/ReSharperCPP_icon.png" alt="ReSharper C++ logo." width=200>](https://www.jetbrains.com/?from=meesong)

## Thanks & References
* Thanks: The scheme to export ZwRoutines is provided by @[xiaobfly](https://github.com/xiaobfly).
* References: [systeminformer](https://github.com/winsiderss/systeminformer)/phnt
* References: [Windows_OS_Internals_Curriculum_Resource_Kit-ACADEMIC](https://github.com/MeeSong/Windows_OS_Internals_Curriculum_Resource_Kit-ACADEMIC)
