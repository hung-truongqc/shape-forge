cmake_minimum_required(VERSION 3.16)
project(shape-forge VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build configuration flag
option(BUILD_RELEASE "Build in release mode" ON)

# Set build type and output directories based on BUILD_RELEASE flag
if(BUILD_RELEASE)
    set(CMAKE_BUILD_TYPE Release)
    set(BUILD_DIR_SUFFIX "release")
    message(STATUS "Building in RELEASE mode")
else()
    set(CMAKE_BUILD_TYPE Debug)
    set(BUILD_DIR_SUFFIX "debug")
    message(STATUS "Building in DEBUG mode")
endif()

# Set output directories with build-specific subdirectories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${BUILD_DIR_SUFFIX})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${BUILD_DIR_SUFFIX})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${BUILD_DIR_SUFFIX})

# Platform detection
if(WIN32)
    set(PLATFORM_NAME "windows")
elseif(APPLE)
    set(PLATFORM_NAME "macos")
else()
    set(PLATFORM_NAME "linux")
endif()

# Third-party libraries path
set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparties/${PLATFORM_NAME})

# Find required system packages
find_package(OpenGL REQUIRED)

# GLFW setup - use static library
if(${PLATFORM_NAME} STREQUAL "linux")
    set(GLFW_STATIC_LIB ${CMAKE_SOURCE_DIR}/thirdparties/linux/glfw/build/src/libglfw3.a)
    set(GLFW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparties/linux/glfw/include)
    
    # Check if the static library exists
    if(NOT EXISTS ${GLFW_STATIC_LIB})
        message(FATAL_ERROR "GLFW static library not found at: ${GLFW_STATIC_LIB}")
    endif()
    
    # Create imported target for GLFW
    add_library(glfw3_static STATIC IMPORTED)
    set_target_properties(glfw3_static PROPERTIES
        IMPORTED_LOCATION ${GLFW_STATIC_LIB}
        INTERFACE_INCLUDE_DIRECTORIES ${GLFW_INCLUDE_DIR}
    )
    
    set(GLFW_LIBRARIES glfw3_static)
elseif(${PLATFORM_NAME} STREQUAL "windows")
    set(GLFW_STATIC_LIB ${CMAKE_SOURCE_DIR}/thirdparties/windows/glfw/build/src/release/glfw3.lib)
    set(GLFW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparties/windows/glfw/include)
    
    # Check if the static library exists
    if(NOT EXISTS ${GLFW_STATIC_LIB})
        message(FATAL_ERROR "GLFW static library not found at: ${GLFW_STATIC_LIB}")
    endif()
    
    # Create imported target for GLFW
    add_library(glfw3_static STATIC IMPORTED)
    set_target_properties(glfw3_static PROPERTIES
        IMPORTED_LOCATION ${GLFW_STATIC_LIB}
        INTERFACE_INCLUDE_DIRECTORIES ${GLFW_INCLUDE_DIR}
    )
    
    set(GLFW_LIBRARIES glfw3_static)
    target_include_directories(gl3w PUBLIC thirdparties/windows/gl3w/include)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    # For other platforms, use find_package
    find_package(glfw3 REQUIRED)
    set(GLFW_LIBRARIES glfw)
endif()

# Platform-specific OpenGL loader setup
if(APPLE)
    # APPLE to be done at latter date
    # TBD
else()
    # Linux/ uses gl3w
    set(GL3W_DIR ${THIRDPARTY_DIR}/gl3w)
    set(OPENGL_LOADER_LIBS)
    set(OPENGL_LOADER_INCLUDE ${GL3W_DIR}/include)
endif()

# Dear ImGui setup
set(IMGUI_DIR ${THIRDPARTY_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
# Add gl3w source for Window/Linux, Mac is not implement yet
if(WIN32 OR UNIX)
    list(APPEND IMGUI_SOURCES ${GL3W_DIR}/src/gl3w.c)
endif()

# Create main executable
set(MAIN_SOURCES
    src/gui/shape_clipboard.cpp
    src/gui/shape_editor_application.cpp
    src/gui/shape_editor_gui.cpp
    src/main.cpp
    ${IMGUI_SOURCES}
)

add_executable(${PROJECT_NAME} ${MAIN_SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${OPENGL_LOADER_INCLUDE}
)

# Add GLFW include directory for Linux
if(${PLATFORM_NAME} STREQUAL "linux")
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW_INCLUDE_DIR})
endif()

# Platform-specific includes
if(NOT WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIRS})
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME}
    OpenGL::GL
    ${GLFW_LIBRARIES}
    ${OPENGL_LOADER_LIBS}
)

# Platform-specific linking
if(WIN32)
    target_link_libraries(${PROJECT_NAME} 
        user32 
        gdi32 
        shell32
        gl3w
    )
else()
    target_link_libraries(${PROJECT_NAME} 
        ${CMAKE_DL_LIBS}
        pthread
    )
    
    # For static GLFW on Linux, we need to link additional system libraries
    if(${PLATFORM_NAME} STREQUAL "linux")
        target_link_libraries(${PROJECT_NAME}
            X11
            Xrandr
            Xinerama
            Xcursor
            Xi
            m
        )
    endif()
endif()

# Build-specific compiler options
if(BUILD_RELEASE)
    # Release build optimizations
    if(MSVC)
        add_definitions(-DNOMINMAX) # Avoid conflict with Window own min/max
        # Experimental for Window build, currently has not been test on Window
        target_compile_options(${PROJECT_NAME} PRIVATE /W4 /DNDEBUG)
        target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -DNDEBUG)
    endif()
else()
    # Debug build options
    if(MSVC)
        add_definitions(-DNOMINMAX) # Avoid conflict with Window own min/max
        # Experimental for Window build, currently has not been test on Window
        target_compile_options(${PROJECT_NAME} PRIVATE /W4 /Od /Zi /DEBUG)
        target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS _DEBUG)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -g -O0 -D_DEBUG)
    endif()
endif()

# Install targets with build-specific directories
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin/${BUILD_DIR_SUFFIX}
    LIBRARY DESTINATION lib/${BUILD_DIR_SUFFIX}
    ARCHIVE DESTINATION lib/${BUILD_DIR_SUFFIX}
)

# Print configuration summary
message(STATUS "Configuration Summary:")
message(STATUS "  Platform: ${PLATFORM_NAME}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Release Flag: ${BUILD_RELEASE}")
message(STATUS "  Output Directory: bin/${BUILD_DIR_SUFFIX}")
message(STATUS "  Third-party Directory: ${THIRDPARTY_DIR}")
if(${PLATFORM_NAME} STREQUAL "linux")
    message(STATUS "  GLFW Static Library: ${GLFW_STATIC_LIB}")
    message(STATUS "  GLFW Include Directory: ${GLFW_INCLUDE_DIR}")
endif()
message(STATUS "  OpenGL Loader: ${OPENGL_LOADER_LIBS}")