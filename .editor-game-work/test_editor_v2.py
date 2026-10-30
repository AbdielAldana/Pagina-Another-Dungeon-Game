"""Basic standard-room exports from the editor, without a native binding."""
import copy
import unittest
import import_room


def fixture():
    objects=[]
    for index,(asset,x,y,w,h) in enumerate([('table_armor',24,176,32,16),('box',88,160,16,16),('player',168,96,16,16)]):
        objects.append(dict(id=str(index),assetId=asset,x=x,y=y,frame=0,rotation=0,
                            flipX=False,solid=asset!='player',role='decoration',behavior=None,
                            hitbox=dict(x=3,y=0,width=w-6,height=h-8)))
    return dict(format='anotherdungeon-room',schemaVersion=2,id='editor-v2',name='Editor',
                width=256,height=256,tileSize=16,coordinateSystem='top-left-pixels',background='room2',
                spawn=dict(x=128,y=128),tiles=[],collisions=[],markers=[],objects=objects,
                rules=dict(roomType='standard',access='normal',onlyTheseSides=True,
                           allowedDoorSides=['left','right','up','down']))


class EditorTests(unittest.TestCase):
    def test_allowed_sides_create_only_declared_seeded_door_slots(self):
        data=fixture()
        original=copy.deepcopy(data)
        parsed=import_room.parse_room(data)
        self.assertEqual(original,data)
        self.assertEqual(3,parsed['decoration_count'])
        self.assertEqual(7,len(parsed['objects']))
        self.assertIn('Asset::TABLE_ARMOR',parsed['objects'][0])
        self.assertIn('Asset::BOX',parsed['objects'][1])
        self.assertIn('Asset::PLAYER',parsed['objects'][2])
        data['rules']['allowedDoorSides']=['right']
        parsed=import_room.parse_room(data)
        self.assertEqual(4,len(parsed['objects']))
        self.assertTrue(parsed['objects'][-1].endswith(', 1},'))

    def test_markers_and_invisible_collision_are_emitted(self):
        data=fixture()
        data['markers']=[{'id':'q1','type':'Q','x':120,'y':100},
                         {'id':'ability1','type':'&','x':150,'y':100,'required':True}]
        data['collisions']=[{'x':0,'y':0,'width':16,'height':16}]
        parsed=import_room.parse_room(data)
        self.assertEqual(['{-8, -28}'], parsed['q_points'])
        self.assertEqual(['{22, -28, true}'], parsed['ability_points'])
        self.assertEqual(['{-128, -128, 16, 16}'], parsed['collisions'])
        generated=import_room.generate(data)
        self.assertIn('constexpr AbilityPoint ability_points_0[]', generated)
        self.assertIn('constexpr Rect collisions_0[]', generated)

    def test_tiles_still_fail_explicitly(self):
        data=fixture()
        data['tiles']=[{'x':0,'y':0}]
        with self.assertRaisesRegex(ValueError,'tiles'):
            import_room.generate(data)

    def test_ability_rewards_need_distinct_positions(self):
        data=fixture()
        data['markers']=[{'id':'a','type':'&','x':120,'y':100,'required':True},
                         {'id':'b','type':'&','x':120,'y':100,'required':False}]
        with self.assertRaisesRegex(ValueError,'distinct positions'):
            import_room.generate(data)


if __name__=='__main__':
    unittest.main()
