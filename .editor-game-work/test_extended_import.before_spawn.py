"""Additional room assets use their actual game roles and generated catalog data."""
import copy
import unittest

import import_room


def obj(asset, role, index, *, x=32, y=64, width=16, height=16, frame=0):
    return dict(id=f"{asset}-{index}", assetId=asset, role=role, x=x, y=y,
                frame=frame, rotation=0, flipX=False, solid=False,
                hitbox=dict(x=3, y=0, width=10, height=8))


def room():
    objects = [obj('box', 'decoration', 0)]
    objects += [obj('bat', 'enemy', 1, x=64, frame=5),
                obj('mushy', 'enemy', 2, x=80),
                obj('plant', 'enemy', 3, x=96),
                obj('spikes', 'hazard', 4, x=112)]
    objects[-1]['alwaysActive'] = True
    objects.append(obj('key', 'item', 6, x=128))
    lever = obj('label', 'lever', 7, x=144)
    lever['behavior'] = dict(type='lever', initialState='initial',
                             frames=dict(initial=0, transition=1, activated=2),
                             staysVisible=True)
    objects.append(lever)
    objects.append(obj('campfire', 'interactive', 8, x=160, height=32))
    objects.append(obj('stairs', 'stairs', 9, x=176))
    door = obj('door_special', 'door', 5, x=232, y=112, height=32)
    door['hitbox'] = dict(x=0, y=24, width=16, height=8)
    door['side'] = 'right'
    door['rule'] = 'possible'
    objects.append(door)
    return dict(format='anotherdungeon-room', schemaVersion=2, id='extended-room',
                name='Extended', width=256, height=256, tileSize=16,
                coordinateSystem='top-left-pixels', background='room2',
                spawn=dict(x=128, y=128), tiles=[], collisions=[], markers=[],
                objects=objects, rules=dict(roomType='standard', access='normal',
                onlyTheseSides=True, allowedDoorSides=['right']))


class ExtendedImportTests(unittest.TestCase):
    def test_enemy_hazard_and_door_roles_generate_rom_entries(self):
        parsed = import_room.parse_room(room())
        self.assertEqual(1, parsed['decoration_count'])
        self.assertEqual([], parsed['enemy_indices'])  # Skeletons use this separate list.
        lines = '\n'.join(parsed['objects'])
        for asset in ('BAT', 'MUSHY', 'PLANT'):
            self.assertIn(f'Asset::{asset}, Role::ENEMY', lines)
        self.assertIn('Asset::SPIKES, Role::HAZARD', lines)
        self.assertIn('Asset::KEY, Role::ITEM', lines)
        self.assertIn('Asset::LEVER, Role::LEVER', lines)
        self.assertIn('Asset::CAMPFIRE, Role::INTERACTIVE', lines)
        self.assertIn('Asset::STAIRS, Role::STAIRS', lines)
        self.assertIn('-1, true}', lines)
        self.assertIn('Asset::DOOR_SPECIAL, Role::DOOR', lines)
        header = import_room.generate(room())
        self.assertIn('constexpr int count(Asset asset) const', header)

    def test_incompatible_roles_and_enemy_limits_are_rejected(self):
        wrong = room()
        wrong['objects'][1]['role'] = 'decoration'
        with self.assertRaisesRegex(ValueError, 'Asset and role disagree'):
            import_room.parse_room(wrong)
        many = room()
        many['objects'] += [obj('bat', 'enemy', index, x=112 + index * 16)
                            for index in range(6, 10)]
        with self.assertRaisesRegex(ValueError, 'At most four'):
            import_room.parse_room(many)


if __name__ == '__main__':
    unittest.main()
