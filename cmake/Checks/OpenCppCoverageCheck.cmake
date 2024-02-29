find_program ( COVERAGE_EXE NAMES OpenCppCoverage OpenCppCoverage.exe
    PATHS "C:/Program Files/OpenCppCoverage"
    DOC "Path to OpenCppCoverage executable"
)

if ( COVERAGE_EXE )
    message(STATUS "Code coverage tool found")
else()
    message(WARNING "No code coverage tool found. Skipping code coverage...")
endif()
