#pragma once

#include <chrono>

namespace Huenicorn
{
  namespace Timing
  {
    using ClockType = std::chrono::steady_clock;
    using Duration = std::chrono::duration<double>;
    using TimePoint = ClockType::time_point;
  }
}
