import { GAME_PROFILES, isComposition, decodeComposition, encodeComposition, compositionErrors, markerLabel } from './editor-game-format.js?v=20261030-3';
export { GAME_PROFILES, isComposition, markerLabel };
// All exported coordinates use the top-left of the complete 256 × 256 BMP.
// Keep this module independent of the browser so import/export stays testable.
export const ROOM_WIDTH = 256;
export const ROOM_HEIGHT = 256;
export const BACKGROUND_SIZE = 256;
export const BACKGROUND_CROP = Object.freeze({ x: 0, y: 0, width: ROOM_WIDTH, height: ROOM_HEIGHT });
export const GAME_GRID = Object.freeze({ x: 8, y: 48, cellSize: 16, columns: 15, rows: 10 });
export const DOOR_SIDES = ['left', 'right', 'up', 'down'];
export const TILE_SIZE = 16;
export const FORMAT = 'anotherdungeon-room';

const sprite = (id, name, file, category, width = 16, height = 16, frames = 1, solid = false) =>
  ({ id, name, file: `${file}.bmp`, category, width, height, frames, solid });

export const BACKGROUNDS = [
  sprite('room2', 'Ruinas', 'room2', 'background', 256, 256),
  sprite('room3', 'Piedra', 'room3', 'background', 256, 256),
  sprite('room_campfire', 'Refugio', 'room_campfire', 'background', 256, 256),
  sprite('land', 'Cuarto limpio', 'land', 'background', 256, 256),
  sprite('room_tutorial', 'Tutorial', 'room_tutorial', 'background', 256, 256),
  sprite('boss_1', 'Boss 1', 'boss_1', 'background', 256, 256),
  sprite('boss_2', 'Boss 2', 'boss_2', 'background', 256, 256),
  sprite('menu_inicio', 'Inicio Menú', 'menu_inicio', 'background', 256, 256),
  sprite('boss_rat', 'Boss Rat', 'boss_rat', 'background', 256, 256),
];

export const SPRITES = [
  sprite('campfire', 'Fogata', 'campfire', 'props', 16, 32, 4, true),
  sprite('column', 'Columna', 'column_16x32', 'props', 16, 32, 1, true),
  sprite('library_front', 'Librero · frente', 'librery_front_16x32', 'props', 16, 32, 1, true),
  sprite('library_back', 'Librero · reverso', 'librery_back_16x32', 'props', 16, 32, 1, true),
  sprite('table', 'Mesa', 'table_16x32', 'props', 32, 16, 1, true),
  sprite('table_armor', 'Mesa con armadura', 'table_armor_16x32', 'props', 32, 16, 1, true),
  sprite('seat', 'Asiento', 'asiento_16x16_4state_down_up_left_right', 'props', 16, 16, 4, false),
  sprite('box', 'Caja', 'box_16x16', 'props', 16, 16, 1, true),
  sprite('vase', 'Jarrón', 'jarron_16x16', 'props', 16, 16, 1, true),
  sprite('rock', 'Piedra', 'piedra1', 'props', 16, 16, 1, true),
  sprite('sign', 'Cartel', 'cartel_16x16', 'props', 16, 16, 1, true),
  sprite('stairs', 'Escaleras', 'stairs', 'doors'),
  sprite('door', 'Puerta abierta', 'door', 'doors', 16, 32),
  sprite('door_special', 'Puerta especial', 'door_special', 'doors', 16, 32),
  sprite('door_closed', 'Puerta cerrada', 'close_door', 'doors', 16, 32, 1, true),
  sprite('door_special_closed', 'Puerta especial cerrada', 'close_door_special', 'doors', 16, 32, 1, true),
  sprite('chest', 'Cofre', 'cheest_16x16_animate_3frames', 'items', 16, 16, 3, true),
  sprite('key', 'Llave', 'key', 'items'),
  sprite('energy', 'Poción de energía', 'posion_energy_16x16_animate', 'items', 16, 16, 3),
  sprite('sprint', 'Poción de sprint', 'posion_sprint_16x16_animate', 'items', 16, 16, 3),
  sprite('label', 'Palanca', 'label_16x16_animate', 'items', 16, 16, 3),
  sprite('spikes', 'Pinchos', 'spikes_16x16_animate', 'hazards', 16, 16, 3),
  sprite('axe', 'Hacha de suelo', 'axe_floor_16x16', 'hazards'),
  sprite('skeleton', 'Esqueleto', 'skeleton', 'enemies', 16, 16, 16),
  sprite('bat', 'Murciélago', 'bat', 'enemies', 16, 16, 16),
  sprite('plant', 'Planta', 'plant_enemy', 'enemies', 16, 16, 16),
  sprite('mushy', 'Hongo', 'mushy', 'enemies', 16, 16, 16),
  sprite('bullet', 'Proyectil de planta', 'bullet_enemy_plant', 'hazards', 8, 8),
  sprite('player', 'Jugador / referencia', 'player', 'actors', 16, 16, 16)
];
export const ASSETS = [...BACKGROUNDS, ...SPRITES];
export const ASSET_MAP = new Map(ASSETS.map(asset => [asset.id, asset]));
export const uid = () => globalThis.crypto?.randomUUID?.() ?? `room-${Date.now()}-${Math.random().toString(36).slice(2)}`;
export const clone = value => JSON.parse(JSON.stringify(value));
export const clamp = (value, min, max) => Math.max(min, Math.min(max, value));
export const snap = (value, step) => Math.round(value / step) * step;

export function createRoom() {
  return {
    format: FORMAT, schemaVersion: 2, id: uid(), name: 'Mi primer cuarto', description: '',
    width: ROOM_WIDTH, height: ROOM_HEIGHT, tileSize: TILE_SIZE,
    coordinateSystem: 'top-left-pixels', background: 'room2', backgroundCrop: { ...BACKGROUND_CROP },
    gameGrid: { ...GAME_GRID }, rules: defaultRules(),
    spawn: null, tiles: [], collisions: [], objects: [], markers: []
  };
}

export function defaultRules() {
  return { roomType: 'standard', onlyTheseSides: false, allowedDoorSides: [...DOOR_SIDES], access: 'normal' };
}

export function defaultHitbox(assetId) {
  const asset = ASSET_MAP.get(assetId);
  return asset.width === 16 && asset.height === 16 ? { x: 3, y: 0, width: 10, height: 8 }
    : { x: 0, y: Math.max(0, asset.height - 8), width: asset.width, height: Math.min(8, asset.height) };
}

export function behaviorDefinition(type) {
  if (type === 'chest') return { type, initialState: 'closed', frames: { closed: 0, transition: 1, openNearby: 2 } };
  if (type === 'lever') return { type, initialState: 'initial', frames: { initial: 0, transition: 1, activated: 2 }, staysVisible: true };
  return null;
}

export function snapToGameGrid(x, y, width = 1, height = 1) {
  const col = clamp(Math.round((x - GAME_GRID.x) / 16), 0, Math.min(14, Math.floor((ROOM_WIDTH - width - GAME_GRID.x) / 16)));
  const row = clamp(Math.round((y - GAME_GRID.y) / 16), 0, Math.min(9, Math.floor((ROOM_HEIGHT - height - GAME_GRID.y) / 16)));
  return { x: GAME_GRID.x + col * 16, y: GAME_GRID.y + row * 16 };
}

export function createMarker(type, x, y) {
  if (!['Q', '&'].includes(type)) throw new Error('Tipo de marcador desconocido.');
  return { id: uid(), type, x: clamp(Math.round(x), 0, 255), y: clamp(Math.round(y), 0, 255), ...(type === '&' ? { required: false } : {}) };
}

export function objectSize(object) {
  const asset = ASSET_MAP.get(object.assetId);
  return object.rotation % 180 ? { width: asset.height, height: asset.width } : { width: asset.width, height: asset.height };
}

export function createObject(assetId, x, y) {
  const asset = ASSET_MAP.get(assetId);
  if (!asset || asset.category === 'background') throw new Error('Sprite desconocido.');
  return {
    id: uid(), assetId, x: clamp(x, 0, ROOM_WIDTH - asset.width), y: clamp(y, 0, ROOM_HEIGHT - asset.height),
    frame: 0, rotation: 0, flipX: false, solid: asset.category === 'doors' ? false : asset.solid,
    hitbox: defaultHitbox(assetId),
    // Destination and role are authoring metadata; a game importer decides behavior.
    role: asset.category === 'doors' ? (assetId === 'stairs' ? 'stairs' : 'door') : asset.category === 'enemies' ? 'enemy' : assetId === 'campfire' ? 'interactive' : assetId === 'axe' ? 'decoration' : assetId === 'label' ? 'lever' : asset.category === 'items' ? 'item' : asset.category === 'hazards' ? 'hazard' : 'decoration',
    ...(asset.category === 'doors' && assetId !== 'stairs' ? { side: null, rule: null } : {}),
    ...(asset.category === 'enemies' ? { enemy: { spawnMode: 'fixed', patrol: null } } : {}),
    behavior: behaviorDefinition(assetId === 'chest' ? 'chest' : assetId === 'label' ? 'lever' : null)
  };
}

// Transform the local hitbox exactly as the image is transformed around its centre.
export function objectCollision(object) {
  const asset = ASSET_MAP.get(object.assetId);
  const box = object.hitbox;
  const size = objectSize(object);
  const angle = object.rotation * Math.PI / 180;
  const corners = [[box.x, box.y], [box.x + box.width, box.y], [box.x, box.y + box.height], [box.x + box.width, box.y + box.height]].map(([x, y]) => {
    x = (object.flipX ? asset.width - x : x) - asset.width / 2;
    y -= asset.height / 2;
    return [Math.round(object.x + size.width / 2 + x * Math.cos(angle) - y * Math.sin(angle)),
      Math.round(object.y + size.height / 2 + x * Math.sin(angle) + y * Math.cos(angle))];
  });
  const x = Math.min(...corners.map(p => p[0]));
  const y = Math.min(...corners.map(p => p[1]));
  return { x, y, width: Math.max(...corners.map(p => p[0])) - x, height: Math.max(...corners.map(p => p[1])) - y };
}

export function contains(rect, point) {
  return point.x >= rect.x && point.y >= rect.y && point.x < rect.x + rect.width && point.y < rect.y + rect.height;
}

export function intersectRects(a, b) {
  const x = Math.max(a.x, b.x), y = Math.max(a.y, b.y);
  const width = Math.min(a.x + a.width, b.x + b.width) - x;
  const height = Math.min(a.y + a.height, b.y + b.height) - y;
  return width > 0 && height > 0 ? { x, y, width, height } : null;
}

function slicePatch(patch, rect) {
  const result = { ...patch, ...rect };
  if (patch.id) result.id = uid();
  if ('assetId' in patch) { result.sx = patch.sx + rect.x - patch.x; result.sy = patch.sy + rect.y - patch.y; }
  return result;
}

// Keep neighbouring fragments when painting over migrated tiles/collision cells.
export function replaceRegion(patches, rect, replacement = null) {
  const result = patches.flatMap(patch => {
    const cut = intersectRects(patch, rect);
    if (!cut) return [patch];
    return [
      { x: patch.x, y: patch.y, width: patch.width, height: cut.y - patch.y },
      { x: patch.x, y: cut.y + cut.height, width: patch.width, height: patch.y + patch.height - cut.y - cut.height },
      { x: patch.x, y: cut.y, width: cut.x - patch.x, height: cut.height },
      { x: cut.x + cut.width, y: cut.y, width: patch.x + patch.width - cut.x - cut.width, height: cut.height }
    ].filter(part => part.width > 0 && part.height > 0).map(part => slicePatch(patch, part));
  });
  if (replacement) result.push(replacement);
  return result;
}

function validateEditorRoom(value, { forExport = false } = {}) {
  const fail = message => { throw new Error(message); };
  const integer = (n, min, max) => Number.isInteger(n) && n >= min && n <= max;
  const string = (s, max) => typeof s === 'string' && s.length <= max;
  if (!value || value.format !== FORMAT || ![1, 2].includes(value.schemaVersion)) fail('El archivo no es un cuarto compatible (versión 1 o 2).');
  const legacy = value.schemaVersion === 1;
  const cropped = value.schemaVersion === 2 && value.width === 240 && [160, 180].includes(value.height);
  const width = cropped ? 240 : ROOM_WIDTH, height = cropped ? value.height : ROOM_HEIGHT;
  const sourceCrop = cropped ? { x: 8, y: (256 - height) / 2, width, height } : BACKGROUND_CROP;
  if (value.width !== width || value.height !== height || value.tileSize !== TILE_SIZE || value.coordinateSystem !== 'top-left-pixels') fail(`El cuarto debe usar ${width} × ${height} píxeles y tiles de 16 px.`);
  if (!legacy && (!value.backgroundCrop || !Object.entries(sourceCrop).every(([key, expected]) => value.backgroundCrop[key] === expected))) fail('El recorte del fondo no coincide con las dimensiones del archivo.');
  if (value.gameGrid && !Object.entries(GAME_GRID).every(([key, expected]) => value.gameGrid[key] === expected)) fail('La cuadrícula del juego debe ser de 15 × 10 celdas de 16 px con origen 8, 48.');
  if (!string(value.id, 100) || !value.id || !string(value.name, 80) || !value.name.trim() || !string(value.description, 1000)) fail('El nombre o la descripción del cuarto no son válidos.');
  if (!BACKGROUNDS.some(a => a.id === value.background)) fail('El fondo del cuarto no existe en el catálogo.');
  if (value.spawn != null && (!integer(value.spawn.x, 0, width - 1) || !integer(value.spawn.y, 0, height - 1))) fail('El punto de aparición opcional no es válido.');
  if (!Array.isArray(value.objects) || value.objects.length > 500 || !Array.isArray(value.tiles) || value.tiles.length > (legacy ? 256 : 2048) || !Array.isArray(value.collisions) || value.collisions.length > (legacy ? 1024 : 4096)) fail('El archivo contiene demasiados elementos o capas inválidas.');
  const ids = new Set();
  const enumValue = (v, options, label, nullable = false) => { if (!(nullable && v === null) && !options.includes(v)) fail(`${label}: valor incompatible «${v}».`); return v; };
  const claimId = (id, label) => { if (!string(id, 100) || !id || ids.has(id)) fail(`${label}: identificador vacío o repetido.`); ids.add(id); };
  const inputRules = value.rules ?? defaultRules();
  if (!inputRules || typeof inputRules.onlyTheseSides !== 'boolean' || !Array.isArray(inputRules.allowedDoorSides) || inputRules.allowedDoorSides.length > 4 || new Set(inputRules.allowedDoorSides).size !== inputRules.allowedDoorSides.length || !inputRules.allowedDoorSides.every(side => DOOR_SIDES.includes(side))) fail('Los lados permitidos del cuarto no son válidos.');
  const roomType = enumValue(inputRules.roomType, ['standard', 'boss', 'boss1'], 'Tipo de cuarto');
  const rules = { roomType: roomType === 'boss1' ? 'boss' : roomType, onlyTheseSides: inputRules.onlyTheseSides,
    allowedDoorSides: [...inputRules.allowedDoorSides], access: enumValue(inputRules.access, ['normal', 'key', 'three_levers'], 'Condición de acceso') };
  const objects = value.objects.map(object => {
    const asset = object && ASSET_MAP.get(object.assetId);
    if (!asset || asset.category === 'background') fail('Hay sprites desconocidos.');
    claimId(object.id, asset.name);
    if (![0, 90, 180, 270].includes(object.rotation) || typeof object.flipX !== 'boolean' || typeof object.solid !== 'boolean' || !integer(object.frame, 0, asset.frames - 1)) fail(`Propiedades inválidas en ${asset.name}.`);
    const size = objectSize(object);
    if (!integer(object.x, 0, width - size.width) || !integer(object.y, 0, height - size.height)) fail(`El sprite ${asset.name} está fuera del cuarto.`);
    const box = object.hitbox;
    if (!box || !integer(box.width, 1, asset.width) || !integer(box.height, 1, asset.height) || !integer(box.x, 0, asset.width - box.width) || !integer(box.y, 0, asset.height - box.height)) fail(`La colisión de ${asset.name} no es válida.`);
    enumValue(object.role, ['decoration', 'enemy', 'door', 'stairs', 'item', 'lever', 'interactive', 'npc', 'hazard'], 'Función del objeto');
    const result = { id: object.id, assetId: asset.id, x: object.x + sourceCrop.x, y: object.y + sourceCrop.y, frame: object.frame, rotation: object.rotation, flipX: object.flipX, solid: object.solid,
      hitbox: { x: box.x, y: box.y, width: box.width, height: box.height }, role: object.role };
    if (asset.id === 'spikes') {
      if (typeof (object.alwaysActive ?? false) !== 'boolean') fail('Pinchos: alwaysActive debe ser true o false.');
      result.alwaysActive = object.alwaysActive ?? false;
    }
    if (object.role === 'door') {
      if (value.rules && object.destination) fail('Una puerta de mazmorra no admite destination: el generador decide su conexión con la semilla.');
      const bossSide = rules.roomType === 'boss' && rules.allowedDoorSides.length === 1 ? rules.allowedDoorSides[0] : null;
      result.side = enumValue(object.side ?? bossSide, DOOR_SIDES, `Lado de ${asset.name}`, true);
      result.rule = enumValue(object.rule ?? 'possible', ['possible', 'required'], `Regla de ${asset.name}`);
    }
    if (object.role === 'enemy') {
      const enemy = object.enemy ?? { spawnMode: 'fixed', patrol: null };
      result.enemy = { spawnMode: enumValue(enemy.spawnMode, ['fixed', 'seeded'], `Aparición de ${asset.name}`), patrol: null };
      if (enemy.patrol !== null && enemy.patrol !== undefined) {
        const p = enemy.patrol, axis = enumValue(p.axis, ['horizontal', 'vertical'], 'Eje de patrulla');
        const limit = axis === 'horizontal' ? width - size.width : height - size.height;
        for (const key of ['from', 'to']) if (p[key] !== null && !integer(p[key], 0, limit)) fail(`Patrulla de ${asset.name}: ${key} debe ser un extremo en píxeles dentro del cuarto.`);
        const offset = axis === 'horizontal' ? sourceCrop.x : sourceCrop.y;
        result.enemy.patrol = { axis, from: p.from === null ? null : p.from + offset, to: p.to === null ? null : p.to + offset };
      }
    }
    const behavior = object.behavior === undefined ? behaviorDefinition(asset.id === 'chest' ? 'chest' : asset.id === 'label' ? 'lever' : null) : object.behavior;
    result.behavior = null;
    if (behavior !== null) {
      const definition = behaviorDefinition(behavior?.type);
      if (!definition || asset.frames < 3 || behavior.initialState !== definition.initialState || !behavior.frames || !Object.entries(definition.frames).every(([key, frame]) => behavior.frames[key] === frame) || (definition.type === 'lever' && behavior.staysVisible !== true)) fail(`Comportamiento incompatible en ${asset.name}: usa los estados y cuadros 0, 1 y 2 documentados.`);
      result.behavior = definition;
    }
    return result;
  });
  const cells = new Set();
  const tiles = value.tiles.map(tile => {
    if (!tile || !BACKGROUNDS.some(a => a.id === tile.assetId)) fail('Hay tiles inválidos.');
    const patchWidth = legacy ? TILE_SIZE : tile.width, patchHeight = legacy ? TILE_SIZE : tile.height;
    if (!integer(patchWidth, 1, TILE_SIZE) || !integer(patchHeight, 1, TILE_SIZE) || !integer(tile.x, 0, width - patchWidth) || !integer(tile.y, 0, height - patchHeight) || !integer(tile.sx, 0, BACKGROUND_SIZE - patchWidth) || !integer(tile.sy, 0, BACKGROUND_SIZE - patchHeight)) fail('Hay tiles fuera de los límites del cuarto o del fondo.');
    if (legacy && ![tile.x, tile.y, tile.sx, tile.sy].every(n => n % TILE_SIZE === 0)) fail('Hay tiles antiguos inválidos.');
    const key = `${tile.x},${tile.y}`;
    if (cells.has(key)) fail('Hay tiles repetidos en la misma celda.');
    cells.add(key);
    return { x: tile.x + sourceCrop.x, y: tile.y + sourceCrop.y, assetId: tile.assetId, sx: tile.sx, sy: tile.sy, width: patchWidth, height: patchHeight };
  });
  const collisions = value.collisions.map(box => {
    if (!box || !integer(box.width, 1, width) || !integer(box.height, 1, height) || !integer(box.x, 0, width - box.width) || !integer(box.y, 0, height - box.height)) fail('Hay colisiones fuera de los límites del cuarto.');
    return { x: box.x + sourceCrop.x, y: box.y + sourceCrop.y, width: box.width, height: box.height };
  });
  if (value.markers !== undefined && (!Array.isArray(value.markers) || value.markers.length > 1000)) fail('La colección de marcadores no es válida.');
  const markers = (value.markers ?? []).map(marker => {
    if (!marker) fail('Marcador no válido.'); claimId(marker.id, 'Marcador');
    enumValue(marker.type, ['Q', '&'], 'Tipo de marcador');
    if (!integer(marker.x, 0, width - 1) || !integer(marker.y, 0, height - 1)) fail('Marcador fuera del cuarto.');
    if ((marker.type === '&' && typeof marker.required !== 'boolean') || (marker.type === 'Q' && marker.required === true)) fail('Solo las recompensas & pueden marcarse como obligatorias.');
    if ('content' in marker || 'assetId' in marker || 'enemy' in marker) fail('Un marcador no puede contener un sprite o contenido ya generado. Lo resuelve el juego con la semilla.');
    return { id: marker.id, type: marker.type, x: marker.x + sourceCrop.x, y: marker.y + sourceCrop.y, ...(marker.type === '&' ? { required: marker.required } : {}) };
  });
  // Explicit reconstruction strips untrusted properties and keeps exports stable.
  const normalized = { format: FORMAT, schemaVersion: 2, id: value.id, name: value.name.trim(), description: value.description,
    width: ROOM_WIDTH, height: ROOM_HEIGHT, tileSize: TILE_SIZE, coordinateSystem: 'top-left-pixels', background: value.background, backgroundCrop: { ...BACKGROUND_CROP }, gameGrid: { ...GAME_GRID }, rules,
    spawn: value.spawn == null ? null : { x: value.spawn.x + sourceCrop.x, y: value.spawn.y + sourceCrop.y }, tiles, collisions, objects, markers };
  if (forExport) {
    const errors = gameplayErrors(normalized);
    if (errors.length) fail(errors.join('\n'));
  }
  return normalized;
}

function editorGameplayErrors(room) {
  const errors = [], doors = room.objects.filter(o => o.role === 'door');
  const seen = new Set();
  for (const door of doors) {
    if (!DOOR_SIDES.includes(door.side) || !['possible', 'required'].includes(door.rule)) errors.push(`Puerta ${door.id}: selecciona lado y regla (r posible / R obligatoria).`);
    if (door.side && seen.has(door.side)) errors.push(`Hay más de una puerta en el lado ${door.side}.`);
    seen.add(door.side);
    if (room.rules.onlyTheseSides && door.side && !room.rules.allowedDoorSides.includes(door.side)) errors.push(`La puerta ${door.side} contradice «solo estos lados».`);
    if (door.destination) errors.push('Las puertas de mazmorra no pueden tener destinos concretos.');
  }
  for (const object of room.objects) {
    if (object.assetId === 'seat' && object.solid) errors.push(`Asiento ${object.id}: desactiva «Bloquea el paso» o aplica los valores recomendados.`);
    const p = object.enemy?.patrol;
    if (object.role === 'enemy' && p) {
      const position = p.axis === 'horizontal' ? object.x : object.y;
      if (!Number.isInteger(p.from) || !Number.isInteger(p.to) || p.from >= p.to) errors.push(`Enemigo ${object.id}: indica dos extremos de patrulla en orden creciente.`);
      else if (position < p.from || position > p.to) errors.push(`Enemigo ${object.id}: su posición inicial está fuera de la patrulla.`);
    }
  }
  return errors;
}

export function applyBossPreset(room) {
  room.rules.roomType = 'boss';
  return room;
}

export function roomWarnings(room) {
  const warnings = [];
  if (!room.objects.some(o => o.role === 'door' || o.role === 'stairs')) warnings.push('El cuarto no tiene puertas ni escaleras.');
  return warnings;
}

// Decode palette indices, not RGB guesses: green differs between 4-bit and 8-bit BMPs.
// Only palette index 0 is transparent, and only when that entry is a green key.
export function decodeBMP(buffer) {
  const view = new DataView(buffer);
  if (view.byteLength < 54 || view.getUint16(0, true) !== 0x4d42) throw new Error('BMP no válido.');
  const offset = view.getUint32(10, true);
  const headerSize = view.getUint32(14, true);
  const width = view.getInt32(18, true);
  const signedHeight = view.getInt32(22, true);
  const height = Math.abs(signedHeight);
  const depth = view.getUint16(28, true);
  const compression = view.getUint32(30, true);
  if (headerSize < 40 || ![4, 8].includes(depth) || compression !== 0 || width <= 0 || width > 4096 || height < 1 || height > 4096) throw new Error('Se necesita un BMP de paleta de 4 u 8 bits sin compresión.');
  const paletteOffset = 14 + headerSize;
  const count = view.getUint32(46, true) || (1 << depth);
  const stride = Math.ceil(width * depth / 32) * 4;
  if (count > (1 << depth) || paletteOffset + count * 4 > offset || offset + stride * height > view.byteLength) throw new Error('El BMP está incompleto.');
  const palette = Array.from({ length: count }, (_, i) => {
    const p = paletteOffset + i * 4;
    return [view.getUint8(p + 2), view.getUint8(p + 1), view.getUint8(p)];
  });
  const key = palette[0];
  const transparent = key[0] <= 8 && key[1] >= 248 && key[2] <= 8;
  const data = new Uint8ClampedArray(width * height * 4);
  for (let y = 0; y < height; y++) {
    const row = offset + (signedHeight > 0 ? height - 1 - y : y) * stride;
    for (let x = 0; x < width; x++) {
      const byte = view.getUint8(row + (depth === 8 ? x : Math.floor(x / 2)));
      const index = depth === 8 ? byte : x % 2 === 0 ? byte >> 4 : byte & 15;
      const color = palette[index];
      if (!color) throw new Error('Índice de paleta inválido.');
      const p = (y * width + x) * 4;
      data[p] = color[0]; data[p + 1] = color[1]; data[p + 2] = color[2];
      data[p + 3] = transparent && index === 0 ? 0 : 255;
    }
  }
  return { width, height, data, key, transparent };
}

const gameContext = () => ({ behaviorDefinition, assetMap: ASSET_MAP });

function assertCompositionShape(value) {
  if (!Object.hasOwn(GAME_PROFILES, value.runtimeBinding) || value.width !== 256 || value.height !== 256 || value.tileSize !== 16 || value.coordinateSystem !== 'top-left-pixels') throw new Error('Perfil o dimensiones de composición incompatibles.');
  if (![value.objects,value.markers,value.collisions,value.tiles].every(Array.isArray) || value.objects.length > 500 || value.markers.length > 1000 || value.collisions.length > 4096) throw new Error('Capas de composición inválidas.');
  if (!ASSET_MAP.has(value.background) || !value.backgroundOptions?.every(id => ASSET_MAP.get(id)?.category === 'background')) throw new Error('Fondo de composición desconocido.');
  const ids = new Set();
  for (const item of [...value.objects,...value.markers,...value.collisions]) {
    if (!item || typeof item.id !== 'string' || !item.id || ids.has(item.id)) throw new Error('Los IDs de objetos, marcadores y colisiones deben ser únicos.');
    ids.add(item.id);
    if (!Number.isInteger(item.x) || !Number.isInteger(item.y) || item.x < 0 || item.x > 256 || item.y < 0 || item.y > 256) throw new Error('Posición fuera del lienzo.');
  }
  for (const object of value.objects) {
    const id = ({ chair:'seat',weapon_table:'table_armor',stone:'rock',lever:'label' })[object.assetId] ?? object.assetId;
    const asset=ASSET_MAP.get(id), box=object.hitbox;
    if (!asset || asset.category==='background') throw new Error(`Sprite desconocido: ${object.assetId}.`);
    if (![0,90,180,270].includes(object.rotation) || !Number.isInteger(object.frame) || object.frame < 0 || object.frame >= asset.frames || typeof object.solid !== 'boolean' || typeof object.flipX !== 'boolean') throw new Error(`Propiedades inválidas en ${asset.name}.`);
    if (!box || !['x','y','width','height'].every(key=>Number.isInteger(box[key])) || box.x<0 || box.y<0 || box.width<1 || box.height<1 || box.x+box.width>asset.width || box.y+box.height>asset.height) throw new Error(`Hitbox inválida en ${asset.name}.`);
  }
  if (value.markers.some(marker=>!['random','ability','npc_delivery'].includes(marker.type))) throw new Error('Marcador de composición desconocido.');
  if (value.collisions.some(rect=>!Number.isInteger(rect.width)||!Number.isInteger(rect.height)||rect.width<1||rect.height<1||rect.x+rect.width>256||rect.y+rect.height>256)) throw new Error('Colisión invisible fuera del lienzo.');
}

export function validateRoom(value, options = {}) {
  if (isComposition(value)) {
    const raw=value._game?.kind==='composition' ? encodeComposition(value,gameContext()) : clone(value);
    assertCompositionShape(raw);
    if (options.forExport) {
      const errors=compositionErrors(raw,ASSET_MAP);
      if(errors.length) throw new Error(errors.join('\n'));
    }
    return decodeComposition(raw,gameContext());
  }
  const result=validateEditorRoom(value,value.schemaVersion===1 ? {...options,forExport:false} : options);
  if(value._game?.kind==='v1') result._game=clone(value._game);
  else if(value.schemaVersion===1) {
    result._game={kind:'v1',original:clone(value)};
    // Legacy files describe door sides through geometry; retain those authored
    // sides instead of forcing the user to recreate every door by hand.
    for(const object of result.objects.filter(object=>object.role==='door')) {
      const size=objectSize(object), x=object.x+size.width/2, y=object.y+size.height/2;
      const distances=[x,256-x,y,256-y];
      object.side ??= DOOR_SIDES[distances.indexOf(Math.min(...distances))];
      object.rule ??= 'possible';
    }
    result.rules={roomType:'standard',onlyTheseSides:true,allowedDoorSides:result.objects.filter(object=>object.role==='door').map(object=>object.side),access:'normal'};
  }
  if(options.forExport && value.schemaVersion===1) {
    const errors=editorGameplayErrors(result);
    if(errors.length) throw new Error(errors.join('\n'));
  }
  return result;
}

export function gameplayErrors(room) {
  if(isComposition(room)) {
    try { return compositionErrors(encodeComposition(room,gameContext()),ASSET_MAP); }
    catch(error) { return [error.message]; }
  }
  return editorGameplayErrors(room);
}

export function prepareCompositionChanges(room) {
  const originals=new Map(room._game.original.objects.map(object=>[object.id,object]));
  const role={campfire:'interactive',stairs:'interactive',label:'lever',spikes:'hazard',bat:'enemy',skeleton:'enemy',plant:'enemy',mushy:'enemy'};
  for(const object of room.objects) {
    if(originals.has(object.id)) continue;
    object.role=role[object.assetId] ?? (object.assetId==='door' ? 'door' : 'decoration');
    object.collisionProfile ??= 'explicit';
    if(object.role==='enemy') {
      object.enemy ??= {spawnMode:'seeded',patrol:null}; object.enemy.spawnMode='seeded'; object.solid=false;
      if(!Number.isInteger(object.runtimeSlot)) {
        const used=new Set(room.objects.filter(other=>other!==object&&other.assetId===object.assetId).map(other=>other.runtimeSlot));
        object.runtimeSlot=[0,1,2,3].find(slot=>!used.has(slot)) ?? 4;
      }
    }
    if(object.assetId==='spikes') object.alwaysActive ??= false;
  }
  for(const rect of room.collisions) rect.id ??= uid();
}

function legacyGameExport(room) {
  const before=room._game.original, raw=clone(before);
  for(const key of ['id','name','description','background','spawn','tiles','collisions']) raw[key]=clone(room[key]);
  raw.schemaVersion=1;
  raw.objects=room.objects.map(object=>{
    const source=before.objects.find(item=>item.id===object.id), result=source?clone(source):{};
    for(const key of ['id','assetId','x','y','frame','rotation','flipX','solid','hitbox','role']) result[key]=clone(object[key]);
    if(object.assetId==='chest' && object.role==='item') result.role='decoration';
    if(object.role==='door') result.destination='';
    return result;
  });
  return raw;
}

export function gameExportErrors(room) {
  if(isComposition(room)) return gameplayErrors(room);
  const errors=[];
  if(room.rules.roomType==='boss') {
    if(room.rules.access!=='three_levers') errors.push('Las salas boss del juego se abren con las tres palancas.');
    if(!room.rules.onlyTheseSides || room.rules.allowedDoorSides.length!==1 || room.objects.filter(object=>object.role==='door').length!==1) errors.push('Cada sala boss necesita una sola puerta y un único lado de entrada.');
    if(room.markers.filter(marker=>marker.type==='Q').length>3 || room.markers.filter(marker=>marker.type==='&').length>1) errors.push('Las salas boss admiten tres puntos Q y un punto & para recompensas.');
  }
  else if(room.rules.roomType!=='standard'||room.rules.access!=='normal') errors.push('Las salas estándar usan acceso normal.');
  if(room.tiles.length) errors.push('Los tiles pintados aún requieren un perfil original; usa el fondo completo del cuarto adicional.');
  if(room.collisions.length>32) errors.push('El cuarto adicional admite hasta 32 colisiones invisibles (%).');
  if(room.markers.filter(marker=>marker.type==='Q').length>16) errors.push('El cuarto adicional admite hasta 16 puntos Q.');
  if(room.markers.filter(marker=>marker.type==='&').length>4) errors.push('El cuarto adicional admite hasta 4 recompensas &.');
  if(new Set(room.markers.filter(marker=>marker.type==='&').map(marker=>`${marker.x},${marker.y}`)).size!==room.markers.filter(marker=>marker.type==='&').length) errors.push('Las recompensas & necesitan posiciones distintas.');
  if(room.objects.filter(object=>['key','energy','sprint'].includes(object.assetId)).length+room.markers.filter(marker=>marker.type==='&').length>4) errors.push('Los recogibles colocados y las recompensas & suman como máximo 4 por cuarto.');
  const supported=new Set(['table','library_front','library_back','column','seat','box','vase','rock','sign','axe','spikes','skeleton','bat','plant','mushy','chest','key','energy','sprint','label','campfire','stairs','door','door_special','door_closed','door_special_closed','table_armor','player']);
  if(!['land','room2','room3','room_campfire','boss_1','boss_2','boss_rat'].includes(room.background)) errors.push('Este fondo requiere un perfil del juego.');
  const unsupported=[...new Set(room.objects.filter(object=>!supported.has(object.assetId)).map(object=>ASSET_MAP.get(object.assetId)?.name ?? object.assetId))];
  if(unsupported.length) errors.push(`Estos sprites requieren un perfil original: ${unsupported.join(', ')}. El proyectil de planta se genera al disparar.`);
  if(room.objects.length>40 || ['skeleton','bat','plant','mushy'].some(asset=>room.objects.filter(object=>object.assetId===asset).length>4) || room.objects.filter(object=>object.assetId==='chest').length>1) errors.push('El cuarto adicional admite 40 objetos, hasta 4 enemigos de cada tipo y un cofre.');
  if(room.objects.filter(object=>object.assetId==='spikes').length>12) errors.push('El cuarto adicional admite hasta 12 grupos de pinchos.');
  if(room.objects.filter(object=>['key','energy','sprint'].includes(object.assetId)).length>4) errors.push('El cuarto adicional admite hasta 4 objetos recogibles.');
  if(room.objects.filter(object=>object.assetId==='label').length>1) errors.push('El cuarto adicional admite una posición candidata de palanca.');
  if(room.objects.filter(object=>object.assetId==='campfire').length>1 || room.objects.filter(object=>object.assetId==='stairs').length>1) errors.push('El cuarto adicional admite una fogata y una escalera.');
  const decorations=room.objects.filter(object=>object.role==='decoration'||['chest','axe'].includes(object.assetId));
  if(decorations.length>32) errors.push('El cuarto adicional admite hasta 32 decoraciones.');
  const sides=room.rules.onlyTheseSides?room.rules.allowedDoorSides:DOOR_SIDES;
  const authoredSides=new Set(room.objects.filter(object=>object.role==='door').map(object=>object.side));
  if(room.objects.length+sides.filter(side=>!authoredSides.has(side)).length>40) errors.push('Las puertas generadas también cuentan en el límite de 40 objetos.');
  if(!room.rules.allowedDoorSides.length) errors.push('Selecciona al menos un lado permitido para las puertas.');
  if(room.objects.some(object=>(object.role==='door')!==['door','door_special','door_closed','door_special_closed'].includes(object.assetId) || (object.role==='enemy')!==['skeleton','bat','plant','mushy'].includes(object.assetId) || (object.role==='hazard')!==(object.assetId==='spikes') && !(object.assetId==='axe'&&object.role==='hazard') || (object.role==='item')!==['chest','key','energy','sprint'].includes(object.assetId) && object.assetId!=='chest' || (object.role==='lever')!==(object.assetId==='label') || (object.role==='interactive')!==(object.assetId==='campfire') || (object.role==='stairs')!==(object.assetId==='stairs') || (!['door','enemy','decoration','hazard','item','lever','interactive','stairs'].includes(object.role)))) errors.push('La función del objeto debe coincidir con el sprite admitido por el juego.');
  if(room.objects.some(object=>object.assetId==='spikes'&&(object.rotation||object.flipX||object.solid))) errors.push('Los pinchos del juego requieren rotation=0, flipX=false y solid=false.');
  if(!decorations.length && room.rules.roomType!=='boss') errors.push('El cuarto adicional necesita al menos una decoración.');
  if(room.objects.some(object=>object.role==='enemy'&&(object.solid||object.rotation||object.flipX||object.enemy?.patrol))) errors.push('Los enemigos de cuartos adicionales requieren solid=false, rotation=0, flipX=false y ninguna patrulla explícita. Su aparición puede ser fija o decidida por la semilla.');
  for(const asset of ['bat','mushy']) {
    const points=new Set();
    if(room.objects.some(object=>{
      if(object.assetId!==asset) return false;
      const key=`${object.x},${object.y}`;
      if(points.has(key)) return true;
      points.add(key);
      return false;
    })) errors.push(`Los ${asset==='bat'?'murciélagos':'hongos'} necesitan posiciones distintas para que la semilla identifique cada aparición.`);
  }
  if(room.objects.some(object=>object.solid&&object.rotation)) errors.push('El importador todavía no admite decoraciones sólidas giradas en cuartos adicionales.');
  if(room._game?.kind==='v1' && !room.objects.some(object=>object.role==='door')) errors.push('El cuarto v1 necesita una puerta.');
  return errors;
}

export function serializeGameRoom(value, { forGame = true } = {}) {
  const room=validateRoom(value,{forExport:forGame});
  if(forGame) {
    const errors=gameExportErrors(room);
    if(errors.length) throw new Error(errors.join('\n'));
  }
  if(isComposition(room)) return encodeComposition(room,gameContext());
  if(room._game?.kind==='v1') return legacyGameExport(room);
  const raw=clone(room);
  delete raw._game;
  if(forGame) for(const object of raw.objects) {
    if(!['door','enemy','hazard','item','lever','interactive','stairs'].includes(object.role)) object.role='decoration';
    if(object.assetId==='chest') object.role='decoration';
    if(object.behavior?.type==='chest') object.behavior=null;
    if(object.enemy?.spawnMode==='fixed'&&!object.enemy.patrol) delete object.enemy;
  }
  return raw;
}
