find_library(COREGRAPHICS CoreGraphics)

find_package(OpenCV REQUIRED COMPONENTS imgproc opencv_highgui)

target_sources(${BINARY_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Platform/Adapters/MacOS/MacOSAdapter.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Adapters/MacOS/MacOSAdapter.mm
)

target_include_directories(${BINARY_NAME} PUBLIC
  /usr/local/include/crow # Ewww ToDo : remove
)
