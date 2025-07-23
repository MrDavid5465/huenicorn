find_library(COREGRAPHICS CoreGraphics)

find_package(OpenCV REQUIRED COMPONENTS imgproc opencv_highgui)

target_sources(${BINARY_NAME} PRIVATE
  include/Huenicorn/Platforms/MacOS/MacOSAdapter.hpp
  src/Platforms/MacOS/MacOSAdapter.mm
)

target_include_directories(${BINARY_NAME} PUBLIC
  /usr/local/include/crow # Ewww ToDo : remove
)
