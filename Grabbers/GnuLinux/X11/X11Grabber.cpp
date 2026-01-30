#include <Grabbers/GnuLinux/X11/X11Grabber.hpp>


#include <X11/Xutil.h>
#include <sys/shm.h>

#include <X11/extensions/Xrandr.h>

#include <Huenicorn/Config.hpp>
#include <Huenicorn/Imaging/ImageProcessing.hpp>
#include <Huenicorn/Logger.hpp>


namespace Huenicorn
{
  /*
  const std::vector<int> X11Grabber::s_xRandrEventFlags = {
    RRScreenChangeNotify,
    RRNotify_OutputChange
  };

  X11Grabber::XRandrBases X11Grabber::s_xrandrBases = {};



  X11Grabber::X11MonitorCache::X11MonitorCache(Display* display):
  m_display(display),
  m_root(DefaultRootWindow(m_display))
  {
    _refresh();
  }


  bool X11Grabber::X11MonitorCache::updateRequired()
  {
    while(XPending(m_display)){
      XEvent e;
      XNextEvent(m_display, &e);
      int type = e.type - X11Grabber::s_xrandrBases.eventBase;
      if(type == RRScreenChangeNotify || type == RRNotify_OutputChange){
        _refresh();
        return true;
      }
    }

    return false;
  }


  bool X11Grabber::X11MonitorCache::isConnected(RROutput output)
  {
    auto it = m_connectedOutputs.find(output);
    return it != m_connectedOutputs.end() && it->second;
  }


  void X11Grabber::X11MonitorCache::_refresh()
  {
    UniqueScreenResources res(XRRGetScreenResourcesCurrent(m_display, m_root));
    m_connectedOutputs.clear();
    for(int i = 0; i < res->noutput; i++){
      UniqueOutputInfo info(XRRGetOutputInfo(m_display, res.get(), res->outputs[i]));
      m_connectedOutputs[res->outputs[i]] = (info && info->connection == RR_Connected && info->crtc);
    }
  }
  */


  // X11MonitorData
  X11Grabber::X11MonitorData::X11MonitorData(const std::string& name, unsigned width, unsigned height, double refreshRate, bool isPrimary, int xPos, int yPos, RROutput outputId):
  MonitorData(name, width, height, refreshRate, isPrimary),
  xPos(xPos),
  yPos(yPos),
  outputId(outputId)
  {}


  // XShmData
  X11Grabber::XShmData::XShmData(Display* display, int screenId, int width, int height):
  m_display(display)
  {
    m_shmInfo = std::make_unique<XShmSegmentInfo>();

    m_ximage.reset(XShmCreateImage(m_display,
      DefaultVisual(m_display, screenId),
      DefaultDepth(m_display, screenId),
      ZPixmap,
      nullptr,
      m_shmInfo.get(),
      width,
      height
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


  XImage* X11Grabber::XShmData::ximage() const
  {
    return m_ximage.get();
  }


  /*
  void X11Grabber::_initDisplayEvents()
  {
    int combinedEventFlags = 0;
    for(auto eventFlag : s_xRandrEventFlags){
      combinedEventFlags |= eventFlag;
    }

    XRRSelectInput(m_display.get(), DefaultRootWindow(m_display.get()), combinedEventFlags);
    XRRQueryExtension(m_display.get(), &s_xrandrBases.eventBase, &s_xrandrBases.errorBase);
  }
  */


  // X11 Grabber
  X11Grabber::X11Grabber(Config* config):
  IGrabber(config)
  {
    _ensureXThreadsInit();

    m_display.reset(XOpenDisplay(nullptr));

    if(!m_display){
      throw std::runtime_error("Could not open any X11 display");
    }

    /*
    _initDisplayEvents();
    m_x11MonitorCache = std::make_unique<X11MonitorCache>(m_display.get());
    */

    m_screenId = XDefaultScreen(m_display.get()); // Once and for all
  }


  const std::string& X11Grabber::name() const
  {
    static const std::string s_identifier = "X11Grabber";
    return s_identifier;
  }


  glm::ivec2 X11Grabber::displayResolution() const
  {
    try{
      auto selectedMonitor = m_monitorSelectionData.selectedMonitor();
      return {selectedMonitor->width, selectedMonitor->height};
    }
    catch(const std::exception& e){
      Logger::error("No selected monitor");
    }

    return {0, 0};
  }


  IGrabber::RefreshRate X11Grabber::displayRefreshRate() const
  {
    try{
      auto selectedMonitor = m_monitorSelectionData.selectedMonitor();
      return selectedMonitor->refreshRate;
    }
    catch(const std::exception& e){
      Logger::error("No selected monitor");
    }

    return 0;
  }


  void X11Grabber::selectMonitor(unsigned monitorId)
  {
    m_xshmData.reset();
    m_monitorSelectionData.selectedMonitorId = monitorId;
    Logger::log("selected ", m_monitorSelectionData.selectedMonitor()->name);
  }



  void X11Grabber::grabFrameSubsample(Imaging::ImageData& imageData)
  {
    /*
    if(
      m_x11MonitorCache->updateRequired() ||
      !m_x11MonitorCache->isConnected(static_cast<X11MonitorData*>(m_monitorSelectionData.selectedMonitor())->outputId)
    ){
      Logger::warn("Monitor disconnected or mode changed — skipping grab.");
      _initMonitorsList();
      m_xshmData.reset();
    }
    */

    if(!_ensureXShmData()){
      return;
    }

    auto* selectedMonitor = dynamic_cast<X11MonitorData*>(m_monitorSelectionData.selectedMonitor());

    int width = selectedMonitor->width;
    int height = selectedMonitor->height;
    auto ximage = m_xshmData->ximage();

    XShmGetImage(m_display.get(), RootWindow(m_display.get(), m_screenId), ximage, selectedMonitor->xPos, selectedMonitor->yPos, AllPlanes);

    Imaging::ImageData grabbedImageData;
    if(ximage->bits_per_pixel > 24){
      grabbedImageData.imageMatrix = cv::Mat(height, width, CV_8UC4, ximage->data);
      Imaging::ImageProcessing::rescale(grabbedImageData, grabbedImageData, m_config->subsampleWidth(), m_config->interpolation());
      Imaging::ImageProcessing::rgbaToRgb(grabbedImageData, grabbedImageData);
    }
    else{
      grabbedImageData.imageMatrix = cv::Mat(height, width, CV_8UC3, ximage->data);
      Imaging::ImageProcessing::rescale(grabbedImageData, grabbedImageData, m_config->subsampleWidth(), m_config->interpolation());
    }

    imageData = std::move(grabbedImageData);
  }


  void X11Grabber::_initMonitorsList()
  {
    MonitorSelectionData monitorSelectionData;

    if(!m_display.get()){
      throw std::runtime_error("No display available");
    }

    Window root = DefaultRootWindow(m_display.get());
    UniqueScreenResources screenResources(XRRGetScreenResources(m_display.get(), root));
    auto monitorsQuantity = screenResources->noutput;

    if(monitorsQuantity <= 0){
      throw std::runtime_error("No screen available");
    }

    RROutput primaryId = XRRGetOutputPrimary(m_display.get(), root);

    int minX = std::numeric_limits<int>::max();
    int minY = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int maxY = std::numeric_limits<int>::min();
    double minRefreshRate = 1000.0; // Let's not take any risk about this

    for(int i = 0; i < monitorsQuantity; i++){
      UniqueOutputInfo outputInfo(XRRGetOutputInfo(m_display.get(), screenResources.get(), screenResources->outputs[i]));

      if(outputInfo->connection == RR_Connected && outputInfo->crtc != 0){
        UniqueCrtcInfo crtcInfo(XRRGetCrtcInfo(m_display.get(), screenResources.get(), outputInfo->crtc));

        double refreshRate = 0.0;
        RRMode modeId = crtcInfo->mode;
        for(int i = 0; i < screenResources->nmode; ++i){
          if(screenResources->modes[i].id == modeId){
            const XRRModeInfo& mode = screenResources->modes[i];
            if(mode.hTotal && mode.vTotal){
              refreshRate = static_cast<double>(mode.dotClock) / (mode.hTotal * mode.vTotal);
            }
            break;
          }
        }

        minRefreshRate = std::min(refreshRate, minRefreshRate);

        int x = crtcInfo->x;
        int y = crtcInfo->y;
        int width = crtcInfo->width;
        int height = crtcInfo->height;
        bool isPrimary = (screenResources->outputs[i] == primaryId);

        monitorSelectionData.monitors.push_back(std::make_unique<X11MonitorData>(
          outputInfo->name,
          width,
          height,
          refreshRate,
          isPrimary,
          x,
          y,
          screenResources->outputs[i]
        ));

        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x + width);
        maxY = std::max(maxY, y + height);

        if(isPrimary){
          monitorSelectionData.selectedMonitorId = monitorSelectionData.monitors.size() - 1;
        }
      }
    }

    // Add whole display surface to choices if there are multiple screens
    /*
    if(monitorSelectionData.monitors.size() > 1){
      monitorSelectionData.monitors.push_back(std::make_unique<X11MonitorData>(
        "Combined displays",
        maxX - minX,
        maxY - minY,
        minRefreshRate,
        false,
        minX,
        minY,
        0
      ));
    }
    */

    std::swap(m_monitorSelectionData, monitorSelectionData);
  }


  void X11Grabber::_ensureXThreadsInit()
  {
    static bool initialized = [](){
      return XInitThreads();
    }();

    if(!initialized){
      throw std::runtime_error("XInitThreads failed");
    }
  }


  bool X11Grabber::_ensureXShmData()
  {
    if(m_xshmData){
      return true;
    }

    if(auto monitor = m_monitorSelectionData.selectedMonitor()){
      m_xshmData = std::make_unique<XShmData>(m_display.get(), m_screenId, monitor->width, monitor->height);
      return true;
    }

    return false;
  }
}
