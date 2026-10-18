# Development

In SourceTree choose **New > Add Existing Local Repository** (or the **Add** tab), then select the repository folder.
Review changes and use ordinary commits and pushes. No custom Git hooks, rebasing or forced pushes are needed.
The plugin lives at the repository root so Tessera can install the plain GitHub URL.
See [Atlassian's instructions](https://support.atlassian.com/sourcetree/kb/add-an-existing-repository-into-sourcetree/).

## Verification

Clone `https://github.com/MaxGramser/tessera-plugins` beside this repository and read its AGENTS.md.
Clone `https://github.com/MaxGramser/homeassistant_espscreen` to `../tessera-core` for the test and preview target.
The CI workflow pins the verified commits. Use Python 3.12 with PyYAML, aiohttp and Pillow.

```sh
python -m unittest discover -s tests -v
g++ -std=c++17 -Wall -Wextra -Werror -pedantic tests/model_test.cpp -o model_test
./model_test
TESSERA_MANIFEST=../tessera-core/screen_manager/app/plugin_manifest.py TESSERA_BOARDS=../tessera-core/screen_manager/app/boards.json python ../tessera-plugins/tools/check.py .
python ../tessera-plugins/tools/preview.py --setup
TESSERA_TREE=../tessera-core python ../tessera-plugins/tools/preview.py . --strict
python ../tessera-plugins/tools/store_image.py . --from .preview
```

The native tests use the actual model header, not a second implementation. Python tests validate manifest/package
mapping, source metadata and the absence of cloud access. The official preview compiles the whole plugin with Tessera.
Its first run may take 20 minutes or more. Windows needs WSL for that host preview; GitHub Actions uses Ubuntu.
Inspect `.preview/summary.json`, the actual tile and card PNGs and `store.png`. Never substitute a hand-drawn mockup.

For repeatable edge cases change the synthetic `states` in `preview.yaml`: solar 0; battery positive/negative; net
positive/negative/0; state unavailable; missing unit; old/absent timestamp. Do not commit real entity IDs or captures.
Generated previews, binaries, logs and build caches are ignored; only the reviewed store image is shipped.

## Reference-board firmware build

The separate **Reference 10.1-inch firmware** workflow is started manually in GitHub Actions. It compiles the plugin
with the pinned Tessera checkout for a Guition JC8012P4A1, a 10.1-inch ESP32-P4 reference board. This does not identify
the owner's exact panel model and does not flash any device. Its profile uses known throwaway credentials and must
never be installed. Only its compiler log is uploaded, not a deployable firmware image.

`python tools/firmware_config.py --core ../tessera-core` makes that ignored profile under `.build/`.
Compile with `esphome compile .build/jc8012p4a1.yaml`. Do not use `esphome run` or an upload command on this profile.
