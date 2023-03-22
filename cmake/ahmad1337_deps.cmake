# I don't know if setting these policies is okay but I did it anyway to get rid
# of warnings
# https://cmake.org/cmake/help/latest/policy/CMP0135.html
cmake_policy(SET CMP0135 NEW)
# https://cmake.org/cmake/help/latest/policy/CMP0077.html
cmake_policy(SET CMP0077 NEW)

# I found out that FetchContent_MakeAvailable does add_subdirectory by default
# (which I did manually for all of the libs anyway) so I will roll with it until
# I find any problems
include(FetchContent)

set(DEP_LIBS "")

# TODO: try to do a find_package before downloading
macro(AddUrlLib name url lib_target)
    FetchContent_Declare(
        ${name}
        URL ${url}
        )
    message("Fetching ${name}...")
    FetchContent_MakeAvailable(${name})
    message("Done")
    list(APPEND DEP_LIBS ${lib_target})
endmacro()

if (USE_CPPUTILS)
    AddUrlLib(
        cpputils
        https://github.com/A-Ih/cpputils/archive/refs/heads/main.zip
        cpputils::cpputils
        )
endif()

if (USE_GTEST)
    AddUrlLib(
        googletest
        https://github.com/A-Ih/googletest/archive/refs/heads/main.zip
        gtest_main
        )
endif()

if(USE_FMT)
    AddUrlLib(
        fmt
        https://github.com/A-Ih/fmt/archive/refs/heads/master.zip
        fmt::fmt
        )
endif()

if(USE_RANGEV3)
    AddUrlLib(
        range-v3
        https://github.com/A-Ih/range-v3/archive/refs/heads/master.zip
        range-v3::range-v3
        )
endif()

if(USE_RE2)
    AddUrlLib(
        re2
        https://github.com/A-Ih/re2/archive/refs/heads/main.zip
        re2::re2
        )
endif()

if(USE_JSON)
    AddUrlLib(
        json
        https://github.com/nlohmann/json/archive/refs/heads/master.zip
        nlohmann_json::nlohmann_json
        )
endif()

if(USE_SPDLOG)
    AddUrlLib(
        spdlog
        https://github.com/A-Ih/spdlog/archive/refs/heads/v1.x.zip
        spdlog::spdlog
        )
endif()

if(USE_ARGPARSE)
    AddUrlLib(
        argparse
        https://github.com/A-Ih/argparse/archive/refs/heads/master.zip
        argparse::argparse
    )
endif()

if (USE_ABSL)
    message("adding abseil to build...")
    set(ABSL_PROPAGATE_CXX_STD ON)
    add_subdirectory(third-party/abseil-cpp-master)
    message("abseil is now available!")
endif()
