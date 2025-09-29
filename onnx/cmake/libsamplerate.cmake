include(FetchContent)

set(LIBSAMPLERATE_VERSION "0.2.2")
set(LIBSAMPLERATE_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/libsamplerate")
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

if (WIN32)
    if (NOT EXISTS "${SAMPLERATE_DIR}/lib/libsamplerate.dll")

        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(LIBSAMPLERATE_URL "https://github.com/libsndfile/libsamplerate/releases/download/${LIBSAMPLERATE_VERSION}/libsamplerate-${LIBSAMPLERATE_VERSION}-win64.zip")
        else()
            set(LIBSAMPLERATE_URL "https://github.com/libsndfile/libsamplerate/releases/download/${LIBSAMPLERATE_VERSION}/libsamplerate-${LIBSAMPLERATE_VERSION}-win32.zip")
        endif()

        FetchContent_Declare(
            libsamplerate_binary
            URL ${LIBSAMPLERATE_URL}
            SOURCE_DIR ${LIBSAMPLERATE_INSTALL_DIR}
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )

        FetchContent_GetProperties(libsamplerate_binary)
        FetchContent_MakeAvailable(libsamplerate_binary)
    endif()
    add_custom_target(libsamplerate_ready COMMENT "libsamplerate has been fetched")
else()

    if (APPLE)
        set(LIB_EXT "dylib")
    elseif(UNIX)
        set(LIB_EXT "so")
    endif()

    file(GLOB SAMPLERATE_DLIB
        "${SAMPLERATE_DIR}/lib/libsamplerate.${LIB_EXT}"
        "${SAMPLERATE_DIR}/lib64/libsamplerate.${LIB_EXT}")

    if (NOT EXISTS "${SAMPLERATE_DLIB}")
        
        set(LIBSAMPLERATE_URL "https://github.com/libsndfile/libsamplerate/releases/download/${LIBSAMPLERATE_VERSION}/libsamplerate-${LIBSAMPLERATE_VERSION}.tar.xz")
        
        FetchContent_Declare(
            libsamplerate_src
            URL ${LIBSAMPLERATE_URL}
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(libsamplerate_src)
        
        execute_process(
            COMMAND ./configure --prefix=${LIBSAMPLERATE_INSTALL_DIR} --disable-static --enable-shared 
            WORKING_DIRECTORY ${libsamplerate_src_SOURCE_DIR}
        )
        execute_process(
            COMMAND make -j
            WORKING_DIRECTORY ${libsamplerate_src_SOURCE_DIR}
        )
        execute_process(
            COMMAND make install
            WORKING_DIRECTORY ${libsamplerate_src_SOURCE_DIR}
        )
    endif()
    add_custom_target(libsamplerate_ready COMMENT "libsamplerate has been fetched")
endif()

set(SAMPLERATE_DIR ${LIBSAMPLERATE_INSTALL_DIR} CACHE PATH "Path to libsamplerate")