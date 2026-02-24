if (NOT TARGET rapidobj::rapidobj)
    FetchContent_Declare(rapidobj
        GIT_REPOSITORY  https://github.com/guybrush77/rapidobj.git
        GIT_TAG         origin/master
        SOURCE_DIR "${EXTERNAL_DIR}/rapidobj"
    )
    FetchContent_MakeAvailable(rapidobj)
endif()
