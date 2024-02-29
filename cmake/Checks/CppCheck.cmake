find_program(CPPCHECK_EXE cppcheck 
    NAMES cppcheck 
    HINTS $ENV{PROGRAMFILES}/cppcheck
    DOC "Path to cppcheck executable"
)

if(CPPCHECK_EXE)
    set(CMAKE_C_CPPCHECK "${CPPCHECK_EXE};--std=c++17")
    set(CMAKE_CXX_CPPCHECK "${CPPCHECK_EXE};--std=c++17")
    
    set(CPP_CHECK_CMD ${CMAKE_CXX_CPPCHECK} --version)
    
    execute_process(COMMAND ${CPP_CHECK_CMD}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE CPP_CHECK_RESULT
        OUTPUT_VARIABLE CPP_CHECK_VERSION
        ERROR_VARIABLE CPP_CHECK_ERROR
    )
    
    if(CPP_CHECK_RESULT EQUAL 0)
        message(STATUS "valid cppcheck result.")
        
        # Get number of CPU cores
        include(ProcessorCount)
        ProcessorCount(CPU_CORES)

        # Append desired arguments to CppCheck
        list(
            APPEND CMAKE_CXX_CPPCHECK

            # Using the below template will allow jumping to any found error from inside Visual Studio output window by double click
            "--template \"${CMAKE_SOURCE_DIR}/{file}({line}): {severity} ({id}): {message}\"" 

            # Use all the available CPU cores
            #"-j ${CPU_CORES}" 

            # Only show found errors
            "--quiet" 

            # Desired warning level in CppCheck
            "--enable=all"

            # Optional: Specified C++ version
            "--std=c++17"

            # Optional: Specified platform
            "--platform=win64"

            # Optional: suppression file stored in same directory as the top level CMake script
            "--suppressions-list=${CMAKE_SOURCE_DIR}/cppcheck_suppressions.txt"

            # Optional: Use inline suppressions
            "--inline-suppr"
            
            # Ignore build directory
            "-ibuild"

            # Run CppCheck from the working directory, as specified in the add_custom_target command below
            "."
            
        )
        
        add_custom_target(ANALYZE_CPPCHECK DEPENDS ${CMAKE_PROJECT_NAME}
            COMMAND ${CMAKE_CXX_CPPCHECK}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Static code analysis using ${CPP_CHECK_VERSION}"
        )
    
    endif()
    
    
else()
  message(WARNING "No cppcheck executable found. Skipping checks...")
endif()
