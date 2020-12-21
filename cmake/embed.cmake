SET(EMBED_FULL_PATH ${CMAKE_CURRENT_LIST_DIR})  

FIND_PACKAGE(Python3 COMPONENTS Interpreter REQUIRED)

function(ADD_RESOURCE_LIBRARY)
    SET(SINGLE_ARGS TARGET NAMESPACE OUTPUT_PATH)
    SET(MULTI_ARGS FILES)
    CMAKE_PARSE_ARGUMENTS(RSRC_ARG "" "${SINGLE_ARGS}" "${MULTI_ARGS}" ${ARGN}) 

    SET(FULL_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${RSRC_ARG_OUTPUT_PATH}")

    FOREACH(FILE_NAME ${RSRC_ARG_FILES})
        GET_FILENAME_COMPONENT(FILE_NAME_ONLY ${FILE_NAME} NAME_WE)
        SET(RESULT_FILE_PATH "${FULL_OUTPUT_PATH}/${FILE_NAME_ONLY}")
        ADD_CUSTOM_COMMAND(
                OUTPUT "${RESULT_FILE_PATH}.cpp" "${RESULT_FILE_PATH}.h"
                COMMAND ${Python3_EXECUTABLE} ${EMBED_FULL_PATH}/file_to_cpp_string.py -s ${FILE_NAME} -d ${FULL_OUTPUT_PATH} -ns ${RSRC_ARG_NAMESPACE}
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                MAIN_DEPENDENCY "${FILE_NAME}")
        LIST(APPEND OUTPUT_FILES "${RESULT_FILE_PATH}.cpp")
    ENDFOREACH()
    
    ADD_LIBRARY(${RSRC_ARG_TARGET} ${OUTPUT_FILES})
endfunction()
