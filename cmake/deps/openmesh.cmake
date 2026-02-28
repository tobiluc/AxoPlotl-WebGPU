if (NOT TARGET OpenMeshCore)
   message(STATUS "Fetching OpenMesh")
   FetchContent_Declare(openmesh
       GIT_REPOSITORY https://www.graphics.rwth-aachen.de:9000/OpenMesh/OpenMesh
       GIT_TAG "OpenMesh-11.0"
       SOURCE_DIR "${EXTERNAL_DIR}/OpenMesh"
       )
   FetchContent_MakeAvailable(openmesh)
   #find_package(OpenMesh REQUIRED)
endif()
