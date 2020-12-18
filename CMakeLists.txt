cmake_minimum_required(VERSION 3.17)

# export compiler commands
set(CMAKE_EXPORT_COMPILECOMMANDS ON)

# set vcpkg triplet and toolchain_file
if(WIN32)
    if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
    endif()
    if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
      set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
    elseif(NOT DEFINED VCPKG_TARGET_TRIPLET)
      set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
    endif()
    message("CMAKE_TOOLCHAIN_FILE: " ${CMAKE_TOOLCHAIN_FILE})
    message("VCPKG_TARGET_TRIPLET: " ${VCPKG_TARGET_TRIPLET})
endif()

project(Renderer VERSION 0.0.0)

# option
option(BUILD_SHARED_LIBS "True when building shared library" True)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

include(cmake/tools.cmake)

# set output directory
SET_OUTPUT_DIRECTORY(${CMAKE_BINARY_DIR})

# set system name
add_definitions(-DPLATFORM_OS=${CMAKE_SYSTEM_NAME})
add_definitions(-D${CMAKE_SYSTEM_NAME})

# set build type (default Release)
SET_DEFAULT_BUILD_TYPE()
#set(CMAKE_BUILD_TYPE "Debug")

# set cpp standard 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# library header file and source file
file(GLOB_RECURSE Header include/*.h)
file(GLOB_RECURSE Source src/*.cpp)

# dependent library 
set(DependentLibraries glfw Vulkan::Vulkan )
if(UNIX AND APPLE)
    list(APPEND DependentLibraries glm::glm)
else()
    list(APPEND DependentLibraries glm)
endif()


# cmake set library 
if(BUILD_SHARED_LIBS)
add_library(${PROJECT_NAME} STATIC 
    ${Header}
    ${Source})
else()
add_library(${PROJECT_NAME} SHARED 
    ${Header}
    ${Source})
endif()

target_link_libraries(${PROJECT_NAME} 
    ${DependentLibraries}
    )
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>    #${CMAKE_INSTALL_PREFIX}/include
    )

#set target postfix 
SET_TARGET_POSTFIX(${PROJECT_NAME})

# configure git version control
set(PRE_CONFIGURE_FILE "cmake/Git.h.in")
set(POST_CONFIGURE_FILE "include/${PROJECT_NAME}/Git.h")
include(cmake/git_watcher.cmake)
add_dependencies(${PROJECT_NAME} check_git)

# test
add_subdirectory(test)

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

