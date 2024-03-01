set( LIBRARY_NAME "cvHelper" )
get_raw_target_name( ${LIBRARY_NAME} LIBRARY_NAME_RAW )

find_package( OpenCV REQUIRED )

include_directories( ${OpenCV_INCLUDE_DIRS} )

add_library( ${LIBRARY_NAME_RAW} SHARED
    
    include/GUI.h
    include/macros.h

    src/GUI.cpp
)

add_library( ${LIBRARY_NAME} ALIAS ${LIBRARY_NAME_RAW} )

target_include_directories( ${LIBRARY_NAME_RAW}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:include/>
        src/
)

set_compiler_warning_flags( 
    STRICT
    TARGET ${LIBRARY_NAME_RAW}
)

if( ENABLE_SOLUTION_FOLDERS )
    set_target_properties (${LIBRARY_NAME_RAW} PROPERTIES FOLDER "libraries" )
endif()

add_version_information(
    TARGET
        ${LIBRARY_NAME_RAW}
)

set_target_properties( ${LIBRARY_NAME_RAW} 
    PROPERTIES PUBLIC_HEADER 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
)

# Generate header file "machinevision/core/export.h"
generate_export_header(${LIBRARY_NAME_RAW} EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/cvHelper/export.h)
target_include_directories ( ${LIBRARY_NAME_RAW}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/>
)

target_link_libraries( ${LIBRARY_NAME_RAW}
    PUBLIC
        ${OpenCV_LIBS}
)

if( BUILD_TESTING )
    # Build unit tests only if testing is enabled
    #add_subdirectory(test)
endif( )



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
