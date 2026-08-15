/*
 * Copyright (C) 2021      Andy Nguyen
 * Copyright (C) 2022-2023 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <stdio.h>
#include <string.h>
#include "settings.h"

#define CONFIG_FILE_PATH DATA_PATH"config.txt"

bool settings_capframerate;
int settings_graphicsqualty;

void settings_reset() {
    settings_capframerate  = true;
    settings_graphicsqualty = 0;
}

void settings_load() {
    settings_reset();

    char buffer[30];
    int value;

    FILE *config = fopen(CONFIG_FILE_PATH, "r");

    if (config) {
        while (EOF != fscanf(config, "%[^ ] %d\n", buffer, &value)) {
            if 		(strcmp("CapFramerate", buffer) == 0) 	settings_capframerate  = (bool)value;
            else if (strcmp("GraphicsQuality", buffer) == 0) settings_graphicsqualty = (int)value;
        }
        fclose(config);
    }
}

void settings_save() {
    FILE *config = fopen(CONFIG_FILE_PATH, "w+");

    if (config) {
        fprintf(config, "%s %d\n", "CapFramerate", (bool)(settings_capframerate));
        fprintf(config, "%s %d\n", "GraphicsQuality", (int)(settings_graphicsqualty));
        fclose(config);
    }
}
