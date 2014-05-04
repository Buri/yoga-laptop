/*
 * File:   controller.h
 * Author: buri
 *
 * Created on 4. květen 2014, 9:49
 */

#ifndef CONTROLLER_H
#define	CONTROLLER_H

#ifdef	__cplusplus
extern "C" {
#endif

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <dbus-1.0/dbus/dbus.h>
#include "../libs/math.h"
#include "config.h"


	/* Default config values */
	const Config Config_default = {
		ERROR,
		// Orientation
		"ELAN Touchscreen",

		// Light config
		1000,
		937,
		true,
		true,
		// Light Events
		// onEnable
		"/usr/bin/notify-send -i /home/buri/.utils/yoga/rotateon.png \"Autorotate enabled\"",
		// onDisable
		"/usr/bin/notify-send -i /home/buri/.utils/yoga/rotateoff.png \"Autorotate disabled\"",

		// Flags
		0, 0
	};

#ifdef	__cplusplus
}
#endif

#endif	/* CONTROLLER_H */

