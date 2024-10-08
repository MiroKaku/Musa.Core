#include <Veil.h>
#include <Musa.Core/Musa.Core.h>


// Logging
#ifdef _DEBUG
#define MusaLOG(fmt, ...) printf("[Musa.Core][%s():%u] " fmt "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__)
#else
#define MusaLOG(...)
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
            Status = MusaCoreStartup();
            if (!NT_SUCCESS(Status)) {
                __leave;
            }

            char* Message = nullptr;

            const unsigned long Chars =
                FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    nullptr, (DWORD)ERROR_INVALID_PARAMETER, 0, reinterpret_cast<char*>(&Message), 0, nullptr);
            if (Chars) {
                MusaLOG("STATUS_INVALID_PARAMETER (%s)", Message);
            }
            else {
                MusaLOG("STATUS_INVALID_PARAMETER format failed.");
            }

            (void)getchar();
        }
        __finally {
            (void)MusaCoreShutdown();
        }

        return Status;
    }

}
