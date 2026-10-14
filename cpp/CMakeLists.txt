## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ FishMIP-OSP plug-in model: C++ library build ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
#
# Builds libfishmip_osp_plugin_models.a, the C++17 port of the FishMIP-OSP
# toy model. Atlantis links against this static library and includes its
# public header (cpp/include/FishMIP_OSP_Plugin_Models.hpp) from the single
# adapter translation unit ateconomic/atFishMIP_OSP_PluginAdapter.cpp.
#
# The R and Python binders (src/R_Binder.cpp, src/Python_Binder.cpp) are
# deliberately NOT part of this library: they need Rcpp / pybind11 and are
# only used to expose the model to R/Python, not to Atlantis.
#
# Build (out-of-source, so the committed cpp/build Mac cache is untouched):
#   cd cpp && cmake -S . -B build && cmake --build build

cmake_minimum_required(VERSION 3.16)
project(FishMIP_OSP_Plugin_Models_cpp CXX)

## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Build settings ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Dependencies: netcdf-c (via pkg-config) and threads ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
# netcdf_utils.cpp includes <netcdf.h>; FishMIP_OSP_Plugin_Models.cpp uses
# std::thread / std::atomic, so we also need the platform threads library.
find_package(PkgConfig REQUIRED)
pkg_check_modules(NETCDF REQUIRED netcdf)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Static library used by Atlantis ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
add_library(fishmip_osp_plugin_models STATIC
    src/csv_utils.cpp
    src/netcdf_utils.cpp
    src/optim.cpp
    src/FishMIP_OSP_Plugin_Models.cpp
)

target_include_directories(fishmip_osp_plugin_models
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${NETCDF_INCLUDE_DIRS}
)

target_link_libraries(fishmip_osp_plugin_models
    PUBLIC ${NETCDF_LIBRARIES} Threads::Threads
)
target_link_directories(fishmip_osp_plugin_models PUBLIC ${NETCDF_LIBRARY_DIRS})

target_compile_options(fishmip_osp_plugin_models PRIVATE -Wall -Wextra)

## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Optional smoke test (exercises every function) ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
# Handy for validating the build on a new machine:
#   ./build/smoke_test /path/to/Historical_drivers config/control_parameters.csv
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/smoke_test.cpp)
    add_executable(smoke_test examples/smoke_test.cpp)
    target_link_libraries(smoke_test PRIVATE fishmip_osp_plugin_models)
endif()
