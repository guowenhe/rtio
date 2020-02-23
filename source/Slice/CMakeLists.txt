
cmake_minimum_required (VERSION 2.8)

if(BUILD_ALL_PROJECT) 
project(rtio-slice)
endif()

if(SLICE_DIR)
else()
    set(SLICE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()


set(GEN_C_DIR ${PROJECT_BINARY_DIR}/slice/cpp)
set(GEN_JS_DIR ${PROJECT_BINARY_DIR}/slice/js)


#set(GEN_C_DIR ${SLICE_DIR}/../Common/slice/cpp)
#set(GEN_JS_DIR ${SLICE_DIR}/../Common/slice/js)

# move slice to self project
#set(GEN_C_DIR ${CMAKE_CURRENT_SOURCE_DIR}/slice-gen/cpp)
#set(GEN_JS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/slice-gen/js)

#message("slice_dir:" ${SLICE_DIR})

set(SLICE_FILE_LIST 
DeviceHub
StatusServer
MessageTrigger
)

set(GEN_SRC_LIST "")
file(MAKE_DIRECTORY ${GEN_C_DIR})
file(MAKE_DIRECTORY ${GEN_JS_DIR})

foreach(SLICE_FILE IN LISTS SLICE_FILE_LIST)
    add_custom_command(
    OUTPUT
    ${GEN_C_DIR}/${SLICE_FILE}.cpp
    ${GEN_C_DIR}/${SLICE_FILE}.h
    COMMAND 
    /usr/bin/slice2cpp -I${SLICE_DIR} ${SLICE_DIR}/${SLICE_FILE}.ice --output-dir ${GEN_C_DIR}
    COMMAND 
    /usr/bin/slice2js -I${SLICE_DIR} ${SLICE_DIR}/${SLICE_FILE}.ice --output-dir ${GEN_JS_DIR}
    DEPENDS 
    ${SLICE_DIR}/${SLICE_FILE}.ice
#    WORKING_DIRECTORY ${SLICE_DIR}
    )
    set(GEN_SRC_LIST 
    ${GEN_SRC_LIST}
    ${GEN_C_DIR}/${SLICE_FILE}.cpp
    ${GEN_C_DIR}/${SLICE_FILE}.h
    )
endforeach()

# gen c++ lib
include_directories(
${GEN_C_DIR}
)
add_library(slice-gen STATIC
${GEN_SRC_LIST}
 )

 






