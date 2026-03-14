---
name: ember-mug-ble-protocol
description: EmberMug BLE protocol documentation. Use when implementing low-level Bluetooth interactions with Ember mugs, working with BLE characteristics, or handling mug events.
user-invocable: false
---

# Ember Mug BLE Protocol Reference

This documentation is based on reverse-engineered documentation from the [ember-mug](https://github.com/sopelj/ember-mug) repository.

## Service UUID

All characteristics use the base service UUID:

```
fc543622-236c-4c94-8fa9-944a3e5353fa
```

## Quick Reference

| Characteristic | UUID Suffix | Methods | Description |
|----------------|-------------|---------|-------------|
| Mug Name | `0001` | R/W | Device name (ASCII, up to 14 bytes) |
| Current Temp | `0002` | R | Current liquid temperature |
| Target Temp | `0003` | R/W | Target heating temperature |
| Temp Unit | `0004` | R/W | Celsius (0) or Fahrenheit (1) |
| Liquid Level | `0005` | R | Liquid level (0=empty, 30=not empty) |
| Time/Date/Zone | `0006` | W | Unix timestamp + timezone offset |
| Battery | `0007` | R | Battery level, charging state, temp |
| Liquid State | `0008` | R | Current heating state |
| OTA Info | `000C` | R | Firmware/hardware versions |
| Push Events | `0012` | R/N | Event notifications |
| Mug Color | `0014` | R/W | LED color (RGBA) |

Full UUID format: `fc54XXXX-236c-4c94-8fa9-944a3e5353fa`

## Supplemental Documentation Lookup

Read these files based on the topic you need:

| Topic | File | When to Read |
|-------|------|--------------|
| UUID details, read/write operations, characteristic properties | `characteristics.md` | Implementing specific BLE reads/writes, understanding characteristic behavior |
| Event codes, notifications, state changes | `push-events.md` | Handling mug events, setting up notification listeners, reacting to state changes |
| Temperature encoding, battery format, liquid state codes, color format | `data-formats.md` | Parsing/encoding data values, converting temperatures, interpreting raw bytes |

File paths relative to this skill directory:
- `.claude/skills/ember-mug-ble-protocol/characteristics.md`
- `.claude/skills/ember-mug-ble-protocol/push-events.md`
- `.claude/skills/ember-mug-ble-protocol/data-formats.md`

## Temperature Ranges

- **Celsius:** 50.0°C - 62.5°C
- **Fahrenheit:** 120.0°F - 145.0°F

## Device Detection

Ember mugs can be identified by:
- Device name starting with "Ember" (case-insensitive)
- Advertising the service UUID `fc543622-236c-4c94-8fa9-944a3e5353fa`
