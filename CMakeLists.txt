cmake_minimum_required(VERSION 3.14)
project(my_project)

set(CMAKE_CXX_STANDARD 20)

################################################################################
#                                   Targets                                    #
################################################################################

add_executable(test common.cc test.cc)
add_executable(generator generator.cc common.cc)

################################################################################
#                            Common compile options                            #
################################################################################

target_compile_options(test PRIVATE -Wall -Wextra -Wshadow=compatible-local -Wno-sign-compare -pedantic)
target_compile_options(generator PRIVATE -Wall -Wextra -Wshadow=compatible-local -Wno-sign-compare -pedantic)

################################################################################
#                                  Sanitizers                                  #
################################################################################

# NOTE: the third argument (value) can be overriden by the user at compile time
option(USE_SANITIZERS "Enable to build with undefined,leak and address sanitizers" OFF)
if (USE_SANITIZERS)
  message(STATUS "Enabling sanitizers...")
  # NOTE: leak sanitizer doesn't work on MacOS
  if (CMAKE_SYSTEM MATCHES "Darwin")
      set(COMPILE_OPTS PUBLIC -fsanitize=address,undefined -fno-sanitize-recover=all)
      set(LINK_OPTS PUBLIC -fsanitize=address,undefined)
  else()
      set(COMPILE_OPTS PUBLIC -fsanitize=address,undefined,leak -fno-sanitize-recover=all)
      set(LINK_OPTS PUBLIC -fsanitize=address,undefined,leak)
  endif()

  # NOTE: these strings should be repeated for all of the targets
  target_compile_options(test PUBLIC ${COMPILE_OPTS})
  target_compile_options(generator PUBLIC ${COMPILE_OPTS})
  target_link_options(test PUBLIC ${LINK_OPTS})
  target_link_options(generator PUBLIC ${LINK_OPTS})
endif()

################################################################################
#                                Debug options                                 #
################################################################################

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  message(STATUS "Enabling debugging compile options...")
  set(DEBUG_COMPILE_OPTS PUBLIC -g -fno-omit-frame-pointer)
  # NOTE: -DGLIBCXX_DEBUG doesn't work with range-v3
  # set(DEBUG_COMPILE_OPTS PUBLIC -g -fno-omit-frame-pointer -D_GLIBCXX_DEBUG)
  # NOTE: target_compile_options appends the options to the target, i.e. it
  # doesn't rewrite existing options:
  # https://cmake.org/cmake/help/latest/command/target_compile_options.html
  target_compile_options(test PUBLIC ${DEBUG_COMPILE_OPTS})
  target_compile_options(generator PUBLIC ${DEBUG_COMPILE_OPTS})
endif()

################################################################################
#                             Clang-specific stuff                             #
################################################################################

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message(STATUS "Enabling libc++...")
  target_compile_options(test PUBLIC -stdlib=libc++)
  target_compile_options(generator PUBLIC -stdlib=libc++)

  target_link_options(test PUBLIC -stdlib=libc++)
  target_link_options(generator PUBLIC -stdlib=libc++)
endif()

################################################################################
#                                 Dependencies                                 #
################################################################################

set(USE_CPPUTILS ON)
set(USE_GTEST ON)
set(USE_FMT OFF)
set(USE_RANGEV3 ON)
set(USE_RE2 OFF)
set(USE_JSON OFF)
set(USE_SPDLOG OFF)
set(USE_ARGPARSE OFF)
set(USE_ABSL ON)

include(cmake/ahmad1337_deps.cmake)

target_link_libraries(test ${DEP_LIBS} absl::strings absl::str_format absl::log)
target_link_libraries(generator ${DEP_LIBS} absl::strings absl::str_format absl::log)
