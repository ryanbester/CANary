cmake_minimum_required(VERSION 3.28.3)
project(canary)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib/glfw)
add_subdirectory(lib/json)

#find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
#find_package(PcapPlusPlus REQUIRED)

file(GLOB IMGUI_SRC "lib/imgui/*.h" "lib/imgui/*.cpp")
list(APPEND IMGUI_SRC lib/imgui/backends/imgui_impl_glfw.h lib/imgui/backends/imgui_impl_glfw.cpp lib/imgui/backends/imgui_impl_opengl3.h lib/imgui/backends/imgui_impl_opengl3.cpp)

add_executable(canary lib/glad/src/glad.c ${IMGUI_SRC} lib/imguifiledialog/ImGuiFileDialog.cpp src/main.cpp
        src/config.cpp
        src/config.hpp
        src/dbc.cpp
        src/dbc.hpp
        src/gui/gui.cpp
        src/gui/gui.hpp
        src/can/socketcand.cpp
        src/can/socketcand.hpp
        src/can/packetprovider.cpp
        src/can/packetprovider.hpp
        src/cmd/commanddispatcher.cpp
        src/cmd/commanddispatcher.hpp
        src/cmd/commandbase.hpp
        src/cmd/helpcmd.cpp
        src/cmd/helpcmd.hpp
        src/gui/cmdline.cpp
        src/gui/cmdline.hpp
        src/socket.cpp
        src/socket.hpp
)

# Copy font resources
file(COPY lib/imgui/misc/fonts/ProggyClean.ttf DESTINATION ${CMAKE_BINARY_DIR}/resources/fonts)
file(COPY lib/imgui/misc/fonts/Roboto-Medium.ttf DESTINATION ${CMAKE_BINARY_DIR}/resources/fonts)

target_link_libraries(canary PRIVATE glfw OpenGL::GL ${OPENGL_gl_LIBRARY} nlohmann_json::nlohmann_json)

if (WIN32)
    target_link_libraries(canary PRIVATE Shcore.lib ws2_32.lib)
endif ()

target_include_directories(canary PRIVATE lib/glad/include lib/glfw-3.4/include lib/imgui lib/imguifiledialog)
target_link_directories(canary PRIVATE lib/nativefiledialog/lib)
