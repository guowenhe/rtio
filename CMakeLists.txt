
cmake_minimum_required (VERSION 2.8)
project(rtio)

message("---------------------------------------------------------------------")
message("Project Name              : " ${PROJECT_NAME})
message("System Processor          : " ${CMAKE_SYSTEM_PROCESSOR})
message("System Platform           : " ${CMAKE_SYSTEM})
message("BOOST_HOME                : " $ENV{BOOST_HOME})
message("---------------------------------------------------------------------")

set(BUILD_ALL_PROJECT "define")  
include(./settings/settings.cmake)

add_subdirectory(source/Slice) 
add_subdirectory(source/Servers/AccessServer) 
add_subdirectory(source/Servers/APIPusher) 
add_subdirectory(source/Servers/DeviceHub)
add_subdirectory(source/Servers/StatusServer)  
add_subdirectory(source/Servers/MessageTrigger) 





