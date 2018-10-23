#!/bin/bash

packer build -var 'version=0.1.0' rdma-experiment.json
