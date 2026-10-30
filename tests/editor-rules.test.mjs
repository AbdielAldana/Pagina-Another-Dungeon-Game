import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { createRoom, createObject, createMarker, validateRoom, defaultHitbox, applyBossPreset, snapToGameGrid, gameplayErrors, behaviorDefinition, serializeGameRoom } from '../editor-core.js';

const roundTrip = room => validateRoom(JSON.parse(JSON.stringify(room)), { forExport: true });

test('the v2 document retains the complete canvas and stable IDs with zero-based frames', () => {
  const room = createRoom(), object = createObject('skeleton', 80, 96);
  object.rotation = 90; object.flipX = true; object.frame = 15; room.objects.push(object);
  assert.deepEqual(roundTrip(room), room);
  assert.deepEqual([room.width, room.height, room.schemaVersion], [256, 256, 2]);
  assert.equal(roundTrip(room).objects[0].id, object.id);
});

test('the offset game grid is optional and respects its 15 columns and 10 rows', () => {
  assert.deepEqual(snapToGameGrid(10, 50), { x: 8, y: 48 });
  assert.deepEqual(snapToGameGrid(999, 999), { x: 232, y: 192 });
  assert.deepEqual(snapToGameGrid(255, 255, 32, 16), { x: 216, y: 192 });
  const object = createObject('box', 101, 103); assert.deepEqual([object.x, object.y], [101, 103]);
});

test('new 16x16 objects use the game hitbox, seats are non-solid, larger assets keep editable dimensions', () => {
  assert.deepEqual(defaultHitbox('skeleton'), { x: 3, y: 0, width: 10, height: 8 });
  assert.deepEqual(defaultHitbox('box'), { x: 3, y: 0, width: 10, height: 8 });
  assert.equal(createObject('seat', 80, 80).solid, false);
  assert.deepEqual(defaultHitbox('table'), { x: 0, y: 8, width: 32, height: 8 });
});

test('current and legacy imports retain existing hitboxes, object IDs, frames, rotations and flips', () => {
  const room = createRoom(), object = createObject('box', 80, 96);
  object.hitbox = { x: 1, y: 7, width: 14, height: 9 }; object.frame = 0; object.rotation = 180; object.flipX = true;
  room.objects.push(object);
  for (const version of [1, 2]) {
    const input = JSON.parse(JSON.stringify(room)); input.schemaVersion = version; delete input.rules; delete input.markers; delete input.gameGrid;
    if (version === 1) delete input.backgroundCrop;
    const result = validateRoom(input);
    assert.deepEqual(result.objects[0].hitbox, object.hitbox);
    for (const key of ['id', 'assetId', 'x', 'y', 'frame', 'rotation', 'flipX']) assert.equal(result.objects[0][key], object[key]);
    assert.equal(result.id, room.id);
  }
});

test('old cropped rooms restore background-relative coordinates without changing their hitboxes', () => {
  for (const height of [160, 180]) {
    const room = createRoom(); room.width = 240; room.height = height; room.backgroundCrop = { x: 8, y: (256 - height) / 2, width: 240, height };
    delete room.rules; delete room.markers;
    const object = createObject('column', 40, 60); object.hitbox = { x: 2, y: 20, width: 12, height: 12 }; room.objects.push(object);
    room.tiles.push({ x: 16, y: 16, width: 16, height: 16, assetId: 'land', sx: 32, sy: 32 });
    const result = validateRoom(room);
    assert.deepEqual([result.objects[0].x, result.objects[0].y], [48, 60 + room.backgroundCrop.y]);
    assert.deepEqual(result.objects[0].hitbox, object.hitbox);
    assert.deepEqual([result.tiles[0].x, result.tiles[0].y, result.tiles[0].sx, result.tiles[0].sy], [24, 16 + room.backgroundCrop.y, 32, 32]);
  }
});

test('arbitrary invisible rectangles export without sprites', () => {
  const room = createRoom(); room.collisions.push({ x: 13, y: 19, width: 101, height: 37 });
  const result = roundTrip(room); assert.deepEqual(result.collisions, room.collisions); assert.equal(result.objects.length, 0);
  room.collisions[0].width = 256; assert.throws(() => roundTrip(room), /colisiones/);
});

test('invisible rectangles may overlap and share an origin to represent a union', () => {
  const room = createRoom(); room.collisions = [{ x: 24, y: 48, width: 100, height: 16 }, { x: 24, y: 48, width: 16, height: 100 }];
  assert.deepEqual(roundTrip(room).collisions, room.collisions);
});

test('doors explicitly specify side and possible/required rules, independent of access', () => {
  const room = createRoom(), door = createObject('door', 232, 112); room.objects.push(door);
  assert.throws(() => roundTrip(room), /selecciona lado y regla/);
  door.side = 'right'; door.rule = 'required'; room.rules.access = 'key';
  assert.deepEqual(roundTrip(room), room);
  room.rules.onlyTheseSides = true; room.rules.allowedDoorSides = ['left'];
  assert.throws(() => roundTrip(room), /solo estos lados/);
});

test('legacy destinations import but are omitted; new rules reject concrete dungeon destinations', () => {
  const room = createRoom(), door = createObject('door', 8, 112);
  door.destination = 'old-room'; room.objects.push(door); delete room.rules;
  const imported = validateRoom(room); assert.equal('destination' in imported.objects[0], false);
  const fresh = createRoom(); fresh.objects.push(door); assert.throws(() => roundTrip(fresh), /destination/);
});

test('Boss type preserves authored doors and accepts different access rules', () => {
  const room = createRoom();
  const door = createObject('door',8,112); door.side='left'; door.rule='required'; room.objects.push(door);
  room.rules.onlyTheseSides=true; room.rules.allowedDoorSides=['left']; room.rules.access='key';
  applyBossPreset(room);
  assert.deepEqual(roundTrip(room).rules,{roomType:'boss',onlyTheseSides:true,allowedDoorSides:['left'],access:'key'});
  assert.equal(room.objects[0].id,door.id);
  assert.throws(()=>serializeGameRoom(room),/jefes adicionales/);
  assert.equal(serializeGameRoom(room,{forGame:false}).rules.roomType,'boss');
  room.rules.access='three_levers'; room.rules.allowedDoorSides=['left','up'];
  assert.deepEqual(roundTrip(room).rules,room.rules);
});

test('legacy boss1 drafts import as editable Boss rooms',()=>{
  const room=createRoom(); room.rules.roomType='boss1';
  const imported=roundTrip(room);
  assert.equal(imported.rules.roomType,'boss');
  assert.equal(serializeGameRoom(imported,{forGame:false}).rules.roomType,'boss');
});

test('Q and required reward markers are stable logical entities without predetermined content', () => {
  const room = createRoom(); room.markers.push(createMarker('Q', 40, 80), createMarker('&', 120, 112)); room.markers[1].required = true;
  assert.deepEqual(roundTrip(room), room); assert.equal(room.objects.length, 0);
  room.markers[0].content = 'skeleton'; assert.throws(() => roundTrip(room), /semilla/);
});

test('enemy appearance and patrol stay independent of the reference frame', () => {
  const room = createRoom(), enemy = createObject('skeleton', 80, 96); room.objects.push(enemy);
  enemy.frame = 12; assert.equal(roundTrip(room).objects[0].enemy.patrol, null);
  enemy.enemy = { spawnMode: 'seeded', patrol: { axis: 'horizontal', from: 40, to: 200 } };
  assert.deepEqual(roundTrip(room).objects[0].enemy, enemy.enemy);
  enemy.enemy.patrol = { axis: 'vertical', from: 48, to: 192 }; assert.deepEqual(roundTrip(room).objects[0].enemy.patrol, enemy.enemy.patrol);
  enemy.enemy.patrol.from = null; assert.throws(() => roundTrip(room), /dos extremos/);
});

test('invalid patrol endpoints and axes report incompatible data instead of inventing a path', () => {
  const room = createRoom(), enemy = createObject('bat', 80, 80); room.objects.push(enemy);
  for (const patrol of [{ axis: 'diagonal', from: 8, to: 80 }, { axis: 'horizontal', from: 100, to: 200 }, { axis: 'horizontal', from: 8, to: 256 }, { axis: 'vertical', from: 100, to: 50 }]) {
    enemy.enemy.patrol = patrol; assert.throws(() => roundTrip(room));
  }
});

test('chest and lever states are static reference frames with game-owned initial states', () => {
  const room = createRoom(), chest = createObject('chest', 80, 80), lever = createObject('label', 120, 80);
  chest.frame = 2; lever.frame = 1; room.objects.push(chest, lever);
  const result = roundTrip(room);
  assert.deepEqual(result.objects[0].behavior, behaviorDefinition('chest'));
  assert.equal(result.objects[0].behavior.initialState, 'closed');
  assert.deepEqual(result.objects[1].behavior, behaviorDefinition('lever'));
  assert.equal(result.objects[1].behavior.staysVisible, true);
  lever.behavior.staysVisible = false; assert.throws(() => roundTrip(room), /Comportamiento incompatible/);
});

test('an imported solid seat preserves its data but must be explicitly made non-solid to export', () => {
  const room = createRoom(), seat = createObject('seat', 80, 80); seat.solid = true; seat.hitbox = { x: 0, y: 8, width: 16, height: 8 }; room.objects.push(seat);
  const imported = validateRoom(room); assert.deepEqual(imported.objects[0].hitbox, seat.hitbox); assert.equal(imported.objects[0].solid, true);
  assert.throws(() => roundTrip(imported), /Asiento/);
  imported.objects[0].solid = false; assert.equal(gameplayErrors(imported).length, 0);
});

test('unknown schemas, sprites, duplicate IDs, invalid frames, rules and out-of-bounds values are rejected', () => {
  const cases = [
    r => { r.schemaVersion = 3; }, r => { r.background = '../missing'; }, r => { r.spawn = { x: 256, y: 128 }; },
    r => { r.rules.access = 'key_and_levers'; }, r => { r.objects = [createObject('box', 0, 0)]; r.objects[0].x = 250; },
    r => { const o = createObject('box', 0, 0); r.objects = [o, { ...o }]; },
    r => { r.objects = [createObject('seat', 0, 0)]; r.objects[0].frame = 4; },
    r => { r.objects = [createObject('box', 0, 0)]; r.objects[0].hitbox.width = 17; },
    r => { const m = createMarker('Q', 0, 0); r.markers = [m, { ...m }]; }
  ];
  for (const mutate of cases) { const room = createRoom(); mutate(room); assert.throws(() => roundTrip(room)); }
});

test('delivered examples validate and retain deterministic contents across export round trips', async () => {
  for (const name of ['editor-rules.room.json', 'boss1.room.json']) {
    const example = JSON.parse(await readFile(new URL(`../examples/${name}`, import.meta.url), 'utf8'));
    const expected = structuredClone(example);
    if(expected.rules.roomType==='boss1') expected.rules.roomType='boss';
    assert.deepEqual(roundTrip(example), expected);
    assert.equal(JSON.stringify(roundTrip(example)), JSON.stringify(roundTrip(example)));
  }
});
