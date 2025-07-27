#pragma once

#include <thread>

#include <Grabbers/GnuLinux/X11/INotifiable.hpp>

namespace Huenicorn
{
  class X11Grabber;

  class X11MonitorWatcher
  {
  public:
    X11MonitorWatcher(INotifiable* notifiable);

    ~X11MonitorWatcher();

  private:
    void _loopThread(std::stop_token stopToken);

    // Attributes
    std::stop_source m_stopSource;
    std::jthread m_loopThread;
    INotifiable* m_notifiable{nullptr};
  };
}
