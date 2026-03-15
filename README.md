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

  services.embermug-tray.enable = true;
}
```

This will install the application and set up a systemd user service that starts automatically with your graphical session.

## Configuration

The application uses QSettings for persistent configuration. On Linux, settings are stored in `~/.config/embermug-tray/embermug-tray.conf`.

| Key | Default | Description |
|-----|---------|-------------|
| `device.address` | (none) | Preferred mug Bluetooth address |
| `notifications.batteryLow` | `20` | Battery percentage for low warning |
| `notifications.batteryCritical` | `5` | Battery percentage for critical alert |

### Manual

```bash
cmake --install build --prefix ~/.local
```

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

## License

MIT License. See [LICENSE](LICENSE) for details.
