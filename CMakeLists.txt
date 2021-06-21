cmake_minimum_required(VERSION 3.19)


if(NOT DRIVER)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}\\scripts\\buildsystems\\vcpkg.cmake" CACHE STRING "" FORCE)
message(STATUS "Use toolchain file ${CMAKE_TOOLCHAIN_FILE}")

set(VCPKG_TARGET_TRIPLET "x86-windows-static")
message(STATUS "Use triplet ${VCPKG_TARGET_TRIPLET}")

project(automation CXX)

message(STATUS "${CMAKE_SYSTEM_NAME}")

if(NOT CMAKE_SYSTEM_NAME MATCHES "Windows")
  message(FATAL_ERROR "Please build on Windows")
endif()

message(STATUS "Welcome Build")

if(NOT DEFINED ENV{VCPKG_ROOT})
  message(FATAL_ERROR "Need vcpkg")
endif()

message(STATUS "Base Flags ${CMAKE_CXX_FLAGS}") 
message(STATUS "Release Flags ${CMAKE_CXX_FLAGS_RELEASE}") 


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED ${CMAKE_BUILD_TYPE})
  message(STATUS "Not define build type, default Release")
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenCV CONFIG REQUIRED)
if(OpenCV_FOUND)
  message(STATUS "Found ${OpenCV_DIR} ${OpenCV_LIBS}")
else() 
  message(FATAL_ERROR, "Can not found Opencv library")
endif()

add_executable(main 
  src/main.cpp
  src/driver_control.cpp
  )

set_property(TARGET main PROPERTY
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


set(LUA_INCLUDE_DIR $ENV{VCPKG_ROOT}/installed/x86-windows-static/include)
if(CMAKE_BUILD_TYPE STREQUAL "Release") 
set(LUA_LIBRARIES $ENV{VCPKG_ROOT}/installed/x86-windows-static/lib/lua.lib)
else()
set(LUA_LIBRARIES $ENV{VCPKG_ROOT}/installed/x86-windows-static/debug/lib/lua.lib)
endif()

message(STATUS "Lua ${LUA_INCLUDE_DIR}")
message(STATUS "Lua ${LUA_LIBRARIES}")


target_compile_options(main PRIVATE /utf-8)

target_include_directories(main PRIVATE src ${LUA_INCLUDE_DIR})

target_link_libraries(main PRIVATE ${OpenCV_LIBS} ${LUA_LIBRARIES})

else()

project(automation CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED ${CMAKE_BUILD_TYPE})
  message(STATUS "Not define build type, default Release")
  set(CMAKE_BUILD_TYPE Release)
endif()

list(APPEND CMAKE_MODULE_PATH "cmake")
find_package(WDK REQUIRED)

if(WDK_FOUND)
message(STATUS "Find WDK ${WDK_ROOT}")
endif()

wdk_add_driver(click 
  src/driver/dispatch.cpp
  src/driver/mouse.cpp
  src/driver/memory.cpp
  src/driver/driver.cpp
)
target_compile_options(click PRIVATE /utf-8)
target_include_directories(click PRIVATE src/driver)

endif()