# cmake/FetchFFTW3.cmake

include(ExternalProject)

# Destination directory for the extracted/built library
set(FFTW3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/fftw")
set(FFTW3_VERSION "3.3.10")  # version for Linux/macOS
set(FFTW3_WIN_VERSION "3.3.5")  # version for Windows DLLs

# Determine platform and architecture
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(OS_TAG "windows")

    # Normalize architecture
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" ARCH_TAG)
    if (ARCH_TAG MATCHES "^(amd64|x86_64|x64)$")
        set(FFTW_ARCH "dll64")
    elseif (ARCH_TAG MATCHES "^(x86|i386|i686)$")
        set(FFTW_ARCH "dll32")
    else()
        message(FATAL_ERROR "Unsupported Windows architecture: ${ARCH_TAG}")
    endif()

    # Construct Windows download URL
    set(FFTW3_URL "https://fftw.org/pub/fftw/fftw-${FFTW3_WIN_VERSION}-${FFTW_ARCH}.zip")
    set(FFTW3_ARCHIVE_NAME "fftw-${FFTW3_WIN_VERSION}-${FFTW_ARCH}.zip")

elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(OS_TAG "unix")
    set(FFTW3_URL "https://www.fftw.org/fftw-${FFTW3_VERSION}.tar.gz")
    set(FFTW3_ARCHIVE_NAME "fftw-${FFTW3_VERSION}.tar.gz")
else()
    message(FATAL_ERROR "Unsupported OS: ${CMAKE_SYSTEM_NAME}")
endif()

message(STATUS "FFTW3 download URL: ${FFTW3_URL}")

# Check if the library already exists
if (NOT EXISTS "${FFTW3_DIR}/lib")
    if (OS_TAG STREQUAL "windows")
        ExternalProject_Add(fftw3_binary
            PREFIX ${CMAKE_BINARY_DIR}/_deps/fftw3
            URL ${FFTW3_URL}
            DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/_downloads
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${FFTW3_DIR}
            LOG_DOWNLOAD ON
        )
    else()
        # Unix systems — build from source (static float-only lib)
        ExternalProject_Add(fftw3_binary
            PREFIX ${CMAKE_BINARY_DIR}/_deps/fftw3
            URL ${FFTW3_URL}
            DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/_downloads
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
            CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-float --disable-shared --prefix=${FFTW3_DIR}
            BUILD_COMMAND make
            INSTALL_COMMAND make install
            LOG_DOWNLOAD ON
            LOG_CONFIGURE ON
            LOG_BUILD ON
            LOG_INSTALL ON
        )
    endif()
    set(FFTW3_DEP fftw3_binary PARENT_SCOPE)
else()
    message(STATUS "FFTW3 already exists at ${FFTW3_DIR}, skipping download.")
    set(FFTW3_DEP "" PARENT_SCOPE)
endif()

# Export variable to parent CMakeLists
set(FFTW3_DIR ${FFTW3_DIR} PARENT_SCOPE)
