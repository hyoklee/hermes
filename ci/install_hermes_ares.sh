#!/bin/bash
# rm -rf ~/.spack
# rm -rf /tmp/${USER}
# rm -rf ~/spack/
# git clone https://github.com/hyoklee/spack
# git clone https://github.com/mochi-hpc/mochi-spack-packages
# git clone https://github.com/hyoklee/hermes
. ./spack/share/spack/setup-env.sh
module load cmake
module swap openmpi3 mpich/3.2.1
spack external find autoconf
spack external find automake
spack external find berkeley-db
spack external find cmake
spack external find diffutils
spack external find libtool
spack external find m4
spack external find mpich
spack external find ncurses
spack external find xz
spack install mpi ^mpich
spack load mpi
# spack repo add mochi-spack-packages
spack repo add hermes/ci/hermes
# spack spec ior+hermes+hdf5 ^abseil-cpp@20200225.2 ^mpi ^mpich ^hermes@master
spack spec ior+hermes+hdf5  ^mpi ^mpich ^hermes@master
# spack install ior+hermes+hdf5 ^abseil-cpp@20200225.2 ^mpi ^mpich ^hermes@master
spack install ior+hermes+hdf5 ^mpi ^mpich ^hermes@master
spack load ior+hermes+hdf5
ior
ior -a MPIIO
ior -a HDF5
spack view --verbose symlink install ior+hermes+hdf5
./hermes/ci/test_ior_mpi.sh
./hermes/ci/test_ior_hdf5.sh

