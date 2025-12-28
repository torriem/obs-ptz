/* OBS WebSocket vendor request support for PTZ controls
 *
 * Copyright 2025
 *
 * SPDX-License-Identifier: GPLv2
 */

#include <obs-module.h>
#include <obs-websocket-api.h>
#include <util/platform.h>
#include <algorithm>
#include <QString>
#include <QMetaObject>
#include "ptz-websocket.h"
#include "ptz-controls.hpp"

static obs_websocket_vendor vendor = nullptr;

/* Vendor request: ptz_move
 * Controls continuous movement of the active PTZ device
 */
static void ptz_move_request_cb(obs_data_t *request_data,
				obs_data_t *response_data, void *priv_data)
{
	UNUSED_PARAMETER(priv_data);

	// Extract movement parameters (default to 0.0 if not provided)
	double pan = obs_data_has_user_value(request_data, "pan")
			     ? obs_data_get_double(request_data, "pan")
			     : 0.0;
	double tilt = obs_data_has_user_value(request_data, "tilt")
			      ? obs_data_get_double(request_data, "tilt")
			      : 0.0;
	double zoom = obs_data_has_user_value(request_data, "zoom")
			      ? obs_data_get_double(request_data, "zoom")
			      : 0.0;

	// Clamp to valid range
	pan = std::clamp(pan, -1.0, 1.0);
	tilt = std::clamp(tilt, -1.0, 1.0);
	zoom = std::clamp(zoom, -1.0, 1.0);

	// Execute on main thread
	QString device_name;
	QString error_msg;
	bool success = false;

	PTZControls *controls = PTZControls::getInstance();
	if (controls) {
		QMetaObject::invokeMethod(
			controls, "websocketMove",
			Qt::BlockingQueuedConnection,
			Q_RETURN_ARG(bool, success),
			Q_ARG(QString&, device_name),
			Q_ARG(QString&, error_msg),
			Q_ARG(double, pan),
			Q_ARG(double, tilt),
			Q_ARG(double, zoom));
	} else {
		error_msg = "PTZ Controls not initialized";
	}

	// Build response
	obs_data_set_bool(response_data, "success", success);
	if (success) {
		obs_data_set_string(response_data, "device_name",
				    QT_TO_UTF8(device_name));
	} else {
		obs_data_set_string(response_data, "message",
				    QT_TO_UTF8(error_msg));
	}

	blog(LOG_DEBUG,
	     "[obs-ptz-websocket] ptz_move: pan=%.2f tilt=%.2f zoom=%.2f -> %s",
	     pan, tilt, zoom, success ? "success" : "failed");
}

/* Vendor request: ptz_stop
 * Stops all movement on the active PTZ device
 */
static void ptz_stop_request_cb(obs_data_t *request_data,
				obs_data_t *response_data, void *priv_data)
{
	UNUSED_PARAMETER(request_data);
	UNUSED_PARAMETER(priv_data);

	QString device_name;
	QString error_msg;
	bool success = false;

	PTZControls *controls = PTZControls::getInstance();
	if (controls) {
		QMetaObject::invokeMethod(controls, "websocketStop",
					  Qt::BlockingQueuedConnection,
					  Q_RETURN_ARG(bool, success),
					  Q_ARG(QString&, device_name),
					  Q_ARG(QString&, error_msg));
	} else {
		error_msg = "PTZ Controls not initialized";
	}

	obs_data_set_bool(response_data, "success", success);
	if (success) {
		obs_data_set_string(response_data, "device_name",
				    QT_TO_UTF8(device_name));
	} else {
		obs_data_set_string(response_data, "message",
				    QT_TO_UTF8(error_msg));
	}

	blog(LOG_DEBUG, "[obs-ptz-websocket] ptz_stop -> %s",
	     success ? "success" : "failed");
}

/* Vendor request: ptz_get_active_device
 * Returns information about the currently selected PTZ device
 */
static void ptz_get_active_device_cb(obs_data_t *request_data,
				     obs_data_t *response_data,
				     void *priv_data)
{
	UNUSED_PARAMETER(request_data);
	UNUSED_PARAMETER(priv_data);

	uint32_t device_id = 0;
	QString device_name;
	QString error_msg;
	bool success = false;

	PTZControls *controls = PTZControls::getInstance();
	if (controls) {
		QMetaObject::invokeMethod(
			controls, "websocketGetActiveDevice",
			Qt::BlockingQueuedConnection,
			Q_RETURN_ARG(bool, success),
			Q_ARG(uint32_t&, device_id),
			Q_ARG(QString&, device_name),
			Q_ARG(QString&, error_msg));
	} else {
		error_msg = "PTZ Controls not initialized";
	}

	obs_data_set_bool(response_data, "success", success);
	if (success) {
		obs_data_set_int(response_data, "device_id", device_id);
		obs_data_set_string(response_data, "device_name",
				    QT_TO_UTF8(device_name));
	} else {
		obs_data_set_string(response_data, "message",
				    QT_TO_UTF8(error_msg));
	}
}

/* Vendor request: ptz_get_presets
 * Returns a list of all PTZ presets for the active device
 */
static void ptz_get_presets_cb(obs_data_t *request_data,
			       obs_data_t *response_data,
			       void *priv_data)
{
	UNUSED_PARAMETER(request_data);
	UNUSED_PARAMETER(priv_data);

	QString error_msg;
	bool success = false;
	obs_data_array_t *presets = obs_data_array_create();

	PTZControls *controls = PTZControls::getInstance();
	if (controls) {
		QMetaObject::invokeMethod(controls, "websocketGetPresets",
					  Qt::BlockingQueuedConnection,
					  Q_RETURN_ARG(bool, success),
					  Q_ARG(obs_data_array_t *, presets),
					  Q_ARG(QString&, error_msg));
	} else {
		error_msg = "PTZ Controls not initialized";
	}

	obs_data_set_bool(response_data, "success", success);
	if (success) {
		obs_data_set_array(response_data, "presets", presets);
	} else {
		obs_data_set_string(response_data, "message",
				    QT_TO_UTF8(error_msg));
	}

	obs_data_array_release(presets);

	blog(LOG_DEBUG, "[obs-ptz-websocket] ptz_get_presets -> %s",
	     success ? "success" : "failed");
}

/* Vendor request: ptz_recall_preset
 * Recalls a PTZ preset by ID
 */
static void ptz_recall_preset_cb(obs_data_t *request_data,
				 obs_data_t *response_data,
				 void *priv_data)
{
	UNUSED_PARAMETER(priv_data);

	// Extract preset ID from request
	if (!obs_data_has_user_value(request_data, "preset_id")) {
		obs_data_set_bool(response_data, "success", false);
		obs_data_set_string(response_data, "message",
				    "Missing required parameter: preset_id");
		return;
	}

	int preset_id = (int)obs_data_get_int(request_data, "preset_id");

	QString device_name;
	QString error_msg;
	bool success = false;

	PTZControls *controls = PTZControls::getInstance();
	if (controls) {
		QMetaObject::invokeMethod(
			controls, "websocketRecallPreset",
			Qt::BlockingQueuedConnection,
			Q_RETURN_ARG(bool, success),
			Q_ARG(int, preset_id),
			Q_ARG(QString&, device_name),
			Q_ARG(QString&, error_msg));
	} else {
		error_msg = "PTZ Controls not initialized";
	}

	obs_data_set_bool(response_data, "success", success);
	if (success) {
		obs_data_set_string(response_data, "device_name",
				    QT_TO_UTF8(device_name));
	} else {
		obs_data_set_string(response_data, "message",
				    QT_TO_UTF8(error_msg));
	}

	blog(LOG_DEBUG,
	     "[obs-ptz-websocket] ptz_recall_preset: preset_id=%d -> %s",
	     preset_id, success ? "success" : "failed");
}

/* Vendor request: ptz_set_preset
 * Saves the current camera position to a PTZ preset by ID
 */
static void ptz_set_preset_cb(obs_data_t *request_data,
			       obs_data_t *response_data,
			       void *priv_data)
{
	UNUSED_PARAMETER(priv_data);

	// Extract preset ID from request
	if (!obs_data_has_user_value(request_data, "preset_id")) {
		obs_data_set_bool(response_data, "success", false);
		obs_data_set_string(response_data, "message",
				    "Missing required parameter: preset_id");
		return;
	}

	int preset_id = (int)obs_data_get_int(request_data, "preset_id");

	QString device_name;
	QString error_msg;
	bool success = false;

	PTZControls *controls = PTZControls::getInstance();
	if (controls) {
		QMetaObject::invokeMethod(
			controls, "websocketSetPreset",
			Qt::BlockingQueuedConnection,
			Q_RETURN_ARG(bool, success),
			Q_ARG(int, preset_id),
			Q_ARG(QString&, device_name),
			Q_ARG(QString&, error_msg));
	} else {
		error_msg = "PTZ Controls not initialized";
	}

	obs_data_set_bool(response_data, "success", success);
	if (success) {
		obs_data_set_string(response_data, "device_name",
				    QT_TO_UTF8(device_name));
	} else {
		obs_data_set_string(response_data, "message",
				    QT_TO_UTF8(error_msg));
	}

	blog(LOG_DEBUG,
	     "[obs-ptz-websocket] ptz_set_preset: preset_id=%d -> %s",
	     preset_id, success ? "success" : "failed");
}

void ptz_load_websocket(void)
{
	blog(LOG_INFO, "[obs-ptz-websocket] ptz_load_websocket() called");

	// Register vendor with obs-websocket
	vendor = obs_websocket_register_vendor("obs-ptz");
	if (!vendor) {
		blog(LOG_ERROR,
		     "[obs-ptz-websocket] Failed to register vendor with obs-websocket - obs-websocket may not be loaded yet");
		return;
	}

	blog(LOG_INFO, "[obs-ptz-websocket] Vendor registered successfully");

	// Register request handlers
	if (!obs_websocket_vendor_register_request(vendor, "ptz_move",
						   ptz_move_request_cb,
						   nullptr)) {
		blog(LOG_ERROR,
		     "[obs-ptz-websocket] Failed to register ptz_move request");
	}

	if (!obs_websocket_vendor_register_request(vendor, "ptz_stop",
						   ptz_stop_request_cb,
						   nullptr)) {
		blog(LOG_ERROR,
		     "[obs-ptz-websocket] Failed to register ptz_stop request");
	}

	if (!obs_websocket_vendor_register_request(
		    vendor, "ptz_get_active_device", ptz_get_active_device_cb,
		    nullptr)) {
		blog(LOG_ERROR,
		     "[obs-ptz-websocket] Failed to register ptz_get_active_device request");
	}

	if (!obs_websocket_vendor_register_request(vendor, "ptz_get_presets",
						   ptz_get_presets_cb,
						   nullptr)) {
		blog(LOG_ERROR,
		     "[obs-ptz-websocket] Failed to register ptz_get_presets request");
	}

	if (!obs_websocket_vendor_register_request(vendor, "ptz_recall_preset",
						   ptz_recall_preset_cb,
						   nullptr)) {
		blog(LOG_ERROR,
		     "[obs-ptz-websocket] Failed to register ptz_recall_preset request");
	}

	if (!obs_websocket_vendor_register_request(vendor, "ptz_set_preset",
						   ptz_set_preset_cb,
						   nullptr)) {
		blog(LOG_ERROR,
		     "[obs-ptz-websocket] Failed to register ptz_set_preset request");
	}

	blog(LOG_INFO,
	     "[obs-ptz-websocket] Vendor requests registered: ptz_move, ptz_stop, ptz_get_active_device, ptz_get_presets, ptz_recall_preset, ptz_set_preset");
}

void ptz_unload_websocket(void)
{
	if (vendor) {
		obs_websocket_vendor_unregister_request(vendor, "ptz_move");
		obs_websocket_vendor_unregister_request(vendor, "ptz_stop");
		obs_websocket_vendor_unregister_request(vendor,
							"ptz_get_active_device");
		obs_websocket_vendor_unregister_request(vendor, "ptz_get_presets");
		obs_websocket_vendor_unregister_request(vendor, "ptz_recall_preset");
		obs_websocket_vendor_unregister_request(vendor, "ptz_set_preset");
		vendor = nullptr;
	}
	blog(LOG_INFO, "[obs-ptz-websocket] Vendor unregistered");
}
