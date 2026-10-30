import { ROOM_WIDTH, ROOM_HEIGHT, BACKGROUND_CROP, GAME_GRID, DOOR_SIDES, TILE_SIZE, BACKGROUNDS, SPRITES, ASSETS, ASSET_MAP, uid, clone, clamp, snap,
  createRoom, createObject, objectSize, objectCollision, contains, validateRoom, roomWarnings, decodeBMP, replaceRegion,
  defaultHitbox, behaviorDefinition, createMarker, snapToGameGrid, gameplayErrors, applyBossPreset, serializeGameRoom, GAME_PROFILES, isComposition, markerLabel, prepareCompositionChanges } from './editor-core.js?v=20261031-4';

const $ = id => document.getElementById(id);
const canvas = $('room-canvas');
const ctx = canvas.getContext('2d');
const images = new Map();
const DRAFT_KEY = 'anotherdungeon.editor.draft.v1';
const LIBRARY_KEY = 'anotherdungeon.editor.library.v1';
const MAX_HISTORY = 80;
// Only the editor's visible window is cropped. The canvas and all data stay 256 × 256.
const ROOM_VIEW = { x: GAME_GRID.x, y: GAME_GRID.y, width: 240, height: 160 };
let room = createRoom();
let tool = 'select';
let selectedId = null;
let selectedMarkerId = null;
let selectedCollisionIndex = -1;
let selectedAsset = 'campfire';
let selectedTile = { assetId: 'room2', sx: 0, sy: 0 };
let gridVisible = true;
let collisionsVisible = false;
let preview = false;
let zoom = 2;
let snapSize = 1;
let snapMode = 'free';
let ready = false;
let gesture = null;
let hover = null;
let history = [];
let future = [];
let toastTimer;
let storageFailed = false;
let draftDirty = false;
let pendingConfirm = null;
let library = [];
let paintErase = false;
const pointers = new Map();
let pinchGesture = null;
const compactLayout = window.matchMedia('(max-width: 1100px)');
const touchInput = window.matchMedia('(pointer: coarse)');
const viewport = $('canvas-viewport');

const selectedObject = () => room.objects.find(object => object.id === selectedId);
const selectedMarker = () => room.markers.find(marker => marker.id === selectedMarkerId);
const serialize = () => JSON.stringify(room);

function placement(point, size = { width: 1, height: 1 }, free = false) {
  if (!free && snapMode === 'game') return snapToGameGrid(point.x, point.y, size.width, size.height);
  const step = free ? 1 : snapSize;
  return { x: clamp(snap(point.x, step), 0, ROOM_WIDTH - size.width), y: clamp(snap(point.y, step), 0, ROOM_HEIGHT - size.height) };
}

function toast(message) {
  clearTimeout(toastTimer);
  $('toast').textContent = message;
  $('toast').hidden = false;
  toastTimer = setTimeout(() => { $('toast').hidden = true; }, 4200);
}

function store(key, value) {
  try {
    localStorage.setItem(key, JSON.stringify(value));
    storageFailed = false;
    return true;
  } catch {
    $('save-status').textContent = 'Sin guardado local · exporta una copia';
    if (!storageFailed) toast('El navegador no permite guardar más datos. Descarga el JSON para conservar el cuarto.');
    storageFailed = true;
    return false;
  }
}

function readStorage() {
  try {
    const savedDraft = JSON.parse(localStorage.getItem(DRAFT_KEY) || 'null');
    if (savedDraft) {
      room = validateRoom(savedDraft);
      if (savedDraft.width !== ROOM_WIDTH) toast('Borrador restaurado en el lienzo de 256 × 256; se conservan las posiciones respecto al fondo.');
    }
  } catch { toast('No se pudo recuperar el borrador anterior. Puedes importar una copia JSON.'); }
  try {
    const savedLibrary = JSON.parse(localStorage.getItem(LIBRARY_KEY) || '[]');
    if (Array.isArray(savedLibrary)) {
      for (const entry of savedLibrary.slice(0, 100)) {
        try {
          const data = validateRoom(entry.room);
          library.push({ room: data, savedAt: typeof entry.savedAt === 'string' ? entry.savedAt : '',
            thumbnail: typeof entry.thumbnail === 'string' && /^data:image\/png;base64,/.test(entry.thumbnail) && entry.thumbnail.length < 300000 ? entry.thumbnail : '' });
        } catch { /* Skip only an invalid entry, keeping the other saved rooms. */ }
      }
    }
  } catch { toast('No se pudo leer la colección local. El borrador actual sigue disponible.'); }
  updateLibraryCount();
}

function saveDraft() {
  draftDirty = false;
  if (store(DRAFT_KEY, room)) $('save-status').textContent = 'Borrador guardado en este navegador';
}

function finishChange(before) {
  if (isComposition(room)) prepareCompositionChanges(room);
  if (before === serialize()) return;
  history.push(before);
  if (history.length > MAX_HISTORY) history.shift();
  future = [];
  draftDirty = true;
  saveDraft();
  refresh();
}

function change(action) {
  const before = serialize();
  action();
  finishChange(before);
}

function undo() {
  if (!history.length || gesture) return;
  future.push(serialize());
  room = JSON.parse(history.pop());
  if (!selectedObject()) selectedId = null; if (!selectedMarker()) selectedMarkerId = null;
  saveDraft(); refresh();
}

function redo() {
  if (!future.length || gesture) return;
  history.push(serialize());
  room = JSON.parse(future.pop());
  if (!selectedObject()) selectedId = null; if (!selectedMarker()) selectedMarkerId = null;
  saveDraft(); refresh();
}

function imageCanvas(asset, data) {
  const sheet = document.createElement('canvas');
  sheet.width = data.width; sheet.height = data.height;
  sheet.getContext('2d').putImageData(new ImageData(data.data, data.width, data.height), 0, 0);
  if (data.width !== asset.width || data.height !== asset.height * asset.frames) throw new Error(`Medidas inesperadas: ${asset.file}`);
  return sheet;
}

async function loadAssets() {
  let completed = 0;
  $('load-banner').hidden = false;
  $('load-banner').classList.remove('error');
  $('load-banner').textContent = `Cargando recursos: 0 de ${ASSETS.length}…`;
  const results = await Promise.allSettled(ASSETS.map(async asset => {
    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), 15000);
    try {
      const url = new URL(`assets_game/${asset.file}`, import.meta.url);
      const response = await fetch(url, { signal: controller.signal, cache: 'no-cache' });
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      const data = decodeBMP(await response.arrayBuffer());
      images.set(asset.id, imageCanvas(asset, data));
    } catch (error) {
      throw new Error(`${asset.file}: ${controller.signal.aborted ? 'la descarga tardó más de 15 segundos' : error.message}`);
    } finally {
      clearTimeout(timeout);
      completed++;
      $('load-banner').textContent = `Cargando recursos: ${completed} de ${ASSETS.length}…`;
    }
  }));
  const failed = results.flatMap(result => result.status === 'rejected' ? [result.reason.message] : []);
  ready = images.has(room.background);
  $('canvas-loading').hidden = ready;
  if (failed.length) {
    $('load-banner').classList.add('error');
    const message = document.createElement('p');
    message.textContent = `No se pudieron cargar ${failed.length} recursos. Comprueba que assets_game esté junto a editor.html y que sus archivos se hayan subido completos.`;
    const details = document.createElement('details');
    const summary = document.createElement('summary'); summary.textContent = 'Ver archivos que fallaron';
    const list = document.createElement('ul');
    for (const error of failed) { const item = document.createElement('li'); item.textContent = error; list.append(item); }
    details.append(summary, list);
    const retry = document.createElement('button'); retry.className = 'secondary'; retry.textContent = 'Reintentar carga';
    retry.addEventListener('click', loadAssets);
    $('load-banner').replaceChildren(message, details, retry);
    $('canvas-loading').textContent = 'No se pudo cargar el fondo. Revisa el aviso de arriba.';
  } else $('load-banner').hidden = true;
  for (const selectId of ['background-select', 'tile-source']) {
    for (const option of $(selectId).options) option.disabled = !images.has(option.value);
  }
  renderPalette(); renderTilePicker(); refresh();
  if (ready) fitCanvas();
}

function drawSprite(target, object, overrideFrame = null) {
  const asset = ASSET_MAP.get(object.assetId);
  const image = images.get(object.assetId);
  if (!image) return;
  const size = objectSize(object);
  target.save();
  target.translate(object.x + size.width / 2, object.y + size.height / 2);
  target.rotate(object.rotation * Math.PI / 180);
  target.scale(object.flipX ? -1 : 1, 1);
  target.drawImage(image, 0, (overrideFrame ?? object.frame) * asset.height, asset.width, asset.height,
    -asset.width / 2, -asset.height / 2, asset.width, asset.height);
  target.restore();
}

function drawRoom(target, overlays = false) {
  target.imageSmoothingEnabled = false;
  target.fillStyle = '#d8daa3';
  target.fillRect(0, 0, ROOM_WIDTH, ROOM_HEIGHT);
  const background = images.get(room.background);
  if (background) target.drawImage(background, BACKGROUND_CROP.x, BACKGROUND_CROP.y, ROOM_WIDTH, ROOM_HEIGHT, 0, 0, ROOM_WIDTH, ROOM_HEIGHT);
  for (const tile of room.tiles) {
    const sheet = images.get(tile.assetId);
    if (sheet) {
      target.fillStyle = '#d8daa3';
      target.fillRect(tile.x, tile.y, tile.width, tile.height);
      target.drawImage(sheet, tile.sx, tile.sy, tile.width, tile.height, tile.x, tile.y, tile.width, tile.height);
    }
  }
  for (const object of room.objects) drawSprite(target, object);
  if (!overlays || preview) return;
  if (gridVisible) {
    target.strokeStyle = '#34492c2b'; target.lineWidth = 1 / 3;
    target.beginPath();
    for (let x = 8; x <= 248; x += 16) { target.moveTo(x, 48); target.lineTo(x, 208); }
    for (let y = 48; y <= 208; y += 16) { target.moveTo(8, y); target.lineTo(248, y); }
    target.stroke();
  }
  if (collisionsVisible || tool === 'collision') {
    for (const box of [...room.collisions, ...room.objects.filter(o => o.solid).map(objectCollision)]) {
      target.fillStyle = '#bc503449'; target.fillRect(box.x, box.y, box.width, box.height);
      target.strokeStyle = '#ad4d37'; target.lineWidth = 1 / 3;
      target.strokeRect(box.x + .3, box.y + .3, box.width - .6, box.height - .6);
    }
  }
  if (room.spawn) {
    const { x, y } = room.spawn;
    target.save(); target.strokeStyle = '#f6f6d8'; target.lineWidth = 2;
    target.beginPath(); target.arc(x, y, 4, 0, Math.PI * 2); target.stroke();
    target.strokeStyle = '#3e733f'; target.lineWidth = 1;
    target.beginPath(); target.arc(x, y, 4, 0, Math.PI * 2); target.moveTo(x - 7, y); target.lineTo(x + 7, y); target.moveTo(x, y - 7); target.lineTo(x, y + 7); target.stroke();
    target.restore();
  }
  for (const marker of room.markers) {
    target.save(); target.fillStyle = marker.type === 'Q' ? '#476bb3' : '#ab7135'; target.strokeStyle = '#f5edc5'; target.lineWidth = .8;
    target.beginPath(); target.arc(marker.x, marker.y, selectedMarkerId === marker.id ? 7 : 5, 0, Math.PI * 2); target.fill(); target.stroke();
    target.fillStyle = '#fff6d6'; target.font = 'bold 8px monospace'; target.textAlign = 'center'; target.textBaseline = 'middle'; target.fillText(marker.type === 'npc_delivery' ? String(Number(marker.slot ?? 0) + 1) : marker.type, marker.x, marker.y); target.restore();
  }
  for (const object of room.objects.filter(o => o.role === 'enemy' && o.enemy?.patrol)) {
    const p = object.enemy.patrol;
    if (!Number.isInteger(p.from) || !Number.isInteger(p.to)) continue;
    target.save(); target.strokeStyle = '#516ca7'; target.lineWidth = .8; target.setLineDash([2, 2]); target.beginPath();
    if (p.axis === 'horizontal') { target.moveTo(p.from, object.y); target.lineTo(p.to, object.y); }
    else { target.moveTo(object.x, p.from); target.lineTo(object.x, p.to); }
    target.stroke(); target.restore();
  }
  if (gesture?.type === 'collision') {
    const box = collisionRect(gesture.start, gesture.end);
    target.fillStyle = gesture.erase ? '#f1ecd2aa' : '#bc503466'; target.fillRect(box.x, box.y, box.width, box.height);
    target.strokeStyle = '#a74c32'; target.lineWidth = .7; target.strokeRect(box.x, box.y, box.width, box.height);
  }
  const object = selectedObject();
  if (object) {
    const size = objectSize(object);
    target.strokeStyle = '#f6f2d6'; target.lineWidth = 1.5;
    target.strokeRect(object.x - .5, object.y - .5, size.width + 1, size.height + 1);
    target.strokeStyle = '#4d6e35'; target.lineWidth = .6;
    target.strokeRect(object.x - 1, object.y - 1, size.width + 2, size.height + 2);
    target.fillStyle = '#456531';
    for (const [x, y] of [[object.x - 1, object.y - 1], [object.x + size.width - 1, object.y - 1], [object.x - 1, object.y + size.height - 1], [object.x + size.width - 1, object.y + size.height - 1]]) target.fillRect(x, y, 2, 2);
  }
  if (hover && !gesture) {
    target.save(); target.globalAlpha = .6;
    if (tool === 'stamp' && images.has(selectedAsset)) {
      const asset = ASSET_MAP.get(selectedAsset), point = placement(hover, asset);
      const object = createObject(selectedAsset, point.x, point.y);
      drawSprite(target, object);
    } else if (tool === 'tile') {
      const image = images.get(selectedTile.assetId);
      if (image) target.drawImage(image, selectedTile.sx, selectedTile.sy, 16, 16, Math.floor(hover.x / 16) * 16, Math.floor(hover.y / 16) * 16, 16, 16);
    } else if (tool === 'collision') {
      target.fillStyle = paintErase ? '#f3ecd2' : '#ad4d37'; target.fillRect(Math.floor(hover.x / 8) * 8, Math.floor(hover.y / 8) * 8, 8, 8);
    }
    target.restore();
  }
}

function renderScene() {
  ctx.setTransform(3, 0, 0, 3, 0, 0);
  drawRoom(ctx, true);
}

function renderThumbnail(target, assetId, frame = 0) {
  const targetCtx = target.getContext('2d');
  targetCtx.clearRect(0, 0, target.width, target.height);
  const asset = ASSET_MAP.get(assetId);
  const sheet = images.get(assetId);
  if (!sheet) return;
  targetCtx.imageSmoothingEnabled = false;
  const scale = Math.min((target.width - 4) / asset.width, (target.height - 4) / asset.height);
  const width = Math.floor(asset.width * scale), height = Math.floor(asset.height * scale);
  targetCtx.drawImage(sheet, 0, frame * asset.height, asset.width, asset.height, Math.floor((target.width - width) / 2), Math.floor((target.height - height) / 2), width, height);
}

function makePreview(assetId, size = 40, frame = 0) {
  const target = document.createElement('canvas');
  target.width = size; target.height = size;
  target.setAttribute('aria-hidden', 'true');
  renderThumbnail(target, assetId, frame);
  return target;
}

function renderPalette() {
  const search = $('asset-search').value.trim().toLocaleLowerCase('es');
  const category = $('asset-category').value;
  const filtered = SPRITES.filter(asset => (category === 'all' || category === asset.category) && `${asset.name} ${asset.file}`.toLocaleLowerCase('es').includes(search));
  $('asset-count').textContent = `${SPRITES.length} SPRITES`;
  const fragment = document.createDocumentFragment();
  for (const asset of filtered) {
    const card = document.createElement('button');
    card.className = 'sprite-card'; card.draggable = images.has(asset.id); card.disabled = !images.has(asset.id);
    card.dataset.assetId = asset.id;
    card.title = `${asset.name} · ${asset.width} × ${asset.height} px${asset.frames > 1 ? ` · ${asset.frames} cuadros` : ''}`;
    card.setAttribute('aria-pressed', String(selectedAsset === asset.id));
    card.append(makePreview(asset.id));
    const name = document.createElement('span'); name.textContent = asset.name; card.append(name);
    if (asset.frames > 1) { const frames = document.createElement('small'); frames.textContent = `×${asset.frames}`; card.append(frames); }
    card.addEventListener('click', () => { selectedAsset = asset.id; setTool('stamp'); updatePaletteSelection(); closeResourceSheet(); });
    card.addEventListener('dragstart', event => {
      selectedAsset = asset.id;
      event.dataTransfer.setData('application/x-anotherdungeon-sprite', asset.id);
      event.dataTransfer.effectAllowed = 'copy';
      event.dataTransfer.setDragImage(card.querySelector('canvas'), 20, 20);
      updatePaletteSelection();
    });
    fragment.append(card);
  }
  if (!filtered.length) { const message = document.createElement('p'); message.className = 'no-results'; message.textContent = 'No hay sprites con ese nombre.'; fragment.append(message); }
  $('sprite-grid').replaceChildren(fragment);
}

function updatePaletteSelection() {
  for (const card of document.querySelectorAll('.sprite-card')) card.setAttribute('aria-pressed', String(card.dataset.assetId === selectedAsset));
}

function renderTilePicker() {
  const picker = $('tile-picker'), target = picker.getContext('2d');
  target.clearRect(0, 0, 256, 256); target.imageSmoothingEnabled = false;
  const image = images.get(selectedTile.assetId);
  if (!image) return;
  target.drawImage(image, 0, 0);
  target.strokeStyle = '#3f593b44'; target.lineWidth = .5;
  target.beginPath();
  for (let i = 16; i < 256; i += 16) { target.moveTo(i, 0); target.lineTo(i, 256); target.moveTo(0, i); target.lineTo(256, i); }
  target.stroke(); target.strokeStyle = '#fffbd9'; target.lineWidth = 3;
  target.strokeRect(selectedTile.sx + 1, selectedTile.sy + 1, 14, 14);
  target.strokeStyle = '#425f30'; target.lineWidth = 1; target.strokeRect(selectedTile.sx + .5, selectedTile.sy + .5, 15, 15);
  const previewCtx = $('tile-preview').getContext('2d'); previewCtx.clearRect(0, 0, 16, 16);
  previewCtx.drawImage(image, selectedTile.sx, selectedTile.sy, 16, 16, 0, 0, 16, 16);
  $('tile-coordinates').textContent = `Tile ${selectedTile.sx / 16}, ${selectedTile.sy / 16}`;
}

function setTool(next) {
  tool = next; preview = false; hover = null;
  document.body.classList.remove('preview-mode'); $('preview-button').textContent = 'Vista limpia';
  canvas.dataset.tool = tool;
  for (const button of document.querySelectorAll('.drawing-tools [data-tool]')) button.setAttribute('aria-pressed', String(button.dataset.tool === next));
  $('paint-options').hidden = !['tile', 'collision'].includes(next);
  $('paint-kind').textContent = next === 'collision' ? 'Colisión invisible = %' : 'Terreno visual';
  const hints = {
    select: touchInput.matches ? 'Toca un objeto y arrástralo para moverlo. Usa dos dedos para acercar o desplazar la vista.' : 'Selecciona un objeto para moverlo. Las flechas ajustan su posición píxel a píxel.',
    stamp: `${ASSET_MAP.get(selectedAsset).name}: toca el lienzo para colocar. Puedes repetir tantas veces como quieras.`,
    erase: 'Toca o arrastra para borrar objetos y tiles. Para quitar colisiones, usa su herramienta y el botón «Quitar».',
    tile: 'Pinta el tile seleccionado. Arrastra para cubrir más celdas; usa «Quitar» para recuperar el fondo.',
    collision: 'Arrastra de una esquina a otra para dibujar un rectángulo invisible (%). «Quitar» borra su área.',
    spawn: 'Toca para colocar una referencia opcional. En el juego, la entrada depende de la puerta de llegada.',
    pan: 'Arrastra para recorrer el cuarto ampliado. Acerca con dos dedos o con los botones + y −; toca el porcentaje para ajustar la vista.',
    markerQ: 'Toca para colocar Q: candidato de aparición aleatoria. El juego decide el contenido con la semilla.',
    markerReward: 'Toca para colocar &: recompensa de habilidad. Después puedes marcarla como obligatoria.'
  };
  $('tool-hint').textContent = hints[next];
  renderScene();
}

function renderInspector() {
  $('room-name').value = room.name; $('room-description').value = room.description;
  $('background-select').value = room.background;
  $('spawn-coordinates').textContent = room.spawn ? `${room.spawn.x}, ${room.spawn.y}` : 'Entrada por puerta';
  $('edit-spawn-button').textContent = room.spawn ? 'Mover' : 'Colocar';
  $('remove-spawn-button').hidden = !room.spawn || isComposition(room);
  const object = selectedObject();
  $('empty-selection').hidden = !!object; $('object-properties').hidden = !object;
  $('selection-size').textContent = object ? `${objectSize(object).width} × ${objectSize(object).height} PX` : '—';
  $('touch-selection').hidden = !object;
  $('touch-selection-name').textContent = object ? `${ASSET_MAP.get(object.assetId).name} · ${object.x}, ${object.y}` : '';
  $('layer-up').disabled = !object || room.objects.at(-1)?.id === object.id;
  $('layer-down').disabled = !object || room.objects[0]?.id === object.id;
  if (!object) return;
  const asset = ASSET_MAP.get(object.assetId), size = objectSize(object);
  $('selected-name').textContent = asset.name; renderThumbnail($('selected-preview'), asset.id, object.frame);
  $('object-x').value = object.x; $('object-y').value = object.y;
  $('object-x').max = ROOM_WIDTH - size.width; $('object-y').max = ROOM_HEIGHT - size.height;
  const options = Array.from({ length: asset.frames }, (_, i) => {
    const option = document.createElement('option'); option.value = i;
    option.textContent = asset.id === 'seat' ? `${i} · ${['Abajo', 'Arriba', 'Izquierda', 'Derecha'][i]}` : `Frame ${i} · ${asset.frames} cuadros`;
    return option;
  });
  $('object-frame').replaceChildren(...options); $('object-frame').value = object.frame;
  $('object-frame').disabled = asset.frames === 1;
  $('flip-button').setAttribute('aria-pressed', String(object.flipX));
  $('object-role').value = object.role; $('object-role').disabled = isComposition(room);
  $('native-object-fields').hidden = !isComposition(room);
  $('object-collision-profile').value = object.collisionProfile ?? 'explicit';
  $('enemy-slot-label').hidden = object.role !== 'enemy'; $('enemy-slot').value = object.runtimeSlot ?? 0;
  $('hazard-active-label').hidden = object.assetId !== 'spikes'; $('hazard-active').checked = object.alwaysActive ?? false;
  $('door-fields').hidden = object.role !== 'door'; $('door-side').value = object.side ?? ''; $('door-rule').value = object.rule ?? '';
  $('enemy-fields').hidden = object.role !== 'enemy';
  $('enemy-spawn').value = object.enemy?.spawnMode ?? 'fixed'; $('enemy-patrol').value = object.enemy?.patrol?.axis ?? 'none';
  $('patrol-fields').hidden = !object.enemy?.patrol;
  $('patrol-from').value = object.enemy?.patrol?.from ?? ''; $('patrol-to').value = object.enemy?.patrol?.to ?? '';
  $('object-behavior').value = object.behavior?.type ?? 'none'; $('object-behavior').disabled = asset.frames < 3;
  $('enemy-spawn').disabled = isComposition(room);
  $('enemy-patrol').disabled = isComposition(room) && object.assetId !== 'skeleton';
  $('object-behavior').disabled = $('object-behavior').disabled || isComposition(room);
  $('behavior-preview').hidden = !object.behavior;
  $('behavior-note').textContent = object.behavior?.type === 'chest' ? '0 cerrado · 1 transición · 2 abierto por cercanía. El juego controla el estado.' : '0 inicial · 1 transición · 2 activada. La palanca permanece visible.';
  for (const button of document.querySelectorAll('[data-preview-frame]')) button.setAttribute('aria-pressed', String(Number(button.dataset.previewFrame) === object.frame));
  $('object-solid').checked = object.solid;
  $('object-solid').disabled = object.assetId === 'seat' && !object.solid;
  $('hitbox-fields').hidden = false;
  for (const key of ['x', 'y', 'width', 'height']) $('hitbox-' + key).value = object.hitbox[key];
  $('hitbox-x').max = asset.width - object.hitbox.width; $('hitbox-y').max = asset.height - object.hitbox.height;
  $('hitbox-width').max = asset.width - object.hitbox.x; $('hitbox-height').max = asset.height - object.hitbox.y;
}

function renderObjectList() {
  $('object-count').textContent = room.objects.length;
  const fragment = document.createDocumentFragment();
  for (const object of [...room.objects].reverse()) {
    const button = document.createElement('button'); button.className = `object-row${selectedId === object.id ? ' selected' : ''}`;
    button.setAttribute('aria-pressed', String(selectedId === object.id));
    button.append(makePreview(object.assetId, 24, object.frame));
    const name = document.createElement('span'); name.className = 'row-name'; name.textContent = ASSET_MAP.get(object.assetId).name;
    const coordinates = document.createElement('span'); coordinates.className = 'row-coordinates'; coordinates.textContent = `${object.x}, ${object.y}`;
    button.append(name, coordinates);
    button.addEventListener('click', () => { selectedId = object.id; selectedMarkerId = null; setTool('select'); refresh(); });
    fragment.append(button);
  }
  if (!room.objects.length) { const empty = document.createElement('p'); empty.className = 'empty-list'; empty.textContent = 'Tu cuarto está listo para empezar.'; fragment.append(empty); }
  $('object-list').replaceChildren(fragment);
}

function renderGameplay() {
  const native = isComposition(room);
  $('native-room-fields').hidden = !native;
  $('runtime-profile').textContent = native ? GAME_PROFILES[room.runtimeBinding].name : '';
  $('room-type').value = room.rules.roomType; $('room-access').value = room.rules.access;
  for (const id of ['room-type','room-access','only-door-sides','boss-preset', ...DOOR_SIDES.map(side => 'side-' + side)]) $(id).disabled = native;
  for (const background of BACKGROUNDS) {
    const option = $('background-option-' + background.id);
    if(option) option.checked = Boolean(room.backgroundOptions?.includes(background.id));
  }
  $('add-marker-npc').hidden = room.runtimeBinding !== 'fogata';
  $('only-door-sides').checked = room.rules.onlyTheseSides;
  for (const side of DOOR_SIDES) $('side-' + side).checked = room.rules.allowedDoorSides.includes(side);
  const markerOptions = [new Option('Sin selección', '')];
  for (const marker of room.markers) markerOptions.push(new Option(`${markerLabel(marker)}${marker.required ? ' obligatoria' : ''} · ${marker.x}, ${marker.y}`, marker.id));
  $('marker-select').replaceChildren(...markerOptions); $('marker-select').value = selectedMarkerId ?? '';
  const marker = selectedMarker(); $('marker-fields').hidden = !marker;
  if (marker) { $('marker-x').value = marker.x; $('marker-y').value = marker.y; $('marker-required-label').hidden = marker.type !== '&' && !(native && marker.type === 'Q'); $('marker-slot-label').hidden = marker.type !== 'npc_delivery'; $('marker-slot').value = marker.slot ?? 0; $('marker-required').checked = marker.required ?? false; }
  const collisionOptions = [new Option('Sin selección', '')];
  room.collisions.forEach((box, i) => collisionOptions.push(new Option(`${i + 1}: ${box.x},${box.y} · ${box.width}×${box.height}`, String(i))));
  $('collision-select').replaceChildren(...collisionOptions);
  if (!room.collisions[selectedCollisionIndex]) selectedCollisionIndex = -1;
  $('collision-select').value = selectedCollisionIndex < 0 ? '' : String(selectedCollisionIndex);
  const box = room.collisions[selectedCollisionIndex]; $('collision-fields').hidden = !box;
  if (box) for (const key of ['x', 'y', 'width', 'height']) $('collision-' + key).value = box[key];
  $('rules-errors').replaceChildren(...gameplayErrors(room).map(message => { const p = document.createElement('p'); p.textContent = message; return p; }));
}

function refresh() {
  $('undo-button').disabled = !history.length;
  $('redo-button').disabled = !future.length;
  $('grid-button').setAttribute('aria-pressed', String(gridVisible));
  $('collision-view-button').setAttribute('aria-pressed', String(collisionsVisible));
  $('grid-toggle').checked = gridVisible; $('collisions-toggle').checked = collisionsVisible;
  renderInspector(); renderObjectList(); renderGameplay(); renderScene();
}

function setZoom(value, precise = false) {
  zoom = clamp(precise ? Math.round(value * 100) / 100 : Math.round(value * 4) / 4, .5, 4);
  $('canvas-wrap').style.width = `${ROOM_VIEW.width * zoom}px`; $('canvas-wrap').style.height = `${ROOM_VIEW.height * zoom}px`;
  canvas.style.width = `${ROOM_WIDTH * zoom}px`; canvas.style.height = `${ROOM_HEIGHT * zoom}px`;
  canvas.style.left = `${-ROOM_VIEW.x * zoom}px`; canvas.style.top = `${-ROOM_VIEW.y * zoom}px`;
  $('zoom-value').textContent = `${Math.round(zoom * 100)}%`;
  $('zoom-out').disabled = zoom <= .5; $('zoom-in').disabled = zoom >= 4;
}

function fitCanvas() {
  const padding = compactLayout.matches ? 26 : innerWidth <= 1150 ? 42 : 54;
  const available = Math.max(128, viewport.clientWidth - padding);
  const availableHeight = compactLayout.matches ? Math.max(128, viewport.clientHeight - padding) : 640;
  setZoom(Math.min(2.5, available / ROOM_VIEW.width, availableHeight / ROOM_VIEW.height), true);
  viewport.scrollLeft = 0; viewport.scrollTop = 0;
}

function canvasPoint(event, target = canvas) {
  const rect = target.getBoundingClientRect();
  const width = target === canvas ? ROOM_WIDTH : target.width;
  const height = target === canvas ? ROOM_HEIGHT : target.height;
  return { x: clamp(Math.floor((event.clientX - rect.left) / rect.width * width), 0, width - 1),
    y: clamp(Math.floor((event.clientY - rect.top) / rect.height * height), 0, height - 1) };
}

function hitObject(point, allowTouchTolerance = false) {
  const frontToBack = [...room.objects].reverse();
  const exact = frontToBack.find(object => contains({ ...object, ...objectSize(object) }, point));
  if (exact || !allowTouchTolerance) return exact;
  // Prefer an exact hit; otherwise make tiny sprites easier to grab with a finger.
  return frontToBack.map(object => {
    const size = objectSize(object);
    const dx = point.x - clamp(point.x, object.x, object.x + size.width);
    const dy = point.y - clamp(point.y, object.y, object.y + size.height);
    return { object, distance: Math.hypot(dx, dy) };
  }).filter(hit => hit.distance <= 12 / zoom).sort((a, b) => a.distance - b.distance)[0]?.object;
}

function hitMarker(point) {
  return [...room.markers].reverse().find(marker => Math.hypot(marker.x - point.x, marker.y - point.y) <= 9 / zoom);
}

function collisionRect(a, b) {
  return { x: Math.min(a.x, b.x), y: Math.min(a.y, b.y), width: Math.max(1, Math.abs(a.x - b.x)), height: Math.max(1, Math.abs(a.y - b.y)) };
}

function tileCell(point) {
  if (snapMode === 'game') {
    const x = 8 + clamp(Math.floor((point.x - 8) / 16), 0, 14) * 16;
    const y = 48 + clamp(Math.floor((point.y - 48) / 16), 0, 9) * 16;
    return { x, y, width: 16, height: 16 };
  }
  const x = Math.floor(point.x / 16) * 16, y = Math.floor(point.y / 16) * 16;
  return { x, y, width: Math.min(16, ROOM_WIDTH - x), height: Math.min(16, ROOM_HEIGHT - y) };
}

function placeObject(assetId, point, center = false) {
  if (room.objects.length >= 500) { toast('Este cuarto ya tiene 500 objetos.'); return; }
  const asset = ASSET_MAP.get(assetId);
  if (!asset || asset.category === 'background' || !images.has(assetId)) return;
  change(() => {
    const position = placement({ x: point.x - (center ? asset.width / 2 : 0), y: point.y - (center ? asset.height / 2 : 0) }, asset);
    const object = createObject(assetId, position.x, position.y);
    room.objects.push(object); selectedId = object.id; selectedMarkerId = null;
  });
}

function paintAt(point, erase = false) {
  if (tool === 'tile') {
    const rect = tileCell(point);
    room.tiles = replaceRegion(room.tiles, rect, !erase && images.has(selectedTile.assetId) ? { ...rect, ...selectedTile } : null);
  } else if (tool === 'collision') {
    const x = Math.floor(point.x / 8) * 8, y = Math.floor(point.y / 8) * 8;
    const rect = { x, y, width: Math.min(8, ROOM_WIDTH - x), height: Math.min(8, ROOM_HEIGHT - y) };
    room.collisions = replaceRegion(room.collisions, rect, erase ? null : rect);
  } else if (tool === 'erase') {
    const marker = hitMarker(point);
    if (marker) { room.markers = room.markers.filter(m => m.id !== marker.id); if (selectedMarkerId === marker.id) selectedMarkerId = null; return; }
    const object = hitObject(point);
    if (object) {
      room.objects = room.objects.filter(o => o.id !== object.id);
      if (selectedId === object.id) selectedId = null;
    } else {
      room.tiles = replaceRegion(room.tiles, tileCell(point));
    }
  }
}

function startPinch() {
  if (gesture) finishGesture({ pointerId: gesture.pointerId }, true);
  const [a, b] = [...pointers.values()];
  const rect = canvas.getBoundingClientRect();
  const center = { x: (a.x + b.x) / 2, y: (a.y + b.y) / 2 };
  pinchGesture = { distance: Math.max(1, Math.hypot(a.x - b.x, a.y - b.y)), zoom,
    anchorX: (center.x - rect.left) / rect.width, anchorY: (center.y - rect.top) / rect.height };
  hover = null;
  for (const id of pointers.keys()) canvas.setPointerCapture(id);
}

canvas.addEventListener('pointerdown', event => {
  if (!ready || ![0, 2].includes(event.button)) return;
  event.preventDefault(); canvas.focus({ preventScroll: true });
  if (event.pointerType === 'touch') {
    pointers.set(event.pointerId, { x: event.clientX, y: event.clientY });
    canvas.setPointerCapture(event.pointerId);
    if (pointers.size >= 2) { startPinch(); return; }
    if (pinchGesture) return;
  }
  if (gesture) return;
  const point = canvasPoint(event); hover = null;
  if (tool === 'pan' || preview) {
    gesture = { type: 'pan', pointerId: event.pointerId, initialX: event.clientX, initialY: event.clientY, scrollLeft: viewport.scrollLeft, scrollTop: viewport.scrollTop };
  } else if (['stamp', 'spawn', 'markerQ', 'markerReward'].includes(tool) && event.button === 0) {
    // Commit taps on release: a second finger can become a pinch without placing anything.
    gesture = { type: 'tap', tool, pointerId: event.pointerId, point };
  } else if (tool === 'select') {
    if (event.button !== 0) return;
    const marker = hitMarker(point), object = marker ? null : hitObject(point, event.pointerType === 'touch');
    selectedId = object?.id ?? null; selectedMarkerId = marker?.id ?? null;
    const entity = marker ?? object;
    if (entity) gesture = { type: marker ? 'moveMarker' : 'move', pointerId: event.pointerId, before: serialize(), dx: point.x - entity.x, dy: point.y - entity.y, initial: point, moved: false };
    refresh();
  } else if (tool === 'collision') {
    const start = placement(point);
    gesture = { type: 'collision', pointerId: event.pointerId, before: serialize(), start, end: start, erase: event.button === 2 || paintErase };
  } else {
    gesture = { type: 'paint', pointerId: event.pointerId, before: serialize(), erase: event.button === 2 || paintErase, previous: point };
    paintAt(point, gesture.erase); renderScene();
  }
  if (gesture) canvas.setPointerCapture(event.pointerId);
});

canvas.addEventListener('pointermove', event => {
  if (pointers.has(event.pointerId)) pointers.set(event.pointerId, { x: event.clientX, y: event.clientY });
  if (pinchGesture) {
    if (pointers.size >= 2) {
      const [a, b] = [...pointers.values()], center = { x: (a.x + b.x) / 2, y: (a.y + b.y) / 2 };
      setZoom(pinchGesture.zoom * Math.hypot(a.x - b.x, a.y - b.y) / pinchGesture.distance, true);
      const rect = canvas.getBoundingClientRect();
      viewport.scrollLeft += rect.left + rect.width * pinchGesture.anchorX - center.x;
      viewport.scrollTop += rect.top + rect.height * pinchGesture.anchorY - center.y;
    }
    return;
  }
  const point = canvasPoint(event);
  $('coordinates').textContent = `X ${String(point.x).padStart(3, '0')}   Y ${String(point.y).padStart(3, '0')}`;
  if (!gesture) { hover = event.pointerType === 'touch' ? null : point; renderScene(); return; }
  if (event.pointerId !== gesture.pointerId) return;
  if (gesture.type === 'pan') {
    viewport.scrollLeft = gesture.scrollLeft + gesture.initialX - event.clientX;
    viewport.scrollTop = gesture.scrollTop + gesture.initialY - event.clientY;
    return;
  }
  if (gesture.type === 'tap') { gesture.point = point; return; }
  if (gesture.type === 'collision') { gesture.end = placement(point); renderScene(); return; }
  if (['move', 'moveMarker'].includes(gesture.type)) {
    const object = gesture.type === 'moveMarker' ? selectedMarker() : selectedObject();
    if (!object) return;
    if (Math.abs(point.x - gesture.initial.x) + Math.abs(point.y - gesture.initial.y) < 2 && !gesture.moved) return;
    gesture.moved = true;
    const size = gesture.type === 'moveMarker' ? { width: 1, height: 1 } : objectSize(object);
    Object.assign(object, placement({ x: point.x - gesture.dx, y: point.y - gesture.dy }, size, event.altKey));
    if (gesture.type === 'move') { $('object-x').value = object.x; $('object-y').value = object.y; }
  } else {
    // Interpolate fast pointer strokes so tiles and collision cells have no gaps.
    const from = gesture.previous;
    const steps = Math.max(1, Math.ceil(Math.max(Math.abs(point.x - from.x), Math.abs(point.y - from.y)) / 4));
    for (let i = 1; i <= steps; i++) paintAt({ x: from.x + (point.x - from.x) * i / steps, y: from.y + (point.y - from.y) * i / steps }, gesture.erase);
    gesture.previous = point;
  }
  renderScene();
});

function finishGesture(event, cancel = false) {
  if (!gesture || event.pointerId !== gesture.pointerId) return;
  const previous = gesture; gesture = null;
  if (canvas.hasPointerCapture(event.pointerId)) canvas.releasePointerCapture(event.pointerId);
  if (cancel) { if (previous.before) room = JSON.parse(previous.before); refresh(); }
  else if (previous.type === 'tap') {
    if (previous.tool === 'stamp') placeObject(selectedAsset, previous.point);
    else if (previous.tool === 'spawn') change(() => { room.spawn = placement(previous.point); });
    else change(() => {
      if (room.markers.length >= 1000) { toast('Límite de 1000 marcadores por cuarto.'); return; }
      const point = placement(previous.point), marker = createMarker(previous.tool === 'markerQ' ? 'Q' : '&', point.x, point.y);
      room.markers.push(marker); selectedMarkerId = marker.id; selectedId = null;
    });
  } else if (previous.type === 'collision') {
    const rect = collisionRect(previous.start, previous.end);
    room.collisions = replaceRegion(room.collisions, rect, previous.erase ? null : rect);
    selectedCollisionIndex = previous.erase ? -1 : room.collisions.length - 1;
    finishChange(previous.before); refresh();
  } else if (previous.before) { finishChange(previous.before); refresh(); }
}
function releasePointer(event, cancel = false) {
  pointers.delete(event.pointerId);
  if (pinchGesture) {
    if (canvas.hasPointerCapture(event.pointerId)) canvas.releasePointerCapture(event.pointerId);
    if (!pointers.size) { pinchGesture = null; refresh(); }
    return;
  }
  finishGesture(event, cancel);
  if (canvas.hasPointerCapture(event.pointerId)) canvas.releasePointerCapture(event.pointerId);
}
canvas.addEventListener('pointerup', event => releasePointer(event));
canvas.addEventListener('pointercancel', event => releasePointer(event, true));
canvas.addEventListener('lostpointercapture', event => { if (gesture?.pointerId === event.pointerId) releasePointer(event, true); });
canvas.addEventListener('pointerleave', () => { hover = null; $('coordinates').textContent = 'X —   Y —'; if (!gesture) renderScene(); });
canvas.addEventListener('contextmenu', event => event.preventDefault());
canvas.addEventListener('dragover', event => { if (ready && !preview && [...event.dataTransfer.types].includes('application/x-anotherdungeon-sprite')) { event.preventDefault(); event.dataTransfer.dropEffect = 'copy'; } });
canvas.addEventListener('drop', event => {
  event.preventDefault();
  if (!ready || preview) return;
  const id = event.dataTransfer.getData('application/x-anotherdungeon-sprite');
  if (!SPRITES.some(asset => asset.id === id)) return;
  placeObject(id, canvasPoint(event), true); setTool('select'); refresh();
});

function updateObject(action) {
  if (!selectedObject()) return;
  change(() => action(selectedObject()));
}

function removeSelected() {
  if (selectedMarker()) { change(() => { room.markers = room.markers.filter(m => m.id !== selectedMarkerId); selectedMarkerId = null; }); return; }
  if (!selectedObject()) return;
  change(() => { room.objects = room.objects.filter(o => o.id !== selectedId); selectedId = null; });
}

function duplicateSelected() {
  const object = selectedObject();
  if (!object) return;
  if (room.objects.length >= 500) { toast('Este cuarto ya tiene 500 objetos.'); return; }
  change(() => {
    const copy = clone(object), size = objectSize(copy); copy.id = uid();
    copy.x = clamp(copy.x + snapSize, 0, ROOM_WIDTH - size.width); copy.y = clamp(copy.y + snapSize, 0, ROOM_HEIGHT - size.height);
    room.objects.push(copy); selectedId = copy.id;
  });
}

function rotateSelected() {
  updateObject(object => {
    object.rotation = (object.rotation + 90) % 360;
    const size = objectSize(object);
    object.x = clamp(object.x, 0, ROOM_WIDTH - size.width); object.y = clamp(object.y, 0, ROOM_HEIGHT - size.height);
  });
}

for (const button of document.querySelectorAll('.drawing-tools [data-tool]')) {
  const label = document.createElement('span'); label.className = 'tool-label';
  label.textContent = { select: 'Mover', stamp: 'Colocar', erase: 'Borrar', collision: 'Colisión', spawn: 'Referencia', pan: 'Vista' }[button.dataset.tool];
  button.append(label);
  button.addEventListener('click', () => setTool(button.dataset.tool));
}
for (const [id, erase] of [['paint-add', false], ['paint-remove', true]]) $(id).addEventListener('click', () => {
  paintErase = erase;
  $('paint-add').setAttribute('aria-pressed', String(!erase)); $('paint-remove').setAttribute('aria-pressed', String(erase));
  renderScene();
});
$('asset-search').addEventListener('input', renderPalette); $('asset-category').addEventListener('change', renderPalette);
$('objects-tab').addEventListener('click', () => {
  $('objects-palette').hidden = false; $('terrain-palette').hidden = true;
  $('objects-tab').setAttribute('aria-pressed', 'true'); $('terrain-tab').setAttribute('aria-pressed', 'false');
  if (tool === 'tile') setTool('select');
});
$('terrain-tab').addEventListener('click', () => {
  $('objects-palette').hidden = true; $('terrain-palette').hidden = false;
  $('objects-tab').setAttribute('aria-pressed', 'false'); $('terrain-tab').setAttribute('aria-pressed', 'true');
  selectedId = null; setTool('tile'); refresh();
});
$('tile-source').addEventListener('change', () => { selectedTile.assetId = $('tile-source').value; renderTilePicker(); setTool('tile'); });
$('tile-picker').addEventListener('pointerdown', event => {
  event.preventDefault(); const point = canvasPoint(event, $('tile-picker'));
  selectedTile.sx = Math.floor(point.x / 16) * 16; selectedTile.sy = Math.floor(point.y / 16) * 16;
  $('tile-picker').focus({ preventScroll: true }); renderTilePicker(); setTool('tile'); closeResourceSheet();
});
$('tile-picker').addEventListener('keydown', event => {
  const directions = { ArrowLeft: [-16, 0], ArrowRight: [16, 0], ArrowUp: [0, -16], ArrowDown: [0, 16] };
  const direction = directions[event.key];
  if (direction) { event.preventDefault(); selectedTile.sx = clamp(selectedTile.sx + direction[0], 0, 240); selectedTile.sy = clamp(selectedTile.sy + direction[1], 0, 240); renderTilePicker(); }
  else if (event.key === 'Enter') { event.preventDefault(); setTool('tile'); canvas.focus(); }
  event.stopPropagation();
});
$('undo-button').addEventListener('click', undo); $('redo-button').addEventListener('click', redo);
$('grid-button').addEventListener('click', () => { gridVisible = !gridVisible; refresh(); });
$('collision-view-button').addEventListener('click', () => { collisionsVisible = !collisionsVisible; refresh(); });
$('grid-toggle').addEventListener('change', () => { gridVisible = $('grid-toggle').checked; refresh(); });
$('collisions-toggle').addEventListener('change', () => { collisionsVisible = $('collisions-toggle').checked; refresh(); });
$('zoom-in').addEventListener('click', () => setZoom(zoom + .25)); $('zoom-out').addEventListener('click', () => setZoom(zoom - .25));
$('fit-button').addEventListener('click', fitCanvas);
$('snap-size').addEventListener('change', () => { snapMode = $('snap-size').value; snapSize = snapMode === 'game' ? 16 : snapMode === '8' ? 8 : 1; renderScene(); });
$('preview-button').addEventListener('click', () => {
  preview = !preview; hover = null; document.body.classList.toggle('preview-mode', preview);
  $('preview-button').textContent = preview ? 'Volver a editar' : 'Vista limpia'; renderScene();
});
$('room-name').addEventListener('change', () => change(() => { room.name = $('room-name').value.trim().slice(0, 80) || 'Cuarto sin título'; }));
$('room-description').addEventListener('change', () => change(() => { room.description = $('room-description').value.slice(0, 1000); }));
$('background-select').addEventListener('change', () => change(() => { room.background = $('background-select').value; if (isComposition(room) && !room.backgroundOptions.includes(room.background)) room.backgroundOptions.push(room.background); ready = images.has(room.background); $('canvas-loading').hidden = ready; }));
$('edit-spawn-button').addEventListener('click', () => { setTool('spawn'); $('properties-dialog').close(); });
$('remove-spawn-button').addEventListener('click', () => change(() => { room.spawn = null; }));
for (const key of ['x', 'y']) $('object-' + key).addEventListener('change', () => {
  const value = Number($('object-' + key).value);
  if (!Number.isFinite(value)) { renderInspector(); return; }
  updateObject(object => { object[key] = clamp(Math.round(value), 0, (key === 'x' ? ROOM_WIDTH : ROOM_HEIGHT) - objectSize(object)[key === 'x' ? 'width' : 'height']); });
});
$('object-frame').addEventListener('change', () => updateObject(object => { object.frame = Number($('object-frame').value); }));
$('object-role').addEventListener('change', () => updateObject(object => {
  object.role = $('object-role').value;
  if (object.role === 'door') { object.side ??= null; object.rule ??= null; } else { delete object.side; delete object.rule; }
  if (object.role === 'enemy') object.enemy ??= { spawnMode: 'fixed', patrol: null }; else delete object.enemy;
}));
$('door-side').addEventListener('change', () => updateObject(object => { object.side = $('door-side').value || null; }));
$('door-rule').addEventListener('change', () => updateObject(object => { object.rule = $('door-rule').value || null; }));
$('enemy-spawn').addEventListener('change', () => updateObject(object => { object.enemy.spawnMode = $('enemy-spawn').value; }));
$('enemy-patrol').addEventListener('change', () => updateObject(object => {
  const axis = $('enemy-patrol').value;
  object.enemy.patrol = axis === 'none' ? null : { axis, from: null, to: null };
}));
for (const key of ['from', 'to']) $('patrol-' + key).addEventListener('change', () => updateObject(object => {
  const value = $('patrol-' + key).value;
  if (object.enemy?.patrol) object.enemy.patrol[key] = value === '' ? null : Number(value);
}));
$('object-behavior').addEventListener('change', () => updateObject(object => { object.behavior = behaviorDefinition($('object-behavior').value); }));
for (const button of document.querySelectorAll('[data-preview-frame]')) button.addEventListener('click', () => updateObject(object => { object.frame = Number(button.dataset.previewFrame); }));
$('apply-default-hitbox').addEventListener('click', () => updateObject(object => {
  object.hitbox = defaultHitbox(object.assetId); if (isComposition(room)) object.collisionProfile = 'explicit'; if (object.assetId === 'seat') object.solid = false;
}));
$('room-type').addEventListener('change', () => change(() => { room.rules.roomType = $('room-type').value; }));
$('room-access').addEventListener('change', () => change(() => { room.rules.access = $('room-access').value; }));
$('only-door-sides').addEventListener('change', () => change(() => { room.rules.onlyTheseSides = $('only-door-sides').checked; }));
for (const side of DOOR_SIDES) $('side-' + side).addEventListener('change', () => change(() => {
  room.rules.allowedDoorSides = DOOR_SIDES.filter(value => $('side-' + value).checked);
}));
$('boss-preset').addEventListener('click', () => {
  change(() => { applyBossPreset(room); });
});
$('add-marker-q').addEventListener('click', () => { setTool('markerQ'); closeResourceSheet(); });
$('add-marker-reward').addEventListener('click', () => { setTool('markerReward'); closeResourceSheet(); });
$('marker-select').addEventListener('change', () => { selectedMarkerId = $('marker-select').value || null; if (selectedMarkerId) selectedId = null; refresh(); });
for (const key of ['x', 'y']) $('marker-' + key).addEventListener('change', () => {
  if (!selectedMarker()) return;
  const value = Number($('marker-' + key).value);
  if (Number.isFinite(value)) change(() => { selectedMarker()[key] = clamp(Math.round(value), 0, 255); });
});
$('marker-required').addEventListener('change', () => { if (selectedMarker()?.type === '&' || (isComposition(room) && selectedMarker()?.type === 'Q')) change(() => { selectedMarker().required = $('marker-required').checked; }); });
$('marker-slot').addEventListener('change', () => { if(selectedMarker()?.type === 'npc_delivery') change(() => { selectedMarker().slot = Number($('marker-slot').value); }); });
$('add-marker-npc').addEventListener('click', () => change(() => {
  const marker = { id: uid(), type: 'npc_delivery', x: 128, y: 128, slot: 0, required: false };
  room.markers.push(marker); selectedMarkerId = marker.id; selectedId = null;
}));
$('object-collision-profile').addEventListener('change', () => updateObject(object => { object.collisionProfile = $('object-collision-profile').value; }));
$('enemy-slot').addEventListener('change', () => updateObject(object => { object.runtimeSlot = Number($('enemy-slot').value); }));
$('hazard-active').addEventListener('change', () => updateObject(object => { object.alwaysActive = $('hazard-active').checked; }));
for (const background of BACKGROUNDS) {
  const option = $('background-option-' + background.id);
  if(!option) continue;
  option.addEventListener('change', () => change(() => {
    if (!isComposition(room)) return;
    const previous = room.backgroundOptions;
    const selected = BACKGROUNDS.filter(item => $('background-option-' + item.id)?.checked).map(item => item.id);
    room.backgroundOptions = [...previous.filter(id => selected.includes(id)), ...selected.filter(id => !previous.includes(id))];
  }));
}
$('open-game-template').addEventListener('click', async () => {
  const binding = $('game-template').value;
  if (!Object.hasOwn(GAME_PROFILES,binding)) return;
  try {
    const response = await fetch(new URL(`examples/game-rooms/${binding}.json`, import.meta.url));
    if(!response.ok) throw new Error('No se pudo cargar la plantilla.');
    const data = validateRoom(await response.json(), { forExport: true });
    protectDraft(() => { openRoom(data); toast(`Plantilla cargada: ${data.name}. Conserva su función en el juego.`); });
  } catch(error) { toast(error.message); }
});
$('delete-marker').addEventListener('click', () => { if (selectedMarker()) removeSelected(); });
$('draw-collision').addEventListener('click', () => { setTool('collision'); $('properties-dialog').close(); });
$('collision-select').addEventListener('change', () => { selectedCollisionIndex = $('collision-select').value === '' ? -1 : Number($('collision-select').value); refresh(); });
for (const key of ['x', 'y', 'width', 'height']) $('collision-' + key).addEventListener('change', () => {
  const box = room.collisions[selectedCollisionIndex], value = Number($('collision-' + key).value);
  if (!box || !Number.isFinite(value)) return;
  change(() => {
    const max = key === 'x' ? 256 - box.width : key === 'y' ? 256 - box.height : key === 'width' ? 256 - box.x : 256 - box.y;
    box[key] = clamp(Math.round(value), ['width', 'height'].includes(key) ? 1 : 0, max);
  });
});
$('delete-collision').addEventListener('click', () => { if (room.collisions[selectedCollisionIndex]) change(() => { room.collisions.splice(selectedCollisionIndex, 1); selectedCollisionIndex = -1; }); });
$('object-solid').addEventListener('change', () => updateObject(object => { object.solid = $('object-solid').checked; }));
for (const key of ['x', 'y', 'width', 'height']) $('hitbox-' + key).addEventListener('change', () => {
  const value = Number($('hitbox-' + key).value);
  if (!Number.isFinite(value)) { renderInspector(); return; }
  updateObject(object => {
    const asset = ASSET_MAP.get(object.assetId), box = object.hitbox;
    const max = key === 'x' ? asset.width - box.width : key === 'y' ? asset.height - box.height : key === 'width' ? asset.width - box.x : asset.height - box.y;
    box[key] = clamp(Math.round(value), ['width', 'height'].includes(key) ? 1 : 0, max);
  });
});
$('rotate-button').addEventListener('click', rotateSelected);
$('flip-button').addEventListener('click', () => updateObject(object => { object.flipX = !object.flipX; }));
$('duplicate-button').addEventListener('click', duplicateSelected); $('delete-button').addEventListener('click', removeSelected);
$('touch-rotate').addEventListener('click', rotateSelected); $('touch-duplicate').addEventListener('click', duplicateSelected); $('touch-delete').addEventListener('click', removeSelected);
$('touch-edit').addEventListener('click', () => { $('properties-dialog').showModal(); $('object-properties').scrollIntoView({ block: 'start' }); });
for (const [id, delta] of [['layer-up', 1], ['layer-down', -1]]) $(id).addEventListener('click', () => updateObject(object => {
  const index = room.objects.indexOf(object), next = clamp(index + delta, 0, room.objects.length - 1);
  room.objects.splice(index, 1); room.objects.splice(next, 0, object);
}));

function pngDataUrl() {
  const target = document.createElement('canvas'); target.width = ROOM_WIDTH; target.height = ROOM_HEIGHT;
  drawRoom(target.getContext('2d')); return target.toDataURL('image/png');
}

function updateLibraryCount() { $('library-count').textContent = library.length; }

function saveRoom() {
  try { room = validateRoom(room); } catch (error) { toast(error.message); return false; }
  const next = library.filter(entry => entry.room.id !== room.id);
  next.unshift({ room: clone(room), savedAt: new Date().toISOString(), thumbnail: ready ? pngDataUrl() : '' });
  if (next.length > 100) { toast('La colección tiene 100 cuartos. Exporta y retira alguno para guardar otro.'); return false; }
  if (!store(LIBRARY_KEY, next)) return false;
  library = next; updateLibraryCount(); saveDraft();
  toast(`«${room.name}» guardado en Mis cuartos.`); return true;
}

function confirmAction(title, message, label, action) {
  $('confirm-title').textContent = title; $('confirm-message').textContent = message; $('confirm-action').textContent = label;
  pendingConfirm = action; $('confirm-dialog').showModal();
}
$('confirm-action').addEventListener('click', () => { const action = pendingConfirm; pendingConfirm = null; $('confirm-dialog').close(); action?.(); });
$('confirm-cancel').addEventListener('click', () => { pendingConfirm = null; $('confirm-dialog').close(); });
$('confirm-dialog').addEventListener('close', () => { pendingConfirm = null; });

function openRoom(data) {
  room = validateRoom(data); history = []; future = []; selectedId = null; selectedMarkerId = null; selectedCollisionIndex = -1;
  ready = images.has(room.background); $('canvas-loading').hidden = ready;
  setTool('select'); saveDraft(); refresh();
}

function protectDraft(action) {
  const saved = library.find(entry => entry.room.id === room.id);
  const baseline = createRoom(); baseline.id = room.id;
  const hasWork = serialize() !== JSON.stringify(baseline);
  if (hasWork && JSON.stringify(saved?.room) !== serialize()) {
    confirmAction('Conservar tu cuarto', 'Guarda el diseño actual en Mis cuartos antes de abrir otro. También puedes cancelar y exportarlo.', 'Guardar y continuar', () => { if (saveRoom()) action(); });
  } else action();
}

function renderLibrary() {
  const fragment = document.createDocumentFragment();
  for (const entry of library) {
    const row = document.createElement('article'); row.className = 'saved-room';
    if (entry.thumbnail) { const image = document.createElement('img'); image.src = entry.thumbnail; image.alt = ''; row.append(image); }
    const info = document.createElement('div'); info.className = 'saved-room-info';
    const name = document.createElement('strong'); name.textContent = entry.room.name;
    const meta = document.createElement('small');
    const date = new Date(entry.savedAt);
    meta.textContent = `${entry.room.objects.length} objetos${Number.isNaN(date.getTime()) ? '' : ` · ${date.toLocaleDateString('es-MX')}`}`;
    info.append(name, meta);
    const actions = document.createElement('div'); actions.className = 'saved-room-actions';
    const open = document.createElement('button'); open.className = 'secondary'; open.textContent = 'Abrir';
    open.addEventListener('click', () => { $('library-dialog').close(); protectDraft(() => { openRoom(entry.room); toast(`Abierto: ${entry.room.name}`); }); });
    const remove = document.createElement('button'); remove.className = 'quiet'; remove.textContent = 'Retirar';
    remove.addEventListener('click', () => confirmAction('Retirar de Mis cuartos', `Se retirará «${entry.room.name}» de la colección local. El cuarto abierto en el editor se conserva. Descarga antes su JSON si necesitas otra copia.`, 'Retirar', () => {
      const next = library.filter(item => item.room.id !== entry.room.id);
      if (store(LIBRARY_KEY, next)) { library = next; updateLibraryCount(); renderLibrary(); toast('Cuarto retirado de la colección.'); }
    }));
    actions.append(open, remove); row.append(info, actions); fragment.append(row);
  }
  if (!library.length) { const empty = document.createElement('p'); empty.className = 'small-copy'; empty.textContent = 'Aquí aparecerán tus diseños al pulsar Guardar. También puedes importar un cuarto en formato JSON.'; fragment.append(empty); }
  $('saved-rooms').replaceChildren(fragment);
}

$('save-button').addEventListener('click', saveRoom);
$('library-button').addEventListener('click', () => { renderLibrary(); $('library-dialog').showModal(); });
$('new-button').addEventListener('click', () => protectDraft(() => { openRoom(createRoom()); toast('Un nuevo cuarto, una nueva aventura.'); }));

function exportRoom() {
  const forGame = $('export-target').value !== 'editor';
  try { room = validateRoom(room, { forExport: forGame }); } catch (error) { toast(error.message); return; }
  try { $('export-code').value = JSON.stringify(serializeGameRoom(room, { forGame }), null, 2); } catch(error) { toast(error.message); return; }
  const warnings = roomWarnings(room);
  $('export-warnings').replaceChildren(...warnings.map(warning => { const p = document.createElement('p'); p.textContent = `△ ${warning}`; return p; }));
  $('export-dialog').showModal();
}

function download(data, filename) {
  const url = URL.createObjectURL(data), anchor = document.createElement('a');
  anchor.href = url; anchor.download = filename; document.body.append(anchor); anchor.click(); anchor.remove();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
}
function slug() { return room.name.normalize('NFD').replace(/[\u0300-\u036f]/g, '').replace(/[^a-z0-9]+/gi, '-').replace(/^-|-$/g, '').toLowerCase() || 'cuarto'; }
$('export-button').addEventListener('click', exportRoom);
$('download-button').addEventListener('click', () => { download(new Blob([$('export-code').value], { type: 'application/json' }), isComposition(room) ? `${room.runtimeBinding}.json` : `${slug()}.room.json`); toast('JSON exportado. Ya puedes compartir tu cuarto.'); });
$('copy-button').addEventListener('click', async () => {
  try { await navigator.clipboard.writeText($('export-code').value); toast('Código copiado.'); }
  catch { $('export-code').focus(); $('export-code').select(); toast('Código seleccionado. Pulsa Ctrl+C para copiarlo.'); }
});
$('png-button').addEventListener('click', () => {
  if (!ready) { toast('Primero debe cargarse el fondo para exportar la imagen.'); return; }
  const target = document.createElement('canvas'); target.width = ROOM_WIDTH * 3; target.height = ROOM_HEIGHT * 3;
  const targetCtx = target.getContext('2d'); targetCtx.scale(3, 3); drawRoom(targetCtx);
  target.toBlob(blob => { if (blob) download(blob, `${slug()}.png`); else toast('No se pudo exportar la imagen.'); }, 'image/png');
});

function importText(text, success) {
  if (text.length > 2_000_000) throw new Error('El archivo supera el límite de 2 MB.');
  let data;
  try { const parsed = JSON.parse(text); data = validateRoom(parsed, { forExport: Boolean(parsed.rules) }); } catch (error) { if (error instanceof SyntaxError) throw new Error('El código no es un JSON válido.'); throw error; }
  protectDraft(() => {
    openRoom(data); success?.(); toast(`Cuarto importado: ${data.name}. IDs y hitboxes conservados.`);
  });
}
$('import-button').addEventListener('click', () => $('import-file').click());
$('import-file').addEventListener('change', async event => {
  const file = event.target.files[0]; event.target.value = '';
  if (!file) return;
  if (file.size > 2_000_000) { toast('El archivo supera el límite de 2 MB.'); return; }
  try { importText(await file.text(), () => $('library-dialog').close()); }
  catch (error) { toast(error.message); }
});
$('paste-button').addEventListener('click', () => { $('library-dialog').close(); $('import-error').textContent = ''; $('paste-dialog').showModal(); });
$('import-code-button').addEventListener('click', () => {
  try { importText($('import-code').value, () => { $('paste-dialog').close(); $('import-code').value = ''; }); }
  catch (error) { $('import-error').textContent = error.message; }
});
$('help-button').addEventListener('click', () => $('help-dialog').showModal());
for (const button of document.querySelectorAll('.dialog-close')) button.addEventListener('click', () => button.closest('dialog').close());

document.addEventListener('keydown', event => {
  const editing = event.target.closest('input,textarea,select,[contenteditable=true]');
  if (document.querySelector('dialog[open]')) return;
  const command = event.ctrlKey || event.metaKey;
  if (command && event.key.toLowerCase() === 's') { event.preventDefault(); if (editing) event.target.blur(); saveRoom(); return; }
  if (editing || gesture) return;
  const key = event.key.toLowerCase();
  if (command) {
    if (key === 'z') { event.preventDefault(); event.shiftKey ? redo() : undo(); }
    else if (key === 'y') { event.preventDefault(); redo(); }
    else if (key === 'd') { event.preventDefault(); duplicateSelected(); }
    return;
  }
  if (event.target === $('tile-picker')) return;
  const tools = { v: 'select', b: 'stamp', e: 'erase', c: 'collision', p: 'spawn' };
  if (tools[key]) { event.preventDefault(); setTool(tools[key]); }
  else if (key === 'g') { gridVisible = !gridVisible; refresh(); }
  else if (key === 'r') rotateSelected();
  else if (key === 'delete' || key === 'backspace') { event.preventDefault(); removeSelected(); }
  else if (key === 'escape') { selectedId = null; setTool('select'); refresh(); }
  else if (['ArrowLeft', 'ArrowRight', 'ArrowUp', 'ArrowDown'].includes(event.key) && selectedObject()) {
    event.preventDefault();
    const step = event.shiftKey ? snapSize : 1;
    updateObject(object => {
      const size = objectSize(object);
      object.x = clamp(object.x + (event.key === 'ArrowLeft' ? -step : event.key === 'ArrowRight' ? step : 0), 0, ROOM_WIDTH - size.width);
      object.y = clamp(object.y + (event.key === 'ArrowUp' ? -step : event.key === 'ArrowDown' ? step : 0), 0, ROOM_HEIGHT - size.height);
    });
  }
});

function closeResourceSheet() {
  if ($('resources-dialog').open) { $('resources-dialog').close(); canvas.focus({ preventScroll: true }); }
}

const paletteHome = document.createComment('Resources in desktop layout');
const inspectorHome = document.createComment('Inspector in desktop layout');
$('palette-panel').before(paletteHome); $('inspector-panel').before(inspectorHome);

function configureCompactLayout() {
  if (compactLayout.matches) {
    $('resources-dialog').append($('palette-panel'));
    $('properties-dialog').append($('inspector-panel'));
  } else {
    $('resources-dialog').close(); $('properties-dialog').close();
    paletteHome.after($('palette-panel')); inspectorHome.after($('inspector-panel'));
  }
}

$('mobile-resources').addEventListener('click', () => $('resources-dialog').showModal());
$('mobile-properties').addEventListener('click', () => $('properties-dialog').showModal());
$('resources-close').addEventListener('click', closeResourceSheet);
$('properties-close').addEventListener('click', () => $('properties-dialog').close());
for (const id of ['resources-dialog', 'properties-dialog']) $(id).addEventListener('click', event => {
  if (event.target !== $(id)) return;
  const rect = $(id).getBoundingClientRect();
  if (event.clientX < rect.left || event.clientX > rect.right || event.clientY < rect.top || event.clientY > rect.bottom) $(id).close();
});
compactLayout.addEventListener('change', () => { configureCompactLayout(); requestAnimationFrame(fitCanvas); });

function updateVisibleHeight() {
  document.documentElement.style.setProperty('--visible-height', `${window.visualViewport?.height ?? innerHeight}px`);
}
window.visualViewport?.addEventListener('resize', updateVisibleHeight);
updateVisibleHeight();
let resizeTimer;
window.addEventListener('resize', () => {
  updateVisibleHeight();
  if (document.activeElement?.matches('input,textarea,select')) return;
  clearTimeout(resizeTimer); resizeTimer = setTimeout(fitCanvas, 120);
});
window.addEventListener('pagehide', () => { if (draftDirty) saveDraft(); });
for (const id of ['background-select', 'tile-source']) {
  $(id).replaceChildren(...BACKGROUNDS.map(asset => { const option = document.createElement('option'); option.value = asset.id; option.textContent = asset.name; return option; }));
}
configureCompactLayout(); $('game-template').replaceChildren(...Object.entries(GAME_PROFILES).map(([key,profile]) => new Option(profile.name,key)));
readStorage(); setTool('select'); refresh();
await loadAssets();
