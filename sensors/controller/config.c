/*
 * File:   controller.h
 * Author: buri
 *
 * Created on 4. květen 2014, 10:23
 */

#include "config.h"

bool loadConfig(char* file, Config* config) {
	/* Setup variables */
	GKeyFile* gfile = g_key_file_new();
	GError* gerr = NULL;
	printf("%s\n", file);

	/* Run parsing */
	if (!g_key_file_load_from_file(gfile, file, G_KEY_FILE_NONE, &gerr)) {
		fprintf(stderr, "Error parsing configuration: %s\n", gerr->message);
		return false;
	}

	config->debug_level = g_key_file_get_integer(gfile, "Main", "DebugLevel", &gerr);
	if (gerr) goto conf_error;
	printf("%d\n", config->debug_level);

	g_key_file_free(gfile);
	return true;

	conf_error:
	fprintf(stderr, "Error reading config file: %s\n", gerr->message);
	return false;
}

bool parseArguments(char** argv, int argc, Config* config) {

	struct option long_options[] = {
		{"version", no_argument, &(config->version_flag), 1},
		{"help", no_argument, &(config->help_flag), 1},
		{"touchscreen", required_argument, 0, 't'},
		{"debug", required_argument, 0, 'd'},
		{0, 0, 0, 0}
	};

	int option_index = 0, c;
	char *dummy;

	while ((c = getopt_long(argc, argv, "d:t", long_options, &option_index))
			!= -1) {
		switch (c) {
			case 0:
				break;
			case 't':
				config->touch_screen_name = optarg;
				break;
			case 'd':
				config->debug_level = strtol(optarg, &dummy, 10);
				break;
			case '?':
				return false;
		}
	}
	return true;
}
