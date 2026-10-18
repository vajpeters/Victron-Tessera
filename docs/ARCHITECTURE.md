# Architecture

User decision: Home Assistant owns sensor selection and any aggregation. Direct VRM diagnostics was investigated,
then dropped before delivery at the user's request. This package contains no fetch manifest or VRM authentication.

Each tile is bound at layout-edit time to one Home Assistant sensor. Its state is battery percentage and its named
attributes carry the other readings and their units. Tessera sends that selected entity to the screen whenever it
changes. There are no entity substitutions or Home Assistant sensor IDs in the firmware build.
Text values preserve `unavailable` and reject formatted values instead of silently interpreting them as zero.
Each tile and its detail card owns a validated C++ model made from its latest state message; no temporary JSON
references or cross-tile global cache are retained.

Sign switches and the age limit are ordinary ESPHome config entities. There are no actions on Victron equipment.
The dashboard never draws a balanced energy-flow claim from independently mapped sensors.

References inspected: Tessera core c49854c8567fd079678445761a3a712736afc42b (0.4.89, plugin API 0.14),
tessera-plugins 69a0264063c9dc8fb105b833b19c26cb47a5a23e, official plugin manifest and firmware API.
Only API 0.9 features are required by this plugin. Upstream dependencies used for verification are pinned in CI.

Known limits: Tessera currently offers one entity selector per plugin tile, so Home Assistant bundles the selected
source sensors into one dashboard sensor; source time is shared by all bundled readings;
no source-time claim when omitted; no per-phase calculation, daily kWh, alarms, inverter state or history in version 0.3.0.
The local machine has no configured WSL preview environment; Linux CI runs the official firmware bench.
