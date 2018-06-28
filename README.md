# RDMA experiment

See the [experiment instructions](http://htmlpreview.github.io/?https://github.com/haggaie/rdma-experiment/blob/master/doc/RDMA-Experiment.html) (Hebrew).

## VM installation instructions

You will need the following tools:

* [VirtualBox](https://www.virtualbox.org/wiki/Downloads) - a desktop virtualization system.
* [Vagrant](https://www.vagrantup.com/downloads.html) - a scripted tool for managing virtual machines.

To create the virtual machines use:

    vagrant up

After the first time, a new kernel is installed and the VMs have to be restarted and reprovisioned using:

    vagrant halt && vagrant up --provision

This will create two virtual machines named *client* and *server*. To access
the machines, use:

    vagrant ssh client

or

    vagrant ssh server

The two machines are connected with a virtual Ethernet network on the subnet
192.168.0.0/24, with a SoftRoCE installed on the appropriate virtual NICs.
