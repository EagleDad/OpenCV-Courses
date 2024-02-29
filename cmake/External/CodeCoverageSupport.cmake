function(coverage_add_test)

    cmake_parse_arguments(
        pargs 
        "REPORT_FOR_GLOBAL;REPORT_FOR_PROJECT;REPORT_FOR_TARGET"
        "TARGET"
        "SOURCES"
        ${ARGN}
    )

    if(pargs_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown arguments: ${pargs_UNPARSED_ARGUMENTS}")
    endif()
    
    get_target_property(TARGET_SOURCE_DIR ${pargs_TARGET} SOURCE_DIR)
    file(TO_NATIVE_PATH ${TARGET_SOURCE_DIR} TARGET_SOURCE_DIR)

    get_filename_component(PARENT_DIR ${TARGET_SOURCE_DIR} DIRECTORY)
    file(TO_NATIVE_PATH ${PARENT_DIR} PARENT_DIR)

    # Code coverage will only work in debug since the .*pdb files are required
    set(COVERAGE_CONFIG_DIR "${CMAKE_BINARY_DIR}/bin/Debug")

    # list(FIND CMAKE_CONFIGURATION_TYPES Debug LIST_INDEX)
    # if (${LIST_INDEX} GREATER -1)
        # list(GET CMAKE_CONFIGURATION_TYPES ${LIST_INDEX} FOLDER)
        # set(COVERAGE_CONFIG_DIR "${COVERAGE_CONFIG_DIR}$<$<CONFIG:Debug>:${FOLDER}>")
    # endif ()
    
    # list(FIND CMAKE_CONFIGURATION_TYPES Release LIST_INDEX)
    # if (${LIST_INDEX} GREATER -1)
        # list(GET CMAKE_CONFIGURATION_TYPES ${LIST_INDEX} FOLDER)
        # set(COVERAGE_CONFIG_DIR "${COVERAGE_CONFIG_DIR}$<$<CONFIG:Release>:${FOLDER}>")
    # endif ()
    
    # list(FIND CMAKE_CONFIGURATION_TYPES MinSizeRel LIST_INDEX)
    # if (${LIST_INDEX} GREATER -1)
        # list(GET CMAKE_CONFIGURATION_TYPES ${LIST_INDEX} FOLDER)
        # set(COVERAGE_CONFIG_DIR "${COVERAGE_CONFIG_DIR}$<$<CONFIG:MinSizeRel>:${FOLDER}>")
    # endif ()
    
    # list(FIND CMAKE_CONFIGURATION_TYPES RelWithDebInfo LIST_INDEX)
    # if (${LIST_INDEX} GREATER -1)
        # list(GET CMAKE_CONFIGURATION_TYPES ${LIST_INDEX} FOLDER)
        # set(COVERAGE_CONFIG_DIR "${COVERAGE_CONFIG_DIR}$<$<CONFIG:RelWithDebInfo>:${FOLDER}>")
    # endif ()
    
    set(COVERAGE_HTML_OUTPUT_DIR "${COVERAGE_CONFIG_DIR}/coverage/${pargs_TARGET}")
    file(TO_NATIVE_PATH ${COVERAGE_HTML_OUTPUT_DIR} COVERAGE_HTML_OUTPUT_DIR)
    
    # Configuration (argument) file for individual test target
    set(COVERAGE_CONFIG_INPUT_FILE "${COVERAGE_CONFIG_DIR}/coverage/${pargs_TARGET}/${pargs_TARGET}.cov.txt")
    file(TO_NATIVE_PATH ${COVERAGE_CONFIG_INPUT_FILE} COVERAGE_CONFIG_INPUT_FILE)
    
    set(COVERAGE_COBERTURA_OUTPUT_FILE "${COVERAGE_CONFIG_DIR}/coverage/${pargs_TARGET}/${pargs_TARGET}.obertura.xml")
    file(TO_NATIVE_PATH ${COVERAGE_COBERTURA_OUTPUT_FILE} COVERAGE_COBERTURA_OUTPUT_FILE)

    # Binary coverage (.cov) output file for individual test target
    set(COVERAGE_BINARY_OUTPUT_FILE "${COVERAGE_CONFIG_DIR}/coverage/${pargs_TARGET}/${pargs_TARGET}.cov")
    file(TO_NATIVE_PATH ${COVERAGE_BINARY_OUTPUT_FILE} COVERAGE_NATIVE_BINARY_OUTPUT_FILE)
    
    set(COVERAGE_TEST_EXECUTABLE "${COVERAGE_CONFIG_DIR}/${pargs_TARGET}.exe")
    file(TO_NATIVE_PATH ${COVERAGE_TEST_EXECUTABLE} COVERAGE_TEST_EXECUTABLE)
    
    # Create native path arguments for OpenCppCoverage
    string(APPEND OPENCPPCOVERAGE_COVERAGE_ARGS_MULTILINE)

    set(SOURCE_CPP_DIR "${PARENT_DIR}/src")
    file(TO_NATIVE_PATH ${SOURCE_CPP_DIR} SOURCE_CPP_DIR)
    
    set(SOURCE_HPP_DIR "${PARENT_DIR}/include")
    file(TO_NATIVE_PATH ${SOURCE_HPP_DIR} SOURCE_HPP_DIR)
    
    set(EXCLUDE_MODULE_TEST_DIR "${PARENT_DIR}/test")
    file(TO_NATIVE_PATH ${EXCLUDE_MODULE_TEST_DIR} EXCLUDE_MODULE_TEST_DIR)
    
    
    # FILE TO NATIVE
    string(APPEND OPENCPPCOVERAGE_COVERAGE_ARGS_MULTILINE "excluded_modules=${EXCLUDE_MODULE_TEST_DIR}\n")
    string(APPEND OPENCPPCOVERAGE_COVERAGE_ARGS_MULTILINE "sources=${SOURCE_CPP_DIR}\n")
    string(APPEND OPENCPPCOVERAGE_COVERAGE_ARGS_MULTILINE "sources=${SOURCE_HPP_DIR}\n")
    
    
    #string(APPEND REPORT_ARGS_MULTILINE "export_type=cobertura:${NATIVE_REPORT_COBERTURA_FILE}\n")
    
    # Generate configuration file for test target used to output .cov
    file(WRITE ${COVERAGE_CONFIG_INPUT_FILE} 
        "# Auto-generated config file for OpenCppCoverage to produce coverage output\n"
        "export_type=binary:${COVERAGE_NATIVE_BINARY_OUTPUT_FILE}\n"
        "export_type=html:${COVERAGE_HTML_OUTPUT_DIR}\n"
        "export_type=cobertura:${COVERAGE_COBERTURA_OUTPUT_FILE}\n"
        "${OPENCPPCOVERAGE_COVERAGE_ARGS_MULTILINE}"
    )
    
    # These commands cannot be used at the same time
    #list(APPEND OPENCPPCOVERAGE_CLI_ARGS "--verbose")
    list(APPEND OPENCPPCOVERAGE_CLI_ARGS "--quiet")

    add_test(
        NAME 
            ${pargs_TARGET} 
        COMMAND
            ${COVERAGE_EXE}
            ${OPENCPPCOVERAGE_CLI_ARGS}
            --config_file=${COVERAGE_CONFIG_INPUT_FILE}
            -- $<TARGET_FILE:${pargs_TARGET}>
    )
    
     # Add target that generates coverage data (drives custom command)
    add_custom_target(${pargs_TARGET}_coverage
        DEPENDS ${COVERAGE_BINARY_OUTPUT_FILE})
        
    if(ENABLE_SOLUTION_FOLDERS)
        set_target_properties(${pargs_TARGET}_coverage PROPERTIES FOLDER "coverage")
    endif()
    
    if ( pargs_REPORT_FOR_PROJECT )
    
        set(PROJECT_COVERAGE_REPORT_TARGET ${PROJECT_NAME}_coverage_report)
        if (NOT TARGET ${PROJECT_COVERAGE_REPORT_TARGET})
        
            coverage_add_coverage_report_target(
                "${CMAKE_PROJECT_NAME}"                 # TARGET_NAME
                "${COVERAGE_CONFIG_DIR}/coverage"       # TARGET_BINARY_DIR
                "${COVERAGE_BINARY_OUTPUT_FILE}"        # CPP_COVERAGE_BINARY_OUTPUT_FILE
                "${PROJECT_COVERAGE_REPORT_TARGET}"     # REPORT_TARGET
                "${OPENCPPCOVERAGE_CLI_ARGS}"           # OPENCPPCOVERAGE_CLI_ARGS
            )
            
        endif()
        
        set_property(TARGET ${PROJECT_COVERAGE_REPORT_TARGET} 
            APPEND_STRING PROPERTY 
            CPP_COVERAGE_INPUT_FILES "input_coverage=${COVERAGE_NATIVE_BINARY_OUTPUT_FILE}\n"
        )
        
        add_dependencies(${PROJECT_COVERAGE_REPORT_TARGET} ${pargs_TARGET}_coverage)
        
    endif()
    
    if ( pargs_REPORT_FOR_TARGET )
    
        set(TEST_COVERAGE_REPORT_TARGET ${pargs_TARGET}_coverage_report)
        if (NOT TARGET ${TEST_COVERAGE_REPORT_TARGET})
        
            coverage_add_coverage_report_target(
                "${pargs_TARGET}"                                   # TARGET_NAME
                "${COVERAGE_CONFIG_DIR}/coverage/${pargs_TARGET}"   # TARGET_BINARY_DIR
                "${COVERAGE_BINARY_OUTPUT_FILE}"                    # CPP_COVERAGE_BINARY_OUTPUT_FILE
                "${TEST_COVERAGE_REPORT_TARGET}"                    # REPORT_TARGET
                "${OPENCPPCOVERAGE_CLI_ARGS}"                       # OPENCPPCOVERAGE_CLI_ARGS
            )
            
        endif()
        
        set_property(TARGET ${TEST_COVERAGE_REPORT_TARGET} 
            APPEND_STRING PROPERTY 
            CPP_COVERAGE_INPUT_FILES "input_coverage=${COVERAGE_NATIVE_BINARY_OUTPUT_FILE}\n"
        )
        
        add_dependencies(${TEST_COVERAGE_REPORT_TARGET} ${pargs_TARGET}_coverage)
        
    endif()

endfunction()

function(coverage_add_coverage_report_target 
    TARGET_NAME 
    TARGET_BINARY_DIR 
    CPP_COVERAGE_BINARY_OUTPUT_FILE 
    REPORT_TARGET 
    OPENCPPCOVERAGE_CLI_ARGS)

    set(REPORT_CONFIG_FILE "${TARGET_BINARY_DIR}/${TARGET_NAME}.cov.report.txt")
    set(REPORT_COBERTURA_FILE "${TARGET_BINARY_DIR}/${TARGET_NAME}.cobertura.xml")
    set(HTML_REPORT_DIR "${TARGET_BINARY_DIR}/${TARGET_NAME}")
    set(HTML_REPORT "${HTML_REPORT_DIR}/index.html")

    file(TO_NATIVE_PATH ${REPORT_COBERTURA_FILE} NATIVE_REPORT_COBERTURA_FILE)
    file(TO_NATIVE_PATH ${HTML_REPORT_DIR} NATIVE_HTML_REPORT_DIR)

    set(REPORT_ARTIFACTS "")
    list(APPEND REPORT_ARTIFACTS ${HTML_REPORT})
    
    string(APPEND REPORT_ARGS_MULTILINE "# Auto-generated config file for OpenCppCoverage to produce coverage report\n")
    string(APPEND REPORT_ARGS_MULTILINE "export_type=html:${NATIVE_HTML_REPORT_DIR}\n")
    string(APPEND REPORT_ARGS_MULTILINE "export_type=cobertura:${NATIVE_REPORT_COBERTURA_FILE}\n")
    
    # Note that file(GENERATE) will not generate until all CMakeLists.txt have been processed
    # We utilize this property of that command to make it possible to utilize aggregated
    # property set of input coverage files to be written into report config file.
    file(GENERATE 
       OUTPUT ${REPORT_CONFIG_FILE}
        CONTENT "${REPORT_ARGS_MULTILINE}$<TARGET_PROPERTY:${REPORT_TARGET},CPP_COVERAGE_INPUT_FILES>"
    )
    
    add_custom_target(${REPORT_TARGET}
        COMMAND ${COVERAGE_EXE} ${OPENCPPCOVERAGE_CLI_ARGS} --config_file ${REPORT_CONFIG_FILE}
        #COMMAND ${CPP_COVERAGE_REPORT_GENERATOR_TOOL} 
        #-reports:${NATIVE_REPORT_COBERTURA_FILE} 
        #-reporttypes:${CPP_COVERAGE_REPORT_TYPE} 
        #-targetdir:${TARGET_BINARY_DIR}/report 
        #-historydir:${TARGET_BINARY_DIR}/history
        VERBATIM
    )
    
    if(ENABLE_SOLUTION_FOLDERS)
        set_target_properties(${REPORT_TARGET} PROPERTIES FOLDER "coverage")
    endif()
endfunction()