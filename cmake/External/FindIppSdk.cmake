find_path(IppSdk_DIR "IppSdkConfig.cmake" DOC "Root directory of Ipp SDK")

if(EXISTS "${IppSdk_DIR}")

#When its possible to use the Config script use it.

    if(EXISTS "${IppSdk_DIR}/IppSdkConfig.cmake")

        # Include the standard CMake script
        include("${IppSdk_DIR}/IppSdkConfig.cmake")

    else(EXISTS "${IppSdk_DIR}/IppSdkConfig.cmake")

    endif(EXISTS "${IppSdk_DIR}/IppSdkConfig.cmake")

    set(IppSdk_FOUND TRUE)

else(EXISTS "${IppSdk_DIR}")

    set(ERR_MSG "Please specify IppSdk directory using IppSdk_DIR env. variable")

endif(EXISTS "${IppSdk_DIR}")