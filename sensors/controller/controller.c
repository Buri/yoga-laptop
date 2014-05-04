/*
 * File:   controller.c
 * Author: buri
 *
 * Created on 3. květen 2014, 14:55
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <dbus/dbus.h>
#include "../libs/math.h"
#include "config.h"

/* Global variables */
Config config;

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


	/* Load config */
	FILE* fp_backlight_max = fopen("/sys/class/backlight/intel_backlight/max_brightness", "r");
	if (fp_backlight_max) {
		if (!fscanf(fp_backlight_max, "%d", &config.light_backlight_max)) {
			fprintf(stderr, "Error reading max brightness, using defaults...\n");
		}
		fclose(fp_backlight_max);
	} else {
		fprintf(stderr, "Error reading max brightness, using defaults...\n");
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
	dbus_bus_add_match(conn, "type='signal',interface='org.pfps.sensors'", &err); // see signals from the given interface
	dbus_connection_flush(conn);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Match Error (%s)\n", err.message);
		exit(1);
	}

	DBusMessage* msg;
	DBusMessageIter args;
	static int32_t sigvalue;
	// loop listening for signals being emmitted
	while (true) {

		// blocking read of the next available message
		//printf("Waiting...\n");
		dbus_connection_read_write(conn, -1);
		//printf("Reading message\n");
		msg = dbus_connection_pop_message(conn);

		// loop again if we haven't read a message
		if (NULL == msg) {
			//sleep(1);
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
				dbus_message_iter_get_basic(&args, &sigvalue);
				printf("Got Signal with value %d\n", sigvalue);

				// Process the ambient light
				int backlight = limit_interval(1, config.light_backlight_max,
						sigvalue * config.light_backlight_max / config.light_ambient_max);
				FILE* fp = fopen("/sys/class/backlight/intel_backlight/brightness", "w");
				if (fp) {
					if (fprintf(fp, "%d", backlight) < 0) {
						fprintf(stderr, "Failed to change brightness\n");
						exit(EPERM);
					}
					fclose(fp);
				}
			}
		} else if (dbus_message_is_method_call(msg, "org.pfps.controller", "DisableRotation")) {
			config.light_enabled = false;
		} else if (dbus_message_is_method_call(msg, "org.pfps.controller", "EnableRotation")) {
			config.light_enabled = true;
		} else {
			printf("Unknown message: %s\n", dbus_message_get_interface(msg));
		}

		// free the message
		dbus_message_unref(msg);
	}

	//dbus_connection_close(conn);
	return (EXIT_SUCCESS);
}

