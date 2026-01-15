/* Pan Tilt Zoom OBS Plugin macros
 *
 * Copyright 2020,2021 Grant Likely <grant.likely@secretlab.ca>
 *
 * SPDX-License-Identifier: GPLv2
 *
 * Build configuration macros
 */

#ifndef PLUGINNAME_H
#define PLUGINNAME_H

#define PLUGIN_NAME "obs-ptz"
#define PLUGIN_VERSION "0.15.4-1-gf754462"

#define blog(level, msg, ...) blog(level, "[" PLUGIN_NAME "] " msg, ##__VA_ARGS__)

#endif // PLUGINNAME_H
