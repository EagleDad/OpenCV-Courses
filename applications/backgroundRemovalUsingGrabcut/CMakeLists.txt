# Don't use PROJECT_NAME as target name, it's too tight of a relationship
set(EXECUTABLE_NAME "backgroundRemovalUsingGrabcut")

find_package( OpenCV REQUIRED )

include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(${EXECUTABLE_NAME}
	Interface.cpp
	Interface.h
    main.cpp
)

set_compiler_warning_flags( 
    STRICT
    TARGET ${EXECUTABLE_NAME}
)


if(ENABLE_SOLUTION_FOLDERS)
    set_target_properties(${EXECUTABLE_NAME} PROPERTIES FOLDER "applications")
endif()

target_include_directories(${EXECUTABLE_NAME}
    INTERFACE
    
    PUBLIC
)

copy_runtime_dependencies(

)

target_link_libraries(${EXECUTABLE_NAME}
    PUBLIC
        ${OpenCV_LIBS}
        cvHelper

    PRIVATE

    INTERFACE
)