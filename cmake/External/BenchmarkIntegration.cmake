find_package ( benchmark REQUIRED )

function ( add_benchmark_executable )

    cmake_parse_arguments(
        pargs 
        ""
        "TARGET"
        "SOURCES;HEADERS;DEPENDENCIES"
        ${ARGN}
    )
    
    if ( pargs_UNPARSED_ARGUMENTS )
        message ( FATAL_ERROR " Unknown arguments: ${pargs_UNPARSED_ARGUMENTS}" )
    endif( )
    
    if ( NOT pargs_TARGET )
        message ( FATAL_ERROR " Benchmark target is not defined." )
    endif ( )
    
    if ( NOT "${pargs_TARGET}" MATCHES "benchmark_.*" )
        message ( FATAL_ERROR " Benchmark executable name must start with benchmark_: ${pargs_TARGET}" )
    endif ( )
    
    source_group (
        "Source Files"
        FILES
            ${pargs_SOURCES}
    )
    
    source_group (
        "Header Files"
        FILES
            ${pargs_HEADERS}
    )
    
    add_executable (
        ${pargs_TARGET}
        
        ${pargs_SOURCES}
        
        ${pargs_HEADERS}
    )
    
    target_link_libraries (
        ${pargs_TARGET}
        PRIVATE
            benchmark::benchmark 
            ${pargs_DEPENDENCIES}
    )

    set ( _benchmark_output_dir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/benchmark )
    set ( _benchmark_output_file ${_benchmark_output_dir}/${pargs_TARGET}.json)

    add_custom_command (
        TARGET ${pargs_TARGET}
        PRE_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${_benchmark_output_dir}
    )

    if ( ENABLE_SOLUTION_FOLDERS )
        set_target_properties (
            ${pargs_TARGET}
            PROPERTIES
            FOLDER 
                "benchmarks"
        )
    endif ( )
    
  
	add_test (
		NAME ${pargs_TARGET} 
		COMMAND 
			"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${pargs_TARGET}" --benchmark_out=${_benchmark_output_file} --benchmark_out_format=json
	)

endfunction ( add_benchmark_executable )