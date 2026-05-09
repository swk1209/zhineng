idf_component_register(
    SRCS "app_service.c"
    INCLUDE_DIRS "."
    REQUIRES APPDATA WIFISTA COMM BSP nvs_flash
)
