import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { ASSETS, ASSET_MAP, ROOM_WIDTH, ROOM_HEIGHT, BACKGROUND_CROP, createRoom, createObject, objectCollision, objectSize, validateRoom, decodeBMP, roomWarnings, replaceRegion } from '../editor-core.js';

test('every catalog entry decodes at its real size and preserves visible palette colors', async () => {
  for (const asset of ASSETS) {
    const file = await readFile(new URL(`../assets_game/${asset.file}`, import.meta.url));
    const buffer = file.buffer.slice(file.byteOffset, file.byteOffset + file.byteLength);
    const image = decodeBMP(buffer);
    assert.equal(image.width, asset.width, asset.file);
    assert.equal(image.height, asset.height * asset.frames, asset.file);
    assert.ok(image.data.some((value, index) => index % 4 === 3 && value === 255), asset.file);
    if (asset.category !== 'background') assert.ok(image.data.some((value, index) => index % 4 === 3 && value === 0), `${asset.file} should have transparent pixels`);
  }
});

test('palette index transparency handles 4-bit and both 8-bit green keys', async () => {
  for (const [name, expectedKey] of [['player', [0, 255, 0]], ['campfire', [0, 255, 0]], ['skeleton', [2, 252, 2]]]) {
    const asset = ASSET_MAP.get(name), file = await readFile(new URL(`../assets_game/${asset.file}`, import.meta.url));
    const image = decodeBMP(file.buffer.slice(file.byteOffset, file.byteOffset + file.byteLength));
    assert.deepEqual(image.key, expectedKey);
    assert.equal(image.transparent, true);
    assert.equal(image.data[3], 0);
  }
});

test('rotated and mirrored hitboxes follow the visible sprite including edge clamping', () => {
  const object = createObject('column', 20, 30);
  object.hitbox = { x: 2, y: 24, width: 10, height: 8 };
  assert.deepEqual(objectCollision(object), { x: 22, y: 54, width: 10, height: 8 });
  object.rotation = 90;
  assert.deepEqual(objectSize(object), { width: 32, height: 16 });
  assert.deepEqual(objectCollision(object), { x: 20, y: 32, width: 8, height: 10 });
  object.flipX = true;
  assert.deepEqual(objectCollision(object), { x: 20, y: 34, width: 8, height: 10 });
  const atEdge = createObject('table', 300, -8);
  assert.equal(atEdge.x, 224); assert.equal(atEdge.y, 0);
});

test('invalid or truncated BMP data is rejected', () => {
  assert.throws(() => decodeBMP(new ArrayBuffer(8)));
  assert.throws(() => decodeBMP(new ArrayBuffer(100)));
});

test('painting and erasing retain neighbouring fragments and their exact source pixels', () => {
  const patches = [{ x: 8, y: 2, width: 16, height: 16, assetId: 'land', sx: 16, sy: 48 }];
  const cleared = replaceRegion(patches, { x: 16, y: 0, width: 16, height: 16 });
  assert.deepEqual(cleared, [
    { x: 8, y: 16, width: 16, height: 2, assetId: 'land', sx: 16, sy: 62 },
    { x: 8, y: 2, width: 8, height: 14, assetId: 'land', sx: 16, sy: 48 }
  ]);
  assert.equal(cleared.reduce((area, patch) => area + patch.width * patch.height, 0), 144);
});

