# Changelog

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
