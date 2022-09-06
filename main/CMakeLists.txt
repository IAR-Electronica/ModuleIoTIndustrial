idf_component_register(SRCS "main.c" "archivo.c"
                    INCLUDE_DIRS "." 
                    REQUIRES mcommon mconfig mwifi mlink ADS1115 mqtt_handle
)