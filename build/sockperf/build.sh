#!/bin/bash

ver=3.3

git clone https://github.com/Mellanox/sockperf.git --branch ${ver}
cd sockperf
git fetch https://github.com/haggaie/sockperf.git build-dpkg-source && git merge FETCH_HEAD
build/build-rpm.sh
build/build-rpm-to-deb.sh /root/rpmbuild/SRPMS/sockperf-${ver}*.src.rpm
cp *.deb *.dsc *.tar.gz *.debian.tar.xz *.changes *.buildinfo *.ddeb /export
