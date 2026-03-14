# Data Formats

## Temperature Encoding

All temperatures in the Ember Mug protocol use the same encoding:

- **Type:** UINT16 Little Endian
- **Unit:** Centidegrees Celsius
- **Conversion:** `raw_value * 0.01 = temperature_in_celsius`

### Examples

| Raw Bytes (LE) | Decimal | Temperature |
|----------------|---------|-------------|
| `AE 15` | 5550 | 55.50°C |
| `88 13` | 5000 | 50.00°C |
| `6A 18` | 6250 | 62.50°C |
| `00 00` | 0 | Heater off |

### Converting to/from Celsius

```c
// Read temperature
float celsius = raw_uint16 * 0.01f;

// Write temperature
uint16_t raw = (uint16_t)(celsius * 100.0f);
```

### Temperature Ranges

| Unit | Minimum | Maximum |
|------|---------|---------|
| Celsius | 50.0°C | 62.5°C |
| Fahrenheit | 120.0°F | 145.0°F |

---

## Battery Data Format

**Size:** 5 bytes

| Byte | Type | Description |
|------|------|-------------|
| 0 | UINT8 | Battery percentage (5-100) |
| 1 | UINT8 | Charging state (0=not charging, 1=charging) |
| 2-3 | UINT16 LE | Battery temperature (same encoding as other temps) |
| 4 | UINT8 | Legacy voltage field (unused, always 0) |

### Example

```
4F 00 1C 0C 00
│  │  │     └── Unused
│  │  └──────── Battery temp: 0x0C1C = 3100 -> 31.00°C
│  └─────────── Charging: 0 (not charging)
└────────────── Battery: 79%
```

---

## Mug Color (RGBA)

**Size:** 4 bytes

| Byte | Channel |
|------|---------|
| 0 | Red (0-255) |
| 1 | Green (0-255) |
| 2 | Blue (0-255) |
| 3 | Alpha (0-255) |

### Examples

| Bytes | Color |
|-------|-------|
| `FF 00 00 FF` | Red |
| `00 FF 00 FF` | Green |
| `00 00 FF FF` | Blue |
| `BF FF 00 FF` | Lime (R=191, G=255, B=0) |

---

## Time/Timezone Format

**Size:** 5 bytes

| Byte | Type | Description |
|------|------|-------------|
| 0-3 | UINT32 LE | Unix timestamp (seconds since epoch) |
| 4 | INT8 | Timezone offset from UTC (hours) |

### Example

```
B9 F0 98 63 03
├──────────┘ └── Timezone: +3 (GMT+03)
└── Timestamp: 0x6398F0B9 = 1670967481
              = Tue Dec 13 2022 21:31:21 UTC
```

### Code Example

```c
// Construct time data
uint8_t data[5];
uint32_t timestamp = (uint32_t)time(NULL);
int8_t tz_offset = 3; // GMT+03

memcpy(data, &timestamp, 4); // Already LE on most platforms
data[4] = (uint8_t)tz_offset;
```

---

## Mug Name

**Size:** Up to 14 bytes

**Type:** ASCII string (no spaces allowed)

### Example

```
45 6D 62 65 72 4D 75 67
E  m  b  e  r  M  u  g
```

---

## Liquid Level

**Size:** 1 byte

| Value | Meaning |
|-------|---------|
| 0 | Empty |
| 30 | Not empty |

**Note:** Only updates when mug is on the charging coaster. The sensor cannot detect intermediate fill levels.

---

## Liquid State

**Size:** 1 byte

| Value | State |
|-------|-------|
| 0x01 | Empty |
| 0x02 | Filling |
| 0x03 | Unknown (Cold - not registered by app) |
| 0x04 | Cooling |
| 0x05 | Heating |
| 0x06 | Stable (at target temperature) |

---

## OTA/Firmware Info

**Size:** 4-6 bytes

| Byte | Type | Description |
|------|------|-------------|
| 0-1 | UINT16 LE | Firmware version |
| 2-3 | UINT16 LE | Hardware version |
| 4-5 | UINT16 LE | Bootloader version (optional) |

### Example

```
8A 01 0A 00
│     │
│     └── Hardware: 0x000A = 10
└──────── Firmware: 0x018A = 394
```

Missing bootloader version defaults to 0.

---

## Byte Order

All multi-byte integers in the Ember Mug protocol use **Little Endian** byte order.

```
Value: 0x1234
Bytes: [0x34, 0x12]  (low byte first)
```
