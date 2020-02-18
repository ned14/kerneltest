cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
include(cmake/QuickCppLibBootstrap.cmake)
include(QuickCppLibRequireOutOfSourceBuild)
include(QuickCppLibUtils)
include(QuickCppLibPolicies)

# Parse the version we tell cmake directly from the version header file
ParseProjectVersionFromHpp("${CMAKE_CURRENT_SOURCE_DIR}/include/kerneltest/version.hpp" VERSIONSTRING)
# Sets the usual PROJECT_NAME etc
project(kerneltest VERSION ${VERSIONSTRING} LANGUAGES C CXX)
# Also set a *cmake* namespace for this project
set(PROJECT_NAMESPACE)
# Setup this cmake environment for this project
include(QuickCppLibSetupProject)
if(NOT PROJECT_IS_DEPENDENCY)
  # This file should be updated with the last git SHA next commit
  UpdateRevisionHppFromGit("${CMAKE_CURRENT_SOURCE_DIR}/include/kerneltest/revision.hpp")
endif()
# Find my library dependencies
find_quickcpplib_library(quickcpplib
  GIT_REPOSITORY "https://github.com/ned14/quickcpplib.git"
  REQUIRED
  IS_HEADER_ONLY
)
find_quickcpplib_library(outcome
  GIT_REPOSITORY "https://github.com/ned14/outcome.git"
#  GIT_TAG "develop"
  GIT_TAG "better_optimisation"  ## future Outcome v2.2
  REQUIRED
  IS_HEADER_ONLY
)

# Make an interface only library so dependent CMakeLists can bring in this header-only library
include(QuickCppLibMakeHeaderOnlyLibrary)

# If we have concepts, enable those for both myself and all inclusions
apply_cxx_concepts_to(INTERFACE kerneltest_hl)

# Create a custom doxygen generation target
include(QuickCppLibMakeDoxygen)
# Set the standard definitions for these libraries and bring in the all_* helper functions
include(QuickCppLibApplyDefaultDefinitions)
# Check if our std::tuple can do aggregate initialisation in C++ 14
include(CheckCXXSourceCompiles)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
check_cxx_source_compiles("
#include <tuple>
int main() {
  constexpr std::tuple<int, double, const char *> fc[] = {
      {1, 1.0, \"hi1\"}, {2, 2.0, \"hi2\"}, {3, 3.0, \"hi3\"}};
  static_assert(std::get<0>(fc[0]), \"die\");
  return 0;
}
" CXX_HAS_AGGREGATE_INITIALIZABLE_TUPLE)
unset(CMAKE_CXX_STANDARD)
unset(CMAKE_CXX_STANDARD_REQUIRED)
if(NOT CXX_HAS_AGGREGATE_INITIALIZABLE_TUPLE)
  # Try again with C++ 17, but use the flag directly as older cmake doesn't implement cxx_std_17
  set(CMAKE_REQUIRED_FLAGS "-std=c++1z")
  check_cxx_source_compiles("
  #include <tuple>
  int main() {
    constexpr std::tuple<int, double, const char *> fc[] = {
        {1, 1.0, \"hi1\"}, {2, 2.0, \"hi2\"}, {3, 3.0, \"hi3\"}};
    static_assert(std::get<0>(fc[0]), \"die\");
    return 0;
  }
  " CXX_HAS_AGGREGATE_INITIALIZABLE_TUPLE2)
  unset(CMAKE_REQUIRED_FLAGS)
  if(NOT CXX_HAS_AGGREGATE_INITIALIZABLE_TUPLE2)
    indented_message(FATAL_ERROR "This STL's std::tuple cannot do aggregate initialisation, "
    "and enabling C++ 17 didn't help so bailing out, sorry")
  endif()
  foreach(feature ${CMAKE_CXX_COMPILE_FEATURES})
    if(feature STREQUAL cxx_std_17)
      target_compile_features(kerneltest_hl INTERFACE cxx_std_17)
    endif()
  endforeach()
endif()

# Set the library dependencies this library has
target_link_libraries(kerneltest_hl INTERFACE quickcpplib::hl outcome::hl)

# For all possible configurations of this library, add each test
include(QuickCppLibMakeStandardTests)

# Cache this library's auto scanned sources for later reuse
include(QuickCppLibCacheLibrarySources)

# Make available this library for install and export
include(QuickCppLibMakeInstall)
include(QuickCppLibMakeExport)
