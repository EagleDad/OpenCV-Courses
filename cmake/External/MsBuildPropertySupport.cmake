function(add_msbuild_props_file)

    cmake_parse_arguments(
        pargs 
        ""
        "TARGET"
        "INCLUDE_DIRS;LIBRARY_DIRS_DEBUG;LIBRARY_DIRS_RELEASE;LIBRARIES_DEBUG;LIBRARIES_RELEASE;RUNTIME_LIBRARIES_AND_DIR_RELEASE;RUNTIME_LIBRARIES_AND_DIR_DEBUG"
        ${ARGN}
    )
    
    if(pargs_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown arguments: ${pargs_UNPARSED_ARGUMENTS}")
    endif()
    
    if ( NOT TARGET ${pargs_TARGET} )
        message( FATAL_ERROR "TARGET '${pargs_TARGET}' is not a CMake target" )
    endif()
    
    foreach(_include_dir ${pargs_INCLUDE_DIRS})
        set(_cur_include_dir "$(MSBuildThisFileDirectory)/../${_include_dir}")
        
        file(TO_NATIVE_PATH ${_cur_include_dir} _cur_include_dir)
        
        list(APPEND additional_include_dirs ${_cur_include_dir})
    endforeach()
    
    foreach(_libs_dir_dbg ${pargs_LIBRARY_DIRS_DEBUG})
        set(_cur_lib_dir "$(MSBuildThisFileDirectory)/../${_libs_dir_dbg}")
        
        file(TO_NATIVE_PATH ${_cur_lib_dir} _cur_lib_dir)
        
        list(APPEND additional_libs_dir_dbg ${_cur_lib_dir})
    endforeach()
    
    foreach(_libs_dir_rel ${pargs_LIBRARY_DIRS_RELEASE})
        set(_cur_lib_dir "$(MSBuildThisFileDirectory)/../${_libs_dir_rel}")
        
        file(TO_NATIVE_PATH ${_cur_lib_dir} _cur_lib_dir)
        
        list(APPEND additional_libs_dir_rel ${_cur_lib_dir})
    endforeach()
    
    list(APPEND pargs_LIBRARIES_DEBUG "${LIBRARY_NAME_RAW}${CMAKE_DEBUG_POSTFIX}.lib")
    list(APPEND pargs_LIBRARIES_RELEASE "${LIBRARY_NAME_RAW}.lib")
    
    set(additional_libs_dbg ${pargs_LIBRARIES_DEBUG})
    set(additional_libs_rel ${pargs_LIBRARIES_RELEASE})

    list(APPEND pargs_RUNTIME_LIBRARIES_AND_DIR_DEBUG "bin/${LIBRARY_NAME_RAW}${CMAKE_DEBUG_POSTFIX}.dll")
	if(EXISTS "${CMAKE_BINARY_DIR}/bin/Debug/${LIBRARY_NAME_RAW}${CMAKE_DEBUG_POSTFIX}.pdb")
		list(APPEND pargs_RUNTIME_LIBRARIES_AND_DIR_DEBUG "bin/${LIBRARY_NAME_RAW}${CMAKE_DEBUG_POSTFIX}.pdb")
	endif()
	
    list(APPEND pargs_RUNTIME_LIBRARIES_AND_DIR_RELEASE "bin/${LIBRARY_NAME_RAW}.dll")
	if(EXISTS "${CMAKE_BINARY_DIR}/bin/Release/${LIBRARY_NAME_RAW}.pdb")
		list(APPEND pargs_RUNTIME_LIBRARIES_AND_DIR_RELEASE "bin/${LIBRARY_NAME_RAW}.pdb")
	endif()

    foreach(_dll_dir_dbg ${pargs_RUNTIME_LIBRARIES_AND_DIR_DEBUG})
        set(_cur_dll_dir "$(MSBuildThisFileDirectory)/../${_dll_dir_dbg}")
        
        file(TO_NATIVE_PATH ${_cur_dll_dir} _cur_dll_dir)
        
        list(APPEND additional_dlls_dbg ${_cur_dll_dir})
    endforeach()

    foreach(_dll_dir_rel ${pargs_RUNTIME_LIBRARIES_AND_DIR_RELEASE})
        set(_cur_dll_dir "$(MSBuildThisFileDirectory)/../${_dll_dir_rel}")
        
        file(TO_NATIVE_PATH ${_cur_dll_dir} _cur_dll_dir)
        
        list(APPEND additional_dlls_rel ${_cur_dll_dir})
    endforeach()
     
    get_target_property( _target_binary_dir ${pargs_TARGET} BINARY_DIR )
    
    # Create the final MSBUILD property file
    configure_file( ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../templates/NugetMsBuild.props.in
        ${_target_binary_dir}/${pargs_TARGET}.props
    )
     
    install(
        FILES 
            ${_target_binary_dir}/${pargs_TARGET}.props
        DESTINATION build
    )

    foreach(_prop_file ${MS_BUILD_PROPERTY_FILES})
        list(APPEND existing_prop_files ${_prop_file})
    endforeach()
    
    if(NOT "${pargs_TARGET}.props" IN_LIST existing_prop_files)
        set(MS_BUILD_PROPERTY_FILES ${MS_BUILD_PROPERTY_FILES} ${pargs_TARGET}.props CACHE INTERNAL "")
    endif()
  
    
endfunction(add_msbuild_props_file)


function(add_project_msbuild_props_file)

    set( _props_template_content [==[
<?xml version="1.0" ?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">

    <ImportGroup Label="Dependencies">
]==])

    set(COUNTER 1)
    foreach(_prop_file ${MS_BUILD_PROPERTY_FILES})

        set( _props_template_content ${_props_template_content} [==[
        <Import Project="]==])
       
        set( _props_template_content ${_props_template_content} ${_prop_file})
        
        set( _props_template_content ${_props_template_content} [==[
" />
]==])
        
    endforeach()
    
    set( _props_template_content ${_props_template_content} [==[
    </ImportGroup>

</Project>
]==])

    file( WRITE "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.props.in" ${_props_template_content} )

    configure_file( ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.props.in
        ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.props
    )
    
    install(
        FILES 
           ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.props
        DESTINATION build
    )

endfunction(add_project_msbuild_props_file)