include(FetchContent)

# Determine the current platform (OS and architecture)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(OS_TAG "win")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(OS_TAG "linux")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(OS_TAG "osx")
else()
    message(FATAL_ERROR "Unsupported OS: ${CMAKE_SYSTEM_NAME}")
endif()

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" ARCH_TAG)

if (ARCH_TAG MATCHES "^(amd64|x86_64)$")
    set(ARCH_TAG "x64")
elseif (ARCH_TAG MATCHES "^(arm64|aarch64)$")
    set(ARCH_TAG "arm64")
elseif (ARCH_TAG MATCHES "^(x86|i386|i686)$")
    set(ARCH_TAG "x86")
endif()

set(ONNX_VERSION "1.22.0")

if (OS_TAG STREQUAL "osx" AND ARCH_TAG STREQUAL "x64")
    set(ONNX_RELEASE_NAME "onnxruntime-osx-x86_64-${ONNX_VERSION}.tgz")
elseif (OS_TAG STREQUAL "osx" AND ARCH_TAG STREQUAL "arm64")
    set(ONNX_RELEASE_NAME "onnxruntime-osx-arm64-${ONNX_VERSION}.tgz")
elseif (OS_TAG STREQUAL "osx")
    set(ONNX_RELEASE_NAME "onnxruntime-osx-universal2-${ONNX_VERSION}.tgz")
elseif (OS_TAG STREQUAL "linux")
    set(ONNX_RELEASE_NAME "onnxruntime-linux-${ARCH_TAG}-${ONNX_VERSION}.tgz")
elseif (OS_TAG STREQUAL "win")
    set(ONNX_RELEASE_NAME "onnxruntime-${OS_TAG}-${ARCH_TAG}-${ONNX_VERSION}.zip")
endif()

set(ONNX_URL "https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/${ONNX_RELEASE_NAME}")
set(ONNX_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/onnxruntime")

if (NOT EXISTS "${ONNX_RUNTIME_DIR}/lib/onnxruntime.lib")
    message(STATUS "Downloading ONNX Runtime from ${ONNX_URL}")
    FetchContent_Declare(
        onnxruntime_binary
        URL ${ONNX_URL}
        SOURCE_DIR ${ONNX_RUNTIME_DIR}
    )
    FetchContent_MakeAvailable(onnxruntime_binary)

else()
    message(STATUS "ONNX Runtime already exists at ${ONNX_RUNTIME_DIR}, skipping download.")
endif()

set(ORT_DIR ${ONNX_RUNTIME_DIR})
