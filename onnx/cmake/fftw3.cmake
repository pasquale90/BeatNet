include(ExternalProject)

set(FFTW3_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/fftw")
set(FFTW3_VERSION "3.3.10")
set(FFTW3_WIN_VERSION "3.3.5")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
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

    ExternalProject_Add(fftw3_binary
        PREFIX ${CMAKE_BINARY_DIR}/_deps/fftw3
        URL ${FFTW3_URL}
        DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/_downloads
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR> ${FFTW3_INSTALL_DIR}
        LOG_DOWNLOAD ON
    )

    # generate the .lib file using lib.exe, if it doesnt exist
    add_custom_command(
        OUTPUT ${FFTW3_LIB_FILE}
        COMMAND lib.exe /def:${FFTW3_DEF_FILE} /machine:${ARCH_TAG} /out:${FFTW3_LIB_FILE}
        DEPENDS fftw3_binary
        COMMENT "Generating ${FFTW3_LIB_FILE} from ${FFTW3_DEF_FILE}"
        VERBATIM
    )

    add_custom_target(fftw3_importlib ALL
        DEPENDS ${FFTW3_LIB_FILE}
    )
    
else() # linux/macos
    set(FFTW3_URL "https://www.fftw.org/fftw-${FFTW3_VERSION}.tar.gz")

    ExternalProject_Add(fftw3_binary
        PREFIX ${CMAKE_BINARY_DIR}/_deps/fftw3
        URL ${FFTW3_URL}
        DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/_downloads
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-float --disable-shared --prefix=${FFTW3_INSTALL_DIR}
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        LOG_INSTALL ON
    )
endif()

set(FFTW3_DIR ${FFTW3_INSTALL_DIR})