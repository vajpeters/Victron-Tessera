"""Generate a compile-only reference-board profile, never a flash/upload command."""
import argparse
import base64
import json
import re
from pathlib import Path

parser=argparse.ArgumentParser()
parser.add_argument('--core',type=Path,required=True)
parser.add_argument('--board',default='jc8012p4a1',choices=['jc8012p4a1','jc8012p4a1v3','cyd'])
args=parser.parse_args()
core=args.core.resolve()
plugin=Path(__file__).resolve().parents[1]
entry=core/'checkout'/f'{args.board}.yaml'
if not entry.is_file(): raise SystemExit('Reference board checkout profile missing')
out=plugin/'.build';out.mkdir(exist_ok=True)
# Known throwaway values, only for compiler validation. Never deploy these.
secrets='\n'.join(f'{k}: {json.dumps(v)}' for k,v in {
    'wifi_ssid':'compile-only-do-not-install','wifi_password':'compile-only-do-not-install',
    'api_encryption_key':base64.b64encode(bytes(range(32))).decode(),
    'ota_password':'compile-only-do-not-install','ap_password':'compile-only-do-not-install'}.items())+'\n'
(out/'secrets.yaml').write_text(secrets)
def q(p): return json.dumps(str(p).replace('\\','/'))
# Checkout entries assume they are the root configuration. ESPHome resolves local
# component paths from the root file, so relocate those paths along with includes.
# The supported generated entries use only plain paths for these two fields.
base_lines=[]
for line in entry.read_text(encoding='utf-8').splitlines():
    include=re.fullmatch(r'(\s*[^#\n]+: !include )([^\s"\']+)', line)
    local_path=re.fullmatch(r'(\s+path: )([^\s"\']+)', line)
    match=include or local_path
    if match:
        line=match[1]+q((entry.parent/match[2]).resolve())
    elif '!include ' in line or re.match(r'\s+path:', line):
        raise SystemExit('Unsupported checkout entry path syntax')
    base_lines.append(line)
base=out/'base-board.yaml'
base.write_text('\n'.join(base_lines)+'\n', encoding='utf-8')
profile='\n'.join([
    'substitutions:', '  DEVICE_NAME: victron-tessera-compile', '  DEVICE_FRIENDLY_NAME: Victron compile only',
    '  FONT_DIR: '+q(core/'fonts'), 'packages:', '  tessera: !include '+q(base),
    '  victron: !include '+q(plugin/'plugin.yaml'), 'external_components:',
    '  - source:', '      type: local', '      path: '+q(plugin/'components'),
    '    components: [victron_vrm]', ''])
path=out/(args.board+'.yaml');path.write_text(profile)
print(path)
