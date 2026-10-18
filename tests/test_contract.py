import json
import re
import unittest
from pathlib import Path
import yaml

ROOT=Path(__file__).resolve().parents[1]

class Contract(unittest.TestCase):
    def setUp(self):
        self.manifest=yaml.safe_load((ROOT/'tessera-plugin.yaml').read_text())
        self.package=yaml.safe_load((ROOT/'plugin.yaml').read_text())
    def test_mapping_is_chosen_on_each_tile(self):
        self.assertNotIn('inputs',self.manifest)
        self.assertNotIn('fetch',self.manifest)
        self.assertNotIn('permissions',self.manifest)
        tile=self.manifest['tiles'][0]
        self.assertNotIn('domains',tile)
        options=tile['options']
        self.assertEqual(['grid','ac_load','critical_load','battery','solar'],[o['id'] for o in options])
        self.assertTrue(all(o['kind']=='entity' and o['domains']==['sensor'] for o in options))
        self.assertTrue(all(o['attributes']==['unit_of_measurement'] for o in options))
        self.assertNotIn('text_sensor',self.package)
        self.assertNotIn('defaults',self.package)
    def test_5x5_page_grid_range_is_supported(self):
        self.assertEqual('5x5',self.manifest['tiles'][0]['sizes']['max'])
        preview=yaml.safe_load((ROOT/'preview.yaml').read_text())
        self.assertIn('5x4',preview['tiles'][0]['sizes'])
    def test_plugin_has_no_screen_settings(self):
        self.assertNotIn('switch',self.package)
        self.assertNotIn('number',self.package)
        self.assertNotIn('settings',self.manifest)
    def test_translations_are_complete_in_both_languages(self):
        en=json.loads((ROOT/'translations/en.json').read_text(encoding='utf-8'))
        nl=json.loads((ROOT/'translations/nl.json').read_text(encoding='utf-8'))
        self.assertEqual(set(en['screen']),set(nl['screen']))
        self.assertEqual(set(en['app']),set(nl['app']))
        code='\n'.join(p.read_text() for p in (ROOT/'components/victron_vrm').glob('*') if p.suffix in ('.h','.cpp'))
        for key in re.findall(r'(?:plugin_->)?text\("([a-z_]+)"\)',code):
            self.assertIn(key,en['screen'])
    def test_preview_uses_synthetic_home_assistant_states(self):
        preview=yaml.safe_load((ROOT/'preview.yaml').read_text())
        self.assertNotIn('answers',preview)
        tile=preview['tiles'][0]
        self.assertEqual(set(tile['options'].values()),set(preview['states']))
        self.assertEqual('78.5',preview['states'][tile['options']['battery']]['state'])
    def test_plugin_does_not_take_over_network_or_core(self):
        for forbidden in ('api','wifi','ota','esphome','mqtt','http_request','packages','external_components'):
            self.assertNotIn(forbidden,self.package)
        self.assertEqual('0.15',self.manifest['api'])
        self.assertEqual('vajpeters',self.manifest['maintainer'])

if __name__=='__main__': unittest.main()
