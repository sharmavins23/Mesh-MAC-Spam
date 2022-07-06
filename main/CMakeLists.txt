# Boilerplate CMake file
idf_component_register(
    # Source files for all code
    SRCS
        main.c
    # Any included headerfiles
    INCLUDE_DIRS
        .
    # Certs and the like
    EMBED_TXTFILES
        .
    # Libraries required for implementation
    REQUIRES
        nvs_flash
        json
        esp_http_client
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_CONF_INCLUDE_SIMPLE=1)