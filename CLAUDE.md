# Claude Code Instructions

This is **embermug-tray**, a Linux system tray application for controlling Ember Mug smart mugs via Bluetooth Low Energy.

## Project Structure

```
embermug-tray/
├── lib/                      # Core Ember library (static)
│   ├── include/ember/        # Public headers (mug.h, types.h, ember.h)
│   ├── src/                  # Implementation (mug.cpp, service.cpp, constants.h)
│   └── CMakeLists.txt
├── app/                      # Qt6 tray application
│   ├── src/                  # traycontroller, devicemonitor, connectionmanager
│   ├── resources/            # Icons and Qt resources
│   └── CMakeLists.txt
├── tests/                    # Qt Test unit tests
│   ├── test_types.cpp        # Type conversion tests
│   ├── test_mug.cpp          # Mug static method tests
│   └── CMakeLists.txt
└── nix/                      # Nix package and home-manager module
```

## Technology Stack

- **Language:** C++17
- **Framework:** Qt6 (Core, Widgets, Bluetooth, Svg)
- **Build:** CMake 3.16+ with Ninja
- **Testing:** Qt Test framework
- **Analysis:** Clang-Tidy with Clazy checks

## Building and Testing

```bash
# Enter dev shell (provides all dependencies)
nix develop

# Configure and build
cmake -B build -G Ninja
cmake --build build

# Run tests
ctest --test-dir build

# Run clang-tidy
clang-tidy -p build lib/src/*.cpp app/src/*.cpp
```

## Build Targets

| Target | Type | Description |
|--------|------|-------------|
| `ember` | Static library | Core BLE communication library |
| `embermug-tray` | Executable | System tray application |
| `test_types` | Test executable | Type/conversion unit tests |
| `test_mug` | Test executable | Mug class unit tests |

## Code Style

### Qt Conventions

- Use `Q_OBJECT` macro for signal/slot classes
- Use `Q_PROPERTY` with `NOTIFY` signals for observable state
- Use `Q_DISABLE_COPY_MOVE` on QObject subclasses
- Prefix member variables with `m_`
- Use `QStringLiteral()` for compile-time string constants

### Modern C++

- Use `[[nodiscard]]` on getters
- Use `std::optional` for nullable values
- Use `constexpr` for compile-time constants
- Prefer `auto` for complex types, explicit types for primitives

### Documentation

- Use Doxygen-style comments (`/** @brief */`) for public API
- Document parameters with `@param` and return values with `@return`

### Clang-Tidy

All warnings are treated as errors. The configuration enables:
- `bugprone-*`, `cppcoreguidelines-*`, `modernize-*`, `performance-*`, `readability-*`, `clazy-*`

Disabled checks are Qt-specific accommodations (see `.clang-tidy`).

## Architecture Overview

### Library Layer (`lib/`)

**Mug** (`mug.h`): High-level interface to an Ember Mug
- Properties: temperature, battery, liquid state, heating status, LED color
- Methods: `initialize()`, `setTargetTemperature()`, `setColor()`, `refresh()`
- Signals for all state changes

**Service** (`service.h`): Internal BLE characteristic handler
- Manages service discovery and characteristic subscriptions
- Handles raw BLE read/write operations
- Emits parsed data via signals

**Types** (`types.h`): Enums and value types
- `TempUnit`, `LiquidState`, `BatteryState`, `MugEvent`
- `MugColor` struct with RGBA values

### Application Layer (`app/`)

**TrayController**: System tray icon, context menu, and notifications
- Dynamic icon rendering based on temperature and battery
- Device selection and temperature preset menus
- Desktop notifications via `QSystemTrayIcon::showMessage()` (D-Bus org.freedesktop.Notifications)

**ConnectionManager**: BLE connection lifecycle
- Manages `QLowEnergyController` connections
- 10-second initialization timeout
- Reconnection handling

**DeviceMonitor**: Bluetooth device discovery
- Periodic scanning for Ember Mugs
- Filtering by name prefix and service UUID

## Notifications

TrayController sends desktop notifications for the following events:

| Event | Title | Severity |
|-------|-------|----------|
| Mug connected | "Mug Connected" | Information |
| Mug disconnected | "Mug Disconnected" | Warning |
| Target temperature reached | "Target Temperature Reached" | Information |
| Heating started | "Heating Started" | Information |
| Heating stopped | "Heating Stopped" | Information |
| Started charging | "Charging" | Information |
| Removed from charger | "Not Charging" | Information |
| Battery fully charged | "Fully Charged" | Information |
| Battery low | "Battery Low" | Warning |
| Battery critical | "Battery Critical" | Critical |

Battery threshold notifications are configurable via QSettings (read-only):

| Key | Default | Description |
|-----|---------|-------------|
| `notifications.batteryLow` | `20` | Low battery warning threshold (%) |
| `notifications.batteryCritical` | `5` | Critical battery alert threshold (%) |

## BLE Protocol Reference

For low-level Bluetooth protocol details (characteristic UUIDs, data formats, push events), invoke the skill:

```
/ember-mug-ble-protocol
```

This provides documentation on:
- Service and characteristic UUIDs
- Temperature encoding (uint16, value × 100)
- Battery data format
- Liquid state codes
- Push event notifications
- LED color RGBA format

## Validation Checklist

Before committing changes:

1. **Build succeeds:** `cmake --build build`
2. **Tests pass:** `ctest --test-dir build`
3. **Clang-tidy clean:** `clang-tidy -p build <modified-files>`

## Key Constants

| Constant | Value |
|----------|-------|
| Service UUID | `fc543622-236c-4c94-8fa9-944a3e5353fa` |
| Min temp (C) | 50.0 |
| Max temp (C) | 62.5 |
| Min temp (F) | 120.0 |
| Max temp (F) | 145.0 |
