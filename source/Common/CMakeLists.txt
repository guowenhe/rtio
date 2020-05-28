
cmake_minimum_required (VERSION 2.8)

if(BUILD_ALL_PROJECT) 
project(rtio-common)
message("---------------------------------------------------------------------")
message("Project Name              : " ${PROJECT_NAME})
message("Source Dir                : " ${PROJECT_SOURCE_DIR})
message("Binary Dir                : " ${PROJECT_BINARY_DIR})
message("---------------------------------------------------------------------")
endif()

if(COMMON_SRC_DIR)
else()
    set(COMMON_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# message("COMMON_SRC_DIR:" ${COMMON_SRC_DIR})

include_directories(${COMMON_SRC_DIR}/PublicHeaders)

aux_source_directory(${COMMON_SRC_DIR}/Util COMMON_SRC)
add_library(common-lib STATIC ${COMMON_SRC})






 






