#!/bin/bash

### Install Soft RoCE

# # for building rdma-core
# 
# apt-get update && apt-get install -y \
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
add-apt-repository ppa:linux-rdma/rdma-core-daily
# rdma-experiment ppa - sockperf
add-apt-repository ppa:haggai-eran/rdma-experiment

apt-get update

# install ifupdown for Vagrant
apt-get install -y ifupdown

# The RDMA stack and SoftRoCE require the generic kernel (not virtual kernel)
apt-get install -y linux-generic
apt-get autoremove -y --purge linux-virtual

# install latest kmod since previous version conflicts with rdma-core
apt-get install -y kmod

# librxe is not inbox in Ubuntu yet
apt-get install -y rdma-core

# Generic RDMA utilities
apt-get install -y ibverbs-utils perftest rdmacm-utils

# enable rxe
echo rdma_rxe > /etc/modules-load.d/rxe.conf
mv /tmp/rxe_all.sh /usr/bin/
chmod +x /usr/bin/rxe_all.sh
mv /tmp/rxe.service /etc/systemd/system/
chown root:root /usr/bin/rxe_all /etc/systemd/system/rxe.service
systemctl enable rxe.service
echo rdma_rxe > /etc/modules-load.d/rxe.conf

# sockperf - need to be pre-built or downloaded
apt-get install -y sockperf

# RDMA coding exercise dependencies
apt-get install -y build-essential cmake librdmacm-dev
