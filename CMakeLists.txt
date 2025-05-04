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
    elseif (CMAKE_SYSTEM_NAME STREQUAL "CYGWIN")
        add_compile_options(-fno-rtti -Wall -Wextra -Wpedantic)
    else()
        add_compile_options(-fno-rtti -Wall -Wextra -Wpedantic)
    endif()
endif()


add_executable(cybercafe_monitoring_system
    src/test.cc
    src/cybercafe_monitoring_system.cc
)

target_include_directories(cybercafe_monitoring_system PRIVATE ${CMAKE_SOURCE_DIR})

