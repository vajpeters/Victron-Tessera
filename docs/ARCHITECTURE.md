# Architecture

User decision: Home Assistant owns sensor selection and any aggregation. Direct VRM diagnostics was investigated,
then dropped before delivery at the user's request. This package contains no fetch manifest or VRM authentication.

Tessera's eight entity inputs become ESPHome package substitutions. Seven internal HA text sensors read raw state;
seven companion text sensors read unit metadata, and an optional eighth entity supplies a source timestamp.
Text values preserve `unavailable` and reject formatted values instead of silently interpreting them as zero.
The C++ model validates and normalizes those readings. Tiles and detail cards take a fresh snapshot on each tick,
using the same model and formatting. No temporary JSON references or cross-tile global cache are retained.

Sign switches and the age limit are ordinary ESPHome config entities. There are no actions on Victron equipment.
The dashboard never draws a balanced energy-flow claim from independently mapped sensors.

References inspected: Tessera core c49854c8567fd079678445761a3a712736afc42b (0.4.89, plugin API 0.14),
tessera-plugins 69a0264063c9dc8fb105b833b19c26cb47a5a23e, official plugin manifest and firmware API.
Only API 0.9 features are required by this plugin. Upstream dependencies used for verification are pinned in CI.

Known limits: one mapping per screen; rebuild on mapping change; source time shared by all mapped readings;
no source-time claim when omitted; no per-phase calculation, daily kWh, alarms, inverter state or history in version 0.1.0.
The local machine has no configured WSL preview environment; Linux CI runs the official firmware bench.
