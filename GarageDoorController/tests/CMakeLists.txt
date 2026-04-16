cmake_minimum_required(VERSION 3.16)
project(door_fsm_tests C)
set(CMAKE_C_STANDARD 11)

set(UNITY_DIR "C:/esp/v6.0/esp-idf/components/unity/unity/src")

add_executable(test_door_fsm
    test_door_fsm.c
    ../main/door_fsm.c
    ${UNITY_DIR}/unity.c
)

target_include_directories(test_door_fsm PRIVATE
    ../main
    ${UNITY_DIR}
)

enable_testing()
add_test(NAME door_fsm COMMAND test_door_fsm)