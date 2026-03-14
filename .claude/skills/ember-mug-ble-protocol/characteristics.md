# BLE Characteristics Reference

All characteristics use the service UUID: `fc543622-236c-4c94-8fa9-944a3e5353fa`

Full characteristic UUID format: `fc54XXXX-236c-4c94-8fa9-944a3e5353fa`

---

## Mug Name

**UUID:** `fc540001-236c-4c94-8fa9-944a3e5353fa`

**Methods:** READ, WRITE

**Data Format:**
- Size: Up to 14 bytes
- Type: ASCII string (no spaces)

**Example:**
```
45 6D 62 65 72 4D 75 67  ->  "EmberMug"
```

---

## Current Temperature

**UUID:** `fc540002-236c-4c94-8fa9-944a3e5353fa`

**Methods:** READ

**Data Format:**
- Size: 2 bytes
- Type: UINT16 Little Endian
- Conversion: `value * 0.01 = °C`

**Example:**
```
AE 15  ->  0x15AE = 5550 decimal  ->  55.50°C
```

---

## Target Temperature

**UUID:** `fc540003-236c-4c94-8fa9-944a3e5353fa`

**Methods:** READ, WRITE

**Data Format:**
- Size: 2 bytes
- Type: UINT16 Little Endian
- Conversion: `value * 0.01 = °C`
- Writing `0x0000` turns off the heater

**Example:**
```
To set 55.50°C: write 5550 as UINT16 LE -> 0xAE 0x15
```

---

## Temperature Unit

**UUID:** `fc540004-236c-4c94-8fa9-944a3e5353fa`

**Methods:** READ, WRITE

**Data Format:**
- Size: 1 byte
- Type: UINT8

**Values:**
| Value | Unit |
|-------|------|
| 0x00 | Celsius |
| 0x01 | Fahrenheit |

---

## Liquid Level

**UUID:** `fc540005-236c-4c94-8fa9-944a3e5353fa`

**Methods:** READ

**Data Format:**
- Size: 1 byte

**Values:**
| Value | Meaning |
|-------|---------|
| 0 | Empty |
| 30 | Not Empty |

**Note:** This value only updates when the device is charging. The sensor is too coarse for intermediate values.

---

## Time/Date/Zone

**UUID:** `fc540006-236c-4c94-8fa9-944a3e5353fa`

**Methods:** WRITE

**Data Format:**
- Size: 5 bytes

| Byte | Value |
|------|-------|
| 0-3 | Unix timestamp (UINT32 Little Endian) |
| 4 | Timezone offset (e.g., 3 for GMT+03) |

**Example:**
```
B9 F0 98 63 03
├─────────┘ └── Timezone: GMT+03
└── Timestamp: 0x6398F0B9 = Dec 13, 2022
```

---

## Battery

**UUID:** `fc540007-236c-4c94-8fa9-944a3e5353fa`

**Methods:** READ

**Data Format:**
- Size: 5 bytes

| Byte | Value |
|------|-------|
| 0 | Battery percentage (5-100, not scaled) |
| 1 | Charging status (1=plugged in, 0=unplugged) |
| 2-3 | Battery temperature (UINT16 LE, same encoding as other temps) |
| 4 | (Legacy) Battery voltage - not set by device |

**Example:**
```
4F 00 1C 0C 00
│  │  ├───┘ └── Voltage (unused)
│  │  └── Battery temp: 0x0C1C = 3100 -> 31.00°C
│  └── Not charging
└── 79% battery
```

---

## Liquid State

**UUID:** `fc540008-236c-4c94-8fa9-944a3e5353fa`

**Methods:** READ

**Data Format:**
- Size: 1 byte

**Values:**
| Value | State | Description |
|-------|-------|-------------|
| 0x01 | Empty | Mug is empty |
| 0x02 | Filling | Liquid is being poured |
| 0x03 | Unknown | Device emits but app doesn't register |
| 0x04 | Cooling | Liquid cooling toward target |
| 0x05 | Heating | Liquid being heated toward target |
| 0x06 | Stable | At target temperature |

---

## OTA (Firmware Info)

**UUID:** `fc54000C-236c-4c94-8fa9-944a3e5353fa`

**Methods:** READ

**Data Format:**
- Size: 4-6 bytes

| Byte | Value |
|------|-------|
| 0-1 | Firmware version (UINT16 LE) |
| 2-3 | Hardware version (UINT16 LE) |
| 4-5 | Bootloader version (UINT16 LE, optional) |

**Example:**
```
8A 01 0A 00
├───┘ └───┴── Hardware: 0x000A = 10
└── Firmware: 0x018A = 394
```

---

## Push Events

**UUID:** `fc540012-236c-4c94-8fa9-944a3e5353fa`

**Methods:** READ, NOTIFICATION

**Data Format:**
- Size: 1 byte

See [push-events.md](./push-events.md) for event codes.

**Note:** Enable notifications by writing `0x01 0x00` to the CCCD.

---

## Mug Color

**UUID:** `fc540014-236c-4c94-8fa9-944a3e5353fa`

**Methods:** READ, WRITE

**Data Format:**
- Size: 4 bytes
- Type: RGBA color

| Byte | Channel |
|------|---------|
| 0 | Red |
| 1 | Green |
| 2 | Blue |
| 3 | Alpha |

**Example:**
```
BF FF 00 FF  ->  Lime green (R=191, G=255, B=0, A=255)
```
