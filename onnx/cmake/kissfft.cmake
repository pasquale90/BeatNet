include(FetchContent)

set(KISSFFT_VERSION 131.1.0)
set(KISSFFT_DIR ${LIBRARY_DIR}/kissfft)

FetchContent_Declare(
    kissfft
    URL https://github.com/mborgerding/kissfft/archive/refs/tags/${KISSFFT_VERSION}.zip
    SOURCE_DIR ${KISSFFT_DIR}
)

FetchContent_GetProperties(kissfft)
FetchContent_Populate(kissfft) # FetchContent_Populate instead of FetchContent_MakeAvailable cause it requires pkg_config installed