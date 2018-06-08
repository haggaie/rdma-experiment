# based on https://github.com/joernhees/docker-dpkg-build/blob/master/Dockerfile

FROM ubuntu:18.04
MAINTAINER Haggai Eran <haggai.eran@gmail.com>

RUN apt-get update && apt-get install -y \
  build-essential \
  devscripts \
  wget

RUN apt-get install -y rpm debhelper cpio
#  && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /tmp/build
WORKDIR /tmp/build
VOLUME /export

COPY build.sh /
ENTRYPOINT ["/build.sh"]
