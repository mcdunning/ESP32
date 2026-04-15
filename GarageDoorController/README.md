# Garage Door Controller

ESP32 firmware that controls a garage door opener via a relay, monitors door position with magnetic switches, and publishes state over MQTT. A common-cathode RGB LED provides local visual feedback.

## Hardware

### Components

- ESP32 development board
- NPN transistor or relay module (relay trigger input)
- 2× magnetic switches (open and closed position sensors)
- Common-cathode RGB LED + current-limiting resistors

### Pin Assignments

| Signal | GPIO | Notes |
|---|---|---|
| Relay trigger | 8 | Output, active-HIGH pulse |
| Open sensor | 7 | Input, active-LOW (mag switch to GND) |
| Closed sensor | 6 | Input, active-LOW (mag switch to GND) |
| LED — Red | 21 | LEDC PWM channel 0 |
| LED — Green | 22 | LEDC PWM channel 1 |
| LED — Blue | 23 | LEDC PWM channel 2 |

### Wiring Notes

- The relay/transistor is triggered by a 500 ms pulse on GPIO 8 — wire this in parallel with the existing wall button
- Magnetic switches are wired between the sensor GPIO and GND; internal pull-ups are enabled in firmware
- RGB LED common pin connects to GND; each colour pin connects through a resistor (~100 Ω) to its GPIO

## LED Status Indicator

| Door State | Colour | Pattern |
|---|---|---|
| Closed | Red | Solid |
| Open | Green | Solid |
| Closing | Red | Blinking |
| Opening | Green | Blinking |
| Unknown | Amber | Blinking |

## MQTT

### Topics

| Topic | Direction | Payload |
|---|---|---|
| `garage/door/state` | Published | `open` / `closed` / `opening` / `closing` / `unknown` |
| `garage/door/availability` | Published | `online` / `offline` (last will) |
| `garage/door/command` | Subscribed | `OPEN` / `CLOSE` / `TOGGLE` |

State and availability topics are published with **retain = true**.

### Home Assistant

Add to `configuration.yaml`:

```yaml
mqtt:
  cover:
    - name: "Garage Door"
      command_topic: "garage/door/command"
      state_topic: "garage/door/state"
      availability_topic: "garage/door/availability"
      payload_open: "OPEN"
      payload_close: "CLOSE"
      state_open: "open"
      state_closed: "closed"
      state_opening: "opening"
      state_closing: "closing"
      device_class: garage
```

## Setup

### 1. Configure credentials

```bash
cp main/config.h.example main/config.h
```

Edit `main/config.h` and fill in:

- `WIFI_SSID` / `WIFI_PASSWORD`
- `MQTT_BROKER_URI` (e.g. `mqtt://192.168.1.100`)
- `MQTT_USERNAME` / `MQTT_PASSWORD` if your broker requires auth

`main/config.h` is gitignored — credentials are never committed.

### 2. Build and flash

```bash
idf.py build
idf.py -p COM<N> flash monitor
```

### 3. Verify

On first boot the LED will blink amber (unknown state) until a magnetic switch is triggered. Check the serial monitor for startup logs and confirm MQTT messages appear on your broker.

## Project Structure

```
GarageDoorController/
├── main/
│   ├── main.c              # app_main — system init and task creation
│   ├── wifi_manager.c/h    # WiFi STA with auto-reconnect
│   ├── mqtt_manager.c/h    # MQTT client, pub/sub, last will
│   ├── door_control.c/h    # GPIO, state machine, command queue
│   ├── led_indicator.c/h   # RGB LED via LEDC PWM
│   ├── config.h            # Credentials and pin assignments (gitignored)
│   ├── config.h.example    # Template — copy to config.h
│   └── idf_component.yml   # Component manager dependencies
└── CMakeLists.txt
```