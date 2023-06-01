#include "../micore.hpp"
#include "../miutil.hpp"

EXTERN_C_START

#define _MI_EXPAND_ARGS(Args) Args

#define _MI_FOR_EACH_ARGS_0(...)
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
#define _MI_FOR_EACH_ARGS_14(What,ArgT, ...) What(ArgT,14), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_13(What, __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_15(What,ArgT, ...) What(ArgT,15), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_14(What, __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_16(What,ArgT, ...) What(ArgT,16), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_15(What, __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_17(What,ArgT, ...) What(ArgT,17), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_16(What, __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_18(What,ArgT, ...) What(ArgT,18), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_17(What, __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_19(What,ArgT, ...) What(ArgT,19), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_18(What, __VA_ARGS__))
#define _MI_FOR_EACH_ARGS_20(What,ArgT, ...) What(ArgT,20), _MI_EXPAND_ARGS(_MI_FOR_EACH_ARGS_19(What, __VA_ARGS__))


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

#define _MI_DEFINE_STUB(DefaultT, ReturnT, ConventionT, FunctionT,...) \
    ReturnT ConventionT MI_NAME(FunctionT)(_MI_PARAM_DEFINE_FOR_EACH(_MI_EXPAND_ARGS(_MI_COUNTOF_ARGS(__VA_ARGS__)),__VA_ARGS__)) \
    { \
        constexpr auto NameHash = Util::Fnv1aHash(_VEIL_STRINGIZE(FunctionT)); \
        if (const auto Routine  = static_cast<decltype(::FunctionT)*>(GetZwRoutineAddress(NameHash))) { \
            return Routine(_MI_PARAM_NAME_FOR_EACH(_MI_EXPAND_ARGS(_MI_COUNTOF_ARGS(__VA_ARGS__)), __VA_ARGS__)); \
        } \
        if constexpr(!Util::IS_SAME_TYPE<void, ReturnT>) {\
            return DefaultT; \
        } \
    } \
    _VEIL_DEFINE_IAT_RAW_SYMBOL(FunctionT ## @ ## _MI_EXPAND_ARGS(_MI_SIZEOF_ARGS(__VA_ARGS__)), MI_NAME(FunctionT))

#define _MI_DEFINE_STUB_ZERO_ARGS(DefaultT, ReturnT, ConventionT, FunctionT) \
    ReturnT ConventionT MI_NAME(FunctionT)() \
    { \
        constexpr auto NameHash = Util::Fnv1aHash(_VEIL_STRINGIZE(FunctionT)); \
        if (const auto Routine  = static_cast<decltype(::FunctionT)*>(GetZwRoutineAddress(NameHash))) { \
            return Routine(); \
        } \
        if constexpr(!Util::IS_SAME_TYPE<void, ReturnT>) {\
            return DefaultT; \
        } \
    } \
    _VEIL_DEFINE_IAT_RAW_SYMBOL(FunctionT ## @ ## 0, MI_NAME(FunctionT))
#else
#define _MI_DEFINE_STUB(DefaultT, ReturnT, ConventionT, FunctionT,...) \
    ReturnT ConventionT MI_NAME(FunctionT)(_MI_PARAM_DEFINE_FOR_EACH(_MI_EXPAND_ARGS(_MI_COUNTOF_ARGS(__VA_ARGS__)),__VA_ARGS__)) \
    { \
        constexpr auto NameHash = Util::Fnv1aHash(_VEIL_STRINGIZE(FunctionT)); \
        if (const auto Routine  = static_cast<decltype(::FunctionT)*>(GetZwRoutineAddress(NameHash))) { \
            return Routine(_MI_PARAM_NAME_FOR_EACH(_MI_EXPAND_ARGS(_MI_COUNTOF_ARGS(__VA_ARGS__)), __VA_ARGS__)); \
        } \
        if constexpr(!Util::IS_SAME_TYPE<void, ReturnT>) {\
            return DefaultT; \
        } \
    } \
    _VEIL_DEFINE_IAT_SYMBOL(FunctionT, MI_NAME(FunctionT))

#define _MI_DEFINE_STUB_ZERO_ARGS(DefaultT, ReturnT, ConventionT, FunctionT) \
    ReturnT ConventionT MI_NAME(FunctionT)() \
    { \
        constexpr auto NameHash = Util::Fnv1aHash(_VEIL_STRINGIZE(FunctionT)); \
        if (const auto Routine  = static_cast<decltype(::FunctionT)*>(GetZwRoutineAddress(NameHash))) { \
            return Routine(); \
        } \
        if constexpr(!Util::IS_SAME_TYPE<void, ReturnT>) {\
            return DefaultT; \
        } \
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

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAcceptConnectPort,
        PHANDLE,
        PVOID,
        PPORT_MESSAGE,
        BOOLEAN,
        PPORT_VIEW,
        PREMOTE_PORT_VIEW);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAccessCheck,
        PSECURITY_DESCRIPTOR,
        HANDLE,
        ACCESS_MASK,
        PGENERIC_MAPPING,
        PPRIVILEGE_SET,
        PULONG,
        PACCESS_MASK,
        PNTSTATUS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAccessCheckAndAuditAlarm,
        PUNICODE_STRING,
        PVOID,
        PUNICODE_STRING,
        PUNICODE_STRING,
        PSECURITY_DESCRIPTOR,
        ACCESS_MASK,
        PGENERIC_MAPPING,
        BOOLEAN,
        PACCESS_MASK,
        PNTSTATUS,
        PBOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAccessCheckByType,
        PSECURITY_DESCRIPTOR,
        PSID,
        HANDLE,
        ACCESS_MASK,
        POBJECT_TYPE_LIST,
        ULONG,
        PGENERIC_MAPPING,
        PPRIVILEGE_SET,
        PULONG,
        PACCESS_MASK,
        PNTSTATUS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAccessCheckByTypeAndAuditAlarm,
        PUNICODE_STRING,
        PVOID,
        PUNICODE_STRING,
        PUNICODE_STRING,
        PSECURITY_DESCRIPTOR,
        PSID,
        ACCESS_MASK,
        AUDIT_EVENT_TYPE,
        ULONG,
        POBJECT_TYPE_LIST,
        ULONG,
        PGENERIC_MAPPING,
        BOOLEAN,
        PACCESS_MASK,
        PNTSTATUS,
        PBOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAccessCheckByTypeResultList,
        PSECURITY_DESCRIPTOR,
        PSID,
        HANDLE,
        ACCESS_MASK,
        POBJECT_TYPE_LIST,
        ULONG,
        PGENERIC_MAPPING,
        PPRIVILEGE_SET,
        PULONG,
        PACCESS_MASK,
        PNTSTATUS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAccessCheckByTypeResultListAndAuditAlarm,
        PUNICODE_STRING,
        PVOID,
        PUNICODE_STRING,
        PUNICODE_STRING,
        PSECURITY_DESCRIPTOR,
        PSID,
        ACCESS_MASK,
        AUDIT_EVENT_TYPE,
        ULONG,
        POBJECT_TYPE_LIST,
        ULONG,
        PGENERIC_MAPPING,
        BOOLEAN,
        PACCESS_MASK,
        PNTSTATUS,
        PBOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAccessCheckByTypeResultListAndAuditAlarmByHandle,
        PUNICODE_STRING,
        PVOID,
        HANDLE,
        PUNICODE_STRING,
        PUNICODE_STRING,
        PSECURITY_DESCRIPTOR,
        PSID,
        ACCESS_MASK,
        AUDIT_EVENT_TYPE,
        ULONG,
        POBJECT_TYPE_LIST,
        ULONG,
        PGENERIC_MAPPING,
        BOOLEAN,
        PACCESS_MASK,
        PNTSTATUS,
        PBOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAddAtom,
        PWSTR,
        ULONG,
        PRTL_ATOM);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAddAtomEx,
        PWSTR,
        ULONG,
        PRTL_ATOM,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAddBootEntry,
        PBOOT_ENTRY,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAddDriverEntry,
        PEFI_DRIVER_ENTRY,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAdjustGroupsToken,
        HANDLE,
        BOOLEAN,
        PTOKEN_GROUPS,
        ULONG,
        PTOKEN_GROUPS,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAdjustPrivilegesToken,
        HANDLE,
        BOOLEAN,
        PTOKEN_PRIVILEGES,
        ULONG,
        PTOKEN_PRIVILEGES,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAdjustTokenClaimsAndDeviceGroups,
        HANDLE,
        BOOLEAN,
        BOOLEAN,
        BOOLEAN,
        PTOKEN_SECURITY_ATTRIBUTES_INFORMATION,
        PTOKEN_SECURITY_ATTRIBUTES_INFORMATION,
        PTOKEN_GROUPS,
        ULONG,
        PTOKEN_SECURITY_ATTRIBUTES_INFORMATION,
        ULONG,
        PTOKEN_SECURITY_ATTRIBUTES_INFORMATION,
        ULONG,
        PTOKEN_GROUPS,
        PULONG,
        PULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlertResumeThread,
        HANDLE,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlertThread,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlertThreadByThreadId,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAllocateLocallyUniqueId,
        PLUID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAllocateReserveObject,
        PHANDLE,
        POBJECT_ATTRIBUTES,
        MEMORY_RESERVE_TYPE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAllocateUserPhysicalPages,
        HANDLE,
        PULONG_PTR,
        PULONG_PTR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAllocateUserPhysicalPagesEx,
        HANDLE,
        PULONG_PTR,
        PULONG_PTR,
        PMEM_EXTENDED_PARAMETER,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAllocateUuids,
        PULARGE_INTEGER,
        PULONG,
        PULONG,
        PCHAR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAllocateVirtualMemory,
        HANDLE,
        PVOID*,
        ULONG_PTR,
        PSIZE_T,
        ULONG,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAllocateVirtualMemoryEx,
        HANDLE,
        PVOID*,
        PSIZE_T,
        ULONG,
        ULONG,
        PMEM_EXTENDED_PARAMETER,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcAcceptConnectPort,
        PHANDLE,
        HANDLE,
        ULONG,
        POBJECT_ATTRIBUTES,
        PALPC_PORT_ATTRIBUTES,
        PVOID,
        PPORT_MESSAGE,
        PALPC_MESSAGE_ATTRIBUTES,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcCancelMessage,
        HANDLE,
        ULONG,
        PALPC_CONTEXT_ATTR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcConnectPort,
        PHANDLE,
        PUNICODE_STRING,
        POBJECT_ATTRIBUTES,
        PALPC_PORT_ATTRIBUTES,
        ULONG,
        PSID,
        PPORT_MESSAGE,
        PULONG,
        PALPC_MESSAGE_ATTRIBUTES,
        PALPC_MESSAGE_ATTRIBUTES,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcConnectPortEx,
        PHANDLE,
        POBJECT_ATTRIBUTES,
        POBJECT_ATTRIBUTES,
        PALPC_PORT_ATTRIBUTES,
        ULONG,
        PSECURITY_DESCRIPTOR,
        PPORT_MESSAGE,
        PSIZE_T,
        PALPC_MESSAGE_ATTRIBUTES,
        PALPC_MESSAGE_ATTRIBUTES,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcCreatePort,
        PHANDLE,
        POBJECT_ATTRIBUTES,
        PALPC_PORT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcCreatePortSection,
        HANDLE,
        ULONG,
        HANDLE,
        SIZE_T,
        PALPC_HANDLE,
        PSIZE_T);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcCreateResourceReserve,
        HANDLE,
        ULONG,
        SIZE_T,
        PALPC_HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcCreateSectionView,
        HANDLE,
        ULONG,
        PALPC_DATA_VIEW_ATTR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcCreateSecurityContext,
        HANDLE,
        ULONG,
        PALPC_SECURITY_ATTR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcDeletePortSection,
        HANDLE,
        ULONG,
        ALPC_HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcDeleteResourceReserve,
        HANDLE,
        ULONG,
        ALPC_HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcDeleteSectionView,
        HANDLE,
        ULONG,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcDeleteSecurityContext,
        HANDLE,
        ULONG,
        ALPC_HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcDisconnectPort,
        HANDLE,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcImpersonateClientContainerOfPort,
        HANDLE,
        PPORT_MESSAGE,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcImpersonateClientOfPort,
        HANDLE,
        PPORT_MESSAGE,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcOpenSenderProcess,
        PHANDLE,
        HANDLE,
        PPORT_MESSAGE,
        ULONG,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcOpenSenderThread,
        PHANDLE,
        HANDLE,
        PPORT_MESSAGE,
        ULONG,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcQueryInformation,
        HANDLE,
        ALPC_PORT_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcQueryInformationMessage,
        HANDLE,
        PPORT_MESSAGE,
        ALPC_MESSAGE_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcRevokeSecurityContext,
        HANDLE,
        ULONG,
        ALPC_HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcSendWaitReceivePort,
        HANDLE,
        ULONG,
        PPORT_MESSAGE,
        PALPC_MESSAGE_ATTRIBUTES,
        PPORT_MESSAGE,
        PSIZE_T,
        PALPC_MESSAGE_ATTRIBUTES,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAlpcSetInformation,
        HANDLE,
        ALPC_PORT_INFORMATION_CLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwApphelpCacheControl,
        AHC_SERVICE_CLASS,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAreMappedFilesTheSame,
        PVOID,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAssignProcessToJobObject,
        HANDLE,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwAssociateWaitCompletionPacket,
        HANDLE,
        HANDLE,
        HANDLE,
        PVOID,
        PVOID,
        NTSTATUS,
        ULONG_PTR,
        PBOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCallEnclave,
        PENCLAVE_ROUTINE,
        PVOID,
        BOOLEAN,
        PVOID*);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCallbackReturn,
        PVOID,
        ULONG,
        NTSTATUS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCancelIoFile,
        HANDLE,
        PIO_STATUS_BLOCK);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCancelIoFileEx,
        HANDLE,
        PIO_STATUS_BLOCK,
        PIO_STATUS_BLOCK);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCancelSynchronousIoFile,
        HANDLE,
        PIO_STATUS_BLOCK,
        PIO_STATUS_BLOCK);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCancelTimer,
        HANDLE,
        PBOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCancelTimer2,
        HANDLE,
        PT2_CANCEL_PARAMETERS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCancelWaitCompletionPacket,
        HANDLE,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwChangeProcessState,
        HANDLE,
        HANDLE,
        PROCESS_STATE_CHANGE_TYPE,
        PVOID,
        SIZE_T,
        ULONG64);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwChangeThreadState,
        HANDLE,
        HANDLE,
        THREAD_STATE_CHANGE_TYPE,
        PVOID,
        SIZE_T,
        ULONG64);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwClearEvent,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwClose,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCloseObjectAuditAlarm,
        PUNICODE_STRING,
        PVOID,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCommitComplete,
        PUNICODE_STRING,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCommitEnlistment,
        PUNICODE_STRING,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCommitRegistryTransaction,
        HANDLE,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCommitTransaction,
        HANDLE,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCompactKeys,
        ULONG,
        HANDLE*);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCompareObjects,
        HANDLE,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCompareSigningLevels,
        SE_SIGNING_LEVEL,
        SE_SIGNING_LEVEL);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCompareTokens,
        HANDLE,
        HANDLE,
        PBOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCompleteConnectPort,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCompressKey,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwConnectPort,
        PHANDLE,
        PUNICODE_STRING,
        PSECURITY_QUALITY_OF_SERVICE,
        PPORT_VIEW,
        PREMOTE_PORT_VIEW,
        PULONG,
        PVOID,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwContinue,
        PCONTEXT,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwContinueEx,
        PCONTEXT,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwConvertBetweenAuxiliaryCounterAndPerformanceCounter,
        PLARGE_INTEGER,
        PLARGE_INTEGER,
        PLARGE_INTEGER,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCopyFileChunk,
        HANDLE,
        HANDLE,
        HANDLE,
        PIO_STATUS_BLOCK,
        ULONG,
        PLARGE_INTEGER,
        PLARGE_INTEGER,
        PGUID,
        PGUID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateDebugObject,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateDirectoryObject,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateDirectoryObjectEx,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        HANDLE,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateEnclave,
        HANDLE,
        PVOID*,
        ULONG_PTR,
        SIZE_T,
        SIZE_T,
        ULONG,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateEnlistment,
        PHANDLE,
        ACCESS_MASK,
        HANDLE,
        HANDLE,
        POBJECT_ATTRIBUTES,
        ULONG,
        NOTIFICATION_MASK,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateEvent,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        EVENT_TYPE,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateEventPair,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateFile,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        PIO_STATUS_BLOCK,
        PLARGE_INTEGER,
        ULONG,
        ULONG,
        ULONG,
        ULONG,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateIRTimer,
        PHANDLE,
        ACCESS_MASK);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateIoCompletion,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateJobObject,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateJobSet,
        ULONG,
        PJOB_SET_ARRAY,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateKey,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        ULONG,
        PUNICODE_STRING,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateKeyTransacted,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        ULONG,
        PUNICODE_STRING,
        ULONG,
        HANDLE,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateKeyedEvent,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateLowBoxToken,
        PHANDLE,
        HANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        PSID,
        ULONG,
        PSID_AND_ATTRIBUTES,
        ULONG,
        HANDLE*);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateMailslotFile,
        PHANDLE,
        ULONG,
        POBJECT_ATTRIBUTES,
        PIO_STATUS_BLOCK,
        ULONG,
        ULONG,
        ULONG,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateMutant,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateNamedPipeFile,
        PHANDLE,
        ULONG,
        POBJECT_ATTRIBUTES,
        PIO_STATUS_BLOCK,
        ULONG,
        ULONG,
        ULONG,
        ULONG,
        ULONG,
        ULONG,
        ULONG,
        ULONG,
        ULONG,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreatePagingFile,
        PUNICODE_STRING,
        PLARGE_INTEGER,
        PLARGE_INTEGER,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreatePartition,
        HANDLE,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreatePort,
        PHANDLE,
        POBJECT_ATTRIBUTES,
        ULONG,
        ULONG,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreatePrivateNamespace,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        POBJECT_BOUNDARY_DESCRIPTOR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateProcess,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        HANDLE,
        BOOLEAN,
        HANDLE,
        HANDLE,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateProcessEx,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        HANDLE,
        ULONG,
        HANDLE,
        HANDLE,
        HANDLE,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateProcessStateChange,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        HANDLE,
        ULONG64);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateProfile,
        PHANDLE,
        HANDLE,
        PVOID,
        SIZE_T,
        ULONG,
        PULONG,
        ULONG,
        KPROFILE_SOURCE,
        KAFFINITY);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateProfileEx,
        PHANDLE,
        HANDLE,
        PVOID,
        SIZE_T,
        ULONG,
        PULONG,
        ULONG,
        KPROFILE_SOURCE,
        USHORT,
        PGROUP_AFFINITY);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateRegistryTransaction,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateResourceManager,
        PHANDLE,
        ACCESS_MASK,
        HANDLE,
        LPGUID,
        POBJECT_ATTRIBUTES,
        ULONG,
        PUNICODE_STRING);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateSection,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        PLARGE_INTEGER,
        ULONG,
        ULONG,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateSectionEx,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        PLARGE_INTEGER,
        ULONG,
        ULONG,
        HANDLE,
        PMEM_EXTENDED_PARAMETER,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateSemaphore,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        ULONG,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateSymbolicLinkObject,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        PUNICODE_STRING);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateThread,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        HANDLE,
        PCLIENT_ID,
        PCONTEXT,
        PINITIAL_TEB,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateThreadEx,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        HANDLE,
        PVOID,
        PVOID,
        ULONG,
        SIZE_T,
        SIZE_T,
        SIZE_T,
        PPS_ATTRIBUTE_LIST);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateThreadStateChange,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        HANDLE,
        ULONG64);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateTimer,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        TIMER_TYPE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateTimer2,
        PHANDLE,
        PVOID,
        POBJECT_ATTRIBUTES,
        ULONG,
        ACCESS_MASK);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateToken,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        TOKEN_TYPE,
        PLUID,
        PLARGE_INTEGER,
        PTOKEN_USER,
        PTOKEN_GROUPS,
        PTOKEN_PRIVILEGES,
        PTOKEN_OWNER,
        PTOKEN_PRIMARY_GROUP,
        PTOKEN_DEFAULT_DACL,
        PTOKEN_SOURCE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateTokenEx,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        TOKEN_TYPE,
        PLUID,
        PLARGE_INTEGER,
        PTOKEN_USER,
        PTOKEN_GROUPS,
        PTOKEN_PRIVILEGES,
        PTOKEN_SECURITY_ATTRIBUTES_INFORMATION,
        PTOKEN_SECURITY_ATTRIBUTES_INFORMATION,
        PTOKEN_GROUPS,
        PTOKEN_MANDATORY_POLICY,
        PTOKEN_OWNER,
        PTOKEN_PRIMARY_GROUP,
        PTOKEN_DEFAULT_DACL,
        PTOKEN_SOURCE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateTransaction,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        LPGUID,
        HANDLE,
        ULONG,
        ULONG,
        ULONG,
        PLARGE_INTEGER,
        PUNICODE_STRING);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateTransactionManager,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        PUNICODE_STRING,
        ULONG,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateUserProcess,
        PHANDLE,
        PHANDLE,
        ACCESS_MASK,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        POBJECT_ATTRIBUTES,
        ULONG,
        ULONG,
        PVOID,
        PPS_CREATE_INFO,
        PPS_ATTRIBUTE_LIST);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateWaitCompletionPacket,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateWaitablePort,
        PHANDLE,
        POBJECT_ATTRIBUTES,
        ULONG,
        ULONG,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateWnfStateName,
        PWNF_STATE_NAME,
        WNF_STATE_NAME_LIFETIME,
        WNF_DATA_SCOPE,
        BOOLEAN,
        PCWNF_TYPE_ID,
        ULONG,
        PSECURITY_DESCRIPTOR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwCreateWorkerFactory,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        HANDLE,
        HANDLE,
        PVOID,
        PVOID,
        ULONG,
        SIZE_T,
        SIZE_T);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDebugActiveProcess,
        HANDLE,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDebugContinue,
        HANDLE,
        PCLIENT_ID,
        NTSTATUS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDelayExecution,
        BOOLEAN,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDeleteAtom,
        RTL_ATOM);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDeleteBootEntry,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDeleteDriverEntry,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDeleteFile,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDeleteKey,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDeleteObjectAuditAlarm,
        PUNICODE_STRING,
        PVOID,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDeletePrivateNamespace,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDeleteValueKey,
        HANDLE,
        PUNICODE_STRING);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDeleteWnfStateData,
        PCWNF_STATE_NAME,
        const void*);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDeleteWnfStateName,
        PCWNF_STATE_NAME);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDeviceIoControlFile,
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

    _MI_DEFINE_STUB_ZERO_ARGS(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDisableLastKnownGood);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDisplayString,
        PUNICODE_STRING);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDrawText,
        PUNICODE_STRING);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDuplicateObject,
        HANDLE,
        HANDLE,
        HANDLE,
        PHANDLE,
        ACCESS_MASK,
        ULONG,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwDuplicateToken,
        HANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        BOOLEAN,
        TOKEN_TYPE,
        PHANDLE);

    _MI_DEFINE_STUB_ZERO_ARGS(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwEnableLastKnownGood);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwEnumerateBootEntries,
        PVOID,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwEnumerateDriverEntries,
        PVOID,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwEnumerateKey,
        HANDLE,
        ULONG,
        KEY_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwEnumerateSystemEnvironmentValuesEx,
        SYSTEM_ENVIRONMENT_INFORMATION_CLASS,
        PVOID,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwEnumerateTransactionObject,
        HANDLE,
        KTMOBJECT_TYPE,
        PKTMOBJECT_CURSOR,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwEnumerateValueKey,
        HANDLE,
        ULONG,
        KEY_VALUE_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwExtendSection,
        HANDLE,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFilterBootOption,
        FILTER_BOOT_OPTION_OPERATION,
        ULONG,
        ULONG,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFilterToken,
        HANDLE,
        ULONG,
        PTOKEN_GROUPS,
        PTOKEN_PRIVILEGES,
        PTOKEN_GROUPS,
        PHANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFilterTokenEx,
        HANDLE,
        ULONG,
        PTOKEN_GROUPS,
        PTOKEN_PRIVILEGES,
        PTOKEN_GROUPS,
        ULONG,
        PUNICODE_STRING,
        ULONG,
        PUNICODE_STRING,
        PTOKEN_GROUPS,
        PTOKEN_SECURITY_ATTRIBUTES_INFORMATION,
        PTOKEN_SECURITY_ATTRIBUTES_INFORMATION,
        PTOKEN_GROUPS,
        PHANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFindAtom,
        PWSTR,
        ULONG,
        PRTL_ATOM);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFlushBuffersFile,
        HANDLE,
        PIO_STATUS_BLOCK);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFlushBuffersFileEx,
        HANDLE,
        ULONG,
        PVOID,
        ULONG,
        PIO_STATUS_BLOCK);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFlushInstallUILanguage,
        LANGID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFlushInstructionCache,
        HANDLE,
        PVOID,
        SIZE_T);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFlushKey,
        HANDLE);

    _MI_DEFINE_STUB_ZERO_ARGS(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFlushProcessWriteBuffers);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFlushVirtualMemory,
        HANDLE,
        PVOID*,
        PSIZE_T,
        PIO_STATUS_BLOCK);

    _MI_DEFINE_STUB_ZERO_ARGS(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFlushWriteBuffer);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFreeUserPhysicalPages,
        HANDLE,
        PULONG_PTR,
        PULONG_PTR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFreeVirtualMemory,
        HANDLE,
        PVOID*,
        PSIZE_T,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFreezeRegistry,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFreezeTransactions,
        PLARGE_INTEGER,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwFsControlFile,
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

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwGetCachedSigningLevel,
        HANDLE,
        PULONG,
        PSE_SIGNING_LEVEL,
        PUCHAR,
        PULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwGetCompleteWnfStateSubscription,
        PWNF_STATE_NAME,
        ULONG64*,
        ULONG,
        ULONG,
        PWNF_DELIVERY_DESCRIPTOR,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwGetContextThread,
        HANDLE,
        PCONTEXT);

    _MI_DEFINE_STUB_ZERO_ARGS(0, ULONG, NTAPI, ZwGetCurrentProcessorNumber);

    _MI_DEFINE_STUB(0, ULONG, NTAPI, ZwGetCurrentProcessorNumberEx,
        PPROCESSOR_NUMBER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwGetDevicePowerState,
        HANDLE,
        PDEVICE_POWER_STATE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwGetMUIRegistryInfo,
        ULONG,
        PULONG,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwGetNextProcess,
        HANDLE,
        ACCESS_MASK,
        ULONG,
        ULONG,
        PHANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwGetNextThread,
        HANDLE,
        HANDLE,
        ACCESS_MASK,
        ULONG,
        ULONG,
        PHANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwGetNlsSectionPtr,
        ULONG,
        ULONG,
        PVOID,
        PVOID*,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwGetNotificationResourceManager,
        HANDLE,
        PTRANSACTION_NOTIFICATION,
        ULONG,
        PLARGE_INTEGER,
        PULONG,
        ULONG,
        ULONG_PTR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwGetWriteWatch,
        HANDLE,
        ULONG,
        PVOID,
        SIZE_T,
        PVOID*,
        PULONG_PTR,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwImpersonateAnonymousToken,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwImpersonateClientOfPort,
        HANDLE,
        PPORT_MESSAGE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwImpersonateThread,
        HANDLE,
        HANDLE,
        PSECURITY_QUALITY_OF_SERVICE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwInitializeEnclave,
        HANDLE,
        PVOID,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwInitializeNlsFiles,
        PVOID*,
        PLCID,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwInitializeRegistry,
        USHORT);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwInitiatePowerAction,
        POWER_ACTION,
        SYSTEM_POWER_STATE,
        ULONG,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwIsProcessInJob,
        HANDLE,
        HANDLE);

    _MI_DEFINE_STUB_ZERO_ARGS(FALSE, BOOLEAN, NTAPI, ZwIsSystemResumeAutomatic);

    _MI_DEFINE_STUB_ZERO_ARGS(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwIsUILanguageComitted);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwListenPort,
        HANDLE,
        PPORT_MESSAGE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwLoadDriver,
        PUNICODE_STRING);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwLoadEnclaveData,
        HANDLE,
        PVOID,
        PVOID,
        SIZE_T,
        ULONG,
        PVOID,
        ULONG,
        PSIZE_T,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwLoadKey,
        POBJECT_ATTRIBUTES,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwLoadKey2,
        POBJECT_ATTRIBUTES,
        POBJECT_ATTRIBUTES,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwLoadKey3,
        POBJECT_ATTRIBUTES,
        POBJECT_ATTRIBUTES,
        ULONG,
        PKEY_LOAD_ENTRY,
        ULONG,
        ACCESS_MASK,
        PHANDLE,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwLoadKeyEx,
        POBJECT_ATTRIBUTES,
        POBJECT_ATTRIBUTES,
        ULONG,
        HANDLE,
        HANDLE,
        ACCESS_MASK,
        PHANDLE,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwLockFile,
        HANDLE,
        HANDLE,
        PIO_APC_ROUTINE,
        PVOID,
        PIO_STATUS_BLOCK,
        PLARGE_INTEGER,
        PLARGE_INTEGER,
        ULONG, BOOLEAN,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwLockProductActivationKeys,
        ULONG*,
        ULONG*);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwLockRegistryKey,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwLockVirtualMemory,
        HANDLE,
        PVOID*,
        PSIZE_T,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwMakePermanentObject,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwMakeTemporaryObject,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwManagePartition,
        HANDLE,
        HANDLE,
        PARTITION_INFORMATION_CLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwMapCMFModule,
        ULONG,
        ULONG,
        PULONG,
        PULONG,
        PULONG,
        PVOID*);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwMapUserPhysicalPages,
        PVOID,
        ULONG_PTR,
        PULONG_PTR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwMapUserPhysicalPagesScatter,
        PVOID*,
        ULONG_PTR,
        PULONG_PTR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwMapViewOfSection,
        HANDLE,
        HANDLE,
        PVOID*,
        ULONG_PTR,
        SIZE_T,
        PLARGE_INTEGER,
        PSIZE_T,
        SECTION_INHERIT,
        ULONG,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwMapViewOfSectionEx,
        HANDLE,
        HANDLE,
        PVOID*,
        PLARGE_INTEGER,
        PSIZE_T,
        ULONG,
        ULONG,
        PMEM_EXTENDED_PARAMETER,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwModifyBootEntry,
        PBOOT_ENTRY);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwModifyDriverEntry,
        PEFI_DRIVER_ENTRY);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwNotifyChangeDirectoryFile,
        HANDLE,
        HANDLE,
        PIO_APC_ROUTINE,
        PVOID,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        ULONG,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwNotifyChangeDirectoryFileEx,
        HANDLE,
        HANDLE,
        PIO_APC_ROUTINE,
        PVOID,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        ULONG,
        BOOLEAN,
        DIRECTORY_NOTIFY_INFORMATION_CLASS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwNotifyChangeKey,
        HANDLE,
        HANDLE,
        PIO_APC_ROUTINE,
        PVOID,
        PIO_STATUS_BLOCK,
        ULONG,
        BOOLEAN,
        PVOID,
        ULONG,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwNotifyChangeMultipleKeys,
        HANDLE,
        ULONG,
        OBJECT_ATTRIBUTES*,
        HANDLE,
        PIO_APC_ROUTINE,
        PVOID,
        PIO_STATUS_BLOCK,
        ULONG,
        BOOLEAN,
        PVOID,
        ULONG,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwNotifyChangeSession,
        HANDLE,
        ULONG,
        PLARGE_INTEGER,
        IO_SESSION_EVENT,
        IO_SESSION_STATE,
        IO_SESSION_STATE,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenDirectoryObject,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenEnlistment,
        PHANDLE,
        ACCESS_MASK,
        HANDLE,
        LPGUID,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenEvent,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenEventPair,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenFile,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        PIO_STATUS_BLOCK,
        ULONG,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenIoCompletion,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenJobObject,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenKey,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenKeyEx,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenKeyTransacted,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenKeyTransactedEx,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        ULONG,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenKeyedEvent,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenMutant,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenObjectAuditAlarm,
        PUNICODE_STRING,
        PVOID,
        PUNICODE_STRING,
        PUNICODE_STRING,
        PSECURITY_DESCRIPTOR,
        HANDLE,
        ACCESS_MASK,
        ACCESS_MASK,
        PPRIVILEGE_SET,
        BOOLEAN,
        BOOLEAN,
        PBOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenPartition,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenPrivateNamespace,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        POBJECT_BOUNDARY_DESCRIPTOR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenProcess,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        PCLIENT_ID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenProcessToken,
        HANDLE,
        ACCESS_MASK,
        PHANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenProcessTokenEx,
        HANDLE,
        ACCESS_MASK,
        ULONG,
        PHANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenRegistryTransaction,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenResourceManager,
        PHANDLE,
        ACCESS_MASK,
        HANDLE,
        LPGUID,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenSection,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenSemaphore,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenSession,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenSymbolicLinkObject,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenThread,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        PCLIENT_ID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenThreadToken,
        HANDLE,
        ACCESS_MASK,
        BOOLEAN,
        PHANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenThreadTokenEx,
        HANDLE,
        ACCESS_MASK,
        BOOLEAN,
        ULONG,
        PHANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenTimer,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenTransaction,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        LPGUID,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwOpenTransactionManager,
        PHANDLE,
        ACCESS_MASK,
        POBJECT_ATTRIBUTES,
        PUNICODE_STRING,
        LPGUID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwPlugPlayControl,
        PLUGPLAY_CONTROL_CLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwPowerInformation,
        POWER_INFORMATION_LEVEL,
        PVOID,
        ULONG,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwPrepareComplete,
        HANDLE,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwPrepareEnlistment,
        HANDLE,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwPrePrepareComplete,
        HANDLE,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwPrePrepareEnlistment,
        HANDLE,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwPrivilegeCheck,
        HANDLE,
        PPRIVILEGE_SET,
        PBOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwPrivilegeObjectAuditAlarm,
        PUNICODE_STRING,
        PVOID,
        HANDLE,
        ACCESS_MASK,
        PPRIVILEGE_SET,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwPrivilegedServiceAuditAlarm,
        PUNICODE_STRING,
        PUNICODE_STRING,
        HANDLE,
        PPRIVILEGE_SET,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwPropagationComplete,
        HANDLE,
        ULONG,
        ULONG,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwPropagationFailed,
        HANDLE,
        ULONG,
        NTSTATUS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwProtectVirtualMemory,
        HANDLE,
        PVOID*,
        PSIZE_T,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwPulseEvent,
        HANDLE,
        PLONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryAttributesFile,
        POBJECT_ATTRIBUTES,
        PFILE_BASIC_INFORMATION);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryAuxiliaryCounterFrequency,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryBootEntryOrder,
        PULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryBootOptions,
        PBOOT_OPTIONS,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryDebugFilterState,
        ULONG,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryDefaultLocale,
        BOOLEAN,
        PLCID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryDefaultUILanguage,
        LANGID*);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryDirectoryFile,
        HANDLE,
        HANDLE,
        PIO_APC_ROUTINE,
        PVOID,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        FILE_INFORMATION_CLASS,
        BOOLEAN,
        PUNICODE_STRING,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryDirectoryFileEx,
        HANDLE,
        HANDLE,
        PIO_APC_ROUTINE,
        PVOID,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        FILE_INFORMATION_CLASS,
        ULONG,
        PUNICODE_STRING);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryDirectoryObject,
        HANDLE,
        PVOID,
        ULONG,
        BOOLEAN,
        BOOLEAN,
        PULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryDriverEntryOrder,
        PULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryEaFile,
        HANDLE,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        BOOLEAN,
        PVOID,
        ULONG,
        PULONG,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryEvent,
        HANDLE,
        EVENT_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryFullAttributesFile,
        POBJECT_ATTRIBUTES,
        PFILE_NETWORK_OPEN_INFORMATION);


    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryInformationAtom,
        RTL_ATOM,
        ATOM_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryInformationByName,
        POBJECT_ATTRIBUTES,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        FILE_INFORMATION_CLASS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryInformationEnlistment,
        HANDLE,
        ENLISTMENT_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryInformationFile,
        HANDLE,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        FILE_INFORMATION_CLASS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryInformationJobObject,
        HANDLE,
        JOBOBJECTINFOCLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryInformationPort,
        HANDLE,
        PORT_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryInformationProcess,
        HANDLE,
        PROCESSINFOCLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryInformationResourceManager,
        HANDLE,
        RESOURCEMANAGER_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryInformationThread,
        HANDLE,
        THREADINFOCLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryInformationToken,
        HANDLE,
        TOKEN_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryInformationTransaction,
        HANDLE,
        TRANSACTION_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryInformationTransactionManager,
        HANDLE,
        TRANSACTIONMANAGER_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryInformationWorkerFactory,
        HANDLE,
        WORKERFACTORYINFOCLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryInstallUILanguage,
        LANGID*);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryIntervalProfile,
        KPROFILE_SOURCE,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryIoCompletion,
        HANDLE,
        IO_COMPLETION_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryKey,
        HANDLE,
        KEY_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryLicenseValue,
        PUNICODE_STRING,
        PULONG,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryMultipleValueKey,
        HANDLE,
        PKEY_VALUE_ENTRY,
        ULONG,
        PVOID,
        PULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryMutant,
        HANDLE,
        MUTANT_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryObject,
        HANDLE,
        OBJECT_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryOpenSubKeys,
        POBJECT_ATTRIBUTES,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryOpenSubKeysEx,
        POBJECT_ATTRIBUTES,
        ULONG,
        PVOID,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryPerformanceCounter,
        PLARGE_INTEGER,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB_ZERO_ARGS(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryPortInformationProcess);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryQuotaInformationFile,
        HANDLE,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        BOOLEAN,
        PVOID,
        ULONG,
        PSID,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQuerySection,
        HANDLE,
        SECTION_INFORMATION_CLASS,
        PVOID,
        SIZE_T,
        PSIZE_T);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQuerySecurityAttributesToken,
        HANDLE,
        PUNICODE_STRING,
        ULONG,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQuerySecurityObject,
        HANDLE,
        SECURITY_INFORMATION,
        PSECURITY_DESCRIPTOR,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQuerySemaphore,
        HANDLE,
        SEMAPHORE_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQuerySymbolicLinkObject,
        HANDLE,
        PUNICODE_STRING,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQuerySystemEnvironmentValue,
        PUNICODE_STRING,
        PWSTR,
        USHORT,
        PUSHORT);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQuerySystemEnvironmentValueEx,
        PUNICODE_STRING,
        LPGUID,
        PVOID,
        PULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQuerySystemInformation,
        SYSTEM_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQuerySystemInformationEx,
        SYSTEM_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PVOID,
        ULONG,
        PULONG);

    NTSTATUS NTAPI MI_NAME(ZwQuerySystemTime)(_Out_ PLARGE_INTEGER SystemTime)
    {
        if (SystemTime) {
            KeQuerySystemTime(SystemTime);
            return STATUS_SUCCESS;
        }
        else {
            return STATUS_INVALID_PARAMETER;
        }
    }
#if defined(_M_IX86)
    _VEIL_DEFINE_IAT_RAW_SYMBOL(ZwQuerySystemTime@4, MI_NAME(ZwQuerySystemTime));
#else
    _VEIL_DEFINE_IAT_SYMBOL(ZwQuerySystemTime, MI_NAME(ZwQuerySystemTime));
#endif

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryTimer,
        HANDLE,
        TIMER_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryTimerResolution,
        PULONG,
        PULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryValueKey,
        HANDLE,
        PUNICODE_STRING,
        KEY_VALUE_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryVirtualMemory,
        HANDLE,
        PVOID,
        MEMORY_INFORMATION_CLASS,
        PVOID,
        SIZE_T,
        PSIZE_T);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryVolumeInformationFile,
        HANDLE,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        FS_INFORMATION_CLASS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryWnfStateData,
        PCWNF_STATE_NAME,
        PCWNF_TYPE_ID,
        VOID*,
        PWNF_CHANGE_STAMP,
        PVOID,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueryWnfStateNameInformation,
        PCWNF_STATE_NAME,
        WNF_STATE_NAME_INFORMATION,
        VOID*,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueueApcThread,
        HANDLE,
        PPS_APC_ROUTINE,
        PVOID,
        PVOID,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueueApcThreadEx,
        HANDLE,
        HANDLE,
        PPS_APC_ROUTINE,
        PVOID,
        PVOID,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwQueueApcThreadEx2,
        HANDLE,
        HANDLE,
        QUEUE_USER_APC_FLAGS,
        PPS_APC_ROUTINE,
        PVOID,
        PVOID,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRaiseException,
        PEXCEPTION_RECORD,
        PCONTEXT,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRaiseHardError,
        NTSTATUS,
        ULONG,
        ULONG,
        PULONG_PTR,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReadFile,
        HANDLE,
        HANDLE,
        PIO_APC_ROUTINE,
        PVOID,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        PLARGE_INTEGER,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReadFileScatter,
        HANDLE,
        HANDLE,
        PIO_APC_ROUTINE,
        PVOID,
        PIO_STATUS_BLOCK,
        PFILE_SEGMENT_ELEMENT,
        ULONG,
        PLARGE_INTEGER,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReadOnlyEnlistment,
        HANDLE,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReadRequestData,
        HANDLE,
        PPORT_MESSAGE,
        ULONG,
        PVOID,
        SIZE_T,
        PSIZE_T);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReadVirtualMemory,
        HANDLE,
        PVOID,
        PVOID,
        SIZE_T,
        PSIZE_T);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReadVirtualMemoryEx,
        HANDLE,
        PVOID,
        PVOID,
        SIZE_T,
        PSIZE_T,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRecoverEnlistment,
        HANDLE,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRecoverResourceManager,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRecoverTransactionManager,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRegisterProtocolAddressInformation,
        HANDLE,
        PCRM_PROTOCOL_ID,
        ULONG,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRegisterThreadTerminatePort,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReleaseKeyedEvent,
        HANDLE,
        PVOID,
        BOOLEAN,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReleaseMutant,
        HANDLE,
        PLONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReleaseSemaphore,
        HANDLE,
        LONG,
        PLONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReleaseWorkerFactoryWorker,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRemoveIoCompletion,
        HANDLE,
        PVOID*,
        PVOID*,
        PIO_STATUS_BLOCK,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRemoveIoCompletionEx,
        HANDLE,
        PFILE_IO_COMPLETION_INFORMATION,
        ULONG,
        PULONG,
        PLARGE_INTEGER,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRemoveProcessDebug,
        HANDLE,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRenameKey,
        HANDLE,
        PUNICODE_STRING);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRenameTransactionManager,
        PUNICODE_STRING,
        LPGUID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReplaceKey,
        POBJECT_ATTRIBUTES,
        HANDLE,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReplacePartitionUnit,
        PUNICODE_STRING,
        PUNICODE_STRING,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReplyPort,
        HANDLE,
        PPORT_MESSAGE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReplyWaitReceivePort,
        HANDLE,
        PVOID*,
        PPORT_MESSAGE,
        PPORT_MESSAGE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReplyWaitReceivePortEx,
        HANDLE,
        PVOID*,
        PPORT_MESSAGE,
        PPORT_MESSAGE,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwReplyWaitReplyPort,
        HANDLE,
        PPORT_MESSAGE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRequestPort,
        HANDLE,
        PPORT_MESSAGE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRequestWaitReplyPort,
        HANDLE,
        PPORT_MESSAGE,
        PPORT_MESSAGE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwResetEvent,
        HANDLE,
        PLONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwResetWriteWatch,
        HANDLE,
        PVOID,
        SIZE_T);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRestoreKey,
        HANDLE,
        HANDLE,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwResumeProcess,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwResumeThread,
        HANDLE,
        PULONG);

    _MI_DEFINE_STUB_ZERO_ARGS(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRevertContainerImpersonation);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRollbackComplete,
        HANDLE,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRollbackEnlistment,
        HANDLE,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRollbackRegistryTransaction,
        HANDLE,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRollbackTransaction,
        HANDLE,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwRollforwardTransactionManager,
        HANDLE,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSaveKey,
        HANDLE,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSaveKeyEx,
        HANDLE,
        HANDLE,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSaveMergedKeys,
        HANDLE,
        HANDLE,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSecureConnectPort,
        PHANDLE,
        PUNICODE_STRING,
        PSECURITY_QUALITY_OF_SERVICE,
        PPORT_VIEW,
        PSID,
        PREMOTE_PORT_VIEW,
        PULONG,
        PVOID,
        PULONG);

    _MI_DEFINE_STUB_ZERO_ARGS(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSerializeBoot);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetBootEntryOrder,
        PULONG,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetBootOptions,
        PBOOT_OPTIONS,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetCachedSigningLevel,
        ULONG,
        SE_SIGNING_LEVEL,
        PHANDLE,
        ULONG,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetCachedSigningLevel2,
        ULONG,
        SE_SIGNING_LEVEL,
        PHANDLE,
        ULONG,
        HANDLE,
        SE_SET_FILE_CACHE_INFORMATION*);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetContextThread,
        HANDLE,
        PCONTEXT);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetDebugFilterState,
        ULONG,
        ULONG,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetDefaultHardErrorPort,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetDefaultLocale,
        BOOLEAN,
        LCID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetDefaultUILanguage,
        LANGID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetDriverEntryOrder,
        PULONG,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetEaFile,
        HANDLE,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetEvent,
        HANDLE,
        PLONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetEventBoostPriority,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetHighEventPair,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetHighWaitLowEventPair,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetIRTimer,
        HANDLE,
        PLARGE_INTEGER);


    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationDebugObject,
        HANDLE,
        DEBUGOBJECTINFOCLASS,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationEnlistment,
        HANDLE,
        ENLISTMENT_INFORMATION_CLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationFile,
        HANDLE,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        FILE_INFORMATION_CLASS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationJobObject,
        HANDLE,
        JOBOBJECTINFOCLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationKey,
        HANDLE,
        KEY_SET_INFORMATION_CLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationObject,
        HANDLE,
        OBJECT_INFORMATION_CLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationProcess,
        HANDLE,
        PROCESSINFOCLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationResourceManager,
        HANDLE,
        RESOURCEMANAGER_INFORMATION_CLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationSymbolicLink,
        HANDLE,
        SYMBOLIC_LINK_INFORMATION_CLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationThread,
        HANDLE,
        THREADINFOCLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationToken,
        HANDLE,
        TOKEN_INFORMATION_CLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationTransaction,
        HANDLE,
        TRANSACTION_INFORMATION_CLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationTransactionManager,
        HANDLE,
        TRANSACTIONMANAGER_INFORMATION_CLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationVirtualMemory,
        HANDLE,
        VIRTUAL_MEMORY_INFORMATION_CLASS,
        ULONG_PTR,
        PMEMORY_RANGE_ENTRY,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetInformationWorkerFactory,
        HANDLE,
        WORKERFACTORYINFOCLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetIntervalProfile,
        ULONG,
        KPROFILE_SOURCE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetIoCompletion,
        HANDLE,
        PVOID,
        PVOID,
        NTSTATUS,
        ULONG_PTR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetIoCompletionEx,
        HANDLE,
        HANDLE,
        PVOID,
        PVOID,
        NTSTATUS,
        ULONG_PTR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetLdtEntries,
        ULONG,
        ULONG,
        ULONG,
        ULONG,
        ULONG,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetLowEventPair,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetLowWaitHighEventPair,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetQuotaInformationFile,
        HANDLE,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetSecurityObject,
        HANDLE,
        SECURITY_INFORMATION,
        PSECURITY_DESCRIPTOR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetSystemEnvironmentValue,
        PUNICODE_STRING,
        PUNICODE_STRING);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetSystemEnvironmentValueEx,
        PUNICODE_STRING,
        LPGUID,
        PVOID,
        ULONG,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetSystemInformation,
        SYSTEM_INFORMATION_CLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetSystemPowerState,
        POWER_ACTION,
        SYSTEM_POWER_STATE,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetSystemTime,
        PLARGE_INTEGER,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetThreadExecutionState,
        EXECUTION_STATE,
        EXECUTION_STATE*);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetTimer,
        HANDLE,
        PLARGE_INTEGER,
        PTIMER_APC_ROUTINE,
        PVOID,
        BOOLEAN,
        LONG,
        PBOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetTimer2,
        HANDLE,
        PLARGE_INTEGER,
        PLARGE_INTEGER,
        PT2_SET_PARAMETERS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetTimerEx,
        HANDLE,
        TIMER_SET_INFORMATION_CLASS,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetTimerResolution,
        ULONG,
        BOOLEAN,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetUuidSeed,
        PCHAR);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetValueKey,
        HANDLE,
        PUNICODE_STRING,
        ULONG,
        ULONG,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetVolumeInformationFile,
        HANDLE,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        FS_INFORMATION_CLASS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSetWnfProcessNotificationEvent,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwShutdownSystem,
        SHUTDOWN_ACTION);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwShutdownWorkerFactory,
        HANDLE,
        LONG*);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSignalAndWaitForSingleObject,
        HANDLE,
        HANDLE,
        BOOLEAN,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSinglePhaseReject,
        HANDLE,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwStartProfile,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwStopProfile,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSubscribeWnfStateChange,
        PCWNF_STATE_NAME,
        WNF_CHANGE_STAMP,
        ULONG,
        PULONG64);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSuspendProcess,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSuspendThread,
        HANDLE,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwSystemDebugControl,
        SYSDBG_COMMAND,
        PVOID,
        ULONG,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwTerminateEnclave,
        PVOID,
        BOOLEAN);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwTerminateJobObject,
        HANDLE,
        NTSTATUS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwTerminateProcess,
        HANDLE,
        NTSTATUS);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwTerminateThread,
        HANDLE,
        NTSTATUS);

    _MI_DEFINE_STUB_ZERO_ARGS(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwTestAlert);

    _MI_DEFINE_STUB_ZERO_ARGS(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwThawRegistry);

    _MI_DEFINE_STUB_ZERO_ARGS(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwThawTransactions);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwTraceControl,
        TRACE_CONTROL_INFORMATION_CLASS,
        PVOID,
        ULONG,
        PVOID,
        ULONG,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwTraceEvent,
        HANDLE,
        ULONG,
        ULONG,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwTranslateFilePath,
        PFILE_PATH,
        ULONG,
        PFILE_PATH,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwUmsThreadYield,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwUnloadDriver,
        PUNICODE_STRING);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwUnloadKey,
        POBJECT_ATTRIBUTES);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwUnloadKey2,
        POBJECT_ATTRIBUTES,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwUnloadKeyEx,
        POBJECT_ATTRIBUTES,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwUnlockFile,
        HANDLE,
        PIO_STATUS_BLOCK,
        PLARGE_INTEGER,
        PLARGE_INTEGER,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwUnlockVirtualMemory,
        HANDLE,
        PVOID*,
        PSIZE_T,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwUnmapViewOfSection,
        HANDLE,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwUnmapViewOfSectionEx,
        HANDLE,
        PVOID,
        ULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwUnsubscribeWnfStateChange,
        PCWNF_STATE_NAME);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwUpdateWnfStateData,
        PCWNF_STATE_NAME,
        VOID*,
        ULONG,
        PCWNF_TYPE_ID,
        VOID*,
        WNF_CHANGE_STAMP,
        LOGICAL);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwVdmControl,
        VDMSERVICECLASS,
        PVOID);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwWaitForAlertByThreadId,
        PVOID,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwWaitForDebugEvent,
        HANDLE,
        BOOLEAN,
        PLARGE_INTEGER,
        PDBGUI_WAIT_STATE_CHANGE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwWaitForKeyedEvent,
        HANDLE,
        PVOID,
        BOOLEAN,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwWaitForMultipleObjects,
        ULONG,
        HANDLE*,
        WAIT_TYPE,
        BOOLEAN,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwWaitForMultipleObjects32,
        ULONG,
        LONG*,
        WAIT_TYPE,
        BOOLEAN,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwWaitForSingleObject,
        HANDLE,
        BOOLEAN,
        PLARGE_INTEGER);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwWaitForWorkViaWorkerFactory,
        HANDLE,
        PFILE_IO_COMPLETION_INFORMATION,
        ULONG,
        PULONG,
        PWORKER_FACTORY_DEFERRED_WORK);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwWaitHighEventPair,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwWaitLowEventPair,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwWorkerFactoryWorkerReady,
        HANDLE);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwWriteFile,
        HANDLE,
        HANDLE,
        PIO_APC_ROUTINE,
        PVOID,
        PIO_STATUS_BLOCK,
        PVOID,
        ULONG,
        PLARGE_INTEGER,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwWriteFileGather,
        HANDLE,
        HANDLE,
        PIO_APC_ROUTINE,
        PVOID,
        PIO_STATUS_BLOCK,
        PFILE_SEGMENT_ELEMENT,
        ULONG,
        PLARGE_INTEGER,
        PULONG);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwWriteRequestData,
        HANDLE,
        PPORT_MESSAGE,
        ULONG,
        PVOID,
        SIZE_T,
        PSIZE_T);

    _MI_DEFINE_STUB(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwWriteVirtualMemory,
        HANDLE,
        PVOID,
        PVOID,
        SIZE_T,
        PSIZE_T);

    _MI_DEFINE_STUB_ZERO_ARGS(STATUS_NOT_SUPPORTED, NTSTATUS, NTAPI, ZwYieldExecution);

}

#undef _MI_DEFINE_STUB

EXTERN_C_END
