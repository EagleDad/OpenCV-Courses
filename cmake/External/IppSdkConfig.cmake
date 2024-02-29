# ===================================================================================
#  The IppSdk CMake configuration file
#
#  Usage from an external project:
#    In your CMakeLists.txt, add these lines:
#
#    find_package(IppSdk REQUIRED)
#    include_directories(${IppSdk_INCLUDE_DIRS})
#    target_link_libraries(MY_TARGET_NAME ${IppSdk_LIBS})
#
#    If the module is found then IPP_<MODULE>_FOUND is set to TRUE.
#
#    This file will define the following variables:
#      - IppSdk_LIBS                     : The list of all imported targets for IppSdk modules.
#      - IppSdk_INCLUDE_DIRS             : The IppSdk include directories.
#      - IppSdk_VERSION                  : The version of this IppSdk build: "13.10.0"
#      - IppSdk_VERSION_MAJOR            : Major version part of IppSdk_VERSION: "13"
#      - IppSdk_VERSION_MINOR            : Minor version part of IppSdk_VERSION: "10"
#      - IppSdk_VERSION_PATCH            : Patch version part of IppSdk_VERSION: "0"
#
# ===================================================================================
cmake_minimum_required(VERSION 3.1)
# ======================================================
#  Version variables:
# ======================================================
SET(IppSdk_VERSION 8.2.0)
SET(IppSdk_VERSION_MAJOR  8)
SET(IppSdk_VERSION_MINOR  2)
SET(IppSdk_VERSION_PATCH  0)
#SET(IppSdk_FOUND FALSE)

include(FindPackageHandleStandardArgs)

if(WIN32)
    set(IppSdk_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/include)
    set(IppSDK_LIB_PATH ${CMAKE_CURRENT_LIST_DIR}/lib64)
    set(IppSDK_DLL_PATH ${CMAKE_CURRENT_LIST_DIR}/bin64)
    set(ipp_libs)
    set(ipp_dlls)

    set(IppSdk_LIB_COMPONENTS Ipp_sdk)
    set(IppSdk_LIBS Ipp_sdk)

    add_library(Ipp_sdk SHARED IMPORTED)

    # TODO Figuer out which dll belongs to mt
    # Get a list of all libs for 64 bit
    # file(GLOB ipp_libs "${IppSDK_LIB_PATH}/*.lib")
    # Get a list of all dlls for 64 bit
    # file(GLOB ipp_dlls "${IppSDK_DLL_PATH}/*.dll")

    list(APPEND ipp_libs "${IppSDK_LIB_PATH}/ippcc.lib")
    list(APPEND ipp_libs "${IppSDK_LIB_PATH}/ippch.lib")
    list(APPEND ipp_libs "${IppSDK_LIB_PATH}/ippcore.lib")
    list(APPEND ipp_libs "${IppSDK_LIB_PATH}/ippcv.lib")
    list(APPEND ipp_libs "${IppSDK_LIB_PATH}/ippdc.lib")
    list(APPEND ipp_libs "${IppSDK_LIB_PATH}/ippi.lib")
    list(APPEND ipp_libs "${IppSDK_LIB_PATH}/ippm.lib")
    list(APPEND ipp_libs "${IppSDK_LIB_PATH}/ipps.lib")
    list(APPEND ipp_libs "${IppSDK_LIB_PATH}/ippvm.lib")

    list(APPEND ipp_dlls "${IppSDK_DLL_PATH}/ippcc-8.2.dll")
    list(APPEND ipp_dlls "${IppSDK_DLL_PATH}/ippch-8.2.dll")
    list(APPEND ipp_dlls "${IppSDK_DLL_PATH}/ippcore-8.2.dll")
    list(APPEND ipp_dlls "${IppSDK_DLL_PATH}/ippcv-8.2.dll")
    list(APPEND ipp_dlls "${IppSDK_DLL_PATH}/ippdc-8.2.dll")
    list(APPEND ipp_dlls "${IppSDK_DLL_PATH}/ippi-8.2.dll")
    list(APPEND ipp_dlls "${IppSDK_DLL_PATH}/ippm-8.2.dll")
    list(APPEND ipp_dlls "${IppSDK_DLL_PATH}/ipps-8.2.dll")
    list(APPEND ipp_dlls "${IppSDK_DLL_PATH}/ippvm-8.2.dll")


    #foreach(lib ${ipp_libs})
    #   message(STATUS "\t${lib}")
    #endforeach()

    #foreach(dll ${ipp_dlls})
    #   message(STATUS "\t${dll}")
    #endforeach()

    #message(STATUS "IPP libs: ${ipp_libs}")
    #message(STATUS "IPP libs: ${ipp_dlls}")
    
    set_property(TARGET Ipp_sdk APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    
    if (CMAKE_CL_64)
        set_target_properties(Ipp_sdk PROPERTIES
            IMPORTED_IMPLIB_DEBUG ${ipp_libs}
            IMPORTED_LOCATION_DEBUG ${ipp_dlls}
        )
    else()
        message(FATAL_ERROR "A 32 bit version of IPP is not supported!")
    endif()
    
    set_property(TARGET Ipp_sdk APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    
    if (CMAKE_CL_64)
        set_target_properties(Ipp_sdk PROPERTIES
            IMPORTED_IMPLIB_RELEASE ${ipp_libs}
            IMPORTED_LOCATION_RELEASE ${ipp_dlls}
        )
    else()
        message(FATAL_ERROR "A 32 bit version of IPP is not supported!")
    endif()

    message(STATUS "Found IPP ${IppSdk_VERSION} in ${IppSDK_LIB_PATH}")

else(WIN32)

endif(WIN32)