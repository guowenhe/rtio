
cmake_minimum_required (VERSION 2.8)
project(rtio)

message("---------------------------------------------------------------------")
message("Project Name              : " ${PROJECT_NAME})
message("System Processor          : " ${CMAKE_SYSTEM_PROCESSOR})
message("System Platform           : " ${CMAKE_SYSTEM})
message("ICE_HOME                  : " $ENV{ICE_HOME})
message("BOOST_HOME                : " $ENV{BOOST_HOME})
message("HI_REDIS                  : " $ENV{HI_REDIS})
message("---------------------------------------------------------------------")

set(BUILD_ALL_PROJECT "define")  
include(./settings/settings.cmake)

add_subdirectory(source/Common) 
add_subdirectory(source/Slice) 
add_subdirectory(source/Servers/APINotifier) 
add_subdirectory(source/Servers/AccessServerTCP) 
add_subdirectory(source/Servers/APISender) 
add_subdirectory(source/Servers/APIManager) 
add_subdirectory(source/Servers/DeviceHub)
add_subdirectory(source/Servers/StatusServer)  
add_subdirectory(source/Servers/MessageReporter) 
add_subdirectory(source/Servers/DeviceManager) 



