#pragma once

namespace Huenicorn
{
  class INotifiable
  {
  public:
    virtual void _notify() = 0;
  };
}
