#!/bin/bash
sudo apt update
sudo apt upgrade -y
sudo apt-get install -y gfortran
sudo apt-get install -y autoconf
sudo apt-get install -y automake
sudo apt-get install -y libboost-dev
# sudo apt-get install -y catch2
sudo apt-get install -y cmake
sudo apt-get install -y diffutils
sudo apt-get install -y libdb-dev
sudo apt-get install -y libedit-dev
# sudo apt-get install -y json-c
sudo apt-get install -y libiconv-dev
sudo apt-get install -y libncurses6
sudo apt-get install -y libsigsegv-dev
sudo apt-get install -y libtool
sudo apt-get install -y libtool-bin
sudo apt-get install -y libxml2-dev
sudo apt-get install -y mpich
sudo apt-get install -y lcov
sudo apt-get install -y libsdl2-dev
sudo apt-get install -y openssl
sudo apt-get install -y xz-utils
sudo apt-get install -y zlib1g-dev
sudo apt-get install -y libprotobuf-dev
cd work
mkdir tmp
export TMP=~/work/tmp
export SPACK_LOCAL_DIR=~/work/tmp
git clone https://github.com/hyoklee/spack
# git clone https://github.com/mochi-hpc/mochi-spack-packages
git clone https://github.com/hyoklee/hermes
. ./spack/share/spack/setup-env.sh
spack external find autoconf
spack external find automake
spack external find berkeley-db
spack external find boost
spack external find catch2
spack external find cmake
spack external find diffutils
# spack external find json-c
spack external find libedit
spack external find libtool
spack external find libiconv
spack external find libfabric
spack external find libsigsegv
spack external find libxml2
spack external find m4
spack external find mpich
spack external find ncurses
spack external find openssl
spack external find perl
spack external find pkgconf
spack external find zlib
spack external find xz

spack install mpi ^mpich
spack compiler find
spack install hdf5 ^mpich
spack load hdf5
spack install hermes
# spack install ior+hermes+hdf5
# spack load ior+hermes+hdf5
# ior

