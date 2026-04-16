# Garage Door Controller

ESP32 firmware that controls a garage door opener via a relay, monitors door position with magnetic switches, and publishes state over MQTT. A common-cathode RGB LED provides local visual feedback.

## Hardware

### Components

- ESP32 development board
- PN2222 NPN transistor (door trigger)
- 2× magnetic switches (open and closed position sensors)
- Common-cathode RGB LED + current-limiting resistors

### Pin Assignments

| Signal | GPIO | Notes |
|---|---|---|
| PN2222 base | 4 | Output, active-HIGH pulse |
| Open sensor | 5 | Input, active-LOW (mag switch to GND) |
| Closed sensor | 13 | Input, active-LOW (mag switch to GND) |
| LED — Red | 21 | LEDC PWM channel 0 |
| LED — Green | 22 | LEDC PWM channel 1 |
| LED — Blue | 23 | LEDC PWM channel 2 |

### ESP32 DevKit Pinout

Pins used by this project are marked. GPIO 6–11 are reserved for internal SPI flash and must not be used.

```
                        ┌──────────┬──────────┐
                   3V3  │  1    38 │  GND
                   GND  │  2    37 │  GPIO23  ◄── LED Blue
          (boot) GPIO15 │  3    36 │  GPIO22  ◄── LED Green
          (boot)  GPIO2 │  4    35 │  GPIO21  ◄── LED Red
          (boot)  GPIO0 │  5    34 │  GPIO19
                  GPIO4 │  6    33 │  GPIO18  ◄── (available)
                 GPIO16 │  7    32 │  GPIO5   ◄── Open sensor
                 GPIO17 │  8    31 │  GPIO17
                        │  9    30 │  GPIO16
                        │ 10    29 │  GPIO4   ◄── Relay trigger
           UART0 GPIO1  │ 11    28 │  GND
           UART0 GPIO3  │ 12    27 │  GPIO12  (boot, avoid)
                 GPIO22 │ 13    26 │  GPIO14
                 GPIO21 │ 14    25 │  GPIO27
                   GND  │ 15    24 │  GPIO26
                   VIN  │ 16    23 │  GPIO25
                GPIO13  │ 17    22 │  GPIO33  (input only)
                GPIO12  │ 18    21 │  GPIO32  (input only)
                GPIO14  │ 19    20 │  GPIO35  (input only)
                GPIO27  │ 20    19 │  GPIO34  (input only)
                        └──────────┘
                        GPIO13 ◄── Closed sensor
```

> **Reserved / avoid:** GPIO 6, 7, 8, 9, 10, 11 — connected to internal SPI flash.
> **Input only:** GPIO 34, 35, 36 (VP), 39 (VN) — no internal pull-up/down, output not possible.
> **Boot strapping:** GPIO 0, 2, 12, 15 — safe for GPIO use after boot, but keep in mind their boot-time state.

### Wiring Diagram

```
                        ┌───────────────────────┐
                        │         ESP32         │
                        │                       │
               GPIO  4 ─┤                       │
               GPIO  5 ─┤                       │
               GPIO 13 ─┤                       │
               GPIO 21 ─┤                       ├─ 3V3
               GPIO 22 ─┤                       │
               GPIO 23 ─┤                       ├─ GND ──────────────────────┐
                        └───────────────────────┘                            │
                                                                             │
  ┌── Door Trigger ──────────────────────────────────────────────────────┐   │
  │                                                                      │   │
  │                          PN2222                                      │   │
  │  GPIO 4 ──┤1 kΩ├──────── Base     ◄── trigger pulse                 │   │
  │                          Emitter ─────────────────────────────────── ┼───┤
  │                          Collector ──┐                               │   │
  │                                     ├── Opener wall button terminals │   │
  │                          GND ───────┘  (shorts them like a button)  │   │
  └──────────────────────────────────────────────────────────────────────┘   │
                                                                             │
  ┌── Position Sensors ──────────────────────────────────────────────────┐   │
  │                                                                      │   │
  │  GPIO 13 ─────────────── [Mag Switch — CLOSED] ─────────────────────┼───┤
  │  GPIO  5 ─────────────── [Mag Switch — OPEN  ] ─────────────────────┼───┤
  │                           (active-LOW, internal pull-up enabled)     │   │
  └──────────────────────────────────────────────────────────────────────┘   │
                                                                             │
  ┌── RGB LED (common cathode) ──────────────────────────────────────────┐   │
  │                                                                      │   │
  │  GPIO 21 ──┤100 Ω├──── Red   anode ──┐                              │   │
  │  GPIO 22 ──┤100 Ω├──── Green anode ──┤ Common cathode ──────────────┼───┘
  │  GPIO 23 ──┤100 Ω├──── Blue  anode ──┘                              │
  │                                                                      │
  └──────────────────────────────────────────────────────────────────────┘
```

### Wiring Notes

- The PN2222 collector/emitter is triggered by a 500 ms pulse on GPIO 4 — wire the collector/emitter in parallel with the existing wall button terminals
- Magnetic switches are wired between the sensor GPIO and GND; internal pull-ups are enabled in firmware
- RGB LED common pin connects to GND; each colour pin connects through a resistor (~100 Ω) to its GPIO

## LED Status Indicator

| Door State | Colour | Pattern |
|---|---|---|
| Closed | Red | Solid |
| Open | Green | Solid |
| Closing | Red | Blinking |
| Opening | Green | Blinking |
| Stopped Closing | Amber | Solid |
| Stopped Opening | Amber | Solid |
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