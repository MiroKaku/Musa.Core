// unnecessary, fix ReSharper's code analysis.
#pragma warning(suppress: 4117)
#define _KERNEL_MODE 1

#include <Veil/Veil.h>
#include <MiCore/MiCore.h>


// Logging
#ifdef _DEBUG
#define MiLOG(fmt, ...) DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, \
    "[Mi][%s():%u] " fmt "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__)
#else
#define MiLOG(...)
#endif


EXTERN_C DRIVER_INITIALIZE DriverEntry;
EXTERN_C DRIVER_UNLOAD     DriverUnload;


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, DriverUnload)
#endif


EXTERN_C
NTSTATUS NTAPI RtlFindAndFormatMessage(
    _In_     DWORD      Flags,
    _In_opt_ LPCVOID    Source,
    _In_     DWORD      MessageId,
    _In_     DWORD      LanguageId,
    _When_((Flags& FORMAT_MESSAGE_ALLOCATE_BUFFER) != 0, _At_((LPWSTR*)Buffer, _Outptr_result_z_))
    _When_((Flags& FORMAT_MESSAGE_ALLOCATE_BUFFER) == 0, _Out_writes_z_(*Size))
    LPWSTR     Buffer,
    _Inout_  DWORD* Size,
    _In_opt_ va_list* Arguments
);

namespace Main
{
    EXTERN_C VOID DriverUnload(
        _In_ PDRIVER_OBJECT  DriverObject
    )
    {
        UNREFERENCED_PARAMETER(DriverObject);

        PAGED_CODE();

        (void)MiCoreShutdown();
    }

    EXTERN_C NTSTATUS DriverEntry(
        _In_ PDRIVER_OBJECT  DriverObject,
        _In_ PUNICODE_STRING RegistryPath
    )
    {
        UNREFERENCED_PARAMETER(DriverObject);
        UNREFERENCED_PARAMETER(RegistryPath);

        PAGED_CODE();

        NTSTATUS Status;

        do {
            DriverObject->DriverUnload = Main::DriverUnload;

            Status = MiCoreStartup(DriverObject, RegistryPath);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            char* Message = nullptr;

            // TODO: Fix Me, RtlFindMessage can't access ntdll.
            const unsigned long Chars =
                FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr, (DWORD)ERROR_INVALID_PARAMETER, 0, reinterpret_cast<char*>(&Message), 0, nullptr);
            if (Chars) {
                MiLOG("STATUS_INVALID_PARAMETER (%s)", Message);
            }
            else {
                MiLOG("STATUS_INVALID_PARAMETER format failed.");
            }

        } while (false);

        if (!NT_SUCCESS(Status)) {
            Main::DriverUnload(DriverObject);
        }

        return Status;
    }
}
