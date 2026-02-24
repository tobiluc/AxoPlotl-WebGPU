if (NOT TARGET iconfontcppheaders)
    FetchContent_Declare(iconfontcppheaders
        GIT_REPOSITORY https://github.com/juliettef/IconFontCppHeaders.git
        GIT_TAG main
        SOURCE_DIR "${EXTERNAL_DIR}/iconfontcppheaders"
    )
    FetchContent_MakeAvailable(iconfontcppheaders)
endif()
