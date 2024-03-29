find_package(GTest REQUIRED)

include(GoogleTest)

function(add_gtest_executable)

    cmake_parse_arguments(
        pargs 
        ""
        "TARGET;"
        "SOURCES;HEADERS;DEPENDENCIES"
        ${ARGN}
    )
    
    if(pargs_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown arguments: ${pargs_UNPARSED_ARGUMENTS}")
    endif()
    
    if( "TARGET" IN_LIST pargs_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "TARGET: not defined!")
    endif()
    
    if(NOT "${pargs_TARGET}" MATCHES "test_.*")
        message(FATAL_ERROR "Test executable name must start with test_: ${pargs_TARGET}")
    endif()
    
    source_group("Source Files"
        FILES
            ${pargs_SOURCES}
    )
    
    source_group("Header Files"
        FILES
            ${pargs_HEADERS}
    )
    
    add_executable(
        ${pargs_TARGET}
        
        ${pargs_SOURCES}
        
        ${pargs_HEADERS}
    )
    
    target_link_libraries(${pargs_TARGET}
        PRIVATE
            GTest::gtest
			GTest::gtest_main
			GTest::gmock
			GTest::gmock_main
			gtest::gtest
            ${pargs_DEPENDENCIES}
    )
	
	set_compiler_warning_flags( 
		STRICT
		TARGET ${pargs_TARGET}
	)

    if(ENABLE_SOLUTION_FOLDERS)
        set_target_properties(${pargs_TARGET} PROPERTIES FOLDER "tests")
    endif()
    
    if ( COVERAGE_ON AND COVERAGE_EXE )
        coverage_add_test(
            TARGET
                ${pargs_TARGET}
            REPORT_FOR_PROJECT
		)
    else ( )
        gtest_discover_tests(
            ${pargs_TARGET}
            #WORKING_DIRECTORY ${pargs_WORKING_DIRECTORY}
		)
	endif()

    if ( STATIC_CODE_ANALYSIS )
        set_target_properties(${pargs_TARGET} PROPERTIES
            VS_GLOBAL_RunCodeAnalysis true

            # Use visual studio core guidelines
            VS_GLOBAL_EnableMicrosoftCodeAnalysis true

            # Use clangtidy
            VS_GLOBAL_EnableClangTidyCodeAnalysis true
            VS_GLOBAL_ClangTidyChecks "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
        )
    endif ( STATIC_CODE_ANALYSIS )

endfunction(add_gtest_executable)