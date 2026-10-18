# Development validation for 0.3.0

Verified runtime commit: `7f5a7bf5fc04e26a7dfabf3f70d2cae4ce24dd14`.
Later documentation and image commits do not change the runtime.

- Eight Python contract/tooling tests passed; the native C++ model tests passed with warnings treated as errors.
- The official Tessera manifest/package check passed.
- [Full firmware preview verification](https://github.com/vajpeters/Victron-Tessera/actions/runs/39140505301) passed, with zero visual warnings and no missing Home Assistant fixture data.
- [Real ESP32-P4 reference firmware build](https://github.com/vajpeters/Victron-Tessera/actions/runs/39141261003) passed for Guition JC8012P4A1. This is a compile check, not identification or testing of the owner's panel.

| Preview target | Pixels | Layout |
|---|---|---|
| CYD | 320 x 240 | Compact readings |
| Guition | 480 x 480 | Compact readings |
| Waveshare 4.3 portrait | 480 x 800 | Compact readings |
| Guition JC8012P4A1 reference | 1280 x 800 | Victron-style large tile and detail card; compact smaller tiles |

The strict bench checks light/dark themes, tile sizes, taps, detail cards and settings. Informational findings only explain tile sizes that do not fit a small board's grid; they are not visual warnings. Actual PNGs were inspected, including the large schematic in both themes and small/portrait views. `store.png` was made by the official store-image tool from this successful run and inspected.

All previews use synthetic measurements. No live Home Assistant installation, physical display, touch controller or Victron equipment was exercised. No device was flashed, deployable firmware published or release created. The compile profile uses throwaway credentials and must never be installed. See [hardware acceptance](HARDWARE_ACCEPTANCE.md) for the remaining physical checks.

Large-card direction arrows require valid, fresh measurements and a genuine `source_time` attribute on the selected dashboard sensor. Without it, the view reports age unknown and retains static connection lines. History graphs, individual MPPT details and phase calculations are not implemented.

The primary JC8012P4A1 validation runs on its native 5x5 page grid. The largest landscape plugin span is 5x4 because Tessera reserves the exact 5x5 footprint for its named `full` size, which plugin tiles cannot request in the editor yet.
