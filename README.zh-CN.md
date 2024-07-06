# [Mi Core](https://github.com/MiroKaku/micore)

[![Actions Status](https://github.com/MiroKaku/micore/workflows/build/badge.svg)](https://github.com/MiroKaku/micore/actions)
[![LICENSE](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/MiroKaku/micore/blob/main/LICENSE)
![Visual Studio](https://img.shields.io/badge/Visual%20Studio-2022-purple.svg)
![Windows](https://img.shields.io/badge/Windows-10+-orange.svg)
![Platform](https://img.shields.io/badge/Windows-X86%7CX64%7CARM64-%23FFBCD9)

* [English](https://github.com/MiroKaku/micore/blob/main/README.md)

## 简介

> **Warning**
> 
> MiCore 处于开发阶段中 ...

MiCore 是 MiUCRT (原 [ucxxrt](https://github.com/MiroKaku/ucxxrt)) 的底层API实现的衍生物。

主要是 Windows 平台上用户层函数在内核的实现。推荐与 [Veil](https://github.com/MiroKaku/Veil) 一起食用。

## 感谢 & 参考
* 感谢：Zw 例程获取方案由 @[xiaobfly](https://github.com/xiaobfly) 提供。
* 参考：[systeminformer](https://github.com/winsiderss/systeminformer)/phnt
* 参考：[Windows_OS_Internals_Curriculum_Resource_Kit-ACADEMIC](https://github.com/MeeSong/Windows_OS_Internals_Curriculum_Resource_Kit-ACADEMIC)

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

            Status = MiCoreStartup(DriverObject, RegistryPath);
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

            MiLOG("Loading time is %04d/%02d/%02d %02d:%02d:%02d",
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
