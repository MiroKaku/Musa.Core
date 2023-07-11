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

#if defined _M_IX86
#define MI_IAT_SYMBOL(name, stack) _VEIL_DEFINE_IAT_RAW_SYMBOL(name ## @ ## stack, MI_NAME(name))
#else
#define MI_IAT_SYMBOL(name, stack) _VEIL_DEFINE_IAT_SYMBOL(name, MI_NAME(name))
#endif

#include "micore.hpp"
