#include "../micore.hpp"
#include "../miutil.hpp"

EXTERN_C_START

#define _MI_EXPAND_ARGS(Args) Args

#define _MI_FOR_EACH_ARGS_1(What, ArgT, ...) What(ArgT, 1)
#define _MI_FOR_EACH_ARGS_2(What, ArgT, ...) What(ArgT, 2), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_1(What,  __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_3(What, ArgT, ...) What(ArgT, 3), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_2(What,  __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_4(What, ArgT, ...) What(ArgT, 4), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_3(What,  __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_5(What, ArgT, ...) What(ArgT, 5), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_4(What,  __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_6(What, ArgT, ...) What(ArgT, 6), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_5(What,  __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_7(What, ArgT, ...) What(ArgT, 7), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_6(What,  __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_8(What, ArgT, ...) What(ArgT, 8), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_7(What,  __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_9(What, ArgT, ...) What(ArgT, 9), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_8(What,  __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_10(What,ArgT, ...) What(ArgT,10), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_9(What,  __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_11(What,ArgT, ...) What(ArgT,11), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_10(What, __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_12(What,ArgT, ...) What(ArgT,12), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_11(What, __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_13(What,ArgT, ...) What(ArgT,13), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_12(What, __VA_ARGS__))


#define _MI_PARAM_NAME(ArgT,N) _ ## N
#define _MI_PARAM_DEFINE(ArgT, N) ArgT _MI_PARAM_NAME(ArgT, N)

#define _MI_PARAM_NAME_FOR_EACH(N,...) \
    _MI_EXPAND_ARGS(_VEIL_CONCATENATE(_MI_FOR_EACH_ARGS_, N)(_MI_PARAM_NAME, __VA_ARGS__))

#define _MI_PARAM_DEFINE_FOR_EACH(N,...) \
    _MI_EXPAND_ARGS(_VEIL_CONCATENATE(_MI_FOR_EACH_ARGS_, N)(_MI_PARAM_DEFINE, __VA_ARGS__))


#define _MI_COUNTOF_ARGS_HELPER(\
    _0,  _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9,  \
    _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, \
    _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, \
    _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, \
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, \
    _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, \
    _60, _61, _62, _63, _64, N, ...) N

#define _MI_COUNTOF_ARGS(...) _MI_EXPAND_ARGS(_MI_COUNTOF_ARGS_HELPER(0, __VA_ARGS__,\
    64, 63, 62, 61, 60, \
    59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
    49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
    39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
    29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
    19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
    9,  8,  7,  6,  5,  4,  3,  2,  1,  0))

#if defined _M_IX86
#define _MI_SIZEOF_ARGS(...) _MI_EXPAND_ARGS(_MI_COUNTOF_ARGS_HELPER(0, __VA_ARGS__,\
    256, 252, 248, 244, 240, \
    236, 232, 228, 224, 220, 216, 212, 208, 204, 200, \
    196, 192, 188, 184, 180, 176, 172, 168, 164, 160, \
    156, 152, 148, 144, 140, 136, 132, 128, 124, 120, \
    116, 112, 108, 104, 100, 96,  92,  88,  84,  80, \
    76,  72,  68,  64,  60,  56,  52,  48,  44,  40, \
    36,  32,  28,  24,  20,  16,  12,   8,   4,   0))

#define _MI_DEFINE_STUB(ReturnT, ConventionT, FunctionT,...) \
    ReturnT ConventionT MI_NAME(FunctionT)(_MI_PARAM_DEFINE_FOR_EACH(_MI_EXPAND_ARGS(_MI_COUNTOF_ARGS(__VA_ARGS__)),__VA_ARGS__)) \
    { \
        constexpr auto NameHash = Util::Fnv1aHash(_VEIL_STRINGIZE(FunctionT)); \
        if (const auto Routine  = static_cast<decltype(::FunctionT)*>(GetZwRoutineAddress(NameHash))) { \
            return Routine(_MI_PARAM_NAME_FOR_EACH(_MI_EXPAND_ARGS(_MI_COUNTOF_ARGS(__VA_ARGS__)), __VA_ARGS__)); \
        } \
        return STATUS_NOT_SUPPORTED; \
    } \
    _VEIL_DEFINE_IAT_RAW_SYMBOL(FunctionT ## @ ## _MI_EXPAND_ARGS(_MI_SIZEOF_ARGS(__VA_ARGS__)), MI_NAME(FunctionT))
#else
#define _MI_DEFINE_STUB(ReturnT, ConventionT, FunctionT,...) \
    ReturnT ConventionT MI_NAME(FunctionT)(_MI_PARAM_DEFINE_FOR_EACH(_MI_EXPAND_ARGS(_MI_COUNTOF_ARGS(__VA_ARGS__)),__VA_ARGS__)) \
    { \
        constexpr auto NameHash = Util::Fnv1aHash(_VEIL_STRINGIZE(FunctionT)); \
        if (const auto Routine  = static_cast<decltype(::FunctionT)*>(GetZwRoutineAddress(NameHash))) { \
            return Routine(_MI_PARAM_NAME_FOR_EACH(_MI_EXPAND_ARGS(_MI_COUNTOF_ARGS(__VA_ARGS__)), __VA_ARGS__)); \
        } \
        return STATUS_NOT_SUPPORTED; \
    } \
    _VEIL_DEFINE_IAT_SYMBOL(FunctionT, MI_NAME(FunctionT))
#endif

namespace Mi
{
    //template<typename... U>
    //NTSTATUS NTAPI syscall(void* Routine, U ...Args)
    //{
    //    return reinterpret_cast<NTSTATUS(NTAPI*)(U...)>(Routine)(Args...);
    //}

    _MI_DEFINE_STUB(NTSTATUS, NTAPI, ZwAccessCheck,
        PSECURITY_DESCRIPTOR,
        HANDLE,
        ACCESS_MASK,
        PGENERIC_MAPPING,
        PPRIVILEGE_SET,
        PULONG,
        PACCESS_MASK,
        PNTSTATUS);

    _MI_DEFINE_STUB(NTSTATUS, NTAPI, ZwWorkerFactoryWorkerReady,
        HANDLE);

    _MI_DEFINE_STUB(NTSTATUS, NTAPI, ZwAcceptConnectPort,
        PHANDLE,
        PVOID,
        PPORT_MESSAGE,
        BOOLEAN,
        PPORT_VIEW,
        PREMOTE_PORT_VIEW);

    _MI_DEFINE_STUB(NTSTATUS, NTAPI, ZwMapUserPhysicalPagesScatter,
        PVOID*,
        ULONG_PTR,
        PULONG_PTR);

    _MI_DEFINE_STUB(NTSTATUS, NTAPI, ZwWaitForSingleObject,
        HANDLE,
        BOOLEAN,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(NTSTATUS, NTAPI, ZwCallbackReturn,
        PVOID,
        ULONG,
        NTSTATUS);

    _MI_DEFINE_STUB(NTSTATUS, NTAPI, ZwReadFile,
        HANDLE,
        HANDLE,
        PIO_APC_ROUTINE,
        PVOID,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        PLARGE_INTEGER,
        PULONG);

    _MI_DEFINE_STUB(NTSTATUS, NTAPI, ZwDeviceIoControlFile,
        HANDLE,
        HANDLE,
        PIO_APC_ROUTINE,
        PVOID,
        PIO_STATUS_BLOCK,
        ULONG,
        PVOID,
        ULONG,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(NTSTATUS, NTAPI, ZwWriteFile,
        HANDLE,
        HANDLE,
        PIO_APC_ROUTINE,
        PVOID,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        PLARGE_INTEGER,
        PULONG);

    _MI_DEFINE_STUB(NTSTATUS, NTAPI, ZwRemoveIoCompletion,
        HANDLE,
        PVOID*,
        PVOID*,
        PIO_STATUS_BLOCK,
        PLARGE_INTEGER);
    
    _MI_DEFINE_STUB(NTSTATUS, NTAPI, ZwReleaseSemaphore,
        HANDLE,
        LONG,
        PLONG);

    _MI_DEFINE_STUB(NTSTATUS, NTAPI, ZwReplyWaitReceivePort,
        HANDLE,
        PVOID*,
        PPORT_MESSAGE,
        PPORT_MESSAGE);



}

#undef _MI_DEFINE_STUB

EXTERN_C_END
