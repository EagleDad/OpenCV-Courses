set( VERSION "origin/develop2")
set( PROJECT_NAME "conan")

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}
    GIT_REPOSITORY      https://github.com/conan-io/conan.git
    GIT_TAG             ${VERSION}
    SOURCE_DIR "${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-${VERSION}/conan_src/"
    SOURCE_SUBDIR ""
    BINARY_DIR "${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-${VERSION}/build"
    INSTALL_DIR "${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-${VERSION}/install"
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
    CMAKE_ARGS
)

set(WD "${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}/conan-origin/develop2/conan_src")
set( PYTHON_COMMAND -m pip install -e . )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND python ${PYTHON_COMMAND}
					WORKING_DIRECTORY ${WD}
)

set( CONAN_DEFAULT_PROFILE_COMMAND conan profile detect --force ) 

add_custom_command( TARGET ${PROJECT_NAME}
					POST_BUILD
                    COMMAND ${CONAN_DEFAULT_PROFILE_COMMAND}
)

set( CONAN_QUERY_PROFILE_PATH conan profile path default )

add_custom_command( TARGET ${PROJECT_NAME}
					POST_BUILD
                    COMMAND ${CONAN_QUERY_PROFILE_PATH}
)

set(CONAN_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/build/conan_toolchain.cmake")

MESSAGE(STATUS "Toolchain file: ${CONAN_TOOLCHAIN_FILE}")

list(APPEND EXTRA_CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE:PATH=${CONAN_TOOLCHAIN_FILE}
)

list(APPEND DEPENDENCIES ${PROJECT_NAME})
