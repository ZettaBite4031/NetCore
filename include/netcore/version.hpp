#pragma once

#ifndef NETCORE_VERSION_MAJOR
#define NETCORE_VERSION_MAJOR 0
#define NETCORE_VERSION_MINOR 1
#define NETCORE_VERSION_PATCH 0
#endif
#define NETCORE_VERSION_ENCODE(maj,min,pat) ((maj)*10000 + (min)*100 + (pat))
#define NETCORE_VERSION  NETCORE_VERSION_ENCODE(NETCORE_VERSION_MAJOR,NETCORE_VERSION_MINOR,NETCORE_VERSION_PATCH)
namespace NetCore {
    inline constexpr int version() { return NETCORE_VERSION; }
    inline constexpr int version_major() { return NETCORE_VERSION_MAJOR; }
    inline constexpr int version_minor() { return NETCORE_VERSION_MINOR; }
    inline constexpr int version_patch() { return NETCORE_VERSION_PATCH; }
}