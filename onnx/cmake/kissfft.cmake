include(FetchContent)

set(KISSFFT_VERSION 131.1.0)
set(KISSFFT_DIR ${LIBRARY_DIR}/kissfft)

FetchContent_Declare(
    kissfft
    URL https://github.com/mborgerding/kissfft/archive/refs/tags/${KISSFFT_VERSION}.zip
    SOURCE_DIR ${KISSFFT_DIR}
)

FetchContent_GetProperties(kissfft)
if(NOT kissfft_POPULATED)
    FetchContent_Populate(kissfft)
endif()

add_library(libkissfft STATIC
    ${KISSFFT_DIR}/kiss_fft.c
    ${KISSFFT_DIR}/kiss_fftr.c
)


