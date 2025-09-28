include(FetchContent)

set(FFTW3_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/fftw")
set(FFTW3_VERSION "3.3.10")
set(FFTW3_WIN_VERSION "3.3.5")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if (NOT EXISTS "${FFTW3_INSTALL_DIR}/lib/libfftw3f-3.dll")

        string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" ARCH_TAG)
        if (ARCH_TAG MATCHES "^(amd64|x86_64|x64)$")
            set(FFTW_ARCH "dll64")
        elseif (ARCH_TAG MATCHES "^(x86|i386|i686)$")
            set(FFTW_ARCH "dll32")
        else()
            message(FATAL_ERROR "Unsupported Windows architecture: ${ARCH_TAG}")
        endif()

        set(FFTW3_URL "https://fftw.org/pub/fftw/fftw-${FFTW3_WIN_VERSION}-${FFTW_ARCH}.zip")
        set(FFTW3_ZIP_NAME "fftw-${FFTW3_WIN_VERSION}-${FFTW_ARCH}.zip")
        set(FFTW3_DEF_FILE "${FFTW3_INSTALL_DIR}/libfftw3f-3.def")
        set(FFTW3_LIB_FILE "${FFTW3_INSTALL_DIR}/libfftw3f-3.lib")

        FetchContent_Declare(
            fftw3
            URL ${FFTW3_URL}
            SOURCE_DIR ${FFTW3_INSTALL_DIR}
            DOWNLOAD_EXTRACT_TIMESTAMP FALSE
        )
        FetchContent_GetProperties(fftw3)
        FetchContent_MakeAvailable(fftw3)

        # generate the .lib file using lib.exe, if it doesnt exist
        add_custom_command(
            OUTPUT ${FFTW3_LIB_FILE}
            COMMAND lib.exe /def:${FFTW3_DEF_FILE} /machine:${ARCH_TAG} /out:${FFTW3_LIB_FILE}
            COMMENT "Generating ${FFTW3_LIB_FILE} from ${FFTW3_DEF_FILE}"
            VERBATIM
        )
    endif()
    add_custom_target(fftw3_ready ALL DEPENDS ${FFTW3_LIB_FILE})
elseif(APPLE)
    if (NOT EXISTS "${FFTW3_INSTALL_DIR}/lib64/libfftw3f.dylib")
        set(FFTW3_URL "https://www.fftw.org/fftw-${FFTW3_VERSION}.tar.gz")

        FetchContent_Declare(
            fftw3
            URL ${FFTW3_URL}
            DOWNLOAD_EXTRACT_TIMESTAMP FALSE
        )

        FetchContent_MakeAvailable(fftw3)

        add_custom_target(fftw3_ready ALL
            COMMAND ./configure --enable-float --disable-static --enable-shared --prefix=${FFTW3_INSTALL_DIR}
            COMMAND make
            COMMAND make install
            WORKING_DIRECTORY ${fftw3_SOURCE_DIR}
            COMMENT "Building and installing FFTW3"
        )
    endif()
    add_custom_target(fftw3_ready ALL DEPENDS ${FFTW3_LIB_FILE})
elseif(UNIX)

    if (NOT EXISTS "${FFTW3_INSTALL_DIR}/lib/libfftw3f.so")
        
        set(FFTW3_URL "https://www.fftw.org/fftw-${FFTW3_VERSION}.tar.gz")

        FetchContent_Declare(
            fftw3
            URL ${FFTW3_URL}
            DOWNLOAD_EXTRACT_TIMESTAMP FALSE
        )
        FetchContent_MakeAvailable(fftw3)
        
        execute_process(
            COMMAND ./configure --enable-float --disable-static --enable-shared --prefix=${FFTW3_INSTALL_DIR}
            WORKING_DIRECTORY ${fftw3_SOURCE_DIR}
        )
        execute_process(
            COMMAND make -j4
            WORKING_DIRECTORY ${fftw3_SOURCE_DIR}
        )
        execute_process(
            COMMAND make install
            WORKING_DIRECTORY ${fftw3_SOURCE_DIR}
        )
    endif()
    add_custom_target(fftw3_ready)
endif()

set(FFTW3_DIR ${FFTW3_INSTALL_DIR} CACHE PATH "Path to FFTW3")