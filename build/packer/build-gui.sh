#!/bin/bash

packer build -var-file token.json rdma-experiment-gui.json
