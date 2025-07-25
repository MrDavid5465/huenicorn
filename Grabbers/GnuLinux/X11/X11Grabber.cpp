#include <Grabbers/GnuLinux/X11/X11Grabber.hpp>

#include <err.h>

#include <cstring>

#include <X11/Xutil.h>
#include <sys/shm.h>

#include <Huenicorn/Config.hpp>
#include <Huenicorn/ImageProcessing.hpp>


namespace Huenicorn
{
  X11Grabber::XShmData::XShmData(Display* display, X11Grabber::X11MonitorData* monitor):
  m_display(display)
  {
    int screenId = XDefaultScreen(m_display);
    m_shmInfo = std::make_unique<XShmSegmentInfo>();

    m_ximage.reset(XShmCreateImage(m_display,
      DefaultVisual(m_display, screenId),
      DefaultDepth(m_display, screenId),
      ZPixmap,
      nullptr,
      m_shmInfo.get(),
      monitor->width,
      monitor->height
    ));

    int size = m_ximage->bytes_per_line * m_ximage->height;

    m_shmInfo->shmid = shmget(IPC_PRIVATE, size, IPC_CREAT | 0777);
    m_shmInfo->readOnly = False;
    m_shmInfo->shmaddr = m_ximage->data = reinterpret_cast<char*>(shmat(m_shmInfo->shmid, nullptr, 0));

    XShmAttach(m_display, m_shmInfo.get());
  }


  X11Grabber::XShmData::~XShmData()
  {
    shmdt(m_shmInfo->shmaddr);
    shmctl(m_shmInfo->shmid, IPC_RMID, 0);
    XShmDetach(m_display, m_shmInfo.get());
    m_shmInfo.reset();
  }


  X11Grabber::X11Grabber(Config* config):
  IGrabber(config)
  {
    m_display.reset(XOpenDisplay(nullptr));

    if(!m_display){
      throw std::runtime_error("Could not open any X11 display");
    }
  }


  const std::string& X11Grabber::name() const
  {
    static const std::string s_identifier = "X11Grabber";
    return s_identifier;
  }


  glm::ivec2 X11Grabber::displayResolution() const
  {
    if(!m_selectedMonitor){
      throw std::runtime_error("No selected monitor");
    }

    return {m_selectedMonitor->width, m_selectedMonitor->height};
  }


  void X11Grabber::grabFrameSubsample(ImageData& imageData)
  {
    if(!_ensureXShmData()){
      return;
    }

    int screenId = XDefaultScreen(m_display.get());
    auto ximage = m_xshmData->ximage();

    XShmGetImage(m_display.get(), RootWindow(m_display.get(), screenId), ximage, m_selectedMonitor->xPos, m_selectedMonitor->yPos, AllPlanes);

    ImageData grabbedImageData;
    if(ximage->bits_per_pixel > 24){
      grabbedImageData.imageMatrix = cv::Mat(m_selectedMonitor->height, m_selectedMonitor->width, CV_8UC4, ximage->data);
      ImageProcessing::rescale(grabbedImageData, grabbedImageData, m_config->subsampleWidth(), m_config->interpolation());
      ImageProcessing::rgbaToRgb(grabbedImageData, grabbedImageData);
    }
    else{
      grabbedImageData.imageMatrix = cv::Mat(m_selectedMonitor->height, m_selectedMonitor->width, CV_8UC3, ximage->data);
      ImageProcessing::rescale(grabbedImageData, grabbedImageData, m_config->subsampleWidth(), m_config->interpolation());
    }

    imageData = std::move(grabbedImageData);
  }


  IGrabber::RefreshRate X11Grabber::displayRefreshRate() const
  {
    int screenId = XDefaultScreen(m_display.get());
    Window root = RootWindow(m_display.get(), screenId);
    XRRScreenConfiguration* displayConfig = XRRGetScreenInfo(m_display.get(), root);
    RefreshRate currentRate = XRRConfigCurrentRate(displayConfig);

    return currentRate;
  }


  void X11Grabber::_initMonitorsList()
  {
    m_monitors.clear();

    if(!m_display.get()){
      throw std::runtime_error("No display available");
    }

    Window root = DefaultRootWindow(m_display.get());
    XRRScreenResources* screenResources = XRRGetScreenResources(m_display.get(), root);
    auto monitorsQuantity = screenResources->noutput;

    if(monitorsQuantity <= 0){
      throw std::runtime_error("No screen available");
    }


    for(int i = 0; i < monitorsQuantity; i++){
      XRROutputInfo* outputInfo = XRRGetOutputInfo(m_display.get(), screenResources, screenResources->outputs[i]);

      if(outputInfo->connection == RR_Connected && outputInfo->crtc != 0){
        XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(m_display.get(), screenResources, outputInfo->crtc);

        m_monitors.push_back(std::make_unique<X11MonitorData>(
          outputInfo->name,
          crtcInfo->width,
          crtcInfo->height,
          crtcInfo->x,
          crtcInfo->y,
          screenResources->outputs[i],
          outputInfo->crtc
        ));

        XRRFreeCrtcInfo(crtcInfo);
      }

      XRRFreeOutputInfo(outputInfo);
    }

    XRRFreeScreenResources(screenResources);

    m_selectedMonitor = dynamic_cast<X11MonitorData*>(m_monitors.front().get());
  }


  bool X11Grabber::_ensureXShmData()
  {
    if(!m_selectedMonitor){
      return false;
    }

    if(m_xshmData.has_value()){
      return true;
    }

    m_xshmData.reset();
    m_xshmData.emplace(m_display.get(), m_selectedMonitor);

    return true;
  }
}
