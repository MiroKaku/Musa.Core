#include <Veil/Veil.h>
#include <MiCore/MiCore.h>


namespace Main
{
    extern"C" int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
    {
        UNREFERENCED_PARAMETER(argc);
        UNREFERENCED_PARAMETER(argv);
        UNREFERENCED_PARAMETER(envp);

        NTSTATUS Status;

        do {
            Status = MiCoreStartup();
            if (!NT_SUCCESS(Status)) {
                break;
            }

            (void)getchar();

            Status = MiCoreShutdown();
            if (!NT_SUCCESS(Status)) {
                break;
            }

        } while (false);

        return Status;
    }

}
