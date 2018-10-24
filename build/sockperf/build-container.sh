#!/bin/bash

docker build --tag build-sockperf .
docker run -it --rm -v ${PWD}:/export build-sockperf /export/build.sh

