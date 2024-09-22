#!/bin/bash

set -x
set -e
set -o pipefail

mkdir -p "${HOME}/install"

mkdir build
pushd build

DEPENDENCY_PREFIX="${HOME}/${LOCAL}"
INSTALL_PREFIX="${HOME}/install"

export CXXFLAGS="${CXXFLAGS} -std=c++17 -Wall"
cmake                                                      \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}               \
    -DCMAKE_PREFIX_PATH=${DEPENDENCY_PREFIX}               \
    -DCMAKE_BUILD_RPATH=${DEPENDENCY_PREFIX}/lib           \
    -DCMAKE_INSTALL_RPATH=${DEPENDENCY_PREFIX}/lib         \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE}                       \
    -DCMAKE_CXX_COMPILER=`which mpicxx`                    \
    -DCMAKE_C_COMPILER=`which mpicc`                       \
    -DBUILD_SHARED_LIBS=ON                                 \
    -DHERMES_ENABLE_MPIIO_ADAPTER=ON \
    -DHERMES_MPICH=ON \
    -DHERMES_ENABLE_STDIO_ADAPTER=ON \
    -DHERMES_ENABLE_POSIX_ADAPTER=ON \
    -DHERMES_ENABLE_COVERAGE=ON                            \
    -DSITE=ubu \
    -DBUILDNAME="mpich" \
    ..
sudo ctest -D Experimental
popd
