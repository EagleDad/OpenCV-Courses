
set(_open_vino_folder "openvino")
set(_open_vino_tag "2021.4.1")
set(_open_vino_root "${FETCHCONTENT_BASE_DIR}/${_open_vino_folder}/${_open_vino_folder}-${_open_vino_tag}")


ExternalProject_Add(
    ${_open_vino_folder}
    PREFIX ${FETCHCONTENT_BASE_DIR}/${_open_vino_folder}
    GIT_REPOSITORY      https://github.com/openvinotoolkit/openvino.git
    GIT_TAG             ${_open_vino_tag}
    SOURCE_DIR "${_open_vino_root}/src"
    BINARY_DIR "${_open_vino_root}/build"
    INSTALL_DIR "${_open_vino_root}/install"
    CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    -DCMAKE_BUILD_TYPE=Release
    -DBUILD_TESTING:BOOL=FALSE
    -DENABLE_SAMPLES:BOOL=FALSE
)

list(APPEND DEPENDENCIES openvino)

set(InferenceEngine_DIR "${_open_vino_root}/install/deployment_tools/inference_engine/share")
set(ngraph_DIR "${_open_vino_root}/install/deployment_tools/ngraph/cmake")

list(APPEND EXTRA_CMAKE_ARGS
    -DInferenceEngine_DIR:PATH=${InferenceEngine_DIR}
    -Dngraph_DIR:PATH=${ngraph_DIR}
)