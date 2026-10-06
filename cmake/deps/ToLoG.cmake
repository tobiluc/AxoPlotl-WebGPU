
if (NOT TARGET ToLoG)
    message(STATUS "Fetching ToLoG")
    set(TOLOG_BUILD_UNIT_TESTS FALSE CACHE INTERNAL "")
    FetchContent_Declare(ToLoG
        GIT_REPOSITORY https://github.com/tobiluc/ToLoG.git
        GIT_TAG        main
        SOURCE_DIR "${EXTERNAL_DIR}/ToLoG"
    )
    FetchContent_MakeAvailable(ToLoG)
endif()
