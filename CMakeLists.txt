cmake_minimum_required(VERSION 3.16)

project(NotQuiteMyTempo VERSION 0.1.0 LANGUAGES CXX)

include(CTest)

add_library(not_quite_my_tempo STATIC
    src/benchmark.cpp
    src/metrics/time_collector.cpp
    src/metrics/linux/process_metrics_collector.cpp
    src/metrics/linux/hardware_metrics_collector.cpp
    src/statistics/summary.cpp
    src/statistics/complexity.cpp
    src/output/csv_writer.cpp
    src/system/linux/system_info.cpp
)

target_compile_features(not_quite_my_tempo PUBLIC cxx_std_17)
target_include_directories(not_quite_my_tempo
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(BUILD_TESTING)
    add_executable(structure_smoke_test tests/structure_smoke_test.cpp)
    target_link_libraries(structure_smoke_test PRIVATE not_quite_my_tempo)
    add_test(NAME structure_smoke_test COMMAND structure_smoke_test)
endif()
