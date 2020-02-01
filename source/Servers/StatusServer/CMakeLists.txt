
cmake_minimum_required (VERSION 2.8)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "in-tree build not allowded")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

project(StatusServer)

message("---------------------------------------------------------------------")
message("Project Name              : " ${PROJECT_NAME})
message("Source Dir                : " ${PROJECT_SOURCE_DIR})
message("Binary Dir                : " ${PROJECT_BINARY_DIR})
message("---------------------------------------------------------------------")

include_directories(./)
include_directories($ENV{HI_REDIS}/include/hiredis)
include_directories(../../Common/cpp)
include_directories(${PROJECT_BINARY_DIR}/../../Slice/slice/cpp/)

link_directories($ENV{HI_REDIS}/lib)

aux_source_directory(. SERVER_SRC)
add_executable(${PROJECT_NAME}-server ${SERVER_SRC})
target_link_libraries(${PROJECT_NAME}-server
    hiredis
    slice-gen
    Ice++11
    pthread
)

#add_executable(client_sync
#     Clients/ClientSync.cpp
#)
#target_link_libraries(client_sync
#    slice-gen
#    Ice++11
#    pthread
#)

install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-server 
        PERMISSIONS OWNER_EXECUTE OWNER_READ GROUP_EXECUTE GROUP_READ
        DESTINATION patches/${PROJECT_NAME} RENAME server)


 
