if (NOT TARGET OpenMeshCore)
   message(STATUS "Fetching OpenMesh")
   FetchContent_Declare(openmesh
       GIT_REPOSITORY https://www.graphics.rwth-aachen.de:9000/OpenMesh/OpenMesh
       GIT_TAG "OpenMesh-11.0"
       SOURCE_DIR "${EXTERNAL_DIR}/OpenMesh"
       )
   FetchContent_MakeAvailable(openmesh)
   target_compile_features(OpenMeshCore PRIVATE cxx_std_20)
endif()
