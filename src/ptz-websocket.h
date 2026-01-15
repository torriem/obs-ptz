/* Pan Tilt Zoom OBS WebSocket vendor request support
 *
 * Copyright 2025
 *
 * SPDX-License-Identifier: GPLv2
 */
#ifndef PTZ_WEBSOCKET_H
#define PTZ_WEBSOCKET_H

#include <obs-module.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void ptz_load_websocket(void);
extern void ptz_unload_websocket(void);

#ifdef __cplusplus
}
#endif

#endif /* PTZ_WEBSOCKET_H */
