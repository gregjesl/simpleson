find_package(benchmark QUIET)
if(NOT benchmark_FOUND)
    message(STATUS "benchmark package not found; skipping baseline benchmark targets")
    return()
endif()

function(simpleson_copy_runtime_dll target_name)
    if(WIN32)
        # Copy DLL next to the executable so it can run from build output.
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:simpleson>
                $<TARGET_FILE_DIR:${target_name}>
        )
    endif()
endfunction()

add_executable(baseline_benchmark_test_time baseline_benchmark_test_time.cpp)
target_link_libraries(baseline_benchmark_test_time PRIVATE simpleson benchmark::benchmark benchmark::benchmark_main)
target_compile_features(baseline_benchmark_test_time PRIVATE cxx_std_17)
simpleson_copy_runtime_dll(baseline_benchmark_test_time)

add_executable(baseline_benchmark_test_time_comprehensive baseline_benchmark_test_time_comprehensive.cpp)
target_link_libraries(baseline_benchmark_test_time_comprehensive PRIVATE simpleson benchmark::benchmark benchmark::benchmark_main)
target_compile_features(baseline_benchmark_test_time_comprehensive PRIVATE cxx_std_17)
simpleson_copy_runtime_dll(baseline_benchmark_test_time_comprehensive)

if(TARGET allocounter)
    add_executable(baseline_benchmark_test_memory baseline_benchmark_test_memory.cpp)
    target_link_libraries(baseline_benchmark_test_memory PRIVATE simpleson allocounter benchmark::benchmark benchmark::benchmark_main)
    target_compile_features(baseline_benchmark_test_memory PRIVATE cxx_std_17)
    simpleson_copy_runtime_dll(baseline_benchmark_test_memory)
else()
    message(STATUS "allocounter target not found; skipping baseline_benchmark_test_memory")
endif()
