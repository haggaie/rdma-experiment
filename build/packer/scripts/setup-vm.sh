#!/bin/bash

### Install Soft RoCE

apt-get update

# install ifupdown for Vagrant
apt-get install -y ifupdown

# The RDMA stack and SoftRoCE require the generic kernel (not virtual kernel)
apt-get install -y linux-generic
apt-get autoremove -y --purge linux-virtual

# rdma-core and utilities
apt-get install -y rdma-core ibverbs-utils perftest rdmacm-utils

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
