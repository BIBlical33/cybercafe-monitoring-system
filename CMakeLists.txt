cmake_minimum_required(VERSION 3.20)

project(cybercafe_monitoring_system)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    foreach(flag_var
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REPLACE "/GR" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/W3" "" ${flag_var} "${${flag_var}}")
    endforeach()
    add_compile_options(/GR- /W4)
    add_compile_options(/Zi /DEBUG)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-fno-rtti -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND MINGW)
        add_compile_options(-fno-rtti -Wall -Wextra -Wpedantic)
    elseif ()
        add_compile_options(-fno-rtti -Wall -Wextra -Wpedantic)
    else()
        add_compile_options(-fno-rtti -Wall -Wextra -Wpedantic)
    endif()
endif()

# Main library
add_library(
    cybercafe_monitoring_system_lib
    src/cybercafe_monitoring_system.cc
    src/read_input_data.cc
)
target_include_directories(cybercafe_monitoring_system_lib PRIVATE ${CMAKE_SOURCE_DIR})

# Main application
add_executable(
  cybercafe_monitoring_system_run
  src/main.cc
)
target_link_libraries(
  cybercafe_monitoring_system_run
  cybercafe_monitoring_system_lib
)
target_include_directories(cybercafe_monitoring_system_run PRIVATE ${CMAKE_SOURCE_DIR})


if (NOT CMAKE_SYSTEM_NAME STREQUAL "CYGWIN")
    include(FetchContent)

    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    # Testing application
    enable_testing()
    add_executable(
      cybercafe_monitoring_system_test
      tests/revenue_calculating_test.cc
      tests/event_handlers_test.cc
      tests/input_data_test.cc
    )
    target_link_libraries(
      cybercafe_monitoring_system_test
      GTest::gtest_main
      cybercafe_monitoring_system_lib
    )
    target_include_directories(cybercafe_monitoring_system_test PRIVATE ${CMAKE_SOURCE_DIR})

    include(GoogleTest)
    gtest_discover_tests(cybercafe_monitoring_system_test)
endif()
