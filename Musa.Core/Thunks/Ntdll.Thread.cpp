#include "Ntdll.Thread.Private.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MUSA_NAME(RtlExitUserThread))
#endif

EXTERN_C_START
namespace Musa
{

#ifdef _KERNEL_MODE
    _IRQL_requires_max_(PASSIVE_LEVEL)
    VOID NTAPI MUSA_NAME(RtlExitUserThread)(
        _In_ NTSTATUS ExitStatus
        )
    {
        PAGED_CODE();

        (void)PsTerminateSystemThread(ExitStatus);
    }
    MUSA_IAT_SYMBOL(RtlExitUserThread, 4);
#endif


}
EXTERN_C_END
