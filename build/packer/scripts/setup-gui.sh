#!/bin/bash

# install XFCE, virtualbox guest additions, gedit code editor
apt-get install -y xubuntu-core virtualbox-guest-x11 gedit

cat > /etc/lightdm/lightdm.conf.d/autologin.conf <<-END
[SeatDefaults]
autologin-user=vagrant
END
