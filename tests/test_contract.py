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
    def test_mapping_is_user_owned_and_scoped(self):
        inputs=self.manifest['inputs']
        self.assertEqual(8,len(inputs))
        self.assertTrue(all(i['kind']=='entity' and i['domains']==['sensor'] and i['scope']=='screen' for i in inputs))
        self.assertNotIn('fetch',self.manifest)
        self.assertNotIn('network',self.manifest['permissions'])
        self.assertEqual({'{'+i['id']+'}' for i in inputs},set(self.manifest['permissions']['read_entities']))
    def test_value_and_unit_subscribe_to_same_user_choice(self):
        sensors={s['id']:s for s in self.package['text_sensor']}
        self.assertEqual(15,len(sensors))
        for metric in ('soc','battery','solar','load','grid','voltage','current'):
            value,unit=sensors['vrm_'+metric+'_value'],sensors['vrm_'+metric+'_unit']
            self.assertEqual('${'+metric.upper()+'}',value['entity_id'])
            self.assertEqual(value['entity_id'],unit['entity_id'])
            self.assertEqual('unit_of_measurement',unit['attribute'])
            self.assertTrue(value['internal'] and unit['internal'])
    def test_sign_settings_are_persisted_and_exposed(self):
        self.assertEqual(2,len(self.package['switch']))
        for switch in self.package['switch']:
            self.assertEqual('RESTORE_DEFAULT_OFF',switch['restore_mode'])
            self.assertTrue(switch['optimistic'])
        self.assertEqual(3,len(self.manifest['settings']))
        self.assertTrue(self.package['number'][0]['restore_value'])
    def test_translations_are_complete_in_both_languages(self):
        en=json.loads((ROOT/'translations/en.json').read_text(encoding='utf-8'))
        nl=json.loads((ROOT/'translations/nl.json').read_text(encoding='utf-8'))
        self.assertEqual(set(en['screen']),set(nl['screen']))
        self.assertEqual(set(en['app']),set(nl['app']))
        code=(ROOT/'components/victron_vrm/victron_vrm.cpp').read_text()
        for key in re.findall(r'(?:plugin_->)?text\("([a-z_]+)"\)',code):
            self.assertIn(key,en['screen'])
    def test_preview_uses_synthetic_home_assistant_states(self):
        preview=yaml.safe_load((ROOT/'preview.yaml').read_text())
        self.assertNotIn('answers',preview)
        self.assertEqual(8,len(preview['states']))
        for key in self.package['defaults'].values(): self.assertIn(key,preview['states'])
    def test_plugin_does_not_take_over_network_or_core(self):
        for forbidden in ('api','wifi','ota','esphome','mqtt','http_request','packages','external_components'):
            self.assertNotIn(forbidden,self.package)
        self.assertEqual('0.9',self.manifest['api'])
        self.assertEqual('vajpeters',self.manifest['maintainer'])

if __name__=='__main__': unittest.main()
