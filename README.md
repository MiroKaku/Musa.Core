# [Musa.Core](https://github.com/MiroKaku/Musa.Core)

[![Actions Status](https://github.com/MiroKaku/Musa.Core/workflows/build/badge.svg)](https://github.com/MiroKaku/Musa.Core/actions)
[![LICENSE](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/MiroKaku/Musa.Core/blob/main/LICENSE)
![Visual Studio](https://img.shields.io/badge/Visual%20Studio-2022-purple.svg)
![Windows](https://img.shields.io/badge/Windows-10+-orange.svg)
![Platform](https://img.shields.io/badge/Windows-X86%7CX64%7CARM64-%23FFBCD9)

* [简体中文](https://github.com/MiroKaku/Musa.Core/blob/main/README.zh-CN.md)

## Introduction

> **Warning**
> 
> Musa.Core is still in development stage ...

Musa.Core is a derivative of the underlying API implementation of Musa.Runtime (formerly [ucxxrt](https://github.com/MiroKaku/ucxxrt)).

Use ntdll/ntoskrnl to implement Kernel32, Advapi32 and other APIs. It includes user-mode and kernel-mode.

Recommended for use with [Musa.Veil](https://github.com/MiroKaku/Veil).

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

- [x] Support part of RtlXxxx API.
- [x] Support part of KernelBase API.
- [ ] Support part of Advapi32 API.

## Progress
See [Project](https://github.com/users/MiroKaku/projects/1/views/1)
