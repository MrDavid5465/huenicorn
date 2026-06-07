find_package(OpenCV REQUIRED COMPONENTS imgproc opencv_highgui)

target_sources(huenicorn PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Adapters/Windows/WindowsAdapter.cpp
)

target_link_libraries(huenicorn PUBLIC ws2_32 mswsock crypt32)
