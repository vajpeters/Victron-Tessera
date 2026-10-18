# Victron Tessera

A native monitoring dashboard for [Tessera](https://github.com/MaxGramser/homeassistant_espscreen).
The Home Assistant source is selected when you add or edit the tile, so changing it does not rebuild the screen firmware.
No VRM access token is needed by this plugin. This is an independent community project, not an official Victron product.

The first target is a 10.1-inch display with a single-phase installation. Large tiles and detail cards use a Victron-style schematic: Grid, AC Loads and Critical Loads above a central inverter, with Battery and Solar below. The tile works on Tessera's 5x5 page grid and keeps a compact readings layout at smaller sizes.
Three-phase calculation and per-phase views are reserved for a later version. You can supply an existing correct three-phase total, but the plugin does not calculate that total itself.

![10.1-inch firmware preview with synthetic measurements](docs/images/dashboard-light.png)

This is the actual compiled detail card with synthetic Home Assistant measurements. [Dark preview](docs/images/dashboard-dark.png).

## Set up

1. Install a Victron integration in Home Assistant and check that its sensors work.
2. Create one Home Assistant template sensor that bundles the readings as described below.
3. In Tessera open **Plugins > Add with a link** and use `https://github.com/vajpeters/Victron-Tessera`.
4. Choose your screen and select **Save and build**. No sensor IDs are requested or compiled into the firmware.
5. Add **Victron dashboard** to the layout. In the tile inspector select your dashboard sensor.
6. Select any standard Tessera background colour in the same inspector. The colour fills the backdrop and softly tints the Victron panels.
7. For the 10.1-inch landscape screen, use 5x4. Tessera names an exact 5x5 page-sized tile `full`, which its plugin editor does not offer yet; tap the 5x4 tile for the full detail card.

Every tile can select a different dashboard sensor. Changing the selected entity or background colour is a layout change and does not require a firmware rebuild.

## Home Assistant dashboard sensor

Tessera currently supports one entity selector for a plugin tile. The selected sensor therefore carries battery percentage in its state and the other source sensors in attributes. Add this example to Home Assistant's `configuration.yaml`, replace the example entity IDs with yours, and reload template entities:

```yaml
template:
  - sensor:
      - name: Victron dashboard
        unique_id: victron_tessera_dashboard
        unit_of_measurement: "%"
        state: "{{ states('sensor.victron_battery_soc') }}"
        attributes:
          battery_power: "{{ states('sensor.victron_battery_power') }}"
          battery_power_unit: "{{ state_attr('sensor.victron_battery_power', 'unit_of_measurement') }}"
          solar_power: "{{ states('sensor.victron_solar_power') }}"
          solar_power_unit: "{{ state_attr('sensor.victron_solar_power', 'unit_of_measurement') }}"
          load_power: "{{ states('sensor.victron_ac_load') }}"
          load_power_unit: "{{ state_attr('sensor.victron_ac_load', 'unit_of_measurement') }}"
          grid_power: "{{ states('sensor.victron_grid_power') }}"
          grid_power_unit: "{{ state_attr('sensor.victron_grid_power', 'unit_of_measurement') }}"
          battery_voltage: "{{ states('sensor.victron_battery_voltage') }}"
          battery_voltage_unit: "{{ state_attr('sensor.victron_battery_voltage', 'unit_of_measurement') }}"
          battery_current: "{{ states('sensor.victron_battery_current') }}"
          battery_current_unit: "{{ state_attr('sensor.victron_battery_current', 'unit_of_measurement') }}"
          critical_load_power: "{{ states('sensor.victron_critical_load') }}"
          critical_load_power_unit: "{{ state_attr('sensor.victron_critical_load', 'unit_of_measurement') }}"
          source_time: "{{ state_attr('sensor.victron_battery_soc', 'victron_timestamp') }}"
```

`battery_voltage`, `battery_current`, `critical_load_power` and their unit attributes are optional. `source_time` is optional too; omit it unless your source exposes the real measurement time as integer epoch seconds. The four power attributes in the example are required so Tessera can distinguish this aggregate sensor from ordinary sensors in the tile picker.

## Sensor contract

| Value | Place | Accepted unit | Meaning |
|---|---|---|---|
| Battery percentage | Sensor state | `%` | Number from 0 to 100 |
| Battery power | `battery_power` | `W`, `kW`, `MW` | Positive charging, negative discharging |
| Solar power | `solar_power` | `W`, `kW`, `MW` | Nonnegative total solar power |
| AC consumption | `load_power` | `W`, `kW`, `MW` | Nonnegative AC load power, not cumulative energy |
| Grid power | `grid_power` | `W`, `kW`, `MW` | Positive import, negative export |
| Critical Loads | `critical_load_power` | `W`, `kW`, `MW` | Optional protected-load power; never inferred from AC Loads |
| Battery voltage | `battery_voltage` | `V`, `mV` | Optional, nonnegative |
| Battery current | `battery_current` | `A`, `mA` | Optional, signed as provided |
| Source timestamp | `source_time` | epoch seconds | Optional time for the complete measurement set |

Each reading's unit is supplied by its matching `*_unit` attribute. The plugin normalizes power to watts. `kWh`, `VA`, missing units, unknown, unavailable and nonnumeric values are not treated as watts or zero. A real zero is valid. Battery charge words use a 30 W dead band. Reversing battery power does not reverse battery current.

The plugin never infers consumption from an energy balance, adds phases or controls Victron equipment. The central inverter is a diagram symbol, not a measured operating state. Direction arrows appear only for valid, fresh measurements above 1 W. The reference screenshot's history graphs, phase rows, individual MPPT details, time-to-go and external-control status require additional data and are not fabricated.

## Freshness

`source_time` must be the real source measurement time for the complete set. Without it, **Age unknown** is intentional. Do not substitute the current clock time. Values older than the configured age (15 minutes by default), or over five minutes in the future, show **Old data**. ISO text and milliseconds are rejected.

## Compatibility and development

Use Tessera 0.4.88 (screen firmware 0.56.0) or newer. That release moved plugin tile data to PSRAM and prevents restarts when internal memory is busy. The manifest requires plugin API 0.9 and ESPHome 2026.6.2 or later; the verification target is Tessera 0.4.89 / ESPHome 2026.9.0. Supported boards are not hardware-certified until tested. The tile reserves 768 bytes in Tessera's layout budget and estimates 24 KB of flash.

See [Development](docs/DEVELOPMENT.md), [Hardware acceptance](docs/HARDWARE_ACCEPTANCE.md) and [Architecture](docs/ARCHITECTURE.md).
Builds and previews run in GitHub Actions. A successful host preview is not a successful upload or physical touch test.

## Privacy

The plugin reads only the Home Assistant dashboard sensor selected on each tile through Tessera's existing connection. It opens no network service, calls no external API and stores no cloud credentials. Your Victron integration remains responsible for any VRM connection and credentials. Sensor IDs are stored in the Tessera layout rather than compiled into firmware.

## License

MIT. Based on the Tessera plugin scaffold, copyright 2026 Max Gramser. The Tessera firmware has its own AGPL license.

See [validation results](docs/VALIDATION.md) for the exact tested code, screen sizes and hardware limits.
