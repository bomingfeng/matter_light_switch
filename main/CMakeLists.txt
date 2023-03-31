set(PRIV_REQUIRES_LIST device spiffs esp_matter esp_matter_console route_hook app_reset esp_wifi esp_matter_controller)


idf_component_register(SRC_DIRS          "."
		              EMBED_TXTFILES "WebsiteFiles/Debug_parameters.html" "WebsiteFiles/Multi_function_timing.html" "WebsiteFiles/upload_script.html" "WebsiteFiles/favicon.ico" "WebsiteFiles/jquery-3.4.1.min.js"
                      PRIV_INCLUDE_DIRS  "."
                      PRIV_REQUIRES      ${PRIV_REQUIRES_LIST})
                      
set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 14)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
