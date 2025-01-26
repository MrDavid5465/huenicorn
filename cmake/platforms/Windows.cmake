find_package(OpenCV REQUIRED COMPONENTS imgproc opencv_highgui)

target_sources(${BINARY_NAME} PRIVATE
  include/Huenicorn/Platforms/Windows/WindowsAdapter.hpp
  src/Platforms/Windows/WindowsAdapter.cpp
)

target_include_directories(${BINARY_NAME} PUBLIC
  C:/msys64/usr/local/include # TODO: Find a robust way to locate Crow
)

target_link_libraries(${BINARY_NAME} PUBLIC ws2_32 mswsock crypt32)
