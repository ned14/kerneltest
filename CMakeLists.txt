cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
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
find_quickcpplib_library(quickcpplib 1.0 REQUIRED)
find_quickcpplib_library(outcome 2.0 REQUIRED)

# Make an interface only library so dependent CMakeLists can bring in this header-only library
include(QuickCppLibMakeHeaderOnlyLibrary)

# Create a custom doxygen generation target
include(QuickCppLibMakeDoxygen)
# Set the standard definitions for these libraries and bring in the all_* helper functions
include(QuickCppLibApplyDefaultDefinitions)
# Set the C++ features this library requires
all_compile_features(PUBLIC
  cxx_std_17  ## needed for aggregate tuple
)
if(ENABLE_CXX_CONCEPTS)
  all_compile_options(PUBLIC -fconcepts)
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
