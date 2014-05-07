/*
 * File:   controller.c
 * Author: buri
 *
 * Created on 3. květen 2014, 14:55
 */

#include "controller.h"

/* Global variables */
Config config;
static char* configFile = "conf/controller.conf";

/*
 *
 */
int main(int argc, char** argv) {
	/* Configuration variables */
	config = Config_default;

	/* DBUS variables */
	static DBusError err;
	static DBusConnection* conn;
	static int ret;
	DBusMessage* msg;
	DBusMessageIter args;

	/* Arguments definition */
	static char* version = "controller version 0.1.00\n";
	static char* help;
	asprintf(&help, "controller monitors the Yoga accelerometer and light sensor and rotates the screen and touchscreen to match and dims screen brightness to match ambient light\n\
\n\
Options:\n\
	--help		Print this help message and exit\n\
	--version		Print version information and exit\n\
	--debug=level		Print out debugging information (0 through 4) [%d]\n\
	--touchscreen=ts_name	TouchScreen name [%s]\n\
|- More options to come, use config file for now\n\
\n\
controller responds to method calls via DBUS on interface org.pfps.controller\n\
Supported methods are:\n\
	AutoBrightnessDisable\n\
	AutoBrightnessEnable\n\
	ConfigReload\n\
Supported signals (from sensors only):\n\
	als <int ambient_light_value>\n\
",
			config.debug_level,
			config.touch_screen_name);

	/* Sensor values */
	static int32_t brightness, accel_x, accel_y, accel_z,
			accel_x_abs, accel_y_abs, accel_z_abs;

	/* Device variables */
	static Orientation orientation, orientation_last = Orientation_OutOfRange;
	static State state, state_last = State_OutOfRange;

	/* Other variables */
	
	/* Load config */
	if (config.light_autodetect) {
		FILE* fp_backlight_max = fopen("/sys/class/backlight/intel_backlight/max_brightness", "r");
		if (fp_backlight_max) {
			if (!fscanf(fp_backlight_max, "%d", &config.light_backlight_max)) {
				fprintf(stderr, "Error reading max brightness, using defaults...\n");
			}
			fclose(fp_backlight_max);
		} else {
			fprintf(stderr, "Error reading max brightness, using defaults...\n");
		}
	}

	/* Parse arguments */
	if (!parseArguments(argv, argc, &config)) {
		fprintf(stderr, "Invalid argument\n");
		return -EINVAL;
	}

	if (!loadConfig(configFile, &config)) {
		fprintf(stderr, "Invalid config file!\n");
		return -ENOKEY;
	}

	if (config.version_flag) {
		printf("%s", version);
		return (EXIT_SUCCESS);
	}
	if (config.help_flag) {
		printf("%s", help);
		return (EXIT_SUCCESS);
	}

	/* Run main */
	// initialise the errors
	dbus_error_init(&err);
	// connect to the bus
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Connection Error (%s)\n", err.message);
		dbus_error_free(&err);
	}
	if (NULL == conn) {
		exit(1);
	}
	// request a name on the bus
	ret = dbus_bus_request_name(conn, "org.pfps.controller", DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Name Error (%s)\n", err.message);
		dbus_error_free(&err);
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
		exit(1);
	}

	// add a rule for which messages we want to see
	// Accept signals only from sensors
	dbus_bus_add_match(conn, "type='signal',interface='org.pfps.sensors'", &err); // see signals from the given interface
	// Accept method calls from any source
	dbus_bus_add_match(conn, "type='method_call'", &err);
	dbus_connection_flush(conn);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Match Error (%s)\n", err.message);
		exit(1);
	}

	// loop listening for signals being emmitted
	while (true) {

		// blocking read of the next available message
		dbus_connection_read_write(conn, -1);
		msg = dbus_connection_pop_message(conn);

		// loop again if we haven't read a message
		if (NULL == msg) {
			printf("Message: NULL\n");
			continue;
		}
		//printf("New message!\n");

		// check if the message is a signal from the correct interface and with the correct name
		if (dbus_message_is_signal(msg, "org.pfps.sensors", "als")) {
			// read the parameters
			if (!dbus_message_iter_init(msg, &args)) {
				fprintf(stderr, "Message has no arguments!\n");
			} else if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
				fprintf(stderr, "Argument is not int!\n");
			} else {
				dbus_message_iter_get_basic(&args, &brightness);
				if (config.debug_level >= INFO) printf("Read ambient light value = %d\n", brightness);

				// If automatic light controll is disabled skip to next loop
				if (!config.light_enabled) {
					continue;
				}

				// Process the ambient light
				int backlight = limit_interval(1, config.light_backlight_max,
						brightness * config.light_backlight_max / config.light_ambient_max);
				FILE* fp = fopen("/sys/class/backlight/intel_backlight/brightness", "w");
				if (fp) {
					if (fprintf(fp, "%d", backlight) < 0) {
						fprintf(stderr, "Failed to change brightness\n");
						exit(EPERM);
					}
					fclose(fp);
				}
			}
		} else if (dbus_message_is_signal(msg, "org.pfps.sensors", "accel_3d")) {
			// read the parameters
			if (!dbus_message_iter_init(msg, &args)) {
				fprintf(stderr, "Message has no arguments!\n");
			} else {
				if (DBUS_TYPE_INT32 == dbus_message_iter_get_arg_type(&args)) {
					dbus_message_iter_get_basic(&args, &accel_x);
					accel_x_abs = abs(accel_x);
				}
				if (DBUS_TYPE_INT32 == dbus_message_iter_get_arg_type(&args)) {
					dbus_message_iter_get_basic(&args, &accel_y);
					accel_y_abs = abs(accel_y);
				}
				if (DBUS_TYPE_INT32 == dbus_message_iter_get_arg_type(&args)) {
					dbus_message_iter_get_basic(&args, &accel_z);
					accel_z_abs = abs(accel_z);
				}
				if (config.debug_level >= INFO) printf("Read acclereration = (%d, %d, %d)\n", accel_x, accel_y, accel_z);

				// If automatic light controll is disabled skip to next loop
				if (!config.rotation_enabled) {
					continue;
				}

				// Process rotation information
				/* Determine orientation */
				accel_x_abs = abs(accel_x);
				accel_y_abs = abs(accel_y);
				accel_z_abs = abs(accel_z);

				if (accel_z_abs > 4 * accel_x_abs && accel_z_abs > 4 * accel_y_abs) {
					orientation = Orientation_Normal;

				} else if (3 * accel_y_abs > 2 * accel_x_abs) {
					orientation = accel_y > 0 ? Orientation_Normal : Orientation_Inverted;
				} else {
					orientation = accel_x > 0 ? Orientation_Left : Orientation_Right;
				}

				if (state == state_last && orientation == orientation_last) {
					continue;
				}

				state_last = state;
				orientation_last = orientation;

				char* param;
				switch (state) {
					case State_Normal:
						param = "normal";
						break;
					case State_Stand:
						param = "stand";
						break;
					case State_Tent:
						param = "tent";
						break;
					case State_Tablet:
						switch (orientation) {
							case Orientation_Normal:
								param = "tablet-normal";
								break;
							case Orientation_Inverted:
								param = "tablet-inverted";
								break;
							case Orientation_Left:
								param = "tablet-left";
								break;
							case Orientation_Right:
								param = "tablet-right";
								break;
							default:
								break;
						}
						break;
					default:
						break;
				}
				if (config.debug_level > 1) printf("Orientation %d, x:%5d, y:%5d, z:%5d\n",
						orientation, accel_x, accel_y, accel_z);
				static char* exec;
				asprintf(&exec, "%s \"%s\" \"%s\"", exec, param, config.touch_screen_name);
				if (fork() == 0) {
					system(exec);
				}else{
					wait(NULL);
				}
			}
		} else if (dbus_message_is_method_call(msg, "org.pfps.controller", "AutoBrightnessToggle")) {
			toggleLight(&config, !config.light_enabled);
		} else if (dbus_message_is_method_call(msg, "org.pfps.controller", "AutoBrightnessDisable")) {
			toggleLight(&config, false);
		} else if (dbus_message_is_method_call(msg, "org.pfps.controller", "AutoBrightnessEnable")) {
			toggleLight(&config, true);
		} else if (dbus_message_is_method_call(msg, "org.pfps.controller", "ConfigReload")) {
			if (loadConfig(configFile, &config)) {
				if (config.debug_level >= INFO) printf("Config reloaded\n");
			} else {
				fprintf(stderr, "Error parsing config\n");
			}
		} else {
			if (config.debug_level >= ERROR) printf("Unknown message: %s\n", dbus_message_get_interface(msg));
		}

		// free the message
		dbus_message_unref(msg);
	}

	//dbus_connection_close(conn);
	return (EXIT_SUCCESS);
}

void toggleLight(Config* config, bool state) {
	config->light_enabled = state;
	if (config->debug_level >= INFO) printf("Rotation %sabled\n", state == true ? "en" : "dis");
	if (0 == fork()) {
		system(state == true ? config->light_onEnable : config->light_onDisable);
	} else {
		wait(NULL);
	}

}