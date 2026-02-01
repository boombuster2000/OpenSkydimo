add_library(openskydimo-common STATIC
        include/openskydimo/logger.h
        src/logger.cpp
)

target_include_directories(openskydimo-common PUBLIC include)