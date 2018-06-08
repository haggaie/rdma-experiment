#!/bin/bash

ver=3.3

git clone https://github.com/Mellanox/sockperf.git --branch ${ver}
cd sockperf
build/build-rpm.sh
build/build-rpm-to-deb.sh /root/rpmbuild/SRPMS/sockperf-${ver}*.src.rpm
cp *.deb /export
