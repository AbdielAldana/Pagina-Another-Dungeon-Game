import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFile, readdir } from 'node:fs/promises';
import { validateRoom, serializeGameRoom, gameplayErrors, createRoom, createObject, createMarker, clone, objectCollision, prepareCompositionChanges } from '../editor-core.js';

const directory=new URL('../examples/game-rooms/',import.meta.url);
const fixtures=await Promise.all((await readdir(directory)).filter(name=>name.endsWith('.json')).map(async name=>[name,JSON.parse(await readFile(new URL(name,directory),'utf8'))]));

for(const [name,raw] of fixtures) test(`game round trip preserves every field in ${name}`,()=>{
  const room=validateRoom(raw,{forExport:true});
  const output=serializeGameRoom(room);
  assert.deepEqual(output,raw);
  assert.deepEqual(serializeGameRoom(validateRoom(JSON.parse(JSON.stringify(room)))),raw);
  assert.deepEqual(serializeGameRoom(validateRoom(output)),raw);
  assert.equal('_game' in output,false);
});

const native=name=>validateRoom(clone(fixtures.find(([file])=>file===name)[1]));

test('new rooms have no fixed player spawn and old coordinates remain importable',()=>{
  const fresh=createRoom();
  assert.equal(fresh.spawn,null);
  assert.equal(validateRoom(fresh).spawn,null);
  const old=clone(fresh); old.spawn={x:128,y:128};
  assert.deepEqual(validateRoom(old).spawn,{x:128,y:128});
  delete old.spawn;
  assert.equal(validateRoom(old).spawn,null);
});

test('additional game room exports authored enemies and spikes with their roles and hitboxes',()=>{
  const room=createRoom();
  room.objects.push(createObject('box',40,80),createObject('bat',80,96),
    createObject('mushy',96,96),createObject('plant',112,96),createObject('spikes',128,96));
  room.objects.at(-1).alwaysActive=true;
  const exported=serializeGameRoom(room);
  const bat=exported.objects.find(object=>object.assetId==='bat');
  assert.equal(bat.role,'enemy');
  assert.equal(bat.x,80);
  assert.equal(bat.y,96);
  assert.deepEqual(bat.hitbox,{x:3,y:0,width:10,height:8});
  assert.equal(bat.enemy,undefined);
  assert.equal(exported.objects.find(object=>object.assetId==='mushy').role,'enemy');
  assert.equal(exported.objects.find(object=>object.assetId==='plant').role,'enemy');
  assert.equal(exported.objects.find(object=>object.assetId==='spikes').role,'hazard');
  assert.equal(exported.objects.find(object=>object.assetId==='spikes').alwaysActive,true);
  room.objects.push(createObject('key',144,96),createObject('energy',160,96));
  room.objects.push(createObject('label',176,96));
  room.objects.push(createObject('campfire',192,96),createObject('stairs',208,112));
  const withItems=serializeGameRoom(room);
  assert.equal(withItems.objects.find(object=>object.assetId==='key').role,'item');
  assert.equal(withItems.objects.find(object=>object.assetId==='energy').role,'item');
  assert.equal(withItems.objects.find(object=>object.assetId==='label').role,'lever');
  assert.equal(withItems.objects.find(object=>object.assetId==='campfire').role,'interactive');
  assert.equal(withItems.objects.find(object=>object.assetId==='stairs').role,'stairs');
});

test('additional rooms export Q, ability rewards, invisible walls and seeded bats',()=>{
  const room=createRoom();
  room.objects.push(createObject('box',40,80),createObject('bat',80,96));
  room.objects[1].enemy={spawnMode:'seeded',patrol:null};
  room.markers.push(createMarker('Q',120,100),createMarker('&',150,100));
  room.markers[1].required=true;
  room.collisions.push({x:0,y:0,width:16,height:16});
  const exported=serializeGameRoom(room);
  assert.equal(exported.schemaVersion,2);
  assert.deepEqual(exported.markers.map(marker=>marker.type),['Q','&']);
  assert.equal(exported.markers[1].required,true);
  assert.deepEqual(exported.collisions[0],{x:0,y:0,width:16,height:16});
  assert.deepEqual(exported.objects.find(object=>object.assetId==='bat').enemy,
    {spawnMode:'seeded',patrol:null});
  assert.deepEqual(serializeGameRoom(validateRoom(exported)),exported);
  room.markers.push(createMarker('&',150,100));
  assert.throws(()=>serializeGameRoom(room),/posiciones distintas/);
});

test('editing native positions, backgrounds, names and hitboxes updates the export without losing metadata',()=>{
  const room=native('fogata.json'); room.name='Fogata editada';
  const column=room.objects.find(object=>object.assetId==='column'); column.x+=1;
  column.collisionProfile='explicit'; column.hitbox={x:4,y:16,width:8,height:8};
  room.backgroundOptions.push('room2'); room.background='room2';
  const output=serializeGameRoom(room), exported=output.objects.find(object=>object.id===column.id);
  assert.equal(output.name,room.name); assert.equal(exported.x,column.x);
  assert.deepEqual(exported.hitbox,column.hitbox); assert.equal(exported.collisionProfile,'explicit');
  assert.deepEqual(output.backgroundOptions,['room_campfire','room2']);
  assert.deepEqual(exported.sourceCell,column.sourceCell);
});

test('native skeleton endpoints use pixel centers on disk and top-left positions in the inspector',()=>{
  const room=native('cuarto_4.json'), skeleton=room.objects.find(object=>object.role==='enemy');
  assert.deepEqual(skeleton.enemy.patrol,{axis:'horizontal',from:56,to:184});
  skeleton.enemy.patrol.to=168; skeleton.y+=1;
  const output=serializeGameRoom(room), exported=output.objects.find(object=>object.id===skeleton.id);
  assert.deepEqual(exported.patrol.to,{x:176,y:89});
  assert.equal(exported.runtimeSlot,skeleton.runtimeSlot);
  assert.equal(exported.behavior,'patrol');
});

test('required Q rewards, NPC slots, invisible collision IDs and source grid survive edits',()=>{
  const boss=native('boss_1_ant.json');
  assert.equal(boss.markers.filter(marker=>marker.type==='Q'&&marker.required).length,3);
  const collision=boss.collisions[0]; collision.y+=1; collision.height-=1;
  const exported=serializeGameRoom(boss);
  assert.equal(exported.collisions[0].id,collision.id);
  assert.deepEqual(exported.source,boss.source);
  boss.markers[0].required=false;
  assert.throws(()=>serializeGameRoom(boss),/obligatorios/);
  const fire=native('fogata.json'), marker=fire.markers.find(marker=>marker.type==='npc_delivery');
  marker.x+=1;
  assert.equal(serializeGameRoom(fire).markers.find(value=>value.id===marker.id).slot,marker.slot);
});

test('native slots and gameplay constraints produce errors while incomplete browser drafts remain recoverable',()=>{
  const room=native('cuarto_5.json');
  room.objects.find(object=>object.role==='enemy').runtimeSlot=3;
  assert.ok(gameplayErrors(room).some(error=>/slots/i.test(error)));
  assert.throws(()=>serializeGameRoom(room),/slots/i);
  assert.equal(validateRoom(JSON.parse(JSON.stringify(room))).runtimeBinding,room.runtimeBinding);
  assert.equal('_game' in serializeGameRoom(room,{forGame:false}),false);
});

test('new native entities get persistent IDs and native roles without choosing random content',()=>{
  const room=native('fogata.json'), object=createObject('vase',80,80);
  room.objects.push(object); room.collisions.push({x:20,y:50,width:16,height:16});
  prepareCompositionChanges(room);
  const id=room.collisions.at(-1).id;
  assert.ok(id); assert.equal(object.role,'decoration');
  assert.equal(serializeGameRoom(room).collisions.at(-1).id,id);
  assert.deepEqual(serializeGameRoom(room),serializeGameRoom(room));
});
