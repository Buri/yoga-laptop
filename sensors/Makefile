SCRIPTS = setup unstick status

SOURCES = iio_utils.h generic_buffer.c orientation.c light.c

EXECS = orientation generic_buffer light

LOGNAME := $(shell /usr/bin/logname)
GROUP := $(shell /usr/bin/id -gn `/usr/bin/logname`)

all: generic_buffer orientation light

generic_buffer: utils/generic_buffer.c
	cc utils/generic_buffer.c -o dist/generic_buffer

orientation: legacy/orientation.c
	@echo NOTE: In most cases you need to install drivers in the drivers directory for orientation to work
	cc legacy/orientation.c -o dist/orientation

hinge: hinge.c iio_utils.h
	cc hinge.c -o hinge

light: legacy/light.c
	cc -g `pkg-config --cflags glib-2.0 dbus-1 dbus-glib-1` -Wall -Wextra -o dist/light legacy/light.c `pkg-config --libs glib-2.0 dbus-1 dbus-glib-1`

controller: controller/controller.c
	cc -g `pkg-config --cflags glib-2.0 dbus-1 dbus-glib-1` -Wall -Wextra -o dist/controller controller/controller.c `pkg-config --libs glib-2.0 dbus-1 dbus-glib-1`

install:	orientation
	@echo Making the control files writable by you and putting the command in /usr/bin
	@echo NOTE: In most cases you also need to install drivers from the drivers directory
	chgrp $(GROUP) /sys/bus/iio/devices/iio:device*/scan_elements/in_*_en /sys/bus/iio/devices/iio:device*/buffer/* /sys/bus/iio/devices/iio:device*/trigger/current_trigger
	chmod g+w /sys/bus/iio/devices/iio:device*/scan_elements/in_*_en /sys/bus/iio/devices/iio:device*/buffer/* /sys/bus/iio/devices/iio:device*/trigger/current_trigger
	chgrp $(GROUP) /dev/iio:device*
	chmod g+rw /dev/iio:device*
	install --owner=root --group=root --mode=755 orientation /usr/bin
	install --owner=root --group=root --mode=644 conf/org.pfps.sensors.conf /etc/dbus-1/system.d
	install --owner=root --group=root --mode=644 conf/org.pfps.controller.conf /etc/dbus-1/system.d
	@echo NOTE: In most cases you also need to install drivers from the drivers directory

