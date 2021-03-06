cmake_minimum_required(VERSION 3.7)
project(Vulkan)

find_package(PythonInterp 2)
find_package(Vulkan REQUIRED)

if(NOT PYTHONINTERP_FOUND OR UNIX)
#    if(APPLE)
        set(PYTHON_EXECUTABLE python)
#    else()
#        message( FATAL_ERROR "Python 3 not found" )
#    endif()
endif()


include(../../CMake/LibraryTarget.cmake)
include(ExternalProject)

ExternalProject_Add(spirvcross
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
        GIT_TAG refs/tags/2020-04-03
        GIT_SHALLOW ON
        GIT_PROGRESS ON
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        BUILD_BYPRODUCTS
            ${CMAKE_CURRENT_BINARY_DIR}/spirvcross-prefix/src/spirvcross/spirv_cfg.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/spirvcross-prefix/src/spirvcross/spirv_cross_parsed_ir.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/spirvcross-prefix/src/spirvcross/spirv_parser.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/spirvcross-prefix/src/spirvcross/spirv_cross.cpp
        INSTALL_COMMAND "")


set(VERSION_MAJOR 0)
set(VERSION_MINOR 5)
set(VERSION_PATCH 0)
set(VERSION_ABI 1)
set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Sources/RNVulkanDispatchTable.h ${CMAKE_CURRENT_SOURCE_DIR}/Sources/RNVulkanDispatchTable.cpp
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/generators/generate-dispatch-table ${CMAKE_CURRENT_SOURCE_DIR}/Sources/RNVulkanDispatchTable.h
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/generators/generate-dispatch-table ${CMAKE_CURRENT_SOURCE_DIR}/Sources/RNVulkanDispatchTable.cpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/generators/generate-dispatch-table
        COMMENT "Generate Vulkan Dispatch Table")

add_custom_target(VulkanGenerateDispatchTable DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Sources/RNVulkanDispatchTable.h ${CMAKE_CURRENT_SOURCE_DIR}/Sources/RNVulkanDispatchTable.cpp)


set(SOURCES
        Sources/RNVulkanDebug.cpp
        Sources/RNVulkanDevice.cpp
        Sources/RNVulkanDispatchTable.cpp
        Sources/RNVulkanFramebuffer.cpp
        Sources/RNVulkanGPUBuffer.cpp
        Sources/RNVulkanInstance.cpp
        Sources/RNVulkanRenderer.cpp
        Sources/RNVulkanRendererDescriptor.cpp
        Sources/RNVulkanTexture.cpp
        Sources/RNVulkanWindow.cpp
        Sources/RNVulkanSwapChain.cpp
        Sources/RNVulkanInit.cpp
        Sources/RNVulkanInternals.cpp
        Sources/RNVulkanShader.cpp
        Sources/RNVulkanShaderLibrary.cpp
        Sources/RNVulkanStateCoordinator.cpp
        Sources/RNVulkanConstantBuffer.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/spirvcross-prefix/src/spirvcross/spirv_cfg.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/spirvcross-prefix/src/spirvcross/spirv_cross_parsed_ir.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/spirvcross-prefix/src/spirvcross/spirv_parser.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/spirvcross-prefix/src/spirvcross/spirv_cross.cpp)

set(HEADERS
        Sources/RNVulkan.h
        Sources/RNVulkanDebug.h
        Sources/RNVulkanDevice.h
        Sources/RNVulkanDispatchTable.h
        Sources/RNVulkanFramebuffer.h
        Sources/RNVulkanGPUBuffer.h
        Sources/RNVulkanInstance.h
        Sources/RNVulkanRenderer.h
        Sources/RNVulkanRendererDescriptor.h
        Sources/RNVulkanTexture.h
        Sources/RNVulkanWindow.h
        Sources/RNVulkanSwapChain.h
        Sources/RNVulkanInternals.h
        Sources/RNVulkanShader.h
        Sources/RNVulkanShaderLibrary.h
        Sources/RNVulkanStateCoordinator.h
        Sources/RNVulkanConstantBuffer.h
        #../../Vendor/SPIRV-Cross/spirv.hpp
        #../../Vendor/SPIRV-Cross/spirv_common.hpp
        #../../Vendor/SPIRV-Cross/spirv_cfg.hpp
        #../../Vendor/SPIRV-Cross/spirv_cross_parsed_ir.hpp
        #../../Vendor/SPIRV-Cross/spirv_parser.hpp
        #../../Vendor/SPIRV-Cross/spirv_cross.hpp
        )

set(RESOURCES
        Resources)

set(DEFINES "RN_BUILD_VULKAN")

if(WIN32)
    set(DEFINES ${DEFINES} "VK_USE_PLATFORM_WIN32_KHR")
elseif(ANDROID)
    set(DEFINES ${DEFINES} "VK_USE_PLATFORM_ANDROID_KHR")
elseif(UNIX AND NOT APPLE)
    set(DEFINES ${DEFINES} "VK_USE_PLATFORM_XCB_KHR")
endif()

set(RAYNE_LIBRARIES Rayne)

rayne_add_library(RayneVulkan "${SOURCES}" "${HEADERS}" "${RAYNE_LIBRARIES}" "${VERSION_STRING}" "${VERSION_ABI}")
rayne_set_module_output_directory(RayneVulkan)
rayne_set_module_resources(RayneVulkan ${RESOURCES})

add_dependencies(RayneVulkan_copyHeaderTarget VulkanGenerateDispatchTable)
add_dependencies(RayneVulkan_copyHeaderTarget spirvcross)

target_include_directories(RayneVulkan SYSTEM PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/spirvcross-prefix/src/spirvcross)
target_include_directories(RayneVulkan SYSTEM PRIVATE ${Rayne_BINARY_DIR}/include)
target_include_directories(RayneVulkan SYSTEM PRIVATE ${Vulkan_INCLUDE_DIR})
target_compile_definitions(RayneVulkan PRIVATE "${DEFINES}")

rayne_install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../../Build/RayneVulkan/. DESTINATION lib/Rayne/RayneVulkan)
