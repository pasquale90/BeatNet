include(ExternalProject)

set(LIBSAMPLERATE_VERSION "0.2.2")
set(LIBSAMPLERATE_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/libsamplerate")

if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(LIBSAMPLERATE_URL "https://github.com/libsndfile/libsamplerate/releases/download/${LIBSAMPLERATE_VERSION}/libsamplerate-${LIBSAMPLERATE_VERSION}-win64.zip")
    else()
        set(LIBSAMPLERATE_URL "https://github.com/libsndfile/libsamplerate/releases/download/${LIBSAMPLERATE_VERSION}/libsamplerate-${LIBSAMPLERATE_VERSION}-win32.zip")
    endif()

    ExternalProject_Add(libsamplerate_binary
        PREFIX ${CMAKE_BINARY_DIR}/_deps/libsamplerate
        URL ${LIBSAMPLERATE_URL}
        DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/_downloads
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${LIBSAMPLERATE_INSTALL_DIR}
        LOG_DOWNLOAD ON
    )

else()
    # Linux / macOS: build from source
    set(LIBSAMPLERATE_TAR_URL "https://github.com/libsndfile/libsamplerate/releases/download/${LIBSAMPLERATE_VERSION}/libsamplerate-${LIBSAMPLERATE_VERSION}.tar.xz")

    ExternalProject_Add(libsamplerate_binary
        PREFIX ${CMAKE_BINARY_DIR}/_deps/libsamplerate
        URL ${LIBSAMPLERATE_TAR_URL}
        DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/_downloads
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${LIBSAMPLERATE_INSTALL_DIR} --disable-shared
        BUILD_COMMAND make -j
        INSTALL_COMMAND make install
        BUILD_IN_SOURCE 1
        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
    )
endif()

set(LIBSAMPLERATE_DIR ${LIBSAMPLERATE_INSTALL_DIR})