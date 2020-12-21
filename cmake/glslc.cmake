FIND_PROGRAM(GLSLC glslc)
FIND_PACKAGE(Python3 COMPONENTS Interpreter REQUIRED)

SET(SPIRV_EMBED_PATH "${CMAKE_CURRENT_LIST_DIR}/spirv_num_to_cpp.py")

function(ADD_SPIRV_LIBRARY)
    SET(SINGLE_ARGS TARGET NAMESPACE OUTPUT_PATH HEADER_NAME)
    SET(MULTI_ARGS HEADERS SOURCES)
    CMAKE_PARSE_ARGUMENTS(RSRC_ARG "" "${SINGLE_ARGS}" "${MULTI_ARGS}" ${ARGN}) 

    SET(FULL_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${RSRC_ARG_OUTPUT_PATH}")

    FOREACH(FILE_NAME ${RSRC_ARG_SOURCES})
    
        GET_FILENAME_COMPONENT(FILE_NAME_ONLY ${FILE_NAME} NAME)
        SET(RESULT_FILE_PATH "${FULL_OUTPUT_PATH}/${FILE_NAME_ONLY}")
        SET(SPIRV_OUTPUT "${RESULT_FILE_PATH}.spv")
        SET(EMBED_OUTPUT "${SPIRV_OUTPUT}.cpp")
        
        ADD_CUSTOM_COMMAND(
                OUTPUT "${SPIRV_OUTPUT}"
                COMMAND ${GLSLC} -o "${SPIRV_OUTPUT}" --target-env=vulkan1.1 -mfmt=num -g -Werror -O -fshader-stage=compute "${FILE_NAME}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                MAIN_DEPENDENCY "${FILE_NAME}"
                DEPENDS "${RSRC_ARG_HEADERS}"
                VERBATIM)
                
        ADD_CUSTOM_COMMAND(
                OUTPUT "${EMBED_OUTPUT}"
                COMMAND ${Python3_EXECUTABLE} "${SPIRV_EMBED_PATH}" source -s "${SPIRV_OUTPUT}" -d "${FULL_OUTPUT_PATH}" -ns ${RSRC_ARG_NAMESPACE}
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                MAIN_DEPENDENCY "${SPIRV_OUTPUT}"
                VERBATIM)
                
        LIST(APPEND OUTPUT_FILES "${EMBED_OUTPUT}")
        
    ENDFOREACH()
    
    SET(GENERATED_HEADER "${FULL_OUTPUT_PATH}/${RSRC_ARG_HEADER_NAME}")
    ADD_CUSTOM_COMMAND(
            OUTPUT "${GENERATED_HEADER}"
            COMMAND ${Python3_EXECUTABLE} "${SPIRV_EMBED_PATH}" header -s "${RSRC_ARG_SOURCES}" -d "${GENERATED_HEADER}" -ns ${RSRC_ARG_NAMESPACE}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            DEPENDS "${RSRC_ARG_SOURCES}"
            VERBATIM)

    ADD_LIBRARY(${RSRC_ARG_TARGET} ${OUTPUT_FILES} "${GENERATED_HEADER}")
    TARGET_INCLUDE_DIRECTORIES(${RSRC_ARG_TARGET} INTERFACE "${FULL_OUTPUT_PATH}")
endfunction()