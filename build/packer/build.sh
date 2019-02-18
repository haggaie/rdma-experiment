#!/bin/bash

packer build -var-file token.json rdma-experiment.json
