#!/bin/bash

### Install Soft RoCE

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

# librxe is not inbox in Ubuntu yet
sudo add-apt-repository ppa:linux-rdma/rdma-core-daily
# rdma-experiment ppa - sockperf
sudo add-apt-repository ppa:haggai-eran/rdma-experiment

sudo apt-get update

# install ifupdown for Vagrant
sudo apt-get install -y ifupdown

# The RDMA stack and SoftRoCE require the generic kernel (not virtual kernel)
sudo apt-get install -y linux-generic
sudo apt-get autoremove -y --purge linux-virtual

# install latest kmod since previous version conflicts with rdma-core
sudo sudo apt-get install -y kmod

# librxe is not inbox in Ubuntu yet
sudo sudo apt-get install -y rdma-core

# Generic RDMA utilities
sudo apt-get install -y ibverbs-utils perftest

# enable rxe
#sudo rxe_cfg add enp0s8
mkdir -p /var/lib/rxe
echo enp0s8 > /var/lib/rxe/rxe
sudo rxe_cfg start

# sockperf - need to be pre-built or downloaded
sudo apt-get install -y sockperf
