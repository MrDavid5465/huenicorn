#pragma once

#ifdef __linux__
#include <Huenicorn/Platform/Adapters/GnuLinux/GnuLinuxAdapter.hpp>
namespace Huenicorn::Platform
{
  using Adapter = GnuLinuxAdapter;
  extern Adapter adapter;
}
#elif __APPLE__
#include <Adapters/MacOS/MacOSAdapter.hpp>
namespace Huenicorn::Platform
{
  using Adapter = MacOSAdapter;
  extern Adapter adapter;
}
#elif WIN32
#include <Adapters/Windows/WindowsAdapter.hpp>
namespace Huenicorn::Platform
{
  using Adapter = WindowsAdapter;
  extern Adapter adapter;
}
#else
#error "Unsupported platform"
#endif
