# Adapter
target_sources(huenicorn PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Platform/Adapters/GnuLinux/GnuLinuxAdapter.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Adapters/GnuLinux/GnuLinuxAdapter.cpp
)

# Check grabber-related libraries
# Begin X11-related
find_package(X11)
# End X11-related

# Begin Pipewire-related
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
find_package(Gio)
pkg_search_module(LIB_PIPEWIRE OPTIONAL libpipewire-0.3)
pkg_check_modules(LIBGLIB glib-2.0 gio-2.0)
# End Pipewire-related


if(${X11_FOUND})
  set(X11_GRABBER_AVAILABLE TRUE)
  message("Able to build X11 Grabber !")

  set(X11_GRABBER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Grabber/GnuLinux/X11/X11Grabber.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Grabber/GnuLinux/X11/X11Grabber.hpp
  )
endif()

if(LIB_PIPEWIRE_FOUND AND GIO_FOUND AND LIBGLIB_FOUND)
  set(PIPEWIRE_GRABBER_AVAILABLE TRUE)
  message("Able to build Pipewire Grabber !")

  set(PIPEWIRE_GRABBER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Grabber/GnuLinux/Pipewire/PipewireGrabber.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Grabber/GnuLinux/Pipewire/XdgDesktopPortal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Grabber/GnuLinux/Pipewire/PipewireGrabber.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Grabber/GnuLinux/Pipewire/XdgDesktopPortal.hpp
  )
endif()


if(NOT (PIPEWIRER_GRABBER_AVAILABLE OR X11_GRABBER_AVAILABLE))
  message(FATAL_ERROR "Missing dependencies to compile a least one grabber")
endif()

if(X11_GRABBER_AVAILABLE)
  target_compile_definitions(huenicorn PUBLIC X11_GRABBER_AVAILABLE=1)
  target_sources(huenicorn PRIVATE
    ${X11_GRABBER_SOURCES}
  )

  target_link_libraries(huenicorn PUBLIC
    X11::X11
    X11::Xext
    X11::Xrandr
  )
endif(X11_GRABBER_AVAILABLE)


if(PIPEWIRE_GRABBER_AVAILABLE)
  target_compile_definitions(huenicorn PUBLIC PIPEWIRE_GRABBER_AVAILABLE=1)
  target_sources(huenicorn PRIVATE
    ${PIPEWIRE_GRABBER_SOURCES}
  )

  target_include_directories(huenicorn PUBLIC
    ${LIB_PIPEWIRE_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
  )

  target_link_libraries(huenicorn PUBLIC
    GIO::GIO
    ${LIB_PIPEWIRE_LIBRARIES}
    ${LIBGLIB_LIBRARIES}
  )
endif(PIPEWIRE_GRABBER_AVAILABLE)
