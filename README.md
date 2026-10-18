# Victron Tessera

A native monitoring dashboard for [Tessera](https://github.com/MaxGramser/homeassistant_espscreen).
Choose your own Home Assistant sensors in the plugin's setup screen. No VRM access token is needed by this plugin.
Your existing Victron integration supplies the data. This is an independent community project, not an official Victron product.

The first target is a 10.1-inch display with a single-phase installation. Smaller screens use the same responsive layout.
Three-phase calculation and per-phase views are reserved for a later version. You may map an existing, correct three-phase
total sensor, but this version does not calculate that total itself.

## Set up

1. Install a Victron integration in Home Assistant and check that its sensors work.
2. In Tessera open **Plugins > Add with a link** and use `https://github.com/vajpeters/Victron-Tessera`.
3. Choose your screen. Under its plugin setup choose the Home Assistant sensors for battery percentage, battery power,
   total solar power, AC consumption and grid power. Grid power is optional on off-grid installations.
4. Optionally choose battery voltage, battery current and a source timestamp sensor. Leave unused fields unselected.
5. Choose **Save and build**. Sensor mappings are ESPHome subscriptions, so changing a mapping rebuilds that screen.
6. Add **Victron dashboard** to the layout. On a 10.1-inch screen start with a large tile (for example 5x4 where the grid allows it).
7. Tap the tile for details. Small tiles show fewer readings; the detail card uses the available screen space.
8. If needed, change **Battery discharge is positive** and **Grid export is positive** in the plugin's settings.
   These switches take effect immediately and are also on the screen's plugin settings page.

Inputs are configured per screen. Every tile of this plugin on that screen uses that screen's mapping.
The default `sensor.victron_vrm_unassigned_*` names are deliberately unassigned placeholders, not automatically discovered sensors.

## Sensor contract

| Reading | Accepted Home Assistant unit | Meaning |
|---|---|---|
| Battery percentage | `%` | A number from 0 to 100 |
| Battery power | `W`, `kW`, `MW` | Positive charging, negative discharging by default |
| Solar power | `W`, `kW`, `MW` | Nonnegative total of the solar sources you want to include |
| AC consumption | `W`, `kW`, `MW` | Nonnegative AC load power, not cumulative energy |
| Grid power | `W`, `kW`, `MW` | Positive import, negative export by default |
| Battery voltage | `V`, `mV` | Optional, nonnegative |
| Battery current | `A`, `mA` | Optional, signed as provided by the selected sensor |
| Source timestamp | State in integer epoch seconds | Optional, timestamps the measurement set |

The plugin reads `unit_of_measurement` and normalizes power to watts. `kWh`, `VA`, missing units, unknown,
unavailable and nonnumeric values are not treated as watts or zero. A real zero is valid.
Battery charge words use a 30 W dead band; this is a display classification, not the inverter's operating state.
Reversing battery power does not reverse the separately mapped battery current sensor.

Use an existing aggregate sensor, or create a Home Assistant helper/template if you want to add solar sources.
Choose the correct totals yourself: the plugin never infers consumption from an energy balance and never adds phases.
No inverter/ESS/relay/generator controls are exposed.

## Freshness

Without a source timestamp, **Age unknown** is intentional. A changed HA state or an active connection does not
prove when the underlying Victron measurement was taken. Do not use a clock sensor as the source timestamp.
The optional timestamp must apply to the chosen measurement set. Source age is not validated separately per sensor.
Timestamp input accepts epoch seconds only; ISO text and milliseconds are rejected.

With a timestamp, values older than the configured age (15 minutes by default), or over five minutes in the future,
show **Old data**. A missing screen clock is identified separately. With no native API clients connected the last
known readings are also marked old. A connected API client is not necessarily Home Assistant, hence no freshness
claim is based on that connection alone. Unavailable HA states clear the corresponding reading to `--`.

## Compatibility and development

The manifest requires plugin API 0.9 (the block tone helper), ESPHome 2026.6.2 or later, and a Tessera version that
supports those. The verification target is Tessera 0.4.89 / ESPHome 2026.9.0. Supported boards are not hardware-certified
until tested. The 8192-byte tile budget and 24 KB flash estimate are conservative reservations, not measured chip usage.

See [Development](docs/DEVELOPMENT.md), [Hardware acceptance](docs/HARDWARE_ACCEPTANCE.md) and [Architecture](docs/ARCHITECTURE.md).
Builds and previews run in GitHub Actions. A successful host preview is not a successful upload or physical touch test.

## Privacy

This plugin reads only the Home Assistant entities you select, through the screen's existing native API connection.
It opens no network service, calls no external API and stores no cloud credentials. Your Victron integration remains
responsible for any VRM connection and its credentials. Sensor IDs are included in the selected screen's firmware.

## License

MIT. Based on the Tessera plugin scaffold, copyright 2026 Max Gramser. The Tessera firmware has its own AGPL license.
