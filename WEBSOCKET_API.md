# OBS PTZ WebSocket API Documentation

## Overview

The OBS PTZ plugin exposes a WebSocket vendor API through the OBS WebSocket plugin. All requests are sent to the `obs-ptz` vendor.

**Connection Requirements:**
- OBS WebSocket plugin must be installed and running
- OBS PTZ plugin must be loaded with `ENABLE_WEBSOCKET=ON` (default)
- Connect to OBS WebSocket on default port 4455 (or configured port)

**Vendor Name:** `obs-ptz`

## General Request Format

All requests follow the OBS WebSocket vendor request format:

```json
{
  "vendor": "obs-ptz",
  "request": "<command_name>",
  "<param1>": "<value1>",
  "<param2>": "<value2>"
}
```

## General Response Format

All responses include a `success` boolean field:

**Success Response:**
```json
{
  "success": true,
  // ... additional response fields
}
```

**Error Response:**
```json
{
  "success": false,
  "message": "Error description"
}
```

---

## API Commands

### 1. ptz_move

Controls continuous movement of the active PTZ device.

**Parameters:**
- `pan` (double, optional): Pan speed from -1.0 (left) to 1.0 (right). Default: 0.0
- `tilt` (double, optional): Tilt speed from -1.0 (down) to 1.0 (up). Default: 0.0
- `zoom` (double, optional): Zoom speed from -1.0 (wide) to 1.0 (tele). Default: 0.0

**Request Example:**
```json
{
  "vendor": "obs-ptz",
  "request": "ptz_move",
  "pan": 0.5,
  "tilt": 0.3,
  "zoom": 0.0
}
```

**Success Response:**
```json
{
  "success": true,
  "device_name": "PTZ Camera 1"
}
```

**Error Response:**
```json
{
  "success": false,
  "message": "No PTZ device selected"
}
```

**Notes:**
- Values are automatically clamped to [-1.0, 1.0] range
- Movement continues until stopped with `ptz_stop` or `ptz_move` with 0.0 values
- Operates on the currently selected/active PTZ device in the UI

---

### 2. ptz_stop

Stops all movement on the active PTZ device.

**Parameters:** None

**Request Example:**
```json
{
  "vendor": "obs-ptz",
  "request": "ptz_stop"
}
```

**Success Response:**
```json
{
  "success": true,
  "device_name": "PTZ Camera 1"
}
```

**Error Response:**
```json
{
  "success": false,
  "message": "No PTZ device selected"
}
```

**Notes:**
- Equivalent to calling `ptz_move` with all speeds set to 0.0
- Stops pan, tilt, and zoom movements

---

### 3. ptz_get_active_device

Returns information about the currently selected PTZ device.

**Parameters:** None

**Request Example:**
```json
{
  "vendor": "obs-ptz",
  "request": "ptz_get_active_device"
}
```

**Success Response:**
```json
{
  "success": true,
  "device_id": 42,
  "device_name": "PTZ Camera 1"
}
```

**Error Response:**
```json
{
  "success": false,
  "message": "No PTZ device selected"
}
```

**Notes:**
- Returns the device currently selected in the PTZ Controls UI
- `device_id` is an internal unique identifier

---

### 4. ptz_get_presets

Returns a list of all PTZ presets for the active device.

**Parameters:** None

**Request Example:**
```json
{
  "vendor": "obs-ptz",
  "request": "ptz_get_presets"
}
```

**Success Response:**
```json
{
  "success": true,
  "presets": [
    {
      "id": 0,
      "name": "Wide Shot"
    },
    {
      "id": 1,
      "name": "Close Up"
    },
    {
      "id": 2,
      "name": "Preset 3"
    }
  ]
}
```

**Error Responses:**
```json
{
  "success": false,
  "message": "No PTZ device selected"
}
```

```json
{
  "success": false,
  "message": "Preset model not available"
}
```

**Notes:**
- Preset IDs are zero-indexed (0 to N-1)
- Default preset count is 16, configurable up to 128 per device
- Preset names can be customized in the PTZ Controls UI
- Default names follow the pattern "Preset N"

---

### 5. ptz_recall_preset

Recalls/activates a PTZ preset, moving the camera to the saved position.

**Parameters:**
- `preset_id` (integer, required): The ID of the preset to recall (0-based index)

**Request Example:**
```json
{
  "vendor": "obs-ptz",
  "request": "ptz_recall_preset",
  "preset_id": 1
}
```

**Success Response:**
```json
{
  "success": true,
  "device_name": "PTZ Camera 1"
}
```

**Error Responses:**
```json
{
  "success": false,
  "message": "No PTZ device selected"
}
```

```json
{
  "success": false,
  "message": "Invalid preset ID: 5 (valid range: 0-3)"
}
```

```json
{
  "success": false,
  "message": "Missing required parameter: preset_id"
}
```

**Notes:**
- Preset ID must be within valid range (use `ptz_get_presets` to see available presets)
- Camera will move to the stored pan/tilt/zoom position
- Movement speed depends on the camera's configuration

---

### 6. ptz_set_preset

Saves the current camera position (pan/tilt/zoom) to an existing preset.

**Parameters:**
- `preset_id` (integer, required): The ID of the preset to update (0-based index)

**Request Example:**
```json
{
  "vendor": "obs-ptz",
  "request": "ptz_set_preset",
  "preset_id": 1
}
```

**Success Response:**
```json
{
  "success": true,
  "device_name": "PTZ Camera 1"
}
```

**Error Responses:**
```json
{
  "success": false,
  "message": "No PTZ device selected"
}
```

```json
{
  "success": false,
  "message": "Invalid preset ID: 5 (valid range: 0-3)"
}
```

```json
{
  "success": false,
  "message": "Missing required parameter: preset_id"
}
```

**Notes:**
- Does NOT change the preset name, only updates the stored position
- Preset ID must be within valid range
- Stores the current pan/tilt/zoom position to the camera's memory
- For ONVIF cameras, preset is stored on the device itself
- For VISCA cameras, preset is stored in camera memory

---

## Common Workflows

### Typical Movement Control Flow

```
1. Get active device info (optional, for confirmation)
   → ptz_get_active_device

2. Start moving camera
   → ptz_move with desired pan/tilt/zoom speeds

3. Stop when desired position is reached
   → ptz_stop
```

### Preset Management Flow

```
1. Get list of available presets
   → ptz_get_presets

2. Recall a preset to move camera
   → ptz_recall_preset with preset_id

3. Optionally: Position camera manually, then save to preset
   → ptz_move (position camera)
   → ptz_set_preset (save position)
```

---

## Error Handling

All commands can return these common errors:

- **"PTZ Controls not initialized"** - Plugin not loaded or WebSocket support disabled
- **"No PTZ device selected"** - No active camera in the PTZ Controls UI
- **"Preset model not available"** - Device does not support presets (rare)
- **"Invalid preset ID: X (valid range: 0-Y)"** - Preset ID out of bounds
- **"Missing required parameter: <param>"** - Required parameter not provided

**Best Practices:**
1. Always check the `success` field in responses
2. Use `ptz_get_active_device` to verify a camera is selected before operations
3. Use `ptz_get_presets` to determine valid preset IDs before recall/set operations
4. Handle errors gracefully and provide user feedback

---

## Thread Safety

All WebSocket commands are thread-safe and use Qt's `QMetaObject::invokeMethod` with `Qt::BlockingQueuedConnection` to execute on the main UI thread. This ensures proper synchronization with the OBS frontend.

---

## Example Client Pseudocode

```python
import obsws_python as obs

# Connect to OBS WebSocket
ws = obs.ReqClient(host='localhost', port=4455, password='your_password')

# Check active device
response = ws.call_vendor("obs-ptz", "ptz_get_active_device")
if response.success:
    print(f"Active device: {response.device_name}")

# Get presets
response = ws.call_vendor("obs-ptz", "ptz_get_presets")
if response.success:
    for preset in response.presets:
        print(f"Preset {preset['id']}: {preset['name']}")

# Recall preset 1
response = ws.call_vendor("obs-ptz", "ptz_recall_preset", {"preset_id": 1})

# Move camera right and up
response = ws.call_vendor("obs-ptz", "ptz_move", {
    "pan": 0.5,   # Move right at 50% speed
    "tilt": 0.3,  # Move up at 30% speed
    "zoom": 0.0   # No zoom
})

# Stop movement
response = ws.call_vendor("obs-ptz", "ptz_stop")

# Save current position to preset 2
response = ws.call_vendor("obs-ptz", "ptz_set_preset", {"preset_id": 2})
```

---

## Implementation Details

**Source Files:**
- `src/ptz-websocket.h` - WebSocket API header
- `src/ptz-websocket.cpp` - WebSocket command handlers (366 lines)
- `src/ptz-controls.hpp` - PTZControls class with Q_INVOKABLE methods
- `src/ptz-controls.cpp` - Implementation of WebSocket methods

**Build Configuration:**
- Enabled by default with CMake option `ENABLE_WEBSOCKET=ON`
- Requires OBS WebSocket plugin to be installed
- Uses OBS WebSocket vendor extension API

**Logging:**
- Debug logs available in OBS log files
- Format: `[obs-ptz-websocket] <command>: <parameters> -> <result>`
- Use OBS log level DEBUG to see detailed WebSocket activity
