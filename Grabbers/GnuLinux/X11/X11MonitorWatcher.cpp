#include <Grabbers/GnuLinux/X11/X11MonitorWatcher.hpp>

#include <X11/extensions/Xrandr.h>

#include <Huenicorn/Logger.hpp>

namespace Huenicorn
{
  X11MonitorWatcher::X11MonitorWatcher(INotifiable* notifiable):
  m_notifiable(notifiable),
  m_loopThread(&X11MonitorWatcher::_loopThread, this, m_stopSource.get_token())
  {}


  X11MonitorWatcher::~X11MonitorWatcher()
  {
    m_stopSource.request_stop();
  }


  void X11MonitorWatcher::_loopThread(std::stop_token stopToken)
  {
    Display* display = XOpenDisplay(nullptr);
    Window root = DefaultRootWindow(display);
    int randr_event_base, randr_error_base;

    if (!XRRQueryExtension(display, &randr_event_base, &randr_error_base)) {
      Logger::warn("RandR not supported. Cannot ensure monitor hotplug");
      return;
    }

    XRRSelectInput(display, root, RROutputChangeNotifyMask | RRScreenChangeNotifyMask);

    while(!stopToken.stop_requested()){
      while(XPending(display)){
        XEvent event;
        XNextEvent(display, &event);

        if(event.type == randr_event_base + RRNotify){
          auto* outputEvent = reinterpret_cast<XRROutputChangeNotifyEvent*>(&event);
          if(outputEvent->subtype == RRNotify_OutputChange){
            bool connected = (outputEvent->connection == RR_Connected);
            m_notifiable->_notify();
          }
        }
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
}
