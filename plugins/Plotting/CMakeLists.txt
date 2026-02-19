if (NOT TARGET ibex)
    message(STATUS "Fetching ibex")
    FetchContent_Declare(ibex
        GIT_REPOSITORY https://github.com/tobiluc/ibex.git
        GIT_TAG main
        SOURCE_DIR "${EXTERNAL_DIR}/ibex"
    )
    FetchContent_MakeAvailable(ibex)
endif()

add_axoplotl_plugin(PlottingPlugin.cpp LINK_LIBS ibex)
