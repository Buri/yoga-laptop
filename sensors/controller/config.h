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

	/* Config structure with default values */
	struct Config_s {
		/* Common config */
		// Output level
		int debug_level;

		/* Orientation config*/
		char* touch_screen_name;

		/* Light config */
		// Top value used for scaling
		unsigned int light_ambient_max;
		// Max backlight value
		unsigned int light_backlight_max;
		// Enable automatic light controll
		bool light_enabled;

		// Enable rotation of screen
	} Config_default = {
		0,
		// Orientation
		"ELAN Touchscreen",

		// Light
		1000,
		937,
		true
	};

	typedef struct Config_s Config;

#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */

