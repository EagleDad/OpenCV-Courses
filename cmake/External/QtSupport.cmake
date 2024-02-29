
set( PROJECT_VERSION "6.4.1")
set( PROJECT_NAME "Qt")

#cmake -E chdir C:\Users\dirk.adler\source\repos\image-processing-toolbox\build\_deps\Qt\Qt-6.4.1\build C:\Users\dirk.adler\source\repos\image-processing-toolbox\build\_deps\Qt\Qt-6.4.1\src\configure.bat -prefix C:\Users\dirk.adler\source\repos\image-processing-toolbox\build\_deps\Qt\Qt-6.4.1\install
#..\configure -prefix C:\Users\dirk.adler\source\repos\image-processing-toolbox\build\_deps\Qt\Qt-6.4.1\install
#cmake --build . --parallel

#C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools
#VsDevCmd.bat  
#VS140COMNTOOLS
#   ${CMAKE_COMMAND} --build <BINARY_DIR> --config $<CONFIG>

#cmake --build C:\Users\dirk.adler\source\repos\image-processing-toolbox\build\_deps\Qt\Qt-6.4.1\build --parallel

MESSAGE(STATUS $ENV{VS140COMNTOOLS})

set(buildDir "${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-${PROJECT_VERSION}/build")
set(installDir "${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-${PROJECT_VERSION}/install")
set(srcDir "${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-${PROJECT_VERSION}/src")
set(configCmd "${srcDir}/configure.bat -prefix ${installDir}")

ExternalProject_Add(
    ${PROJECT_NAME}
    PREFIX ${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}
    GIT_REPOSITORY      https://github.com/qt/qtbase.git
    GIT_TAG             "v${PROJECT_VERSION}"
    SOURCE_DIR "${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-${PROJECT_VERSION}/src/"
    SOURCE_SUBDIR ""
    #BINARY_DIR "${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-${PROJECT_VERSION}/build"
    #INSTALL_DIR "${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-${PROJECT_VERSION}/install"
    BINARY_DIR ""
    INSTALL_DIR ""
    CMAKE_ARGS ""
    #-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    #-DCMAKE_BUILD_TYPE=Release
    #-DCMAKE_DEBUG_POSTFIX:STRING=-d
    #CONFIGURE_COMMAND "mkdir ${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-${PROJECT_VERSION}/build"
    CONFIGURE_COMMAND   ${CMAKE_COMMAND} -E echo "Creating build directory"
    COMMAND             ${CMAKE_COMMAND} -E make_directory ${buildDir}
    COMMAND             ${CMAKE_COMMAND} -E echo "Creating install directory"
    COMMAND             ${CMAKE_COMMAND} -E make_directory ${installDir}
    COMMAND             ${CMAKE_COMMAND} -E echo "Configuring project"
    COMMAND             ${CMAKE_COMMAND} -E chdir ${buildDir} ${srcDir}/configure.bat -prefix ${installDir}
    COMMAND             ${CMAKE_COMMAND} -E echo "Configuration done"

    BUILD_COMMAND   ${CMAKE_COMMAND} -E echo "Starting Visual Studio Developer Shell"
    COMMAND         ${CMAKE_COMMAND} -E chdir $ENV{VS140COMNTOOLS} VsDevCmd.bat
    COMMAND         ${CMAKE_COMMAND} -E echo "Starting Qt build"
    COMMAND         ${CMAKE_COMMAND} --build ${buildDir} --parallel
    COMMAND         ${CMAKE_COMMAND} -E echo "Building Qt done"

    INSTALL_COMMAND ${CMAKE_COMMAND} -E echo "Installing Qt"
    COMMAND         ${CMAKE_COMMAND} --install ${buildDir}
    COMMAND         ${CMAKE_COMMAND} -E echo "Installing Qt done"
   
)

list(APPEND DEPENDENCIES ${PROJECT_NAME})

set(Qt6_DIR "${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-${PROJECT_VERSION}/install/lib/cmake/Qt6")
set(Qt6CoreTools_DIR "${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-${PROJECT_VERSION}/install/lib/cmake/Qt6CoreTools")
set(Qt6GuiTools_DIR "${FETCHCONTENT_BASE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}-${PROJECT_VERSION}/install/lib/cmake/Qt6GuiTools")

list(APPEND EXTRA_CMAKE_ARGS
    -DQt6_DIR:PATH=${Qt6_DIR}
    -DQt6CoreTools_DIR:PATH=${Qt6CoreTools_DIR}
    -DQt6GuiTools_DIR:PATH=${Qt6GuiTools_DIR}
)
