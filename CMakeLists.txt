cmake_minimum_required(VERSION 3.11)

cmake_policy(SET CMP0091 NEW)

# Configure C++
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

set(COPYRIGHT_YEAR_START 2022)

list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Checks
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Common
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/External
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/.NetSupport
)

# In productive projects, we often use libraries which license require to link
# dynamically (e.g. LGPL). Further more, shared libraries shortens build time
# when building incrementally, since less files have to be rewritten when a
# library has to be rebuilt.
# So build shared libraries by default. This option is the standard option
# read by CMake.
# Force all libraries to be build dynamic if ON, also they are set to STATIC
# If we set this flag to on and use gtest with FectContent we have to
# implement the main function for our unit tests by ourself
option(BUILD_SHARED_LIBS            "Build project libraries as shared libraries" OFF)
option(BUILD_TESTING                "Build with tests" ON)
option(BUILD_DOC                    "Build documentation" OFF)
option(BUILD_WITH_BENCHMARK         "Build with benchmark" ON)

option(DEBUG_POSTFIX                "Set to ON to force _d postfix for the binary names." ON)
option(ENABLE_SOLUTION_FOLDERS      "Solution folder in Visual Studio or in other IDEs" ON)
option(STATIC_CODE_ANALYSIS         "Enables static code analysis with clang-tidy and CppCheck, if installed." OFF)
option(COVERAGE_ON                  "Enable code coverage" OFF)

# Build type
set(BUILD_TYPE "Engineering" CACHE STRING "The build type to use for the projects")
set_property(CACHE BUILD_TYPE PROPERTY STRINGS Internal Official Engineering)

include(VersionInfo)
project(OpenCVCourses VERSION ${PROJECT_VERSION_FULL})

if(ENABLE_SOLUTION_FOLDERS)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")
endif()

# Standard CMake modules
include(FindPackageHandleStandardArgs)
include(GenerateExportHeader)

# Modules
include(Common)
include(CompilerFlags)
include(VersionInfo)
include(MsBuildPropertySupport)

if(ENABLE_SOLUTION_FOLDERS)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")
endif()

if(DEBUG_POSTFIX)
    # For debug libs and exes, add "-d" postfix
    set(CMAKE_DEBUG_POSTFIX "-d" CACHE STRING "Set debug library postfix")
endif(DEBUG_POSTFIX)

if(BUILD_TESTING)
    if(COVERAGE_ON)
        include(CodeCoverageSupport)
        if (MSVC)
            include(OpenCppCoverageCheck)
        endif(MSVC)
    endif()
    # Activate test support
    # Enable integration with CTest
    include(CTest)
    include(GoogleTestTesting)
    enable_testing()
    list (APPEND CMAKE_CTEST_ARGUMENTS "--verbose")
endif(BUILD_TESTING)

if(BUILD_WITH_BENCHMARK)
    include( BenchmarkIntegration )
endif(BUILD_WITH_BENCHMARK)

if(BUILD_DOC)
    add_subdirectory(doc)
endif(BUILD_DOC)

# Add CppCheck if installed
include(CppCheck) 

# Add clang-format fo all projects
add_custom_target(ALL
    SOURCES 
        .clang-format
        .clang-tidy
)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation Directory" FORCE)

message(STATUS "")
message(STATUS "  Install to: " "${CMAKE_INSTALL_PREFIX}")
message(STATUS "-----------------------------------------------------------------")
message(STATUS "")

cmake_host_system_information(RESULT PRETTY_NAME QUERY HAS_MMX)
message(STATUS "Distributen Name: ${PRETTY_NAME}")

# Let CMake load the CMakeLists.txt in the subdirectory to load the
# sub-projects. Prefer lower-case names.
#add_subdirectory(libraries)
add_subdirectory(applications)

message(STATUS "Version: ${PROJECT_VERSION_FULL}")

add_project_msbuild_props_file()

set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION_FULL})

#Set the name of the generated installation package
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})

#Set the directory path to install to
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/packages") 

set(CPACK_PACKAGE_VENDOR "Dirk Adler")

set(CPACK_NUGET_PACKAGE_LICENSE_FILE_NAME "${CMAKE_SOURCE_DIR}/License")

if(WIN32)
    set(CPACK_NUGET_PACKAGE_TAGS "Native" "native")
    set(CPACK_GENERATOR ZIP NuGet)
else()
    set(CPACK_GENERATOR TGZ)
endif()

set(CPACK_INSTALL_CMAKE_CONFIGURATIONS Debug Release)

include(CPack)
