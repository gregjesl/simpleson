# For integration in a ESP project
# See https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#using-third-party-cmake-projects-with-components
if (ESP_PLATFORM)

    idf_component_register(
        SRCS "json.cpp"
        INCLUDE_DIRS "."
    )

else()

    cmake_minimum_required (VERSION 3.8)

    project(simpleson VERSION 2.0.0)

    add_library(${PROJECT_NAME}
        ../json.cpp
    )

    enable_testing()
    add_subdirectory (test)

endif()
