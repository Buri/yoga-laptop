/* Rotate Lenovo Yoga (2 Pro) display and touchscreen to match orientation of screen
 * Copyright (c) 2014 Peter F. Patel-Schneider
 *
 * Modified from industrialio buffer test code.
 * Copyright (c) 2008 Jonathan Cameron
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * Command line parameters
 * orientation -n <iio device name> -c <iterations (<0 is forever)> -d <debug level>
 * iio device name defaults to accel_3d
 * iterations defaults to -1
 * debug level defaults to 0 (useful range is -1 to 4)
 *
 * This program reads the Invensense accelerometer and uses gravity to
 * determine which edge of the Yoga screen is up.  If none of the edges is
 * up very much, this is treated as being flat.   When an edge is up twice
 * in a row, the orientation of the screen and touchscreen is adjusted
 * to match.
 *
 * WARNING:  This is not production quality code.
 *	     This code exec's xrandr and xinput as root.
 */

#define _GNU_SOURCE

#include "../libs/common.h"
#include "../libs/config.h"
#include "../libs/device_info.h"
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dbus-1.0/dbus/dbus.h>

static char *dev_dir_name;
DBusError derr;
DBusConnection* dconn;
dbus_uint32_t serial = 0; // unique number to associate replies with requests

/**
 * process_scan() - print out the values in SI units
 * @data:               pointer to the start of the scan
 * @channels:           information about the channels. Note
 *  size_from_channelarray must have been called first to fill the
 *  location offsets.
 * @num_channels:       number of channels
 **/
int process_scan(SensorData data, Device_info info, Config config) {
	if (info.channels_count != 1 || info.channels[0].bytes != 4) {
		return -1;
	}
	struct iio_channel_info channel = info.channels[0];
	if (channel.is_signed) {
		int32_t val = *(int32_t *) (data.data + channel.location);
		val = val >> channel.shift;
		if (channel.bits_used < 32) val &= ((uint32_t) 1 << channel.bits_used) - 1;
		val = (int32_t) (val << (32 - channel.bits_used)) >> (32 - channel.bits_used);
		if (config.debug_level > 2) printf("Current ambient light: %d\n", val);

		/* Send signal to DBUS */
		DBusMessage* msg;
		DBusMessageIter args;

		// create a signal and check for errors
		msg = dbus_message_new_signal("/org/pfps/sensors", // object name of the signal
				"org.pfps.sensors", // interface name of the signal
				"als"); // name of the signal
		if (NULL == msg) {
			fprintf(stderr, "Message Null\n");
			exit(1);
		}


		// append arguments onto signal
		dbus_message_iter_init_append(msg, &args);
		if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &val)) {
			fprintf(stderr, "Out Of Memory!\n");
			exit(1);
		}
		// create a signal and check for errors


		// send the message and flush the connection
		if (!dbus_connection_send(dconn, msg, &serial)) {
			fprintf(stderr, "Out Of Memory!\n");
			exit(1);
		}
		dbus_connection_flush(dconn);

		// free the message
		//dbus_message_unref(msg);

		printf("Message sent\n");

		/*FILE* fp = fopen("/sys/class/backlight/intel_backlight/brightness", "w");
		if (fp) {
			if (fprintf(fp, "%d", backlight) < 0) {
				fprintf(stderr, "Failed to change brightness\n");
				exit(EPERM);
			}
			fclose(fp);
		}*/
	}
	return 0;
}

void sigint_callback_handler(int signum) {
	if (dev_dir_name) {
		/* Disconnect the trigger - just write a dummy name. */
		write_sysfs_string("trigger/current_trigger", dev_dir_name, "NULL");
		/* Stop the buffer */
		write_sysfs_int("buffer/enable", dev_dir_name, 0);
	}
	exit(signum);
}

int main(int argc, char **argv) {
	/* Configuration variables */
	char *trigger_name = NULL;
	Config config = Config_default;
	static int version_flag = 0, help_flag = 0;

	// Update default settings
	config.device_name = "als";
	FILE* fp_backlight_max = fopen("/sys/class/backlight/intel_backlight/max_brightness", "r");
	if (fp_backlight_max) {
		if (!fscanf(fp_backlight_max, "%d", &config.light_backlight_max)) {
			fprintf(stderr, "Error reading max brightness, using defaults...\n");
		}
		fclose(fp_backlight_max);
	} else {
		fprintf(stderr, "Error reading max brightness, using defaults...\n");
	}

	/* Arguments definition */
	static char* version = "light version 0.3\n";
	static char* help;
	asprintf(&help, "light monitors ambient light sensor and adjusts backlight acordingly\
\n\
Options:\n\
  --help			Print this help message and exit\n\
  --version			Print version information and exit\n\
  --count=<iterations>		If >0 run for only this number of iterations [%d]\n\
  --name=<device>		Industrial IO accelerometer device name [%s]\n\
  --usleep=<microtime>		Polling sleep time in microseconds [%u]\n\
  --debug=<level>		Print out debugging information (-1 through 4) [%d]\n\
  --ambient-max=<integer>	Minimum ambient light required for full backlight [%u]\n\
  --backlight-max=<integer>	Max output defined /sys/class/backlight/intel_backlight/max_brightness [auto/%d]\n",
			config.iterations, (char*) config.device_name, config.poll_timeout, config.debug_level,
			config.light_ambient_max, config.light_backlight_max);

	/* Device info */
	Device_info info;

	/* Other variables */
	int ret = 0, c, i;
	char * dummy;

	static struct option long_options[] = {
		{"version", no_argument, &version_flag, 1},
		{"help", no_argument, &help_flag, 1},
		{"count", required_argument, 0, 'c'},
		{"name", required_argument, 0, 'n'},
		{"usleep", required_argument, 0, 'u'},
		{"debug", required_argument, 0, 'd'},
		{"ambient-max", required_argument, 0, 'a'},
		{"backlight-max", required_argument, 0, 'b'},
		{0, 0, 0, 0}
	};
	int option_index = 0;

	while ((c = getopt_long(argc, argv, "c:n:d:t:u:", long_options, &option_index))
			!= -1) {
		switch (c) {
			case 0:
				break;
			case 'c':
				config.iterations = strtol(optarg, &dummy, 10);
				break;
			case 'n':
				config.device_name = optarg;
				break;
			case 'd':
				config.debug_level = strtol(optarg, &dummy, 10);
				break;
			case 'u':
				config.poll_timeout = strtol(optarg, &dummy, 10);
				break;
			case 'a':
				config.light_ambient_max = strtol(optarg, &dummy, 10);
				break;
			case 'b':
				config.light_backlight_max = strtol(optarg, &dummy, 10);
				break;
			case '?':
				printf("Invalid flag, use --help for aditional info\n");
				return -1;
		}
	}

	if (version_flag) {
		printf("%s", version);
		exit(0);
	}
	if (help_flag) {
		printf("%s", help);
		exit(0);
	}

	signal(SIGINT, sigint_callback_handler);
	signal(SIGHUP, sigint_callback_handler);

	/* Find the device requested */
	info.device_id = find_type_by_name(config.device_name, "iio:device");
	if (info.device_id < 0) {
		printf("Failed to find the %s sensor\n", config.device_name);
		ret = -ENODEV;
		goto error_ret;
	}
	if (config.debug_level > DEBUG_INFO) printf("iio device number being used is %d\n", info.device_id);

	/* enable the sensors in the device */
	asprintf(&dev_dir_name, "%siio:device%d", iio_dir, info.device_id);
	if (ret < 0) {
		ret = -ENOMEM;
		goto error_ret;
	}

	enable_sensors(dev_dir_name);

	if (trigger_name == NULL) {
		/* Build the trigger name. */
		ret = asprintf(&trigger_name,
				"%s-dev%d", config.device_name, info.device_id);
		if (ret < 0) {
			ret = -ENOMEM;
			goto error_ret;
		}
	}
	/* Verify the trigger exists */
	ret = find_type_by_name(trigger_name, "trigger");
	if (ret < 0) {
		printf("Failed to find the trigger %s\n", trigger_name);
		ret = -ENODEV;
		goto error_free_triggername;
	}
	if (config.debug_level > DEBUG_INFO) printf("iio trigger number being used is %d\n", ret);

	/* Parse the files in scan_elements to identify what channels are present */
	ret = build_channel_array(dev_dir_name, &(info.channels), &(info.channels_count));
	if (ret) {
		printf("Problem reading scan element information\n");
		printf("diag %s\n", dev_dir_name);
		goto error_free_triggername;
	}

	/* Devices ready, connect to DBUS */
	// initialise the errors
	dbus_error_init(&derr);
	// connect to the bus
	dconn = dbus_bus_get(DBUS_BUS_SYSTEM, &derr);
	if (dbus_error_is_set(&derr)) {
		fprintf(stderr, "Cannot connect to dbus (%s)\n", derr.message);
		dbus_error_free(&derr);
		ret = -ECONNREFUSED;
		goto error_free_triggername;
	}
	if (NULL == dconn) {
		ret = -ECONNREFUSED;
		goto error_free_triggername;
	}
	// request a name on the bus
	ret = dbus_bus_request_name(dconn, "org.pfps.sensors", DBUS_NAME_FLAG_REPLACE_EXISTING, &derr);
	if (dbus_error_is_set(&derr)) {
		fprintf(stderr, "DBUS name rejected (%s)\n", derr.message);
		dbus_error_free(&derr);
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
		goto error_free_triggername;
	}

	for (i = 0; i != config.iterations; i++) {
		prepare_output(&info, dev_dir_name, trigger_name, &process_scan, config);
		usleep(config.poll_timeout);
	}

	// Free dbus connection
	dbus_connection_close(dconn);

	return EXIT_SUCCESS;

error_free_triggername:
	free(trigger_name);
	free(dev_dir_name);
error_ret:
	return ret;
}
