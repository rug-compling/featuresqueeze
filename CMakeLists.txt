cmake_minimum_required(VERSION 2.6)
project(featuresqueeze)

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

list(APPEND CMAKE_MODULE_PATH "${featuresqueeze_SOURCE_DIR}/cmake")

include_directories(${featuresqueeze_SOURCE_DIR}/libfsqueeze)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wno-long-long")

# OpenMP currently deadlocks on OS X.
if (NOT APPLE)
  find_package(OpenMP)
  if(OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  endif()
endif()

find_package(Eigen REQUIRED)
if(EIGEN_FOUND)
  include_directories(${EIGEN_INCLUDE_DIR})
endif()

set (LIBFSQUEEZE_SOURCES
  libfsqueeze/src/DataSet/DataSet.cpp
  libfsqueeze/src/corr_selection/corr_selection.cpp
  libfsqueeze/src/feature_selection/feature_selection.cpp
  libfsqueeze/src/maxent/maxent.cpp
  libfsqueeze/src/lbfgs/lbfgs.c
)

set (FSQUEEZE_SOURCES
  util/fsqueeze/fsqueeze.cpp
  util/fsqueeze/ProgramOptions.cpp
)

add_library(fsqueeze SHARED
  ${LIBFSQUEEZE_SOURCES}
)

add_executable(squeeze
  ${FSQUEEZE_SOURCES}
)

target_link_libraries(
	squeeze fsqueeze
)
