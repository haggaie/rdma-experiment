#!/bin/bash
rxe_cfg start
for netdev in $(ls /sys/class/net | grep -v '^lo$') ; do
    rxe_cfg add $netdev
done
