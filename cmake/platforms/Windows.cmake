find_package(OpenCV REQUIRED COMPONENTS imgproc opencv_highgui)

target_sources(${BINARY_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/Adapters/Windows/WindowsAdapter.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Adapters/Windows/WindowsAdapter.cpp
)

target_include_directories(${BINARY_NAME} PUBLIC
  C:/msys64/usr/local/include # TODO: Find a robust way to locate Crow
)

target_link_libraries(${BINARY_NAME} PUBLIC ws2_32 mswsock crypt32)
