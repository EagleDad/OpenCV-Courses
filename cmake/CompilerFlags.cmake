if(NOT CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    message(FATAL_ERROR "Unsupported compiler")
endif()

# This function is used to set the compiler flags for the whol project or 
# can be used to change them for individual projects if needed.
#
# The following presets are supported:
#   STRICT      - Uses as less as possible disabled warnings
#   THIRD_PARTY - Used, when 3rd party modules are involved

function( set_compiler_warning_flags )

	cmake_parse_arguments(
        pargs 
        "STRICT;THIRD_PARTY;"
        "TARGET;"
        ""
        ${ARGN}
    )

    if(pargs_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown arguments: ${pargs_UNPARSED_ARGUMENTS}")
    endif()
    
    if( "TARGET" IN_LIST pargs_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "TARGET: not defined!")
    endif()

    if ( MSVC )

        if( NOT pargs_THIRD_PARTY )

            target_compile_options( ${pargs_TARGET}
                PRIVATE
                    -Wall       # Enable all warnings
                    /WX         # Warnings as errors
                    /wd4251     # Disable: class 'type1' needs to have dll-interface to be used by clients of class 'type2'
                    /wd4275     # Disable: non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
                    /wd4514     # Disable: unreferenced inline function has been removed
                    /wd4668     # Disable: is not defined as a preprocessor macro, replacing with '0'
                    /wd4710     # Disable: function not inlined
                    /wd4711     # Disable: function selected for inline expansion
                    /wd5045     # Disable: Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
                    /wd4820     # Disable: bytes padding added after data member
                    /wd5264     # 'variable-name': 'const' variable is not used
                    /wd5262     # implicit fall-through occurs here; are you missing a break statement? Use [[fallthrough]] when a break statement is intentionally omitted between cases
            )

        else()

            target_compile_options( ${pargs_TARGET}
                PRIVATE
                    -W2         # Warning level 2
                    /WX-        # Warnings are no errors
            )

        endif()

    endif( MSVC )

endfunction( set_compiler_warning_flags )