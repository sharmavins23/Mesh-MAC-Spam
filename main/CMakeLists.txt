# Boilerplate CMake file
idf_component_register(
    # Source files for all code
    SRCS
        main.c
        wifiConnect.c
        updateSpam.c
    INCLUDE_DIRS
        .
        include
)