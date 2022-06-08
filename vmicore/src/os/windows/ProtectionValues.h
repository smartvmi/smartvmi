#ifndef VMICORE_PROTECTIONVALUES_H
#define VMICORE_PROTECTIONVALUES_H

#include <cstdint>

enum class ProtectionValues
{
    PAGE_NOACCESS,
    PAGE_READONLY,
    PAGE_EXECUTE,
    PAGE_EXECUTE_READ,
    PAGE_READWRITE,
    PAGE_WRITECOPY,
    PAGE_EXECUTE_READWRITE,
    PAGE_EXECUTE_WRITECOPY,
    PAGE_NOACCESS_2,
    PAGE_NOCACHE_PAGE_READONLY,
    PAGE_NOCACHE_PAGE_EXECUTE,
    PAGE_NOCACHE_PAGE_EXECUTE_READ,
    PAGE_NOCACHE_PAGE_READWRITE,
    PAGE_NOCACHE_PAGE_WRITECOPY,
    PAGE_NOCACHE_PAGE_EXECUTE_READWRITE,
    PAGE_NOCACHE_PAGE_EXECUTE_WRITECOPY,
    PAGE_NOACCESS_3,
    PAGE_GUARD_PAGE_READONLY,
    PAGE_GUARD_PAGE_EXECUTE,
    PAGE_GUARD_PAGE_EXECUTE_READ,
    PAGE_GUARD_PAGE_READWRITE,
    PAGE_GUARD_PAGE_WRITECOPY,
    PAGE_GUARD_PAGE_EXECUTE_READWRITE,
    PAGE_GUARD_PAGE_EXECUTE_WRITECOPY,
    PAGE_NOACCESS_4,
    PAGE_WRITECOMBINE_PAGE_READONLY,
    PAGE_WRITECOMBINE_PAGE_EXECUTE,
    PAGE_WRITECOMBINE_PAGE_EXECUTE_READ,
    PAGE_WRITECOMBINE_PAGE_READWRITE,
    PAGE_WRITECOMBINE_PAGE_WRITECOPY,
    PAGE_WRITECOMBINE_PAGE_EXECUTE_READWRITE,
    PAGE_WRITECOMBINE_PAGE_EXECUTE_WRITECOPY,
};

#endif // VMICORE_PROTECTIONVALUES_H