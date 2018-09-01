include_directories(../)

file(GLOB tests
    "*.cpp"
)

# Set debug to ensure asserts work
set(CMAKE_CONFIGURATION_TYPES "debug")

foreach(test ${tests})
    string(REGEX REPLACE ".*/" "" test_name "${test}")
    string(REGEX REPLACE ".cpp$" "" test_name "${test_name}")
    add_executable ("${test_name}_test" ${test})
    target_link_libraries("${test_name}_test" json)
    add_test(NAME ${test_name} COMMAND "${test_name}_test")
endforeach()