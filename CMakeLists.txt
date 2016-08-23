cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
# If necessary bring in the Boost lite cmake tooling
list(FIND CMAKE_MODULE_PATH "boost-lite" boost_lite_idx)
if(${boost_lite_idx} EQUAL -1)
  if(EXISTS "${CMAKE_SOURCE_DIR}/../boost-lite/cmake")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../boost-lite/cmake")
  elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/boost/kerneltest/boost-lite/cmake")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/include/boost/kerneltest/boost-lite/cmake")
  else()
    message(FATAL_ERROR "FATAL: A copy of boost-lite cannot be found. Try running 'git submodule update --init --recursive'")
  endif()
endif()
include(BoostLiteRequireOutOfSourceBuild)
include(BoostLiteUtils)
include(BoostLitePolicies)

# Parse the version we tell cmake directly from the version header file
ParseProjectVersionFromHpp("${CMAKE_CURRENT_SOURCE_DIR}/include/boost/kerneltest/version.hpp" VERSIONSTRING)
# Sets the usual PROJECT_NAME etc
project(kerneltest VERSION ${VERSIONSTRING} LANGUAGES CXX)
# Also set a *cmake* namespace for this project
set(PROJECT_NAMESPACE boost--)
# This file should be updated with the last git SHA next commit
UpdateRevisionHppFromGit("${CMAKE_CURRENT_SOURCE_DIR}/include/boost/kerneltest/revision.hpp")
# Setup this cmake environment for this project
include(BoostLiteSetupProject)
# Find my library dependencies
find_boostish_library(boost-lite "include/boost/kerneltest" 1.0 REQUIRED)
find_boostish_library(outcome    "include/boost/kerneltest" 1.0 REQUIRED)

# Make an interface only library so dependent CMakeLists can bring in this header-only library
include(BoostLiteMakeHeaderOnlyLibrary)

# Create a custom doxygen generation target
include(BoostLiteMakeDoxygen)
# Set the standard definitions for these libraries and bring in the all_* helper functions
include(BoostLiteApplyDefaultDefinitions)
# Set the C++ features this library requires
all_compile_features(PUBLIC
  # cxx_exceptions                        ## Annoyingly not supported by cmake 3.6
  cxx_alias_templates
  cxx_variadic_templates
  cxx_noexcept
  cxx_constexpr
  cxx_thread_local
  cxx_lambda_init_captures
  cxx_attributes
  cxx_generic_lambdas
)
if(NOT MSVC OR CMAKE_VERSION VERSION_GREATER 3.59)
  all_compile_features(PUBLIC
    cxx_variable_templates
  )
endif()
# Check if our std::tuple can do aggregate initialisation
include(CheckCXXSourceCompiles)
set(OLD_CMAKE_CXX_STANDARD ${CMAKE_CXX_STANDARD})
set(OLD_CMAKE_CXX_STANDARD_REQUIRED ${CMAKE_CXX_STANDARD_REQUIRED})
set(CMAKE_CXX_STANDARD 17)
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
set(CMAKE_CXX_STANDARD ${OLD_CMAKE_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ${OLD_CMAKE_CXX_STANDARD_REQUIRED})
if(CXX_HAS_AGGREGATE_INITIALIZABLE_TUPLE)
  all_compile_definitions(PUBLIC BOOST_KERNELTEST_HAVE_AGGREGATE_TUPLE=1)
endif()
# Set the library dependencies this library has
target_link_libraries(kerneltest_hl INTERFACE boost-lite_hl)

# For all possible configurations of this library, add each test
include(BoostLiteMakeStandardTests)

# Make available this library for install and export
include(BoostLiteMakeInstall)
include(BoostLiteMakeExport)
