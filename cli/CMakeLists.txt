add_executable(openskydimo-cli
        src/main.cpp
        include/OpenSkydimoClient.h
        src/OpenSkydimoClient.cpp
)

set_target_properties(openskydimo-cli PROPERTIES OUTPUT_NAME "openskydimo")

target_include_directories(openskydimo-cli PRIVATE include)
target_link_libraries(openskydimo-cli PRIVATE openskydimo-common)

install(TARGETS openskydimo-cli RUNTIME DESTINATION bin)