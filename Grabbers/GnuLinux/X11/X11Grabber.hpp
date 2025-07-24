#pragma once

#include <memory>
#include <optional>

#include <Huenicorn/IGrabber.hpp>

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


    struct Monitor
    {
      std::string name;
      int xPos;
      int yPos;
      unsigned width;
      unsigned height;
      RROutput outputId;
      RRCrtc crtcId;
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


      XShmData(Display* display, const Monitor& monitor);
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


    using Monitors = std::vector<Monitor>;


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
    virtual ~X11Grabber();


    // Getters
    virtual const std::string& name() const override;

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
    /**
     * @brief Takes a screen capture and returns a subsample of it as bitmap
     * 
     * @param imageData Subsample of screen capture
     */
    virtual void grabFrameSubsample(ImageData& imageData) override;


  private:

    void _listMonitors(Monitors& monitors);
    void _initXShmData(const Monitor& monitor);

    Monitors m_monitors;
    Monitor* m_selectedMonitor{nullptr};

    // Attributes
    std::unique_ptr<Display, DisplayDeleter> m_display;
    std::optional<XShmData> m_xshmData;
  };
}
