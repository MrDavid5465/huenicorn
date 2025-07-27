message("Building tests")

find_package(OpenCV REQUIRED COMPONENTS imgproc opencv_highgui)

add_executable(TestImageProcessing
  #Huenicorn
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/ImageProcessing.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Interpolation.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Logger.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ImageProcessing.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Interpolation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp

  # Test
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/src/TestImageProcessing.cpp
)

set_target_properties(TestImageProcessing PROPERTIES CXX_STANDARD 20)

target_include_directories(TestImageProcessing PUBLIC
  include
)

target_link_libraries(TestImageProcessing PUBLIC
  ${OpenCV_LIBS}
)


# TestGrabber
add_executable(TestGrabber
  #Huenicorn
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Config.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Credentials.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/DummyGrabber.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/ImageProcessing.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Interpolation.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Logger.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/MonitorData.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/PlatformSelector.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Grabbers/GnuLinux/X11/X11MonitorWatcher.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Grabbers/GnuLinux/X11/X11MonitorWatcher.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Grabbers/GnuLinux/X11/INotifiable.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Config.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Credentials.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/DummyGrabber.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ImageProcessing.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Interpolation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PlatformSelector.cpp

  # Test
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/src/TestGrabber.cpp
)

target_sources(TestGrabber PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/Adapters/GnuLinux/GnuLinuxAdapter.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Adapters/GnuLinux/GnuLinuxAdapter.cpp
)


if(UNIX AND NOT APPLE) # For Gnu/Linux
  #include(cmake/platforms/GnuLinux.cmake) # Already included from main CMakeLists.txt
  if(X11_GRABBER_AVAILABLE)
    target_compile_definitions(TestGrabber PUBLIC X11_GRABBER_AVAILABLE=1)
    target_sources(TestGrabber PRIVATE
      ${X11_GRABBER_SOURCES}
    )

    target_link_libraries(TestGrabber PUBLIC
      X11::X11
      X11::Xext
      X11::Xrandr
    )
  endif(X11_GRABBER_AVAILABLE)


  if(PIPEWIRE_GRABBER_AVAILABLE)
    target_compile_definitions(TestGrabber PUBLIC PIPEWIRE_GRABBER_AVAILABLE=1)
    target_sources(TestGrabber PRIVATE
      ${PIPEWIRE_GRABBER_SOURCES}
    )

    target_include_directories(TestGrabber PUBLIC
      ${LIB_PIPEWIRE_INCLUDE_DIRS}
      ${GIO_INCLUDE_DIRS}
      ${GLIB_INCLUDE_DIRS}
    )

    target_link_libraries(TestGrabber PUBLIC
      GIO::GIO
      ${LIB_PIPEWIRE_LIBRARIES}
      ${LIBGLIB_LIBRARIES}
    )
  endif(PIPEWIRE_GRABBER_AVAILABLE)
endif()


set_target_properties(TestGrabber PROPERTIES CXX_STANDARD 20)

target_include_directories(TestGrabber PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR} # For adapters and grabbers
  include
)

target_link_libraries(TestGrabber PUBLIC
  ${OpenCV_LIBS}
)