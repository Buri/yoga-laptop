/*
 * File:   config.h
 * Author: buri
 *
 * Created on 4. květen 2014, 9:09
 */

#ifndef CONFIG_H
#define	CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glib-2.0/glib.h>
#include <getopt.h>

	typedef enum {
		CRITICAL, ERROR, INFO, ALL
	} DebugLevel;

	/* Config structure with default values */
	struct Config_s {
		/* Common config */
		// Output level
		DebugLevel debug_level;

		/* Orientation config*/
		char* touch_screen_name;

		/* Light config */
		// Top value used for scaling
		unsigned int light_ambient_max;
		// Max backlight value
		unsigned int light_backlight_max;
		// Enable automatic light controll
		bool light_enabled;

		/* Light events */
		char *light_onEnable, *light_onDisable;

		// Enable rotation of screen

		/* Others */
		int help_flag, version_flag;
	};
	typedef struct Config_s Config;

	bool loadConfig(char* file, Config* config);
	bool parseArguments(char** argv, int argc, Config* config);
#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */

