cmake_minimum_required(VERSION 3.17)

# set vcpkg triplet and toolchain_file
if(WIN32)
    if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    endif()
    if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
      set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
    elseif(NOT DEFINED VCPKG_TARGET_TRIPLET)
      set(VCPKG_TARGET_TRIPLET "x64-mingw-static" CACHE STRING "")
    endif()
    message("CMAKE_TOOLCHAIN_FILE: " ${CMAKE_TOOLCHAIN_FILE})
    message("VCPKG_TARGET_TRIPLET: " ${VCPKG_TARGET_TRIPLET})
endif()

project(gdf VERSION 0.0.0)

# option
option(BUILD_SHARED_LIBS "True when build shared library" True)
option(BUILD_TESTS "True when build tests library" True)

if(WIN32)
    # window macro define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # set windows export symbols(spawn .lib)
    #set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

find_package(glfw3 CONFIG REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Catch2 REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
if(NOT(UINX AND APPLE))
    find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
endif()
include(cmake/tools.cmake)

# set output directory
SET_OUTPUT_DIRECTORY(${CMAKE_BINARY_DIR})
# set build type (default Release)
SET_DEFAULT_BUILD_TYPE()
set(CMAKE_BUILD_TYPE "Debug")
message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# set system name
add_definitions(-DGDF_OS_${CMAKE_SYSTEM_NAME})
add_definitions(-DAPPLICATION_NAME="${PROJECT_NAME}")
add_definitions(-DGLFW_INCLUDE_NONE)
add_definitions(-DGLFW_INCLUDE_VULKAN)
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_definitions(-DGDF_DEBUG)
else()
    add_definitions(-DGDF_RELEASE)
endif()


# set cpp standard 
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# get ImGuiSourceFile
file(GLOB IMGUI_SRC lib/ImGui/*.cpp)

# library header file and source file
file(GLOB_RECURSE Header include/*.h)
file(GLOB_RECURSE Source src/*.cpp)

# dependent library 
set(DependentLibraries glfw Vulkan::Vulkan fmt::fmt nlohmann_json::nlohmann_json)
if(UNIX AND APPLE)
    list(APPEND DependentLibraries glm::glm)
else()
    list(APPEND DependentLibraries glm)
endif()

# cmake set library 
if(BUILD_SHARED_LIBS)
add_library(${PROJECT_NAME} SHARED
    ${IMGUI_SRC}
    ${Header}
    ${Source})
else()
    message(FATAL_ERROR "Can't build static library!")
endif()

target_link_libraries(${PROJECT_NAME} 
    ${DependentLibraries}
    )
if(UNIX AND APPLE)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/tinyglTF)
else()
    target_include_directories(${PROJECT_NAME} PRIVATE ${TINYGLTF_INCLUDE_DIRS})
endif()
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>    #${CMAKE_INSTALL_PREFIX}/include
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/exports>
#    $<INSTALL_INTERFACE:exports>    #${CMAKE_INSTALL_PREFIX}/include
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/ImGui>
    $<INSTALL_INTERFACE:/lib/ImGui>    #${CMAKE_INSTALL_PREFIX}/lib/ImGui
    )

# For windows export symbols
#include(GenerateExportHeader)
#generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/exports/gdf_export.h)

#set target postfix 
SET_TARGET_POSTFIX(${PROJECT_NAME})

# configure git version control
set(PRE_CONFIGURE_FILE "cmake/Git.h.in")
set(POST_CONFIGURE_FILE "include/Git.h")
include(cmake/git_watcher.cmake)
add_dependencies(${PROJECT_NAME} check_git)

# test
if(BUILD_TESTS)
    add_subdirectory(test)
endif()

# install targets and create export-set
install(TARGETS ${PROJECT_NAME} 
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION lib 
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

#install header file
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
    PATTERN "*.inl"
    PATTERN "*.in" EXCLUDE
    )

#generate and install export file 
install(EXPORT ${PROJECT_NAME}Targets 
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake
    )

include(CMakePackageConfigHelpers)

#set(version ${PROJECT_VERSION})
#set_property(TARGET ${PEOJECT_NAME} PROPERTY VERSION ${version})
#set_property(TARGET ${PEOJECT_NAME} PROPERTY SOVERSION 3)
#set_property(TARGET ${PEOJECT_NAME} PROPERTY INTERFACE_${PROJECT_NAME}_MAJOR_VERSION 3)
#set_property(TARGET ${PEOJECT_NAME} APPEND PROPERTY COMPATIBLE_INTERFACE_STRING ${PROJECT_NAME}_MAJOR_VERSION)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION lib/cmake
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION lib/cmake
)

export(EXPORT ${PROJECT_NAME}Targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Targets.cmake"
    NAMESPACE ${PROJECT_NAME}::
)