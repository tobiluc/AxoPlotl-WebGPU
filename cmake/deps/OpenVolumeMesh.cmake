if (NOT TARGET OpenVolumeMesh::OpenVolumeMesh)
    message(STATUS "Fetching OpenVolumeMesh")
    FetchContent_Declare(ovm
        # GIT_REPOSITORY https://www.graphics.rwth-aachen.de:9000/OpenVolumeMesh/OpenVolumeMesh
        # GIT_TAG v3.4
        GIT_REPOSITORY https://gitlab.vci.rwth-aachen.de:9000/OpenVolumeMesh/OpenVolumeMesh.git
        GIT_TAG dev/mh/feature-smart-handles
        SOURCE_DIR "${EXTERNAL_DIR}/OpenVolumeMesh"
        )
    FetchContent_MakeAvailable(ovm)
endif()
