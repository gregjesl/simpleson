# For integration in a ESP project
# See https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#using-third-party-cmake-projects-with-components
if (ESP_PLATFORM)

    # The following lines of boilerplate have to be in your project's
    # CMakeLists in this exact order for cmake to work correctly
    cmake_minimum_required(VERSION 3.16)

    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(simpleson_esp32)
    set(EXTRA_COMPONENT_DIRS "esp")
    idf_build_set_property(COMPILE_OPTIONS "-Wimplicit-fallthrough" APPEND)

else()

    cmake_minimum_required (VERSION 3.8)

    project(simpleson VERSION 1.1.0)

    add_library(${PROJECT_NAME}
        json.cpp
    )

    target_include_directories(${PROJECT_NAME} PUBLIC .)

    if(MSVC)
        # ignore warnings about scanf
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        add_compile_options(/W4 /WX)
    elseif(BORLAND)
    else()
        add_compile_options(-Wall -Wextra -Werror)
    endif()

    enable_testing()
    add_subdirectory (test)
    add_subdirectory (examples)
    add_subdirectory (issues)

endif()
