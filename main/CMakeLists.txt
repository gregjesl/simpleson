idf_component_register(SRCS "esp32_main.cpp" "../json.cpp"
                    INCLUDE_DIRS "../"
                    )

target_compile_options(${COMPONENT_LIB} PRIVATE)