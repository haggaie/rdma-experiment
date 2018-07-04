# RDMA experiment

See the [experiment instructions](http://htmlpreview.github.io/?https://github.com/haggaie/rdma-experiment/blob/master/doc/RDMA-Experiment.html) (Hebrew).

## Hebrew editing instructions

To edit hebrew Markdown on Windows, [Notepad++](https://notepad-plus-plus.org/) can be used, 
along with the [markdown-plus-plus](https://github.com/Edditoria/markdown-plus-plus) syntax highlighting plugin.
To change text direction when editing use the <kbd>Ctrl</kbd>+<kbd>Alt</kbd>+<kbd>L</kbd> and 
<kbd>Ctrl</kbd>+<kbd>Alt</kbd>+<kbd>R</kbd> shortcut keys.

On Linux, [gedit](https://wiki.gnome.org/Apps/Gedit) can be used.

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
