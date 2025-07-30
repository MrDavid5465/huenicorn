#pragma once

#include <memory>
#include <optional>

#include <Huenicorn/IGrabber.hpp>

#include <X11/extensions/Xrandr.h>
#include <X11/extensions/XShm.h>

#include <Huenicorn/MonitorData.hpp>


namespace Huenicorn
{
  /**
   * @brief X11 implementation of screen grabber
   * 
   */
  class X11Grabber : public IGrabber
  {
    /*
    struct XRandrBases
    {
      int eventBase;
      int errorBase;
    };



    class X11MonitorCache
    {
    public:
      X11MonitorCache(Display* display);

      bool updateRequired();
      bool isConnected(RROutput output);

    private:
      void _refresh();

      Display* m_display;
      Window m_root;
      std::unordered_map<RROutput, bool> m_connectedOutputs;
    };
    friend X11MonitorCache;
    */


  public:
    template <auto FreeFunc>
    struct XDeleter
    {
      template <typename T>
      void operator()(T* ptr) const noexcept
      {
        if(ptr){
          FreeFunc(ptr);
          ptr = nullptr;
        }
      }
    };

    static inline void destroyXImage(XImage* img)
    {
      if(img){
        XDestroyImage(img);
      }
    }

    template <typename T, auto FreeFunc>
    using XUniquePtr = std::unique_ptr<T, XDeleter<FreeFunc>>;

    using UniqueDisplay = XUniquePtr<Display,    XCloseDisplay>;
    using UniqueOutputInfo = XUniquePtr<XRROutputInfo,    XRRFreeOutputInfo>;
    using UniqueCrtcInfo   = XUniquePtr<XRRCrtcInfo,      XRRFreeCrtcInfo>;
    using UniqueScreenResources  = XUniquePtr<XRRScreenResources, XRRFreeScreenResources>;
    using UniqueXImage = XUniquePtr<XImage, X11Grabber::destroyXImage>;


    struct X11MonitorData : public MonitorData
    {
      X11MonitorData(const std::string& name, unsigned width, unsigned height, double refreshRate, bool isPrimary, int xPos, int yPos, RROutput outputId);

      int xPos{0};
      int yPos{0};
      RROutput outputId{0};
    };


    class XShmData
    {
    public:

      struct XImageDeleter
      {
        void operator()(XImage* ptr);
      };


      XShmData(Display* display, int screenId, int width, int height);
      ~XShmData();

      XImage* ximage() const;

    private:
      Display* m_display{nullptr};
      std::unique_ptr<XShmSegmentInfo> m_shmInfo;
      UniqueXImage m_ximage;
    };


    // Constructor / destructor
    /**
     * @brief X11Grabber constructor
     * 
     * @param config Huenicorn configuration
     */
    X11Grabber(Config* config);


    /**
     * @brief X11Grabber destructor
     * 
     */
    virtual ~X11Grabber(){}


    // Getters
    virtual const std::string& name() const override;

    virtual bool hasCustomScreenManagement() const override
    {
      return true;
    }

    /**
     * @brief Returns the resolution of the selected display
     * 
     * @return Resolution Resolution of the selected display
     */
    virtual glm::ivec2 displayResolution() const override;


    /**
     * @brief Returns the refresh rate of the display
     * 
     * @return RefreshRate Refresh rate of the display
     */
    virtual RefreshRate displayRefreshRate() const override;

    bool isMonitorStillValid(X11MonitorData* monitor);

    // Methods
    virtual void selectMonitor(unsigned monitorId) override;

    /**
     * @brief Takes a screen capture and returns a subsample of it as bitmap
     * 
     * @param imageData Subsample of screen capture
     */
    virtual void grabFrameSubsample(ImageData& imageData) override;

  protected:
    virtual void _initMonitorsList() override;

  private:

    void _ensureXThreadsInit();
    //void _initDisplayEvents();
    bool _ensureMonitorSelection();
    bool _ensureXShmData();

    // Attributes
    int m_screenId; // Once and for all
    UniqueDisplay m_display;
    //std::unique_ptr<X11MonitorCache> m_x11MonitorCache;

    std::unique_ptr<XShmData> m_xshmData;
    //static const std::vector<int> s_xRandrEventFlags;
    //static XRandrBases s_xrandrBases;
  };
}
