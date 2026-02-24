if (NOT TARGET ibex)
    message(STATUS "Fetching ibex")
    FetchContent_Declare(ibex
        GIT_REPOSITORY https://github.com/tobiluc/ibex.git
        GIT_TAG main
        SOURCE_DIR "${EXTERNAL_DIR}/ibex"
    )
    FetchContent_MakeAvailable(ibex)
    # add_library(ibex STATIC
    #     ${EXTERNAL_DIR}/ibex/ibex.cpp
    # )
    #target_include_directories(ibex PUBLIC ${EXTERNAL_DIR}/ibex)
endif()
