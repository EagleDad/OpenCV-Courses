
function( conan_update_config )

	cmake_parse_arguments(
        pargs 
        ""
        "PROFILE_DEBUG;PROFILE_RELEASE;"
        ""
        ${ARGN}
    )

# conan config home returns the root dir
#conan install . --output-folder=build --build=missing --options=zlib/1.2.11:shared=True

	if ( DEFINED pargs_UNPARSED_ARGUMENTS )
		message("pargs_UNPARSED_ARGUMENTS: ${pargs_UNPARSED_ARGUMENTS}")
	endif()
	
	if ( NOT DEFINED pargs_PROFILE_DEBUG )
		message(FATAL_ERROR "PROFILE_DEBUG: not defined!")
	endif()
	
	if ( NOT DEFINED pargs_PROFILE_RELEASE )
		message(FATAL_ERROR "PROFILE_RELEASE: not defined!")
	endif()
	
	MESSAGE(STATUS "Query conan profile path ...")
	# In the first build step we generate a default profile that we can
	# query in the second build step
	set( CONAN_QUERY_PROFILE_PATH conan profile path default )

	execute_process (
		COMMAND ${CONAN_QUERY_PROFILE_PATH}
		OUTPUT_VARIABLE CONAN_PROFILE_PATH
	)

	# The variable is pointing to the default file, so we need to cut the last section
	string(REPLACE "\\default" "" CONAN_PROFILE_PATH ${CONAN_PROFILE_PATH})
	string(STRIP ${CONAN_PROFILE_PATH} CONAN_PROFILE_PATH)
	file(TO_CMAKE_PATH ${CONAN_PROFILE_PATH} CONAN_PROFILE_PATH)
	MESSAGE(STATUS "Conan profile path: ${CONAN_PROFILE_PATH}")

	MESSAGE(STATUS "Copy project profile to conan profiles ...")
	
	#PROFILE_DEBUG "${CMAKE_SOURCE_DIR}/.conan/debug_ipt"
	MESSAGE(STATUS "Copy debug profile ...")
	set(CONAN_PROFILE_DEBUG "${CMAKE_SOURCE_DIR}/.conan/${pargs_PROFILE_DEBUG}")
	MESSAGE(STATUS "Copy ${CONAN_PROFILE_DEBUG} to ${CONAN_PROFILE_PATH}")
	FILE(COPY ${CONAN_PROFILE_DEBUG} DESTINATION ${CONAN_PROFILE_PATH} )
	
	#execute_process(
	#	COMMAND ${CMAKE_COMMAND} -E copy ${CONAN_PROFILE_DEBUG} ${CONAN_PROFILE_PATH} OUTPUT_VARIABLE COPY_OUTPUT )
	
	MESSAGE(STATUS "Copy release profile ...")
	set(CONAN_PROFILE_RELEASE "${CMAKE_SOURCE_DIR}/.conan/${pargs_PROFILE_RELEASE}")
	MESSAGE(STATUS "Copy ${CONAN_PROFILE_RELEASE} to ${CONAN_PROFILE_PATH}")
	FILE(COPY ${CONAN_PROFILE_RELEASE} DESTINATION ${CONAN_PROFILE_PATH} )
	
	#execute_process(
	#	COMMAND ${CMAKE_COMMAND} -E copy ${CONAN_PROFILE_RELEASE} ${CONAN_PROFILE_PATH} OUTPUT_VARIABLE COPY_OUTPUT )
		
	#MESSAGE(STATUS "Copy output: ${COPY_OUTPUT}")

endfunction( conan_update_config )

function( conan_install_missing_packages )

	cmake_parse_arguments(
        pargs 
        ""
        "PROFILE_DEBUG;PROFILE_RELEASE;"
        ""
        ${ARGN}
    )

	if ( DEFINED pargs_UNPARSED_ARGUMENTS )
		message("pargs_UNPARSED_ARGUMENTS: ${pargs_UNPARSED_ARGUMENTS}")
	endif()
	
	if ( NOT DEFINED pargs_PROFILE_DEBUG )
		message(FATAL_ERROR "PROFILE_DEBUG: not defined!")
	endif()
	
	if ( NOT DEFINED pargs_PROFILE_RELEASE )
		message(FATAL_ERROR "PROFILE_RELEASE: not defined!")
	endif()

	MESSAGE(STATUS "Updating conan dependencies ...")
	
	MESSAGE(STATUS "Updating debug dependencies ...")
	set( CONAN_UPDATE_DEBUG_DEPS conan install .conan --output-folder=build --build=missing --profile=${pargs_PROFILE_DEBUG} )
	MESSAGE(STATUS ${CONAN_UPDATE_DEBUG_DEPS})
	execute_process (
		COMMAND ${CONAN_UPDATE_DEBUG_DEPS}
		OUTPUT_VARIABLE CONAN_UPDATE_DEBUG
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)
	
	MESSAGE(STATUS "Updating release dependencies ...")
	set( CONAN_UPDATE_RELEASE_DEPS conan install .conan --output-folder=build --build=missing --profile=${pargs_PROFILE_RELEASE} )
	MESSAGE(STATUS ${CONAN_UPDATE_RELEASE_DEPS})
	execute_process (
		COMMAND ${CONAN_UPDATE_RELEASE_DEPS}
		OUTPUT_VARIABLE CONAN_UPDATE_DEBUG
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)

endfunction( conan_install_missing_packages )
