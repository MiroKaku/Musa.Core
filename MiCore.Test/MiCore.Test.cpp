#include <Veil/Veil.h>
#include <MiCore/MiCore.h>


// Logging
#ifdef _DEBUG
#define MiLOG(fmt, ...) printf("[Mi][%s():%u]" fmt "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__)
#else
#define MiLOG(...)
#endif


namespace Main
{
    extern"C" int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
    {
        UNREFERENCED_PARAMETER(argc);
        UNREFERENCED_PARAMETER(argv);
        UNREFERENCED_PARAMETER(envp);

        NTSTATUS Status = STATUS_SUCCESS;

        __try {
            Status = MiCoreStartup();
            if (!NT_SUCCESS(Status)) {
                __leave;
            }

            LARGE_INTEGER SystemTime{};
            Status = ZwQuerySystemTime(&SystemTime);
            if (!NT_SUCCESS(Status)) {
                __leave;
            }

            Status = RtlSystemTimeToLocalTime(&SystemTime, &SystemTime);
            if (!NT_SUCCESS(Status)) {
                __leave;
            }

            TIME_FIELDS Time{};
            RtlTimeToTimeFields(&SystemTime, &Time);

            MiLOG("Loading time is %04d/%02d/%02d %02d:%02d:%02d",
                Time.Year, Time.Month, Time.Day,
                Time.Hour, Time.Minute, Time.Second);

            (void)getchar();
        }
        __finally {
            (void)MiCoreShutdown();
        }

        return Status;
    }

}
