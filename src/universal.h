#pragma once
#pragma warning(suppress: 4117)
#define _KERNEL_MODE    1
#define POOL_NX_OPTIN   1
#define POOL_ZERO_DOWN_LEVEL_SUPPORT 1

#include <Veil/Veil.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

EXTERN_C_START
extern uintptr_t __security_cookie;
constexpr unsigned long MI_TAG = '-iM-';
EXTERN_C_END

#define MI_NAME(name) _VEIL_CONCATENATE(_Mi_, name)
