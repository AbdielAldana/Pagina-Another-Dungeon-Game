// Exercise real editor event handlers with real BMPs and a small DOM adapter.
// This verifies interaction state, not browser layout or physical touch hardware.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { ASSETS, ROOM_WIDTH, ROOM_HEIGHT, createRoom, createObject } from '../editor-core.js';

const html = await readFile(new URL('../editor.html', import.meta.url), 'utf8');
const files = new Map(await Promise.all(ASSETS.map(async asset => {
  const bytes = await readFile(new URL(`../assets_game/${asset.file}`, import.meta.url));
  return [asset.file, bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength)];
})));
const nativeTimeout = globalThis.setTimeout;
let sequence = 0;

async function setup(compact = true, draft = null) {
  const storage = new Map(draft ? [['anotherdungeon.editor.draft.v1', JSON.stringify(draft)]] : []);
  const ids = new Map();
  const created = [];
  class Element {
    constructor(tag = 'div') {
      this.tag = tag; this.children = []; this.events = new Map(); this.captured = new Set();
      this.value = ''; this.hidden = false; this.open = false; this.dataset = {}; this._text = ''; this.drawCalls = [];
      this.style = { setProperty(key, value) { this[key] = value; } };
      this.classes = new Set(); this.classList = { add: key => this.classes.add(key), remove: key => this.classes.delete(key), toggle: (key, value) => value ? this.classes.add(key) : this.classes.delete(key) };
    }
    set textContent(value) { this._text = String(value); this.children = []; }
    get textContent() { return this._text + this.children.map(child => child.textContent || '').join(' '); }
    setAttribute(key, value) { this[key] = value; }
    addEventListener(type, callback) { if (!this.events.has(type)) this.events.set(type, []); this.events.get(type).push(callback); }
    fire(type, values = {}) {
      const event = { target: this, button: 0, pointerId: 1, pointerType: 'touch', clientX: 0, clientY: 0, preventDefault() {}, stopPropagation() {}, ...values };
      return Promise.all((this.events.get(type) || []).map(callback=>callback(event)));
    }
    append(...nodes) {
      for (const node of nodes) {
        if (node.tag === 'fragment') { this.append(...[...node.children]); continue; }
        if (node.parentElement) node.parentElement.children = node.parentElement.children.filter(child => child !== node);
        node.parentElement = this; this.children.push(node);
      }
    }
    replaceChildren(...nodes) { this.children = []; this._text = ''; this.append(...nodes); }
    before(node) { const parent = this.parentElement; parent.children.splice(parent.children.indexOf(this), 0, node); node.parentElement = parent; }
    after(node) { const parent = this.parentElement; if (node.parentElement) node.parentElement.children = node.parentElement.children.filter(child => child !== node); parent.children.splice(parent.children.indexOf(this) + 1, 0, node); node.parentElement = parent; }
    get options() { return this.children; }
    getContext() { return new Proxy({}, { get: (_, method) => (...args) => { if (method === 'drawImage') this.drawCalls.push(args); }, set: () => true }); }
    toDataURL() { return 'data:image/png;base64,AA=='; }
    focus() { document.activeElement = this; }
    click() { return this.fire('click'); }
    matches() { return false; }
    showModal() { this.open = true; }
    close() { this.open = false; }
    scrollIntoView() {}
    setPointerCapture(id) { this.captured.add(id); }
    hasPointerCapture(id) { return this.captured.has(id); }
    releasePointerCapture(id) { this.captured.delete(id); this.fire('lostpointercapture', { pointerId: id }); }
    getBoundingClientRect() {
      const isRoomCanvas = this === ids.get('room-canvas');
      const width = parseFloat(isRoomCanvas ? this.style.width : ids.get('canvas-wrap').style.width) || 512;
      const height = parseFloat(isRoomCanvas ? this.style.height : ids.get('canvas-wrap').style.height) || 512;
      const left = 20 - ids.get('canvas-viewport').scrollLeft + (isRoomCanvas ? parseFloat(this.style.left) || 0 : 0);
      const top = 100 - ids.get('canvas-viewport').scrollTop + (isRoomCanvas ? parseFloat(this.style.top) || 0 : 0);
      return { left, top, width, height, right: left + width, bottom: top + height };
    }
  }
  for (const match of html.matchAll(/id="([^"]+)"/g)) ids.set(match[1], new Element());
  ids.get('asset-category').value = 'all';
  const viewport = ids.get('canvas-viewport'); viewport.clientWidth = compact ? 360 : 700; viewport.clientHeight = 400; viewport.scrollLeft = 0; viewport.scrollTop = 0;
  const workspace = new Element(); workspace.append(ids.get('palette-panel'), ids.get('inspector-panel'));
  const tools = ['select', 'stamp', 'erase', 'collision', 'spawn', 'pan'].map(tool => { const button = new Element('button'); button.dataset.tool = tool; return button; });
  const previewButtons = [0, 1, 2].map(frame => { const button = new Element('button'); button.dataset.previewFrame = String(frame); return button; });
  const media = { matches: compact, addEventListener(type, callback) { this.change = callback; } };
  globalThis.document = {
    getElementById(id) { assert.ok(ids.has(id), `Missing HTML ID: ${id}`); return ids.get(id); },
    createElement(tag) { const element = new Element(tag); created.push(element); return element; },
    createComment() { return new Element('comment'); }, createDocumentFragment() { return new Element('fragment'); },
    querySelectorAll(selector) { if (selector === '.drawing-tools [data-tool]') return tools; if (selector === '[data-preview-frame]') return previewButtons; if (selector === '.sprite-card') return ids.get('sprite-grid').children; return []; },
    querySelector(selector) { return selector === 'dialog[open]' ? [...ids.values()].find(element => element.open) : null; },
    addEventListener() {}, body: new Element('body'), documentElement: new Element('html'), activeElement: null
  };
  globalThis.window = { addEventListener() {}, matchMedia(query) { return query.includes('max-width') ? media : { matches: compact }; } };
  globalThis.requestAnimationFrame = callback => callback();
  globalThis.innerWidth = compact ? 390 : 1280; globalThis.innerHeight = 844;
  globalThis.localStorage = { getItem: key => storage.get(key) || null, setItem: (key, value) => storage.set(key, value) };
  globalThis.ImageData = class { constructor(data, width, height) { Object.assign(this, { data, width, height }); } };
  globalThis.Option = class extends Element { constructor(text, value) { super('option'); this.textContent = text; this.value = value; } };
  globalThis.setTimeout = (callback, ms, ...args) => { const timer = nativeTimeout(callback, ms, ...args); timer.unref(); return timer; };
  globalThis.fetch = async url => {
    if(new URL(url).pathname.includes('/examples/game-rooms/')) {
      const name=new URL(url).pathname.split('/').at(-1);
      return {ok:true,json:async()=>JSON.parse(await readFile(new URL(`../examples/game-rooms/${name}`,import.meta.url),'utf8'))};
    }
    const data = files.get(new URL(url).pathname.split('/').at(-1)); assert.ok(data); return { ok: true, arrayBuffer: async () => data };
  };
  await import(`../editor.js?interaction-test=${++sequence}`);
  const room = () => JSON.parse(storage.get('anotherdungeon.editor.draft.v1') || 'null');
  const canvas = ids.get('room-canvas');
  const point = (x, y) => { const rect = canvas.getBoundingClientRect(); return { clientX: rect.left + (x + .25) / ROOM_WIDTH * rect.width, clientY: rect.top + (y + .25) / ROOM_HEIGHT * rect.height }; };
  const pointer = (type, id, x, y, extra = {}) => canvas.fire(type, { pointerId: id, ...point(x, y), ...extra });
  const tap = (id, x, y) => { pointer('pointerdown', id, x, y); pointer('pointerup', id, x, y); };
  const tool = name => tools.find(button => button.dataset.tool === name).fire('click');
  const choose = assetId => ids.get('sprite-grid').children.find(button => button.dataset.assetId === assetId).fire('click');
  return { ids, canvas, room, pointer, tap, tool, media, workspace, storage, choose, previewButtons };
}

test('player spawn reference starts absent and can be placed then removed', async () => {
  const ui = await setup();
  assert.equal(ui.ids.get('spawn-coordinates').textContent, 'Entrada por puerta');
  ui.tool('spawn'); ui.tap(1, 128, 128);
  assert.ok(ui.room().spawn);
  ui.ids.get('remove-spawn-button').fire('click');
  assert.equal(ui.room().spawn, null);
  assert.equal(ui.ids.get('spawn-coordinates').textContent, 'Entrada por puerta');
});

test('compact panels open, choose a sprite, close, and return to desktop without losing work', async () => {
  const ui = await setup();
  assert.equal(ui.ids.get('palette-panel').parentElement, ui.ids.get('resources-dialog'));
  ui.ids.get('mobile-resources').fire('click'); assert.equal(ui.ids.get('resources-dialog').open, true);
  ui.ids.get('sprite-grid').children[0].fire('click'); assert.equal(ui.ids.get('resources-dialog').open, false);
  ui.tap(1, 80, 96); assert.equal(ui.room().objects.length, 1);
  ui.ids.get('mobile-properties').fire('click'); assert.equal(ui.ids.get('properties-dialog').open, true);
  ui.media.matches = false; ui.media.change();
  assert.equal(ui.ids.get('properties-dialog').open, false);
  assert.equal(ui.ids.get('palette-panel').parentElement, ui.workspace);
  assert.equal(ui.room().objects.length, 1);
});

test('pinching in stamp mode does not accidentally place a sprite and the next tap still works', async () => {
  const ui = await setup(); ui.tool('stamp');
  const initialZoom = parseInt(ui.ids.get('zoom-value').textContent);
  ui.pointer('pointerdown', 1, 60, 80); assert.equal(ui.room(), null);
  ui.pointer('pointerdown', 2, 160, 80); ui.pointer('pointermove', 2, 200, 80);
  assert.ok(parseInt(ui.ids.get('zoom-value').textContent) > initialZoom);
  ui.pointer('pointerup', 2, 200, 80); ui.pointer('pointerup', 1, 60, 80);
  assert.equal(ui.room(), null);
  ui.tap(3, 100, 100); assert.equal(ui.room().objects.length, 1);
});

test('two fingers roll back a collision stroke, and touch users can paint and erase without right click', async () => {
  const ui = await setup(); ui.tool('collision');
  ui.pointer('pointerdown', 1, 80, 80); ui.pointer('pointerdown', 2, 160, 80);
  ui.pointer('pointerup', 2, 160, 80); ui.pointer('pointerup', 1, 80, 80);
  assert.equal(ui.room(), null);
  ui.tap(3, 80, 80); assert.deepEqual(ui.room().collisions, [{ x: 80, y: 80, width: 1, height: 1 }]);
  ui.ids.get('paint-remove').fire('click'); ui.tap(4, 80, 80);
  assert.deepEqual(ui.room().collisions, []);
});

test('cancelled object drags roll back, successful drags make one undoable change', async () => {
  const ui = await setup(); ui.tool('stamp'); ui.tap(1, 80, 80); ui.tool('select');
  const before = ui.room().objects[0];
  ui.pointer('pointerdown', 2, 88, 88); ui.pointer('pointermove', 2, 120, 88); ui.pointer('pointercancel', 2, 120, 88);
  assert.deepEqual(ui.room().objects[0], before);
  ui.pointer('pointerdown', 3, 88, 88); ui.pointer('pointermove', 3, 120, 88); ui.pointer('pointerup', 3, 120, 88);
  assert.equal(ui.room().objects[0].x, 112);
  ui.ids.get('undo-button').fire('click'); assert.deepEqual(ui.room().objects[0], before);
  ui.ids.get('redo-button').fire('click'); assert.equal(ui.room().objects[0].x, 112);
});

test('view panning changes scroll position while preserving room data and undo history', async () => {
  const ui = await setup(); ui.tool('stamp'); ui.tap(1, 80, 80);
  const before = JSON.stringify(ui.room()); ui.tool('pan');
  ui.pointer('pointerdown', 2, 180, 150); ui.pointer('pointermove', 2, 120, 100); ui.pointer('pointerup', 2, 120, 100);
  assert.ok(ui.ids.get('canvas-viewport').scrollLeft > 0);
  assert.equal(JSON.stringify(ui.room()), before);
  ui.ids.get('undo-button').fire('click'); assert.equal(ui.room().objects.length, 0);
});

test('mouse users can still place sprites and erase collisions with the right button', async () => {
  const ui = await setup(false); ui.tool('stamp');
  ui.pointer('pointerdown', 1, 80, 80, { pointerType: 'mouse' }); ui.pointer('pointerup', 1, 80, 80, { pointerType: 'mouse' });
  assert.equal(ui.room().objects.length, 1);
  ui.tool('collision'); ui.tap(2, 160, 160);
  ui.pointer('pointerdown', 3, 160, 160, { pointerType: 'mouse', button: 2 }); ui.pointer('pointerup', 3, 160, 160, { pointerType: 'mouse', button: 2 });
  assert.deepEqual(ui.room().collisions, []);
});

test('only the visible window is 240 x 160 and the canvas still renders the complete 256 x 256', async () => {
  const ui = await setup();
  assert.ok(ui.canvas.drawCalls.some(call => JSON.stringify(call.slice(1)) === JSON.stringify([0, 0, 256, 256, 0, 0, 256, 256])));
  assert.equal(parseFloat(ui.ids.get('canvas-wrap').style.width) / parseFloat(ui.ids.get('canvas-wrap').style.height), 3 / 2);
  assert.equal(parseFloat(ui.canvas.style.width) / parseFloat(ui.canvas.style.height), 1);
  ui.tool('stamp'); ui.tap(1, 8, 48);
  assert.deepEqual([ui.room().objects[0].x, ui.room().objects[0].y], [8, 48]);
  ui.tool('collision'); ui.pointer('pointerdown', 2, 20, 60); ui.pointer('pointermove', 2, 60, 90); ui.pointer('pointerup', 2, 60, 90);
  assert.deepEqual(ui.room().collisions, [{ x: 20, y: 60, width: 40, height: 30 }]);
});

test('the visual crop leaves the exported document, coordinates and rules exactly intact', async () => {
  const example = JSON.parse(await readFile(new URL('../examples/editor-rules.room.json', import.meta.url), 'utf8'));
  example.objects.push(createObject('seat', 0, 0)); // Even objects outside the visible crop remain in the document.
  const ui = await setup(true, example);
  ui.ids.get('export-target').value = 'editor';
  ui.ids.get('export-button').fire('click');
  assert.deepEqual(JSON.parse(ui.ids.get('export-code').value), example);
  const view = ui.ids.get('canvas-wrap').getBoundingClientRect(), full = ui.canvas.getBoundingClientRect();
  const scale = full.width / 256;
  assert.ok(Math.abs((view.left - full.left) / scale - 8) < .0001);
  assert.ok(Math.abs((view.top - full.top) / scale - 48) < .0001);
});

test('game-grid placement uses the offset origin and free placement can be restored', async () => {
  const ui = await setup(); ui.choose('box');
  ui.ids.get('snap-size').value = 'game'; ui.ids.get('snap-size').fire('change'); ui.tap(1, 10, 50);
  assert.deepEqual([ui.room().objects[0].x, ui.room().objects[0].y], [8, 48]);
  ui.ids.get('snap-size').value = 'free'; ui.ids.get('snap-size').fire('change'); ui.tap(2, 101, 103);
  assert.deepEqual([ui.room().objects[1].x, ui.room().objects[1].y], [101, 103]);
});

test('markers are logical entities, remain stable when moved, and rewards can be required', async () => {
  const ui = await setup(); ui.ids.get('add-marker-q').fire('click'); ui.tap(1, 80, 80);
  const markerId = ui.room().markers[0].id;
  ui.tool('select'); ui.pointer('pointerdown', 2, 80, 80); ui.pointer('pointermove', 2, 96, 88); ui.pointer('pointerup', 2, 96, 88);
  assert.equal(ui.room().markers[0].id, markerId);
  ui.ids.get('add-marker-reward').fire('click'); ui.tap(3, 120, 120);
  ui.ids.get('marker-required').checked = true; ui.ids.get('marker-required').fire('change');
  assert.equal(ui.room().markers[1].required, true); assert.equal(ui.room().objects.length, 0);
});

test('patrol endpoints are authored independently of the visual frame', async () => {
  const ui = await setup(); ui.choose('skeleton'); ui.tap(1, 80, 96);
  ui.ids.get('object-frame').value = '5'; ui.ids.get('object-frame').fire('change');
  assert.equal(ui.room().objects[0].enemy.patrol, null);
  ui.ids.get('enemy-patrol').value = 'horizontal'; ui.ids.get('enemy-patrol').fire('change');
  assert.deepEqual(ui.room().objects[0].enemy.patrol, { axis: 'horizontal', from: null, to: null });
  ui.ids.get('patrol-from').value = '24'; ui.ids.get('patrol-from').fire('change'); ui.ids.get('patrol-to').value = '200'; ui.ids.get('patrol-to').fire('change');
  assert.deepEqual(ui.room().objects[0].enemy.patrol, { axis: 'horizontal', from: 24, to: 200 });
  assert.equal(ui.room().objects[0].frame, 5);
});

test('behavior preview changes only the reference frame and exports no runtime state', async () => {
  const ui = await setup(); ui.choose('label'); ui.tap(1, 80, 80);
  const behavior = ui.room().objects[0].behavior;
  ui.previewButtons[2].fire('click');
  assert.equal(ui.room().objects[0].frame, 2); assert.deepEqual(ui.room().objects[0].behavior, behavior);
  assert.equal(behavior.initialState, 'initial'); assert.equal(behavior.staysVisible, true);
});

test('Boss button keeps the current room editable and exports a draft', async () => {
  const draft=createRoom(), door=createObject('door',8,112);
  door.side='left'; door.rule='required'; draft.objects.push(door);
  draft.rules.onlyTheseSides=true; draft.rules.allowedDoorSides=['left']; draft.rules.access='key';
  const ui = await setup(true,draft); ui.ids.get('boss-preset').fire('click');
  assert.equal(ui.ids.get('export-target').value,'editor');
  assert.equal(ui.ids.get('room-access').disabled,false);
  assert.equal(ui.ids.get('side-left').disabled,false);
  ui.ids.get('export-button').fire('click');
  const data = JSON.parse(ui.ids.get('export-code').value);
  assert.equal(data.rules.roomType,'boss');
  assert.equal(data.objects[0].id,door.id);
  assert.deepEqual(data.rules.allowedDoorSides,['left']);
  assert.equal(data.rules.access,'key');
  ui.ids.get('room-access').value='three_levers'; ui.ids.get('room-access').fire('change');
  assert.equal(ui.room().rules.access,'three_levers');
});

test('import preserves all IDs and old hitboxes until the author explicitly applies defaults', async () => {
  const old = createRoom(); old.schemaVersion = 1; delete old.backgroundCrop; delete old.rules; delete old.markers; delete old.gameGrid;
  const object = createObject('box', 80, 80); object.hitbox = { x: 0, y: 8, width: 16, height: 8 }; old.objects.push(object);
  const ui = await setup(); ui.ids.get('import-code').value = JSON.stringify(old); ui.ids.get('import-code-button').fire('click');
  assert.equal(ui.room().id, old.id); assert.equal(ui.room().objects[0].id, object.id); assert.deepEqual(ui.room().objects[0].hitbox, object.hitbox);
  ui.ids.get('object-list').children[0].fire('click'); ui.ids.get('apply-default-hitbox').fire('click');
  assert.deepEqual(ui.room().objects[0].hitbox, { x: 3, y: 0, width: 10, height: 8 });
});

test('incomplete door rules block export with a clear explanation', async () => {
  const ui = await setup(); ui.choose('box'); ui.tap(9,32,64); ui.choose('door'); ui.tap(1, 232, 112); ui.ids.get('export-button').fire('click');
  assert.equal(ui.ids.get('export-dialog').open, false); assert.match(ui.ids.get('toast').textContent, /lado y regla/);
  ui.ids.get('door-side').value = 'right'; ui.ids.get('door-side').fire('change'); ui.ids.get('door-rule').value = 'required'; ui.ids.get('door-rule').fire('change');
  ui.ids.get('export-button').fire('click'); assert.equal(ui.ids.get('export-dialog').open, true);
  const data = JSON.parse(ui.ids.get('export-code').value), door=data.objects.find(object=>object.role==='door'); assert.equal(door.side, 'right'); assert.equal('destination' in door, false);
});

test('all game profiles import through the UI and export without metadata loss',async()=>{
  for(const name of ['inicio_menu','inicio','boss_1_ant','fogata','cuarto_final_escaleras','cuarto_final_escaleras_2','cuarto_1','cuarto_1_2','cuarto_2','cuarto_2_2','cuarto_3','cuarto_4','cuarto_5','cuarto_6','mi_primer_cuarto']) {
    const raw=JSON.parse(await readFile(new URL(`../examples/game-rooms/${name}.json`,import.meta.url),'utf8'));
    const ui=await setup(false);
    ui.ids.get('import-code').value=JSON.stringify(raw); ui.ids.get('import-code-button').fire('click');
    assert.equal(ui.room().id,raw.id,name);
    ui.ids.get('export-button').fire('click');
    assert.equal(ui.ids.get('export-dialog').open,true,ui.ids.get('toast').textContent);
    assert.deepEqual(JSON.parse(ui.ids.get('export-code').value),raw,name);
  }
});

test('native required Q, NPC slots, backgrounds and collision profile are editable without losing their binding',async()=>{
  const raw=JSON.parse(await readFile(new URL('../examples/game-rooms/fogata.json',import.meta.url),'utf8'));
  const ui=await setup(false,raw);
  const native=ui.room();
  const npc=native.markers.find(marker=>marker.type==='npc_delivery');
  ui.ids.get('marker-select').value=npc.id; ui.ids.get('marker-select').fire('change');
  assert.equal(ui.ids.get('marker-slot-label').hidden,false);
  ui.ids.get('marker-x').value=npc.x+1; ui.ids.get('marker-x').fire('change');
  ui.ids.get('background-option-room2').checked=true; ui.ids.get('background-option-room2').fire('change');
  ui.ids.get('export-button').fire('click');
  const output=JSON.parse(ui.ids.get('export-code').value);
  assert.equal(output.runtimeBinding,'fogata');
  assert.equal(output.markers.find(marker=>marker.id===npc.id).x,npc.x+1);
  assert.equal(output.markers.find(marker=>marker.id===npc.id).slot,npc.slot);
  assert.deepEqual(output.backgroundOptions,['room_campfire','room2']);
});

test('the game template picker loads a complete Boss 1 including all mandatory rewards',async()=>{
  const ui=await setup(false);
  ui.ids.get('game-template').value='boss_1_ant';
  await ui.ids.get('open-game-template').fire('click');
  assert.equal(ui.room().runtimeBinding,'boss_1_ant');
  assert.equal(ui.ids.get('room-type').disabled,true);
  ui.ids.get('export-button').fire('click');
  const result=JSON.parse(ui.ids.get('export-code').value);
  assert.equal(result.markers.filter(marker=>marker.type==='random'&&marker.required).length,3);
  assert.equal(result.markers.filter(marker=>marker.type==='ability'&&marker.required).length,1);
  assert.equal(result.objects.find(object=>object.role==='door').side,'right');
});
