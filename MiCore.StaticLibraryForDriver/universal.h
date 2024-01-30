#pragma once
#pragma warning(disable: 6101 28101 28167)

// Config Macro
#define POOL_NX_OPTIN 1
#define POOL_ZERO_DOWN_LEVEL_SUPPORT 1
#define RTL_USE_AVL_TABLES

// unnecessary, fix ReSharper's code analysis.
#pragma warning(suppress: 4117)
#define _KERNEL_MODE 1

// System Header
#include <Veil/Veil.h>

// C/C++  Header
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Local  Header
#include "MiCore/MiCore.h"

// Global Variable
EXTERN_C_START
extern uintptr_t __security_cookie;
constexpr unsigned long MI_TAG = '-iM-';
EXTERN_C_END

// Global Macro
#define MI_NAME_PUBLIC(name)  _VEIL_CONCATENATE(_Mi_, name)
#define MI_NAME_PRIVATE(name) _VEIL_CONCATENATE(_Mi_Private_, name)
#define MI_NAME MI_NAME_PUBLIC

#if defined _M_IX86
#define MI_IAT_SYMBOL(name, stack) _VEIL_DEFINE_IAT_RAW_SYMBOL(name ## @ ## stack, MI_NAME(name))
#else
#define MI_IAT_SYMBOL(name, stack) _VEIL_DEFINE_IAT_SYMBOL(name, MI_NAME(name))
#endif

// Logging
#ifdef _DEBUG
#define MiLOG(fmt, ...) DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, \
    "[Mi][%s():%u]" fmt "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__)
#else
#define MiLOG(...)
#endif
