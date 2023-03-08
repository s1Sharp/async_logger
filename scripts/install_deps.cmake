if(WIN32)
execute_process(COMMAND conan install "${PROJECT_SOURCE_DIR}" --output-folder=build --build=missing
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
message("deps install win")
endif()

if(UNIX)
execute_process(COMMAND conan install "${PROJECT_SOURCE_DIR}" --output-folder=build --build=missing
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
message("deps install unix")
endif()