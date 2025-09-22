include(FetchContent)

set(LIBSAMPLERATE_VERSION "0.2.2")
set(LIBSAMPLERATE_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/libsamplerate")

if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(LIBSAMPLERATE_URL "https://github.com/libsndfile/libsamplerate/releases/download/${LIBSAMPLERATE_VERSION}/libsamplerate-${LIBSAMPLERATE_VERSION}-win64.zip")
    else()
        set(LIBSAMPLERATE_URL "https://github.com/libsndfile/libsamplerate/releases/download/${LIBSAMPLERATE_VERSION}/libsamplerate-${LIBSAMPLERATE_VERSION}-win32.zip")
    endif()

    FetchContent_Declare(
        libsamplerate_binary
        URL ${LIBSAMPLERATE_URL}
        SOURCE_DIR ${LIBSAMPLERATE_INSTALL_DIR}
    )

    FetchContent_GetProperties(libsamplerate_binary)
    FetchContent_MakeAvailable(libsamplerate_binary)

    add_custom_target(libsamplerate_ready COMMENT "libsamplerate has been fetched")

else()
    # Linux / macOS: build from source
    set(LIBSAMPLERATE_URL "https://github.com/libsndfile/libsamplerate/releases/download/${LIBSAMPLERATE_VERSION}/libsamplerate-${LIBSAMPLERATE_VERSION}.tar.xz")
    
    FetchContent_Declare(
        libsamplerate_src
        URL ${LIBSAMPLERATE_URL}
        SOURCE_SUBDIR ${LIBSAMPLERATE_INSTALL_DIR}
    )

    FetchContent_GetProperties(libsamplerate_src)
    FetchContent_MakeAvailable(libsamplerate_src)

    add_custom_target(libsamplerate_binary ALL
        COMMAND ./configure --prefix=${LIBSAMPLERATE_INSTALL_DIR} --disable-shared
        COMMAND make -j
        COMMAND make install
        WORKING_DIRECTORY ${libsamplerate_src_SOURCE_DIR}
        COMMENT "Building and installing libsamplerate"
    )

    FetchContent_GetProperties(libsamplerate_binary)
    FetchContent_MakeAvailable(libsamplerate_binary)

    add_custom_target(libsamplerate_ready COMMENT "libsamplerate has been fetched")
    
endif()

set(LIBSAMPLERATE_DIR ${LIBSAMPLERATE_INSTALL_DIR})