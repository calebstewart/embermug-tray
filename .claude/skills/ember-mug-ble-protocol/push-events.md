# Push Events

**UUID:** `fc540012-236c-4c94-8fa9-944a3e5353fa`

Push events are notifications sent by the mug to prompt the application to refresh certain characteristics or handle state changes.

## Enabling Notifications

Write `0x01 0x00` (little-endian) to the Client Characteristic Configuration Descriptor (CCCD) to enable notifications.

## Event Codes

| Code | Event | Action |
|------|-------|--------|
| 0x01 | Refresh Battery | Read battery characteristic |
| 0x02 | Charging | Mug placed on charger |
| 0x03 | Not Charging | Mug removed from charger |
| 0x04 | Refresh Target Temp | Read target temperature characteristic |
| 0x05 | Refresh Current Temp | Read current temperature characteristic |
| 0x06 | (Not Implemented) | Reserved |
| 0x07 | Refresh Liquid Level | Read liquid level characteristic |
| 0x08 | Refresh Liquid State | Read liquid state characteristic |

## Event Handling

When receiving a push event, the application should:

1. **0x01 (Refresh Battery):** Read the battery characteristic to get updated battery level and charging state.

2. **0x02 (Charging):** The mug has been placed on the charging coaster. Update UI to show charging state.

3. **0x03 (Not Charging):** The mug has been removed from the charging coaster.

4. **0x04 (Refresh Target Temp):** The target temperature may have changed (e.g., via another app or the mug itself).

5. **0x05 (Refresh Current Temp):** The current liquid temperature has changed significantly.

6. **0x07 (Refresh Liquid Level):** The liquid level has changed (detected when on charger).

7. **0x08 (Refresh Liquid State):** The heating state has changed (empty, filling, cooling, heating, stable).

## Implementation Notes

- Events are single-byte notifications
- Multiple events may arrive in quick succession
- It's recommended to debounce rapid state changes
- The mug may send refresh events even if the value hasn't changed
