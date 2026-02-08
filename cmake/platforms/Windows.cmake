find_package(OpenCV REQUIRED COMPONENTS imgproc opencv_highgui)

target_sources(huenicorn PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Platform/Adapters/Windows/WindowsAdapter.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Adapters/Windows/WindowsAdapter.cpp
)

target_include_directories(huenicorn PUBLIC
  C:/msys64/usr/local/include # TODO: Find a robust way to locate Crow
)

target_link_libraries(huenicorn PUBLIC ws2_32 mswsock crypt32)
