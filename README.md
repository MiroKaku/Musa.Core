# [Mi Core](https://github.com/MiroKaku/micore)

[![Actions Status](https://github.com/MiroKaku/micore/workflows/build/badge.svg)](https://github.com/MiroKaku/micore/actions)
[![LICENSE](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/MiroKaku/micore/blob/main/LICENSE)
![Visual Studio](https://img.shields.io/badge/Visual%20Studio-2022-purple.svg)
![Windows](https://img.shields.io/badge/Windows-7+-orange.svg)
![Platform](https://img.shields.io/badge/Windows-X86%7CX64%7CARM64-%23FFBCD9)

* [简体中文](https://github.com/MiroKaku/micore/blob/main/README.zh-CN.md)

## Introduction

> **Warning**
> 
> MiCore is still in development stage ...

MiCore is a derivative of the underlying API implementation of MiUCRT (formerly [ucxxrt](https://github.com/MiroKaku/ucxxrt)).

It is mainly the implementation of user-level functions in the kernel on the Windows platform.
Recommended for use with [Veil](https://github.com/MiroKaku/Veil).

## Thanks & References
* Thanks: The scheme to export ZwRoutines is provided by @[xiaobfly](https://github.com/xiaobfly).
* References: [systeminformer](https://github.com/winsiderss/systeminformer)/phnt
* References: [Windows_OS_Internals_Curriculum_Resource_Kit-ACADEMIC](https://github.com/MeeSong/Windows_OS_Internals_Curriculum_Resource_Kit-ACADEMIC)

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

- [ ] Support part of RtlXxxx API.
- [ ] Support part of KernelBase API.
- [ ] Support part of Advapi32 API.

## Progress
See [Project](https://github.com/users/MiroKaku/projects/1/views/1)
