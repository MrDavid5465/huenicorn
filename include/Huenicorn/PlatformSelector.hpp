#pragma once

#ifdef __linux__
#include <Adapters/GnuLinux/GnuLinuxAdapter.hpp>
namespace Huenicorn
{
  using PlatformAdapter = GnuLinuxAdapter;
  extern PlatformAdapter platformAdapter;
}
#elif __APPLE__
#include <Adapters/MacOS/MacOSAdapter.hpp>
namespace Huenicorn
{
  using PlatformAdapter = MacOSAdapter;
  extern PlatformAdapter platformAdapter;
}
#elif WIN32
#include <Adapters/Windows/WindowsAdapter.hpp>
namespace Huenicorn
{
  using PlatformAdapter = WindowsAdapter;
  extern PlatformAdapter platformAdapter;
}
#else
#error "Unsupported platform"
#endif
