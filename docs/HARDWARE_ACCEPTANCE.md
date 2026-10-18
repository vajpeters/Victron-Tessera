# Hardware acceptance, not yet performed

First target: the owner's 10.1-inch panel, single-phase Victron installation. No hardware has been flashed by this task.

1. Install from the repository link, choose the real HA sensors and build for the intended screen.
2. Compare each displayed reading with HA; check W and kW, genuine zero solar at night, charge/discharge and grid signs.
3. Verify a large tile, its details, long tile names, light/dark mode and the normal page navigation.
4. Toggle each sign setting; verify its stored value survives a restart.
5. Change a sensor mapping, rebuild and verify the new entity is used.
6. Make a mapped sensor unavailable, restore it and verify recovery without a reboot.
7. Omit source time, then test a real, old and future source time. Disconnect HA and inspect the old-data indication.
8. Switch pages repeatedly, open/close details, enter standby and resume. Check memory and flash margins on the board.

Smaller screens and three-phase source totals need separate acceptance. No automatic firmware upload is configured.
