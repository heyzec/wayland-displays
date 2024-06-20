## Use cases

### Update display configuration on event

- Precondition
  - `way-displays` is running as a daemon.
- Triggers
  - **Hardware Events:**
    - A monitor is plugged in or unplugged.
    - Laptop lid is opened or closed.
- Actions
  - `way-displays` to update the display configuration accordingly.

### Update display configuration manually

- Precondition
  - `way-displays` is running as a daemon.
- Triggers
  - User starts the GUI and makes changes
- Actions
  - `way-displays` to update the display configuration accordingly.

## Type of Changes

### Position: Arrange
- **Details:** Arrange by row/column, align by bottom/middle/top/left/right.
- **Reason:** Different physical configurations, e.g., a laptop will be lower than an external monitor.
- **Context:** [GitHub Issue #8](https://github.com/alex-courtis/way-displays/issues/8)

### Position: Order
- **Problem:** 
  - Multiple display setups, including sane configuration for unseen setups, e.g., external monitor above laptop at home, but might want to connect to home TV or for presentations. We assume such unknown display will usually be physically above laptop.

- **Solution:**
  - Use regex with `ORDER` preferred over `ORDER_LAST`.
  - Example:
    ```yaml
    ORDER:
    - *
    - eDP-1
    ```
  - **Context:** [GitHub Issue #46](https://github.com/alex-courtis/way-displays/issues/46), [Reddit Discussion](https://www.reddit.com/r/swaywm/comments/ruhc0t/comment/hqzkeng/?utm_source=reddit&utm_medium=web2x&context=3)

### Relative Positioning
- **Details:** Handling mixed row and column setups.
- **Context:** [GitHub Issue #158](https://github.com/alex-courtis/way-displays/issues/158), [GitHub Issue #79](https://github.com/alex-courtis/way-displays/issues/79)

### Scaling
- **Problem:** Scaling factor of 0.875 is too small.
- **Context:** [GitHub Issue #135](https://github.com/alex-courtis/way-displays/issues/135)

### Non-Display Changes
- **Problem:** Ability to run shell scripts on display change, e.g., set desktop background.
- **Solution:** Use `ON_CHANGE_CMD`.
- **Context:** [GitHub Issue #65](https://github.com/alex-courtis/way-displays/issues/65)

### Variable Refresh Rate (VRR)
- **Problem:** Per-monitor VRR enabling; some monitors have faulty VRR.
- **Solution:** `VRR_OFF`.
- **Context:** [GitHub Issue #70](https://github.com/alex-courtis/way-displays/issues/70)

### Profiles/Overlays
- **Problem:** Minimize manual intervention, only edit config when adding a new novel station that does not conform to existing layout
- **Solution:** None yet. Suggested kanshi-like profiles.
  ```yaml
  PROFILES:
    a-profile-with-no-match-rule-always-applies:
      ARRANGE: COL # Default: unknown displays go above the laptop screen, e.g., for projectors
      ALIGN: MIDDLE
      ORDER:
        - '!.*$'
        - 'eDP-1' # eDP-1 defaults to last
    horizontal-laptop-right:
      MATCH:
        - '!(^Sharp Corporation 0x148D$)|(^Dell)' # Apply this profile when either display is present
      ARRANGE: ROW
      ALIGN: BOTTOM
    hackerspace:
      MATCH:
        - 'Sharp Corporation 0x148D'
        - 'Monitor Maker ABC123' # Apply this profile when both displays are present
      ORDER:
        - 'eDP-1' # In hackerspace, laptop is on the left
        - '!^Sharp'
        - '!^Monitor Maker'
  ```
- **Context:** [GitHub Issue #83](https://github.com/alex-courtis/way-displays/issues/83)

---

## Design Decisions and Gotchas

1. **Preferred Mode:** Stick to the safety of the preferred mode. 
   - **Context:** [GitHub Issue #6](https://github.com/alex-courtis/way-displays/issues/6)
2. **Substring Issue:** `DP-1` is a substring of `eDP-1`.
   - **Context:** [GitHub Issue #60](https://github.com/alex-courtis/way-displays/issues/60)
3. **Laptop Lid Close Behavior:** Some users do not want the laptop to disable on lid close.
   - **Context:** [GitHub Issue #114](https://github.com/alex-courtis/way-displays/issues/114)
4. **Libinput Dependency:** Avoid requiring libinput.
   - **Context:** [GitHub Issue #159](https://github.com/alex-courtis/way-displays/issues/159)
5. **Single Monitor ON:** Only one monitor should be ON at a time.
   - **Context:** [GitHub Issue #169](https://github.com/alex-courtis/way-displays/issues/169)

