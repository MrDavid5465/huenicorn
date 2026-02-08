find_library(COREGRAPHICS CoreGraphics)

find_package(OpenCV REQUIRED COMPONENTS imgproc opencv_highgui)

target_sources(huenicorn PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Platform/Adapters/MacOS/MacOSAdapter.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Adapters/MacOS/MacOSAdapter.mm
)

target_include_directories(huenicorn PUBLIC
  /usr/local/include/crow # Ewww ToDo : remove
)
