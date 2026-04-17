cmake_minimum_required(VERSION 3.15)
project(KeySound C)

# Ensure static runtime for MSVC
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Mark WIN32 to ensure it builds as a Windows GUI application (no console)
add_executable(${PROJECT_NAME} WIN32 keysound.c resource.rc)

# Link winmm and configure static linking for GCC
if(MINGW OR CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm)
    target_link_options(${PROJECT_NAME} PRIVATE "-static")
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm.lib)
endif()
