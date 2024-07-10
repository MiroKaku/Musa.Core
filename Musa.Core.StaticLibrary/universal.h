#pragma once
#pragma warning(disable: 6101)

// Config Macro
#define POOL_NX_OPTIN 1
#define POOL_ZERO_DOWN_LEVEL_SUPPORT 1
#define RTL_USE_AVL_TABLES

// System Header
#include <Veil.h>

// C/C++  Header
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Local  Header
#include "Musa.Core/Musa.Core.h"

// Global Variable
EXTERN_C_START
extern uintptr_t __security_cookie;
constexpr unsigned long MUSA_TAG = 'asuM';
EXTERN_C_END

// Global Macro
#define MUSA_NAME_PUBLIC(name)  _VEIL_CONCATENATE(_Musa_, name)
#define MUSA_NAME_PRIVATE(name) _VEIL_CONCATENATE(_Musa_Private_, name)
#define MUSA_NAME MUSA_NAME_PUBLIC

#if defined _M_IX86
#define MUSA_IAT_SYMBOL(name, stack) _VEIL_DEFINE_IAT_RAW_SYMBOL(name ## @ ## stack, MUSA_NAME(name))
#else
#define MUSA_IAT_SYMBOL(name, stack) _VEIL_DEFINE_IAT_SYMBOL(name, MUSA_NAME(name))
#endif

#define PAGED_CODE()        NOP_FUNCTION;
#define PAGED_CODE_LOCKED() NOP_FUNCTION;

// Logging
#ifdef _DEBUG
#define MusaLOG(fmt, ...) DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_ERROR_LEVEL, \
    "[Musa.Core][%s():%u] " fmt "\n", __FUNCTION__, __LINE__, ## __VA_ARGS__)
#else
#define MusaLOG(...)
#endif
