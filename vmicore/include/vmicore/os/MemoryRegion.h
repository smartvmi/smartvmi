#ifndef VMICORE_MEMORYREGION_H
#define VMICORE_MEMORYREGION_H

#include "IPageProtection.h"
#include <cstdint>
#include <string>
#include <utility>

struct MemoryRegion
{
    MemoryRegion(uint64_t base,
                 std::size_t size,
                 std::string moduleName,
                 std::unique_ptr<IPageProtection> protection,
                 bool isSharedMemory,
                 bool isBeingDeleted,
                 bool isProcessBaseImage)
        : base(base),
          size(size),
          moduleName(std::move(moduleName)),
          protection(std::move(protection)),
          isSharedMemory(isSharedMemory),
          isBeingDeleted(isBeingDeleted),
          isProcessBaseImage(isProcessBaseImage)
    {
    }

    uint64_t base{};
    std::size_t size{};
    std::string moduleName{};
    std::unique_ptr<IPageProtection> protection;
    bool isSharedMemory = false;
    bool isBeingDeleted = false;
    bool isProcessBaseImage = false;
};

#endif // VMICORE_MEMORYREGION_H
