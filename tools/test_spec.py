"""Regression tests for registry validation, independent of generated codecs."""
import copy
import unittest
import yaml
import generate


class RegistryTests(unittest.TestCase):
    def setUp(self):
        self.data = copy.deepcopy(generate.validate())

    def rejected(self):
        with self.assertRaises(ValueError):
            generate.validate(self.data)

    def test_duplicate_ids(self):
        self.data['protocol']['fields'][1]['id'] = 1
        self.rejected()

    def test_unknown_required_field(self):
        self.data['protocol']['messages'][0]['required'].append(31)
        self.rejected()

    def test_optional_required_overlap(self):
        self.data['protocol']['messages'][0]['optional'].append(1)
        self.rejected()

    def test_wrong_integer_width(self):
        self.data['protocol']['fields'][3]['max_length'] = 7
        self.rejected()

    def test_capability_overflow(self):
        self.data['capabilities']['capabilities'][0]['id'] = 64
        self.rejected()

    def test_category_id_drift(self):
        self.data['events']['events'][1]['id'] = 42
        self.rejected()

    def test_reserved_message_implemented(self):
        self.data['events']['events'][1]['status'] = 'reserved'
        self.rejected()

    def test_yaml_duplicate_key(self):
        with self.assertRaises(ValueError):
            yaml.load('id: 1\nid: 2\n', Loader=generate.UniqueLoader)
