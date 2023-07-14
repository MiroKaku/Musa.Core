#pragma warning(suppress: 4117)
#define _KERNEL_MODE 1
#include <veil/veil.h>
#include <src/micore.hpp>


#define LOG(Format, ...) DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, \
    "[micore] [" __FUNCTION__ ":%u]: " Format "\n", __LINE__, ## __VA_ARGS__)


namespace Main
{
    EXTERN_C_START

    static ULONG FlsIndex = FLS_OUT_OF_INDEXES;

    NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
    {
        UNREFERENCED_PARAMETER(RegistryPath);

        NTSTATUS Status;

        do {

            DriverObject->DriverUnload = [](PDRIVER_OBJECT)
            {
                //if (FlsIndex != FLS_OUT_OF_INDEXES) {
                //    FlsFree(FlsIndex);
                //}

                (void)MiCoreShutdown();
            };

            Status = MiCoreStartup(DriverObject);
            if (!NT_SUCCESS(Status)) {
                break;
            }

            FlsIndex = FlsAlloc([](PVOID FlsData)
            {
                LOG("[%p] FlsCallback -> %hs", PsGetCurrentThreadId(), FlsData);
            });

            LOG("[%p] FlsAlloc -> %u", PsGetCurrentThreadId(), FlsIndex);

            if (!FlsSetValue(FlsIndex, (PVOID)"Test.FlsData.Main")) {
                LOG("[%p] FlsSetValue error %u", PsGetCurrentThreadId(), GetLastError());
            }

            CLIENT_ID           ClientId{};
            OBJECT_ATTRIBUTES   ObjectAttributes;
            InitializeObjectAttributes(&ObjectAttributes, nullptr, OBJ_KERNEL_HANDLE, nullptr, nullptr);

            HANDLE ThreadHandle = nullptr;
            Status = PsCreateSystemThread(&ThreadHandle, THREAD_ALL_ACCESS, &ObjectAttributes, nullptr, &ClientId, [](PVOID)
            {
                const auto FlsData = FlsGetValue(FlsIndex);
                ASSERT(FlsData == nullptr);

                LOG("[%p] FlsGetValue -> %hs", PsGetCurrentThreadId(), FlsData);

                FlsSetValue(FlsIndex, (PVOID)"Test.FlsData.Thread");

            }, nullptr);

            if (NT_SUCCESS(Status)) {
                (void)ZwClose(ThreadHandle);

                LOG("[%p] PsCreateSystemThread -> %p", PsGetCurrentThreadId(), ClientId.UniqueThread);
            }
            else {
                LOG("[%p] PsCreateSystemThread error %u", PsGetCurrentThreadId(), GetLastError());
            }

        } while (false);

        if (!NT_SUCCESS(Status)) {
            (void)MiCoreShutdown();
        }

        return Status;
    }

    EXTERN_C_END
}
