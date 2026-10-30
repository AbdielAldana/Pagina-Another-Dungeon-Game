"""Catalog regression checks; run with python -B tools/test_import_room.py."""
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('import_room', ROOT / 'tools/import_room.py')
IMPORTER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(IMPORTER)


def room(room_id='test-room', enemies=1, decorations=1):
    objects = []
    for index in range(decorations):
        objects.append(dict(id=f'table-{index}', assetId='table', x=56 + index, y=96,
                            frame=0, rotation=0, flipX=False, solid=True,
                            hitbox=dict(x=3, y=0, width=26, height=8), role='decoration'))
    for index in range(enemies):
        objects.append(dict(id=f'enemy-{index}', assetId='skeleton', x=45, y=89,
                            frame=12, rotation=0, flipX=False, solid=False,
                            hitbox=dict(x=3, y=0, width=10, height=8), role='enemy'))
    objects.append(dict(id='door', assetId='door', x=232, y=107,
                        frame=0, rotation=180, flipX=False, solid=False,
                        hitbox=dict(x=0, y=24, width=16, height=8), role='door'))
    return dict(format='anotherdungeon-room', schemaVersion=1, id=room_id,
                name='Test', width=256, height=256, tileSize=16,
                coordinateSystem='top-left-pixels', background='room2',
                spawn=dict(x=127, y=192), tiles=[], collisions=[], objects=objects)


class CatalogTests(unittest.TestCase):
    def test_original_room_geometry_and_legacy_template(self):
        source = ROOT / 'rooms/mi_primer_cuarto.json'
        data = json.loads(source.read_text(encoding='utf-8-sig'))
        header = IMPORTER.generate(data)
        self.assertIn('{RoomTemplateId::IMPORTED,', header)
        self.assertEqual(IMPORTER.template_number(data['id']), 21)
        self.assertIn('Role::DECORATION, 73, 37, 32, 16', header)
        self.assertIn('Role::ENEMY, -75, -31, 16, 16, 12', header)
        self.assertIn('{-8, 8, 0, 8}', header)  # Preserve authored hitboxes.
        self.assertIn('Role::DOOR, -1, 72, 16, 32, 0, 270', header)

    def test_separate_room_data_and_sprite_capacity(self):
        header = IMPORTER.generate_catalog([('a.json', room('a', 1, 2)),
                                            ('b.json', room('b', 4, 8))])
        self.assertIn('room_count = 2;', header)
        self.assertIn('max_decoration_count = 8;', header)
        self.assertIn('max_enemy_count = 4;', header)
        self.assertIn('2, 1, enemies_0, -1,', header)
        self.assertIn('8, 4, enemies_1, -1,', header)
        self.assertIn('{objects_0, 4}', header)
        self.assertIn('{objects_1, 13}', header)

    def test_ids_do_not_depend_on_name_or_other_rooms(self):
        identity = IMPORTER.template_number('stable')
        self.assertGreater(identity, 21)
        alone = IMPORTER.generate_catalog([('old.json', room('stable'))])
        with_other = IMPORTER.generate_catalog([('new.json', room('stable')),
                                               ('first.json', room('aaa'))])
        for header in (alone, with_other):
            self.assertIn(f'static_cast<RoomTemplateId>({identity}), "stable"', header)

    def test_output_order_independent_of_filenames(self):
        first = IMPORTER.generate_catalog([('a.json', room('b')), ('b.json', room('a'))])
        second = IMPORTER.generate_catalog([('renamed.json', room('a')), ('z.json', room('b'))])
        self.assertEqual(first, second)

    def test_duplicate_ids_rejected_with_filename(self):
        with self.assertRaisesRegex(ValueError, r'b.json: Duplicate room id'):
            IMPORTER.generate_catalog([('a.json', room()), ('b.json', room())])

    def test_hash_collisions_rejected(self):
        with patch.object(IMPORTER, 'template_number', return_value=12345):
            with self.assertRaisesRegex(ValueError, 'hash collision'):
                IMPORTER.generate_catalog([('a.json', room('a')), ('b.json', room('b'))])

    def test_reject_unsupported_or_invalid_input(self):
        variants = []
        for key, value in [('schemaVersion', 2), ('id', ''), ('width', 240),
                           ('background', 'unknown')]:
            data = room()
            data[key] = value
            variants.append(data)
        data = room()
        data['objects'][0]['assetId'] = 'unknown'
        variants.append(data)
        data = room()
        data['objects'][0]['hitbox']['width'] = 100
        variants.append(data)
        variants += [None, [], room(enemies=5), room(decorations=33)]
        for data in variants:
            with self.subTest(data=data):
                with self.assertRaisesRegex(ValueError, 'bad.json:'):
                    IMPORTER.generate_catalog([('bad.json', data)])

    def test_directory_add_rename_remove_and_ignore_non_json(self):
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            (folder / 'readme.txt').write_text('not a room')
            a = folder / 'a.json'
            a.write_text(json.dumps(room('a')), encoding='utf-8')
            original = IMPORTER.load_catalog(folder)
            b = folder / 'b.JSON'
            b.write_text(json.dumps(room('b', enemies=0)), encoding='utf-8')
            added = IMPORTER.load_catalog(folder)
            self.assertIn('room_count = 2;', added)
            b.rename(folder / 'renamed.json')
            self.assertEqual(added, IMPORTER.load_catalog(folder))
            (folder / 'renamed.json').unlink()
            self.assertEqual(original, IMPORTER.load_catalog(folder))

    def test_empty_catalog_has_safe_ranges_and_capacity(self):
        header = IMPORTER.generate_catalog([])
        self.assertIn('room_count = 0;', header)
        self.assertIn('max_decoration_count = 1;', header)
        self.assertIn('{empty_objects, 0}', header)
        self.assertNotIn('objects_0[]', header)
        self.assertIn('    empty_room,', header)

    def test_cli_does_not_overwrite_on_invalid_json_or_rewrite_unchanged(self):
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            sources = folder / 'rooms'
            sources.mkdir()
            (sources / 'a.json').write_text(json.dumps(room('a')), encoding='utf-8')
            output = folder / 'catalog.h'
            command = [sys.executable, '-B', str(ROOT / 'tools/import_room.py'), str(sources), str(output)]
            subprocess.run(command, check=True, capture_output=True)
            expected = output.read_bytes()
            stamp = output.stat().st_mtime_ns
            subprocess.run(command, check=True, capture_output=True)
            self.assertEqual(stamp, output.stat().st_mtime_ns)
            (sources / 'broken.json').write_text('{invalid', encoding='utf-8')
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('broken.json', result.stderr)
            self.assertEqual(expected, output.read_bytes())

    def test_cli_single_file_matches_directory(self):
        with tempfile.TemporaryDirectory() as temporary:
            folder = Path(temporary)
            source = folder / 'one.json'
            source.write_text(json.dumps(room()), encoding='utf-8-sig')
            self.assertEqual(IMPORTER.load_catalog(source), IMPORTER.load_catalog(folder))

    def test_utf8_name_is_valid_cpp_byte_literal(self):
        literal = IMPORTER.cpp_string('Habitaci\u00f3n "A" \\ \U0001f525')
        self.assertTrue(literal.isascii())
        self.assertNotIn('\\u', literal)
        self.assertIn('\\303\\263', literal)


if __name__ == '__main__':
    unittest.main()
