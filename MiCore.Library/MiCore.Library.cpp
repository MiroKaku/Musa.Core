#include <Veil/Veil.h>


namespace Main
{
    EXTERN_C BOOL WINAPI DllMain(
        _In_     HINSTANCE  Instance,
        _In_     DWORD      Reason,
        _In_opt_ LPVOID     Reserved
    )
    {
        UNREFERENCED_PARAMETER(Instance);
        UNREFERENCED_PARAMETER(Reason);
        UNREFERENCED_PARAMETER(Reserved);

        return TRUE;
    }

}
