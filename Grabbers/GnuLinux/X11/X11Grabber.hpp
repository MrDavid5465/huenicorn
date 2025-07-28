#pragma once

#include <memory>
#include <optional>

#include <Huenicorn/IGrabber.hpp>

#include <X11/Xlib.h>
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
  public:
    struct DisplayDeleter
    {
      void operator()(Display* ptr);
    };


    struct CrtcInfoDeleter
    {
      void operator()(XRRCrtcInfo* ptr);
    };

    using UniqueCrtcInfo = std::unique_ptr<XRRCrtcInfo, CrtcInfoDeleter>;


    struct ScreenResourcesDeleter
    {
      void operator()(XRRScreenResources* ptr);
    };

    using UniqueScreenResources = std::unique_ptr<XRRScreenResources, ScreenResourcesDeleter>;


    struct OutputInfoDeleter
    {
      void operator()(XRROutputInfo* ptr);
    };

    using UniqueOutputInfo = std::unique_ptr<XRROutputInfo, OutputInfoDeleter>;


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
      std::unique_ptr<XImage, XImageDeleter> m_ximage;
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

    bool isMonitorStillValid(const X11MonitorData& mon);

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
    //virtual void _notify() override;

    void _ensureXThreadsInit();
    bool _ensureMonitorSelection();
    bool _ensureXShmData();

    // Attributes
    int m_screenId; // Once and for all
    std::unique_ptr<Display, DisplayDeleter> m_display;
    std::optional<XShmData> m_xshmData;
  };
}
