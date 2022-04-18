#!/bin/bash
rm -rf ~/.spack
rm -rf /tmp/${USER}
git clone https://github.com/hyoklee/spack
# git clone https://github.com/mochi-hpc/mochi-spack-packages
git clone https://github.com/hyoklee/hermes
. ./spack/share/spack/setup-env.sh
spack compiler find
spack repo add mochi-spack-packages
spack repo add hermes/ci/hermes
spack install ior+hermes+hdf5
spack load ior+hermes+hdf5
ior
ior -a MPIIO
ior -a HDF5
spack view --verbose symlink install ior+hermes+hdf5
./hermes/ci/test_ior_mpiio.sh
./hermes/ci/test_ior_hdf5.sh

