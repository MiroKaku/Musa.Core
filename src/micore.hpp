#pragma once

#if !defined(_AMD64_) && !defined(_X86_) && !defined(_ARM64_)
#error Unsupported architecture
#endif

#define MICORE_API __stdcall

namespace Mi
{
    EXTERN_C_START

    NTSTATUS MICORE_API MiCoreStartup ();
    NTSTATUS MICORE_API MiCoreShutdown();

    EXTERN_C_END
}
