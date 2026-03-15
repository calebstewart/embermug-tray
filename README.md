# EmberMug Tray

A Linux system tray application for [Ember Mug](https://ember.com/) smart mugs. Monitor and control your mug's temperature directly from your desktop.

## Features

- System tray integration with temperature display
- Bluetooth LE connectivity
- Real-time temperature monitoring
- Target temperature control
- Desktop notifications for mug events (via D-Bus org.freedesktop.Notifications)

## Requirements

- Qt 6 (Core, Widgets, Bluetooth, Svg)
- CMake 3.16+
- C++17 compiler
- Linux with Bluetooth LE support

## Building

### With Nix (recommended)

```bash
nix build
```

### With CMake

```bash
cmake -B build -G Ninja
cmake --build build
```

## Installation

### NixOS / Home Manager

Add the flake to your inputs:

```nix
{
  inputs.embermug-tray.url = "github:your-username/embermug-tray";
}
```

Enable the service in your home-manager configuration:

```nix
{
  imports = [ inputs.embermug-tray.homeManagerModules.default ];

  services.embermug-tray = {
    enable = true;

    # Optional: configure settings
    settings = {
      notifications = {
        batteryLow = "25";
        batteryCritical = "10";
      };
    };
  };
}
```

This will install the application and set up a systemd user service that starts automatically with your graphical session.

### Available Settings

| Section | Key | Default | Description |
|---------|-----|---------|-------------|
| `device` | `address` | (none) | Preferred mug Bluetooth address |
| `notifications` | `batteryLow` | `20` | Battery percentage for low warning |
| `notifications` | `batteryCritical` | `5` | Battery percentage for critical alert |

### Manual

```bash
cmake --install build --prefix ~/.local
```

## Device Setup

Before the tray application can connect to your Ember Mug, you must pair the mug with your system's Bluetooth stack. The application discovers and connects to already-paired devices—it does not handle initial pairing.

### Pairing Mode

To put your Ember Mug in pairing mode:

1. Ensure the mug is on its charging coaster
2. Hold down the button on the bottom of the coaster for 5-6 seconds
3. The LED will flash blue, indicating the mug is discoverable

### Using bluetoothctl (CLI)

```bash
bluetoothctl
```

Once in the bluetoothctl shell:

```
[bluetooth]# scan on
```

Wait for your Ember Mug to appear (look for "Ember Ceramic Mug" or similar):

```
[NEW] Device AA:BB:CC:DD:EE:FF Ember Ceramic Mug
```

Then pair and trust the device:

```
[bluetooth]# pair AA:BB:CC:DD:EE:FF
[bluetooth]# trust AA:BB:CC:DD:EE:FF
[bluetooth]# scan off
[bluetooth]# exit
```

### Using a GUI Bluetooth Manager

You can also use your desktop environment's Bluetooth settings (GNOME Settings, KDE Bluedevil, Blueman, etc.). Simply put the mug in pairing mode and connect to it through the GUI. The tray application will be able to discover any mug that has been paired with your system.

## Configuration

The application uses QSettings for persistent configuration. On Linux, settings are stored in `~/.config/embermug/tray.conf`.

## Development

Enter the development shell:

```bash
nix develop
```

Run tests:

```bash
cmake -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build
```

## Credits

The coffee mug icon is from [Zondicons](https://github.com/dukestreetstudio/zondicons) ([commit e5901f4](https://github.com/dukestreetstudio/zondicons/tree/e5901f49e88d05cfddd788161e9f6acd6af90cc7)), licensed under MIT.

## License

MIT License. See [LICENSE](LICENSE) for details.
