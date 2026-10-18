"""Run the actual upstream callback without importing its graphical bench dependencies."""
import ast
import importlib.util
import json
import os
from pathlib import Path
import tempfile
import types
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('fix_preview_tools', ROOT / 'tools' / 'fix_preview_tools.py')
fix = importlib.util.module_from_spec(spec)
spec.loader.exec_module(fix)


class PreviewSubscriptions(unittest.TestCase):
    def test_actual_callback_delivers_states_and_attributes(self):
        plugins = Path(os.environ.get('TESSERA_PLUGINS', ROOT.parent / 'tessera-plugins'))
        source = (plugins / 'tools' / 'bench.py').read_bytes()
        with tempfile.TemporaryDirectory() as directory:
            checkout = Path(directory)
            (checkout / 'tools').mkdir()
            target = checkout / 'tools' / 'bench.py'
            target.write_bytes(source)
            fix.patch(checkout)
            patched = target.read_bytes()
            self.assertFalse(fix.patch(checkout))
            self.assertEqual(target.read_bytes(), patched)
            tree = ast.parse(patched)
            bench = next(n for n in tree.body if isinstance(n, ast.ClassDef) and n.name == 'Bench')
            wants = next(n for n in bench.body if isinstance(n, ast.FunctionDef) and n.name == 'wants')
            namespace = {'json': json}
            exec(compile(ast.Module(body=[wants], type_ignores=[]), str(target), 'exec'), namespace)
            sent, findings = [], []
            receiver = types.SimpleNamespace(
                read=set(), states={'sensor.power': {'state': '1.2', 'attributes': {'unit_of_measurement': 'kW'}}},
                client=types.SimpleNamespace(send_home_assistant_state=lambda *args: sent.append(args)),
                find=lambda *args: findings.append(args))
            callback = types.MethodType(namespace['wants'], receiver)
            callback('sensor.power', '')
            callback('sensor.power', None)
            callback('sensor.power', 'unit_of_measurement')
            self.assertEqual(sent, [('sensor.power', '', '1.2'), ('sensor.power', None, '1.2'),
                                    ('sensor.power', 'unit_of_measurement', 'kW')])
            self.assertEqual(findings, [])
            callback('sensor.missing', '')
            self.assertEqual(len(findings), 1)
            self.assertEqual(findings[0][:2], ('info', 'Home Assistant'))
            self.assertIn('sensor.missing', findings[0][2])

    def test_unexpected_source_is_untouched(self):
        with tempfile.TemporaryDirectory() as directory:
            checkout = Path(directory)
            (checkout / 'tools').mkdir()
            target = checkout / 'tools' / 'bench.py'
            original = b'# unexpected upstream revision\r\n'
            target.write_bytes(original)
            with self.assertRaises(ValueError):
                fix.patch(checkout)
            self.assertEqual(target.read_bytes(), original)


if __name__ == '__main__':
    unittest.main()
