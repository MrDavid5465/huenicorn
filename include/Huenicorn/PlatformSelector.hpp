#pragma once

#ifdef __linux__
#include <Huenicorn/Platforms/GnuLinux/GnuLinuxAdapter.hpp>
namespace Huenicorn
{
  using PlatformAdapter = GnuLinuxAdapter;
  extern PlatformAdapter platformAdapter;
}
#elif __APPLE__
#include <Huenicorn/Platforms/MacOS/MacOSAdapter.hpp>
namespace Huenicorn
{
  using PlatformAdapter = MacOSAdapter;
  extern PlatformAdapter platformAdapter;
}
#else
#error "Unsupported platform"
#endif
