if (NOT TARGET ToLoG)
    message(STATUS "Fetching ToLoG")
    FetchContent_Declare(ToLoG
        GIT_REPOSITORY https://github.com/tobiluc/ToLoG.git
        GIT_TAG main
        SOURCE_DIR "${EXTERNAL_DIR}/ToLoG"
    )
    set(TOLOG_BUILD_UNIT_TESTS FALSE)
    FetchContent_MakeAvailable(ToLoG)
endif()
