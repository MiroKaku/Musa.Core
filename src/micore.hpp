#pragma once

#define MICORE_API __stdcall

namespace Mi
{
    EXTERN_C_START

    NTSTATUS MICORE_API MiCoreStartup ();
    NTSTATUS MICORE_API MiCoreShutdown();

    EXTERN_C_END
}
