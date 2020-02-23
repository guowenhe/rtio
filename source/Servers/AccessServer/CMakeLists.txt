
cmake_minimum_required (VERSION 2.8)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "in-tree build not allowded")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

project(AccessServer)

message("---------------------------------------------------------------------")
message("Project Name              : " ${PROJECT_NAME})
message("Source Dir                : " ${PROJECT_SOURCE_DIR})
message("Binary Dir                : " ${PROJECT_BINARY_DIR})
message("---------------------------------------------------------------------")

if(NOT BUILD_ALL_PROJECT) # start build single project
    include(../../../settings/settings.cmake)
    set(SLICE_DIR ${PROJECT_SOURCE_DIR}/../../Slice/)
    include(../../Slice/CMakeLists.txt)
endif()

include_directories(./)
include_directories(../../Common/cpp)
include_directories(${PROJECT_BINARY_DIR}/../../Slice/slice/cpp/)


#add_executable(server 
#  Server.cpp
#  IceClient.cpp
#  SessionManager.cpp
#  Listener.cpp
#  HttpSession.cpp
#  WebsocketSession.cpp
#)


aux_source_directory(. SERVER_SRC)
add_executable(${PROJECT_NAME}-server ${SERVER_SRC})
target_link_libraries(${PROJECT_NAME}-server
    slice-gen
    Ice++11
    pthread
)

# client_sync
add_executable(${PROJECT_NAME}-client_sync 
  Clients/ClientSync.cpp
)
target_link_libraries(${PROJECT_NAME}-client_sync
    pthread
)
# client_async
add_executable(${PROJECT_NAME}-client_async 
  Clients/ClientAsync.cpp
)
target_link_libraries(${PROJECT_NAME}-client_async
    pthread
)

install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-server 
        PERMISSIONS OWNER_EXECUTE OWNER_READ GROUP_EXECUTE GROUP_READ
        DESTINATION independent/${PROJECT_NAME} RENAME server)
        
install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-client_async  
        PERMISSIONS OWNER_EXECUTE OWNER_READ GROUP_EXECUTE GROUP_READ
        DESTINATION independent/${PROJECT_NAME} RENAME client_async)
 
