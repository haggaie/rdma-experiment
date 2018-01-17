#!/bin/bash

# # for building rdma-core
# 
# sudo apt-get update && sudo apt-get install -y \
#     build-essential \
#     cmake \
#     gcc \
#     git \
#     libudev-dev \
#     libnl-3-dev \
#     libnl-route-3-dev \
#     ninja-build \
#     pkg-config \
#     valgrind

# install kmod from ppa to work around issue https://bugs.launchpad.net/ubuntu/+source/kmod/+bug/1693503
sudo add-apt-repository ppa:bdrung/ppa
# librxe is not inbox in Ubuntu yet
sudo add-apt-repository ppa:linux-rdma/rdma-core-daily

sudo apt-get update

# The RDMA stack and SoftRoCE require the generic kernel (not virtual kernel)
sudo apt-get install -y linux-generic
sudo apt-get autoremove --purge linux-virtual

# install kmod from ppa to work around issue https://bugs.launchpad.net/ubuntu/+source/kmod/+bug/1693503
sudo sudo apt-get install -y kmod

# librxe is not inbox in Ubuntu yet
sudo sudo apt-get install -y rdma-core

# Generic RDMA utilities
sudo apt-get install -y ibverbs-utils perftest

# enable rxe
sudo rxe_cfg start
sudo rxe_cfg add enp0s3
