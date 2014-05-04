/*
 * File:   config.h
 * Author: buri
 *
 * Created on 2. květen 2014, 12:59
 */

#ifndef CONFIG_H
#define	CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif

#define DEBUG_ALL 4
#define DEBUG_TRACE 3
#define DEBUG_INFO 2
#define DEBUG_ERROR 1
#define DEBUG_CRITICAL 0

	struct Config_s {
		/* Common config */
		// Output level
		int debug_level;
		// Time to waite between polls
		unsigned int poll_timeout;
		// Times to run the script
		int iterations;
		// Device name
		char* device_name;

		/* Orientation config*/
		char *or_touchScreenName;

		/* Light config */
		// Top value used for scaling
		unsigned int light_ambient_max;
		// Max backlight value
		unsigned int light_backlight_max;
	} Config_default = {
		DEBUG_ERROR,
		1000000,	// 1 second
		-1,			// infinite iterations
		"",

		// Orientation
		"ELAN Touchscreen",

		// Light
		1400,
		937
	};

	typedef struct Config_s Config;


#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */

