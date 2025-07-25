#pragma once

#include <memory>
#include <optional>

#include <Huenicorn/IGrabber.hpp>
#include <Huenicorn/MonitorData.hpp>

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>

#include <X11/extensions/Xrandr.h>

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
      inline void operator()(Display* ptr)
      {
        XCloseDisplay(ptr);
        ptr = nullptr;
      }
    };


    struct X11MonitorData : public MonitorData
    {
      X11MonitorData(const std::string name, unsigned width, unsigned height, double refreshRate, int xPos, int yPos, bool isPrimary):
      MonitorData(name, width, height, refreshRate),
      xPos(xPos),
      yPos(yPos),
      isPrimary(isPrimary)
      {}

      int xPos{0};
      int yPos{0};
      bool isPrimary{false};
    };


    class XShmData
    {
    public:

      struct XImageDeleter
      {
        inline void operator()(XImage* ptr)
        {
          XDestroyImage(ptr);
          ptr = nullptr;
        }
      };


      XShmData(Display* display, int screenId, X11MonitorData* monitor);
      ~XShmData();

      inline XImage* ximage() const
      {
        return m_ximage.get();
      }

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


    // Methods
    virtual void selectMonitor(MonitorData* monitor) override;

    /**
     * @brief Takes a screen capture and returns a subsample of it as bitmap
     * 
     * @param imageData Subsample of screen capture
     */
    virtual void grabFrameSubsample(ImageData& imageData) override;

  protected:
    virtual void _initMonitorsList() override;

  private:

    bool _ensureXShmData();

    X11MonitorData* m_selectedMonitor{nullptr};

    // Attributes
    int m_screenId; // Once and for all
    std::unique_ptr<Display, DisplayDeleter> m_display;
    std::optional<XShmData> m_xshmData;
  };
}
