# Changelog

## 0.4.0
- Adds five separate, searchable Home Assistant entity selectors for Grid, AC Loads, Critical Loads, Battery and Solar / PV.
- Keeps every source in the tile configuration, so changing a sensor does not rebuild the firmware.
- Requires Tessera plugin API 0.15, which follows all selected option entities and sends their current states to the tile.

## 0.3.2
- Removes the battery-sign, grid-sign and maximum-measurement-age settings.
- Uses one fixed data contract: battery power is positive while charging, grid power is positive while importing, and timestamped measurements become old after 15 minutes.

## 0.3.1
- Prevents repeated dashboard redraws from exhausting runtime memory or tripping the display watchdog.
- Corrects the tile's layout-memory estimate; the large Victron view itself lives in PSRAM on supported screens.

## 0.3.0
- Choose the Home Assistant dashboard sensor while adding or editing each tile; changing it no longer rebuilds firmware.
- Supports Tessera's 5x5 page grid, including 5x4 landscape and 4x5 portrait spans.
- The standard Tessera background-colour picker now colours the dashboard backdrop and softly tints its Victron panels.
- Replaced the eight build-time sensor subscriptions with one runtime dashboard-sensor contract.

## 0.2.0
- Victron-style cards and flow connections on large tiles and detail cards.
- Optional, independently mapped Critical Loads sensor; existing AC load mapping is unchanged.
- Compact readings remain available on small tiles; no synthetic history or inverter status.
- Source time now uses optional `victron_timestamp` on the selected SOC sensor to fit Tessera's eight-input limit.
- Save and rebuild to add the new optional sensor subscription.

## 0.1.0
- User-selected Home Assistant sensors for a Victron dashboard.
- English and Dutch text, automatic power units and configurable sign conventions.
- Native overview and details with honest source-age reporting.
- First target: a 10.1-inch screen and a single-phase installation.
