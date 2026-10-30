// Lossless bridge between the editor view and the game's composition profiles.
// This file does not choose seeds, spawn rewards, or generate dungeon floors.
export const GAME_PROFILES = Object.freeze({
  inicio_menu: { name: 'Inicio Menu', kind: 'menu' }, inicio: { name: 'Inicio', kind: 'start' },
  boss_1_ant: { name: 'Boss 1 Ant', kind: 'reward' }, fogata: { name: 'Fogata', kind: 'campfire' },
  cuarto_final_escaleras: { name: 'Cuarto Final Escaleras', kind: 'stairs' },
  cuarto_final_escaleras_2: { name: 'Cuarto Final Escaleras 2', kind: 'stairs' },
  cuarto_1: { name: 'Cuarto 1', kind: 'normal' }, cuarto_1_2: { name: 'Cuarto 1.2', kind: 'normal' },
  cuarto_2: { name: 'Cuarto 2', kind: 'normal' }, cuarto_2_2: { name: 'Cuarto 2.2', kind: 'normal' },
  cuarto_3: { name: 'Cuarto 3', kind: 'normal' }, cuarto_4: { name: 'Cuarto 4', kind: 'normal' },
  cuarto_5: { name: 'Cuarto 5', kind: 'normal' }, cuarto_6: { name: 'Cuarto 6', kind: 'normal' }
});
export const GAME_ALIASES = Object.freeze({ chair: 'seat', weapon_table: 'table_armor', stone: 'rock', lever: 'label' });
const NATIVE_ASSETS = Object.freeze({ seat: 'chair', table_armor: 'weapon_table', rock: 'stone', label: 'lever' });
const copy = value => value === undefined ? undefined : JSON.parse(JSON.stringify(value));
const equal = (a, b) => JSON.stringify(a) === JSON.stringify(b);
const sides = ['left', 'right', 'up', 'down'];
const markerType = type => ({ random: 'Q', ability: '&', npc_delivery: 'npc_delivery' })[type];
const nativeMarker = type => ({ Q: 'random', '&': 'ability', npc_delivery: 'npc_delivery' })[type];
export const isComposition = room => room?._game?.kind === 'composition' || Boolean(room?.runtimeBinding);
export const markerLabel = marker => marker.type === 'npc_delivery' ? `NPC ${Number(marker.slot ?? 0) + 1}` : marker.type;
const uiBehavior = (behavior, definition) => definition(behavior === 'proximity-chest' ? 'chest' : behavior === 'reward-lever' ? 'lever' : null);

export function decodeComposition(raw, { behaviorDefinition }) {
  const original = copy(raw);
  const room = copy(raw);
  room.backgroundCrop = { x: 0, y: 0, width: 256, height: 256 };
  room.gameGrid = { x: 8, y: 48, cellSize: 16, columns: 15, rows: 10 };
  const doors = raw.objects.filter(object => object.role === 'door');
  room.rules = { roomType: raw.roomKind === 'reward' ? 'boss' : 'standard', onlyTheseSides: true,
    allowedDoorSides: doors.map(door => door.side), access: raw.roomKind === 'reward' ? 'three_levers' : 'normal' };
  room.objects = raw.objects.map(object => {
    const result = { ...copy(object), assetId: GAME_ALIASES[object.assetId] ?? object.assetId,
      behavior: uiBehavior(object.behavior, behaviorDefinition) };
    if (object.role === 'enemy') {
      const patrol = object.patrol;
      result.enemy = { spawnMode: object.spawnMode, patrol: patrol ? { axis: patrol.axis,
        from: patrol.from[patrol.axis === 'horizontal' ? 'x' : 'y'] - 8,
        to: patrol.to[patrol.axis === 'horizontal' ? 'x' : 'y'] - 8 } : null };
    }
    return result;
  });
  room.markers = raw.markers.map(marker => ({ ...copy(marker), type: markerType(marker.type) }));
  room._game = { kind: 'composition', original };
  return room;
}

export function encodeComposition(room, { behaviorDefinition, assetMap }) {
  const original = room._game?.original ?? room;
  const raw = copy(original);
  for (const key of ['id', 'name', 'description', 'background', 'backgroundOptions', 'spawn', 'source', 'placement', 'access', 'doorPolicy', 'roomKind', 'runtimeBinding'])
    if (key in room) raw[key] = copy(room[key]);
  const oldObjects = new Map(original.objects.map(object => [object.id, object]));
  raw.objects = room.objects.map(object => {
    const before = oldObjects.get(object.id);
    const result = before ? copy(before) : { destination: '', collisionProfile: 'explicit' };
    const oldAsset = before && (GAME_ALIASES[before.assetId] ?? before.assetId);
    result.assetId = oldAsset === object.assetId ? before.assetId : (NATIVE_ASSETS[object.assetId] ?? object.assetId);
    for (const key of ['id', 'x', 'y', 'frame', 'rotation', 'flipX', 'solid', 'hitbox', 'role', 'side', 'rule', 'collisionProfile', 'runtimeSlot', 'sourceSymbol', 'sourceCell', 'alwaysActive'])
      if (key in object) result[key] = copy(object[key]);
    if (object.role === 'door') {
      result.destination = '';
      result.access = before?.access ?? (room.roomKind === 'reward' ? 'three-levers' : 'normal');
      result.hiddenUntil = before?.hiddenUntil ?? (room.roomKind === 'reward' ? 'three-levers' : '');
    }
    if (object.role === 'enemy') {
      result.spawnMode = object.enemy?.spawnMode ?? object.spawnMode;
      const patrol = object.enemy?.patrol;
      if (patrol) {
        const axis = patrol.axis, coordinate = axis === 'horizontal' ? 'x' : 'y';
        const asset = assetMap.get(object.assetId);
        const fixed = axis === 'horizontal' ? object.y + asset.height / 2 : object.x + asset.width / 2;
        result.patrol = { ...(before?.patrol ?? {}), axis, mode: 'ping-pong',
          from: { x: axis === 'horizontal' ? patrol.from + 8 : fixed, y: axis === 'vertical' ? patrol.from + 8 : fixed },
          to: { x: axis === 'horizontal' ? patrol.to + 8 : fixed, y: axis === 'vertical' ? patrol.to + 8 : fixed } };
        // Do not coerce an incomplete form into a valid coordinate.
        if (!Number.isInteger(patrol.from)) result.patrol.from[coordinate] = null;
        if (!Number.isInteger(patrol.to)) result.patrol.to[coordinate] = null;
      } else if (before?.patrol) delete result.patrol;
    }
    const previousBehavior = uiBehavior(before?.behavior, behaviorDefinition);
    if (!equal(object.behavior, previousBehavior)) {
      if (object.behavior?.type === 'chest') result.behavior = 'proximity-chest';
      else if (object.behavior?.type === 'lever') result.behavior = 'reward-lever';
      else delete result.behavior;
    }
    return result;
  });
  const oldMarkers = new Map(original.markers.map(marker => [marker.id, marker]));
  raw.markers = room.markers.map(marker => ({ ...(oldMarkers.has(marker.id) ? copy(oldMarkers.get(marker.id)) : {}),
    ...copy(marker), type: nativeMarker(marker.type), required: marker.required ?? false }));
  raw.collisions = copy(room.collisions);
  raw.tiles = copy(room.tiles);
  return raw;
}

export function compositionErrors(raw, assetMap) {
  const errors = [];
  const fail = message => errors.push(message);
  const integer = (value, min = 0, max = 256) => Number.isInteger(value) && value >= min && value <= max;
  if (!raw || raw.format !== 'anotherdungeon-room' || raw.schemaVersion !== 2 || !Object.hasOwn(GAME_PROFILES, raw.runtimeBinding)) return ['Perfil de composición desconocido.'];
  const profile = GAME_PROFILES[raw.runtimeBinding];
  if (raw.roomKind !== profile.kind) fail('El tipo de sala debe conservar su perfil del juego.');
  if (raw.width !== 256 || raw.height !== 256 || raw.tileSize !== 16 || raw.coordinateSystem !== 'top-left-pixels') fail('El juego necesita un lienzo 256×256 con celdas de 16 px.');
  if (typeof raw.name !== 'string' || !raw.name.trim() || typeof raw.id !== 'string' || !raw.id || typeof raw.description !== 'string') fail('Nombre, ID o descripción inválidos.');
  if (!raw.spawn || !integer(raw.spawn.x) || !integer(raw.spawn.y)) fail('Aparición fija inválida.');
  if (raw.doorPolicy !== 'declared-sides-only' || !Array.isArray(raw.tiles) || raw.tiles.length) fail('Los perfiles del juego usan sus lados declarados y no admiten tiles pintados.');
  if (!Array.isArray(raw.backgroundOptions) || !raw.backgroundOptions.length || !raw.backgroundOptions.includes(raw.background) || raw.backgroundOptions.some(id => assetMap.get(id)?.category !== 'background')) fail('El fondo elegido debe formar parte de sus fondos posibles.');
  if (![raw.objects, raw.markers, raw.collisions].every(Array.isArray)) return [...errors, 'Capas de objetos, marcadores o colisiones inválidas.'];
  if (raw.objects.length > 64) fail('El perfil admite hasta 64 objetos.');
  const ids = new Set();
  for (const item of [...raw.objects, ...raw.markers, ...raw.collisions]) {
    if (!item || typeof item.id !== 'string' || !item.id || ids.has(item.id)) fail('Objetos, marcadores y colisiones necesitan IDs únicos.');
    ids.add(item?.id);
    if (!integer(item?.x) || !integer(item?.y)) fail('Hay posiciones fuera del lienzo.');
  }
  const doorSides = new Set();
  const nativeAssets = new Set(['column','library_back','library_front','campfire','stone','box','bat','skeleton','plant','mushy','stairs','lever','vase','sign','chair','table','weapon_table','chest','axe','spikes']);
  const expectedRoles = { campfire: 'interactive', stairs: 'interactive', lever: 'lever', bat: 'enemy', skeleton: 'enemy', plant: 'enemy', mushy: 'enemy', spikes: 'hazard' };
  for (const object of raw.objects) {
    if (object.assetId === 'door') {
      if (object.role !== 'door' || !sides.includes(object.side) || doorSides.has(object.side) || !['possible','required'].includes(object.rule)) fail('Cada puerta necesita un lado único y regla r/R.');
      doorSides.add(object.side);
      if (object.destination) fail('El generador asigna los destinos después de definir la semilla.');
      if (![0,90,180,270].includes(object.rotation)) fail('Giro de puerta inválido.');
      continue;
    }
    const asset = assetMap.get(GAME_ALIASES[object.assetId] ?? object.assetId);
    if (!nativeAssets.has(object.assetId) || !asset) { fail(`Sprite no admitido por este perfil: ${object.assetId}.`); continue; }
    if (object.role !== (expectedRoles[object.assetId] ?? 'decoration')) fail(`La función de ${asset.name} no coincide con el perfil del juego.`);
    if (object.rotation !== 0 || typeof object.solid !== 'boolean' || typeof object.flipX !== 'boolean' || !integer(object.frame,0,asset.frames-1)) fail(`Giro, frame o propiedades inválidos en ${asset.name}.`);
    const box = object.hitbox;
    if (!box || !integer(box.width,1,asset.width) || !integer(box.height,1,asset.height) || !integer(box.x,0,asset.width-box.width) || !integer(box.y,0,asset.height-box.height)) fail(`Hitbox inválida en ${asset.name}.`);
    if (!['explicit','legacy-base'].includes(object.collisionProfile ?? 'explicit')) fail('Perfil de colisión desconocido.');
    if (object.assetId === 'chair' && object.solid) fail('Los asientos no bloquean el paso.');
    if (object.role === 'enemy') {
      if (object.solid || object.spawnMode !== 'seeded' || !integer(object.runtimeSlot,0,3)) fail('Los candidatos enemigos conservan su slot 0–3 y aparición por semilla.');
      if (!box || !Object.entries({ x:3,y:0,width:10,height:8 }).every(([key,value])=>box[key]===value)) fail('Los enemigos 16×16 necesitan la caja superior de 10×8.');
      if (object.assetId === 'skeleton') {
        const p = object.patrol;
        if (!p || p.mode !== 'ping-pong' || !['horizontal','vertical'].includes(p.axis) || !integer(p.from?.x) || !integer(p.from?.y) || !integer(p.to?.x) || !integer(p.to?.y) ||
            !(p.axis === 'horizontal' ? p.from.y === p.to.y && p.from.x < p.to.x : p.from.x === p.to.x && p.from.y < p.to.y)) fail('El esqueleto necesita dos extremos de patrulla válidos.');
      }
    }
  }
  if (!doorSides.size) fail('El perfil requiere al menos una puerta.');
  for (const marker of raw.markers) {
    if (!['random','ability','npc_delivery'].includes(marker.type) || typeof marker.required !== 'boolean') fail('Marcador Q, &, o entrega de NPC inválido.');
    if (marker.type === 'npc_delivery' && !integer(marker.slot,0,2)) fail('La entrega de aventurero necesita slot 0, 1 o 2.');
  }
  for (const rect of raw.collisions) if (!integer(rect.width,1) || !integer(rect.height,1) || rect.x+rect.width>256 || rect.y+rect.height>256) fail('Colisión invisible fuera del lienzo.');
  const count = assetId => raw.objects.filter(object => object.assetId === assetId).length;
  const expected = { cuarto_1:['bat',2],cuarto_1_2:['mushy',2],cuarto_2:['bat',4],cuarto_2_2:['mushy',4],cuarto_3:['skeleton',4],cuarto_4:['skeleton',4],cuarto_5:['plant',4],cuarto_6:['plant',1],cuarto_final_escaleras:['bat',2],cuarto_final_escaleras_2:['plant',2] }[raw.runtimeBinding];
  const enemies = raw.objects.filter(object => object.role === 'enemy');
  if (expected ? enemies.length !== expected[1] || enemies.some(object => object.assetId !== expected[0]) : enemies.length) fail('Este perfil conserva su tipo y cantidad de candidatos enemigos; la semilla decide cuántos aparecen.');
  for (const asset of ['bat','skeleton','plant','mushy']) {
    const slots = raw.objects.filter(object=>object.assetId===asset).map(object=>object.runtimeSlot).sort((a,b)=>a-b);
    if (slots.some((slot,index)=>slot!==index)) fail('Los slots enemigos no pueden repetirse ni tener huecos.');
  }
  if (count('stone')>26 || count('spikes')>12) fail('El juego admite hasta 26 piedras y 12 trampas por perfil.');
  if (['fogata','cuarto_final_escaleras','cuarto_final_escaleras_2'].includes(raw.runtimeBinding) && count('lever')!==2) fail('Fogata y escaleras necesitan dos posiciones candidatas de palanca.');
  if (profile.kind==='normal' && !raw.markers.some(marker=>marker.type==='random')) fail('Los cuartos normales necesitan puntos Q.');
  if (raw.runtimeBinding==='fogata' && (count('campfire')!==1 || raw.markers.filter(marker=>marker.type==='ability').length!==1 || !equal(raw.markers.filter(marker=>marker.type==='npc_delivery').map(marker=>marker.slot).sort(),[0,1,2]))) fail('La fogata conserva el fuego, su recompensa compartida y las tres posiciones de entrega.');
  if (profile.kind==='stairs' && count('stairs')!==1) fail('El perfil requiere una escalera.');
  if (count('chest')>(profile.kind==='menu'?1:0)) fail('El cofre animado pertenece al perfil de menú.');
  if (raw.runtimeBinding==='cuarto_final_escaleras_2' && (doorSides.size!==1 || !doorSides.has('down') || raw.objects.find(object=>object.role==='door')?.rule!=='required')) fail('Escaleras 2 solo admite su puerta inferior obligatoria.');
  if (profile.kind==='reward' && (doorSides.size!==1 || !doorSides.has('right') || raw.objects.find(object=>object.role==='door')?.rule!=='required' || raw.markers.filter(marker=>marker.type==='random'&&marker.required).length!==3 || raw.markers.filter(marker=>marker.type==='ability'&&marker.required).length!==1 || raw.access?.type!=='three-levers' || raw.access?.requiresKey!==false)) fail('Boss 1 conserva la puerta derecha obligatoria, tres Q y un & obligatorios y acceso por tres palancas sin llave.');
  return [...new Set(errors)];
}
