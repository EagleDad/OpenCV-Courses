find_program(NUGET_EXE nuget)

if(NUGET_EXE)
    message(STATUS "nuget executable found. ${NUGET_EXE}")
else(NUGET_EXE)
    message(WARNING "No nuget executable found.")
endif(NUGET_EXE)
