# systemd

The `systemd` directory includes `.service` files that can be used as templates for hooking up systemd services that start Jool on boot.

## Installing SIIT Jool and NAT64 Jool as systemd services

First, install Jool's binaries normally. ([Kernel modules](https://jool.mx/en/install-mod.html) and [userspace applications](https://jool.mx/en/install-usr.html).)

Then, copy the service unit configuration(s) that you need into the service files directory of your distro (Usually `/etc/systemd/system`).

`jool_siit.service` is the SIIT service and `jool.service` is the NAT64 service. You can install both if you want.

	cp *.service /etc/systemd/system

Also copy the start and stop scripts into `/usr/local/bin`.

	cp *.sh /usr/local/bin

By default (ie. unless you customize the `.service` files), the services will fail to start until you configure them. (`man systemd.service` should serve as a comprehensive source of information on how to customize these files.)

## Configuring the services

`jool_siit.conf` is a sample configuration for the SIIT service, and `jool.conf` is a sample configuration for the NAT64 service. The units expect to find their respective file in the `/etc/jool` directory.

	mkdir /etc/jool
	cp *.conf /etc/jool

Tweak the contents of these files as you see fit. They follow the [atomic configuration](https://jool.mx/en/config-atomic.html) format.

## Starting and enabling the services

Reload the systemd daemon (Note: I actually found this step to be unnecesary the first time, but YMMV):

	systemctl daemon-reload

Start the service (ie. Try it out now):

	systemctl start jool.service

	systemctl start jool_siit.service

"Enable" the service (ie. set it up to start on boot):

	systemctl enable jool.service

	systemctl enable jool_siit.service

