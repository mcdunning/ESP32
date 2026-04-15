# ESP32 Projects

A collection of ESP32 firmware projects built with [ESP-IDF](https://github.com/espressif/esp-idf).

## Development Environment

| Tool | Version |
|---|---|
| ESP-IDF | v6.0 |
| Toolchain | xtensa-esp-elf esp-15.2.0 |
| IDE | VS Code with Espressif IDF extension |

### Prerequisites

1. Install [ESP-IDF v6.0](https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32/get-started/index.html) via the VS Code Espressif extension or the standalone installer
2. Clone this repo
3. Open `C:\Workspace\ESP32` as the workspace root in VS Code — the IntelliSense configuration depends on this

### Building a Project

Open an ESP-IDF terminal and navigate to the project folder:

```bash
cd GarageDoorController
idf.py build
idf.py -p COM<N> flash monitor
```

## Projects

| Project | Description |
|---|---|
| [GarageDoorController](GarageDoorController/README.md) | FreeRTOS-based garage door controller with MQTT state reporting and RGB status LED |