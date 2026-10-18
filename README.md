# Victron Tessera

A native Victron monitoring dashboard for [Tessera](https://github.com/MaxGramser/homeassistant_espscreen). It targets a 10.1-inch single-phase installation first and adapts to smaller tiles.

![10.1-inch firmware preview with synthetic measurements](docs/images/dashboard-light.png)

## Set up

1. Check that your Victron entities in Home Assistant expose the right values and a `unit_of_measurement`.
2. In Tessera open **Plugins > Add with a link** and enter `https://github.com/vajpeters/Victron-Tessera`.
3. Choose your screen and select **Save and build**. Sensor IDs are not requested or compiled into the firmware.
4. Add **Victron dashboard** to the layout.
5. In that tile's settings, separately choose the entities for **Grid**, **AC Loads**, **Critical Loads**, **Battery** and **Solar / PV**.
6. Select any standard Tessera background colour in the same inspector.

Entity choices and the background are stored with the tile. You can change them later without rebuilding the firmware, and each tile may use different sources.

Battery accepts either a state-of-charge sensor (`%`) or a power sensor (`W`, `kW` or `MW`). A percentage shows charge level; power shows charging or discharging. Critical Loads and Grid may be left empty. For a three-phase system, select existing total-power sensors; this version does not add phases itself.

## Layout and data

Large tiles and the detail card place Grid, AC Loads and Critical Loads above a central inverter, with Battery and Solar below. The plugin supports Tessera's 5x5 page grid. Use 5x4 on a landscape 10.1-inch screen; smaller tile sizes automatically use a compact readings layout.

Power units may be W, kW or MW. kWh, VA, unknown, unavailable and nonnumeric values are not treated as power or zero. Solar, AC Loads and Critical Loads must be nonnegative. Grid is positive for import and negative for export. Battery power is positive for charging and negative for discharging.

The plugin does not calculate phase totals, invent missing values or control Victron equipment. The inverter is a diagram symbol. Historical graphs and individual MPPT details are not fabricated.

## Compatibility

Version 0.4.0 requires Tessera plugin API 0.15, which adds Home Assistant entity choices to tile options. The tile reserves 768 bytes in Tessera's layout budget and uses PSRAM for runtime data on supported screens.

The plugin opens no network service and stores no VRM token. Home Assistant remains responsible for the Victron connection; only the selected entity IDs are stored in the Tessera layout.

See [Development](docs/DEVELOPMENT.md), [Hardware acceptance](docs/HARDWARE_ACCEPTANCE.md) and [Architecture](docs/ARCHITECTURE.md).

## License

MIT. Based on the Tessera plugin scaffold, copyright 2026 Max Gramser. Tessera firmware has its own AGPL license.
