"""Validate rooms/*.json and generate a deterministic, ROM-only room catalog."""
import json
import copy
import sys
import zlib
from pathlib import Path
from composition_room import generate_cpp as generate_composition_cpp

ASSETS = {
    "table": ("TABLE", 32, 16, 1),
    "library_front": ("LIBRARY_FRONT", 16, 32, 1),
    "library_back": ("LIBRARY_BACK", 16, 32, 1),
    "column": ("COLUMN", 16, 32, 1),
    "seat": ("SEAT", 16, 16, 4),
    "vase": ("VASE", 16, 16, 1),
    "rock": ("ROCK", 16, 16, 1),
    "sign": ("SIGN", 16, 16, 1),
    "axe": ("AXE", 16, 16, 1),
    "spikes": ("SPIKES", 16, 16, 3),
    "skeleton": ("SKELETON", 16, 16, 16),
    "bat": ("BAT", 16, 16, 16),
    "plant": ("PLANT", 16, 16, 16),
    "mushy": ("MUSHY", 16, 16, 16),
    "chest": ("CHEST", 16, 16, 3),
    "key": ("KEY", 16, 16, 1),
    "energy": ("ENERGY", 16, 16, 3),
    "sprint": ("SPRINT", 16, 16, 3),
    "label": ("LEVER", 16, 16, 3),
    "campfire": ("CAMPFIRE", 16, 32, 4),
    "stairs": ("STAIRS", 16, 16, 1),
    "door": ("DOOR", 16, 32, 1),
    "door_special": ("DOOR_SPECIAL", 16, 32, 1),
    "door_closed": ("DOOR_CLOSED", 16, 32, 1),
    "door_special_closed": ("DOOR_SPECIAL_CLOSED", 16, 32, 1),
    "table_armor": ("TABLE_ARMOR", 32, 16, 1),
    "box": ("BOX", 16, 16, 1),
    "player": ("PLAYER", 16, 16, 16),
}
BACKGROUNDS = {"land": "LAND", "room2": "ROOM2", "room3": "ROOM3", "room_campfire": "CAMPFIRE"}
ROLES = {"decoration": "DECORATION", "hazard": "HAZARD", "item": "ITEM", "lever": "LEVER", "interactive": "INTERACTIVE", "stairs": "STAIRS", "enemy": "ENEMY", "door": "DOOR"}


def integer(value, label, minimum, maximum):
    if type(value) is not int or not minimum <= value <= maximum:
        raise ValueError(f"{label}: expected integer in [{minimum}, {maximum}]")
    return value


def normalize_editor_room(data):
    """The editor's standard v2 rooms share the existing static-room runtime."""
    if not isinstance(data.get('rules'), dict):
        raise ValueError('Editor v2 requires explicit room rules')
    data = copy.deepcopy(data)
    rules = data['rules']
    if rules.get('roomType') != 'standard' or rules.get('access') != 'normal':
        raise ValueError('Generic editor v2 currently supports standard rooms with normal access')
    if data.get('markers') or data.get('collisions') or data.get('tiles'):
        raise ValueError('Generic editor v2 markers, invisible collisions and tiles need additional runtime support')
    if data.get('backgroundCrop', {'x':0,'y':0,'width':256,'height':256}) != {'x':0,'y':0,'width':256,'height':256}:
        raise ValueError('Only the full 256x256 background crop is supported')
    if data.get('gameGrid', {'x':8,'y':48,'cellSize':16,'columns':15,'rows':10}) != {'x':8,'y':48,'cellSize':16,'columns':15,'rows':10}:
        raise ValueError('Unsupported game grid')
    if type(rules.get('onlyTheseSides')) is not bool:
        raise ValueError('onlyTheseSides must be boolean')
    sides = ('left','right','up','down')
    allowed = rules.get('allowedDoorSides')
    if not isinstance(allowed,list) or not allowed or len(set(allowed)) != len(allowed) or any(side not in sides for side in allowed):
        raise ValueError('Invalid allowedDoorSides')
    if not rules['onlyTheseSides']:
        allowed = list(sides)
    declared = set()
    positions = {'left':(8,112,0), 'right':(232,112,180), 'up':(112,48,270), 'down':(112,192,90)}
    objects = data.get('objects')
    if not isinstance(objects,list):
        raise ValueError('Expected an object list')
    for obj in objects:
        behavior = obj.get('behavior')
        chest_behavior = obj.get('assetId') == 'chest' and behavior == {
                'type': 'chest', 'initialState': 'closed',
                'frames': {'closed': 0, 'transition': 1, 'openNearby': 2}}
        lever_behavior = obj.get('assetId') == 'label' and behavior == {
                'type': 'lever', 'initialState': 'initial',
                'frames': {'initial': 0, 'transition': 1, 'activated': 2},
                'staysVisible': True}
        if behavior is not None and not (chest_behavior or lever_behavior):
            raise ValueError('Unsupported generic editor v2 behavior')
        if obj.get('assetId') in ('door', 'door_special', 'door_closed', 'door_special_closed'):
            side = obj.get('side')
            if side not in allowed or side in declared:
                raise ValueError('Door side conflicts with allowedDoorSides')
            if obj.get('rule','possible') not in ('possible','required') or obj.get('access','normal') != 'normal':
                raise ValueError('Unsupported generic door rule/access')
            declared.add(side)
    # allowedDoorSides declares candidate r slots even when no door sprite was
    # manually placed. Only seeded floor connections open those doors.
    for side in allowed:
        if side in declared:
            continue
        x,y,rotation = positions[side]
        objects.append(dict(id=f"{data.get('id')}:door:{side}", assetId='door', role='door',
                            x=x,y=y,frame=0,rotation=rotation,flipX=False,solid=False,
                            hitbox=dict(x=0,y=0,width=16,height=32),destination=''))
    data['schemaVersion'] = 1
    return data


def parse_room(data):
    if not isinstance(data, dict):
        raise ValueError("Room must be a JSON object")
    if data.get('schemaVersion') == 2 and 'runtimeBinding' not in data:
        data = normalize_editor_room(data)
    room_id = data.get("id")
    if not isinstance(room_id, str) or not room_id.strip():
        raise ValueError("Room id must be a non-empty stable string")
    if not isinstance(data.get("name", "Imported room"), str):
        raise ValueError("Room name must be a string")
    if data.get("format") != "anotherdungeon-room" or data.get("schemaVersion") != 1:
        raise ValueError("Unsupported room format or schema version")
    if (data.get("width"), data.get("height"), data.get("tileSize"), data.get("coordinateSystem")) != (256, 256, 16, "top-left-pixels"):
        raise ValueError("Expected a 256x256 room, 16px tiles, and top-left-pixels coordinates")
    background = BACKGROUNDS.get(data.get("background"))
    if background is None:
        raise ValueError("Unsupported background")
    if data.get("tiles", []) or data.get("collisions", []):
        raise ValueError("This importer currently supports object hitboxes; tiles/collisions must be empty")
    spawn = data.get("spawn")
    if spawn is None:
        # Connected rooms enter beside the destination door. These values are
        # retained in the generated struct only for older room data.
        spawn_x = spawn_y = 0
    elif isinstance(spawn, dict):
        spawn_x = integer(spawn.get("x"), "spawn.x", 0, 256) - 128
        spawn_y = integer(spawn.get("y"), "spawn.y", 0, 256) - 128
    else:
        raise ValueError("spawn must be null or a pixel coordinate object")
    objects = data.get("objects")
    if not isinstance(objects, list) or not objects or len(objects) > 40:
        raise ValueError("Expected 1..40 objects")
    ids = set()
    doors = set()
    enemies = []
    chest_index = -1
    decorations = 0
    item_count = 0
    lever_count = 0
    campfire_count = 0
    stair_count = 0
    lines = []
    for index, obj in enumerate(objects):
        object_id = obj.get("id")
        if not isinstance(object_id, str) or not object_id or object_id in ids:
            raise ValueError("Object IDs must be unique non-empty strings")
        ids.add(object_id)
        if obj.get("assetId") not in ASSETS or obj.get("role") not in ROLES:
            raise ValueError(f"Unsupported asset/role in object {index}")
        asset, width, height, frames = ASSETS[obj["assetId"]]
        role = ROLES[obj["role"]]
        if asset == "AXE" and role == "HAZARD":
            role = "DECORATION"  # The floor axe is scenery in the original game.
        if asset == "CHEST" and role == "ITEM":
            role = "DECORATION"  # Chest proximity animation is a decoration.
        if (role == "DOOR") != (asset in ("DOOR", "DOOR_SPECIAL", "DOOR_CLOSED", "DOOR_SPECIAL_CLOSED")) or (role == "ENEMY") != (asset in ("SKELETON", "BAT", "PLANT", "MUSHY")):
            raise ValueError(f"Asset and role disagree in object {index}")
        if obj["role"] == "hazard" and asset not in ("AXE", "SPIKES") or obj["role"] == "item" and asset not in ("CHEST", "KEY", "ENERGY", "SPRINT"):
            raise ValueError(f"Asset and role disagree in object {index}")
        if (role == "HAZARD") != (asset == "SPIKES"):
            raise ValueError(f"Spikes must use the hazard role in object {index}")
        if (role == "ITEM") != (asset in ("KEY", "ENERGY", "SPRINT")):
            raise ValueError(f"Collectible sprite must use the item role in object {index}")
        if (role == "LEVER") != (asset == "LEVER"):
            raise ValueError(f"Lever sprite must use the lever role in object {index}")
        if (role == "INTERACTIVE") != (asset == "CAMPFIRE") or (role == "STAIRS") != (asset == "STAIRS"):
            raise ValueError(f"Campfire and stairs roles must match their sprites in object {index}")
        x = integer(obj.get("x"), f"objects[{index}].x", 0, 256)
        y = integer(obj.get("y"), f"objects[{index}].y", 0, 256)
        frame = integer(obj.get("frame", 0), "frame", 0, frames - 1)
        rotation = integer(obj.get("rotation", 0), "rotation", -3600, 3600) % 360
        if rotation % 90:
            raise ValueError("Only quarter-turn rotations are supported")
        flip = obj.get("flipX", False)
        solid = obj.get("solid", False)
        if type(flip) is not bool or type(solid) is not bool:
            raise ValueError("flipX and solid must be booleans")
        if role == "ENEMY" and (solid or rotation or flip):
            raise ValueError("Enemies require solid=false, rotation=0 and flipX=false")
        if role == "ENEMY" and obj.get('enemy') not in (None, {'spawnMode': 'fixed', 'patrol': None}):
            raise ValueError("Generic enemies require fixed spawn and no authored patrol")
        if asset == "SEAT" and solid:
            raise ValueError("Seats cannot have physical collision")
        if role == "DOOR" and solid:
            raise ValueError("Door collision is controlled by the room generator; use solid=false")
        if asset == "SPIKES" and type(obj.get('alwaysActive', False)) is not bool:
            raise ValueError("alwaysActive must be a boolean")
        if solid and rotation:
            raise ValueError("Rotated solid decorations are not supported yet")
        if obj.get("destination", ""):
            raise ValueError("Door destinations are assigned by the seeded floor generator")
        hb = obj.get("hitbox", {})
        hx = integer(hb.get("x"), "hitbox.x", 0, width)
        hy = integer(hb.get("y"), "hitbox.y", 0, height)
        hw = integer(hb.get("width"), "hitbox.width", 1, width)
        hh = integer(hb.get("height"), "hitbox.height", 1, height)
        if hx + hw > width or hy + hh > height:
            raise ValueError("Hitbox exceeds its sprite frame")
        corners = [(hx-width//2, hy-height//2), (hx+hw-width//2, hy-height//2),
                   (hx-width//2, hy+hh-height//2), (hx+hw-width//2, hy+hh-height//2)]
        transformed = []
        for px, py in corners:
            if flip:
                px = -px
            if rotation == 90:
                px, py = -py, px
            elif rotation == 180:
                px, py = -px, -py
            elif rotation == 270:
                px, py = py, -px
            transformed.append((px, py))
        left = min(p[0] for p in transformed)
        right = max(p[0] for p in transformed)
        top = min(p[1] for p in transformed)
        bottom = max(p[1] for p in transformed)
        draw_width, draw_height = (height, width) if rotation in (90, 270) else (width, height)
        center_x = x + draw_width//2 - 128
        center_y = y + draw_height//2 - 128
        direction = -1
        if role == "DOOR":
            distances = [center_x + 128, 128 - center_x, center_y + 128, 128 - center_y]
            direction = min(range(4), key=distances.__getitem__)
            if obj.get('side') is not None and obj['side'] != ('left','right','up','down')[direction]:
                raise ValueError('Door position does not match its declared side')
            if direction in doors:
                raise ValueError("Only one door per side is supported")
            doors.add(direction)
        elif role == "ENEMY":
            if asset == "SKELETON":
                enemies.append(index)
            elif sum(1 for previous in objects[:index] if previous.get('assetId') == obj['assetId']) >= 4:
                raise ValueError("At most four enemies of each type are supported per room")
        elif role == "DECORATION":
            decorations += 1
        elif role == "ITEM":
            item_count += 1
        elif role == "LEVER":
            lever_count += 1
        elif role == "INTERACTIVE":
            campfire_count += 1
        elif role == "STAIRS":
            stair_count += 1
        if asset == "CHEST":
            if chest_index >= 0:
                raise ValueError("Only one animated decorative chest is supported")
            chest_index = index
        flag = lambda value: "true" if value else "false"
        always_active = ", true" if asset == "SPIKES" and obj.get('alwaysActive', False) else ""
        lines.append(f"    {{Asset::{asset}, Role::{role}, {center_x}, {center_y}, {width}, {height}, {frame}, {rotation}, {flag(flip)}, {flag(solid)}, {{{left}, {right}, {top}, {bottom}}}, {direction}{always_active}}},")
    if not doors or len(enemies) > 4 or not 1 <= decorations <= 32 or item_count > 4 or lever_count > 1 or campfire_count > 1 or stair_count > 1:
        raise ValueError("Room requires a door, 1..32 decorations, at most four skeletons and collectibles, one lever, campfire and stairs")
    return {"id": room_id, "name": data.get("name", "Imported room"),
            "background": background, "spawn_x": spawn_x, "spawn_y": spawn_y,
            "decoration_count": decorations, "enemy_indices": enemies,
            "chest_index": chest_index, "objects": lines}


# Preserve the numeric template ID used by the originally integrated room/save.
LEGACY_ROOM_ID = "9d323109-6108-48f2-b079-a3516b6bf034"


def template_number(room_id):
    if room_id == LEGACY_ROOM_ID:
        return 21  # RoomTemplateId::IMPORTED; existing save layout stays unchanged.
    return 0x10000000 | (zlib.crc32(room_id.encode("utf-8")) & 0x0FFFFFFF)


def cpp_string(value):
    # Byte escapes also support names outside the BMP without surrogate escapes.
    result = '"'
    for byte in value.encode("utf-8"):
        if 32 <= byte < 127 and byte not in (34, 92):
            result += chr(byte)
        else:
            result += "\\%03o" % byte
    return result + '"'


def generate_catalog(entries):
    rooms = []
    compositions = []
    seen_ids = set()
    seen_templates = set()
    for source, data in entries:
        try:
            if isinstance(data, dict) and data.get("schemaVersion") == 2 and 'runtimeBinding' in data:
                room_id = data.get("id")
                if not isinstance(room_id, str) or not room_id or room_id in seen_ids:
                    raise ValueError("Invalid or duplicate room id: " + str(room_id))
                seen_ids.add(room_id)
                compositions.append((source, data))
                continue
            room = parse_room(data)
            if room["id"] in seen_ids:
                raise ValueError("Duplicate room id: " + room["id"])
            template = template_number(room["id"])
            if template in seen_templates:
                raise ValueError("Room ID hash collision; give this room a different id")
            room["template"] = template
            seen_ids.add(room["id"])
            seen_templates.add(template)
            rooms.append(room)
        except (ValueError, KeyError, TypeError, AttributeError) as error:
            raise ValueError(f"{source}: {error}") from error
    # File names and directory traversal order do not affect seeded selection.
    rooms.sort(key=lambda room: room["id"])
    max_decorations = max([1] + [room["decoration_count"] for room in rooms])
    max_enemies = max([0] + [len(room["enemy_indices"]) for room in rooms])
    lines = [
        "// Generated by tools/import_room.py. Edit the JSON files in rooms/.",
        "#ifndef IMPORTED_ROOM_H", "#define IMPORTED_ROOM_H", "",
        '#include "room_manager.h"', '#include "sprite_collision.h"', "",
        "namespace imported_room", "{", "",
        "enum class Asset { TABLE, LIBRARY_FRONT, LIBRARY_BACK, COLUMN, SEAT, VASE, ROCK, SIGN, AXE, SPIKES, SKELETON, BAT, PLANT, MUSHY, CHEST, KEY, ENERGY, SPRINT, LEVER, CAMPFIRE, STAIRS, DOOR, DOOR_SPECIAL, DOOR_CLOSED, DOOR_SPECIAL_CLOSED, TABLE_ARMOR, BOX, PLAYER };",
        "enum class Role { DECORATION, ENEMY, DOOR, HAZARD, ITEM, LEVER, INTERACTIVE, STAIRS };", "",
        "struct Object", "{", "    Asset asset;", "    Role role;",
        "    int x;", "    int y;", "    int width;", "    int height;",
        "    int frame;", "    int rotation;", "    bool flip_x;", "    bool solid;",
        "    game::Hitbox hitbox;", "    int door_direction;", "    bool always_active = false;", "};", "",
        "struct ObjectRange", "{", "    const Object* data;", "    int size;",
        "    constexpr const Object* begin() const { return data; }",
        "    constexpr const Object* end() const { return data + size; }",
        "    constexpr const Object& operator[](int index) const { return data[index]; }",
        "};", "",
        "struct Room", "{", "    RoomTemplateId template_id;",
        "    const char* id;", "    const char* name;", "    RoomBackgroundId background;",
        "    int spawn_x;", "    int spawn_y;", "    int decoration_count;",
        "    int enemy_count;", "    const int* enemy_indices;", "    int chest_index;",
        "    ObjectRange objects;", "",
        "    constexpr int count(Asset asset) const",
        "    { int result = 0; for(const Object& object : objects) if(object.asset == asset) ++result; return result; }",
        "    constexpr const Object* by_asset(Asset asset, int index) const",
        "    { for(const Object& object : objects) if(object.asset == asset && index-- == 0) return &object; return nullptr; }",
        "    constexpr const Object& enemy(int index) const",
        "    { return objects[enemy_indices[index]]; }", "",
        "    constexpr const Object* door(RoomDirection direction) const", "    {",
        "        for(const Object& object : objects)",
        "            if(object.role == Role::DOOR && object.door_direction == int(direction))",
        "                return &object;", "        return nullptr;", "    }", "};", "",
        f"constexpr int room_count = {len(rooms)};",
        f"constexpr int max_decoration_count = {max_decorations};",
        f"constexpr int max_enemy_count = {max_enemies};", "",
        "// Safe empty range for non-imported rooms; no runtime construction/RNG.",
        "constexpr Object empty_objects[] = {{Asset::TABLE, Role::DECORATION,",
        "    0, 0, 0, 0, 0, 0, false, false, {0, 0, 0, 0}, -1}};",
        "constexpr int empty_enemies[] = {-1};",
        'constexpr Room empty_room = {RoomTemplateId::FIELD, "", "", RoomBackgroundId::LAND,',
        "    0, 0, 0, 0, empty_enemies, -1, {empty_objects, 0}};", ""
    ]
    for index, room in enumerate(rooms):
        enemies = ", ".join(map(str, room["enemy_indices"])) or "-1"
        lines += [f"constexpr int enemies_{index}[] = {{{enemies}}};",
                  f"constexpr Object objects_{index}[] =", "{", *room["objects"], "};", ""]
    lines += ["constexpr Room rooms[] =", "{"]
    for index, room in enumerate(rooms):
        template = ("RoomTemplateId::IMPORTED" if room["id"] == LEGACY_ROOM_ID
                    else f"static_cast<RoomTemplateId>({room['template']})")
        lines += [f"    {{{template}, {cpp_string(room['id'])}, {cpp_string(room['name'])},",
                  f"     RoomBackgroundId::{room['background']}, {room['spawn_x']}, {room['spawn_y']},",
                  f"     {room['decoration_count']}, {len(room['enemy_indices'])}, enemies_{index}, {room['chest_index']},",
                  f"     {{objects_{index}, {len(room['objects'])}}}}},"]
    if not rooms:
        lines += ["    empty_room,"]
    lines += ["};", "",
              "constexpr const Room* find(RoomTemplateId template_id)", "{",
              "    for(int index = 0; index < room_count; ++index)",
              "        if(rooms[index].template_id == template_id)",
              "            return &rooms[index];", "    return nullptr;", "}", "",
              "constexpr bool contains(RoomTemplateId template_id)",
              "{ return find(template_id) != nullptr; }", "",
              "constexpr const Room& get(RoomTemplateId template_id)", "{",
              "    const Room* room = find(template_id);",
              "    return room ? *room : empty_room;", "}", "",
              "constexpr RoomTemplateId template_at(int index)",
              "{ return rooms[index].template_id; }", "", "}", "", "#endif", ""]
    header = "\n".join(lines)
    if compositions:
        native = generate_composition_cpp(compositions, cpp_string)
        header = header.replace("\n#endif\n", "\n" + native + "\n#endif\n")
    return header


def generate(data):
    """Single-room entry point retained for converter callers."""
    return generate_catalog([("<room>", data)])


def load_catalog(source):
    if source.is_dir():
        paths = sorted(path for path in source.iterdir()
                       if path.is_file() and path.suffix.lower() == ".json")
    elif source.is_file():
        paths = [source]
    else:
        raise ValueError(f"Room source does not exist: {source}")
    entries = []
    for path in paths:
        try:
            data = json.loads(path.read_text(encoding="utf-8-sig"))
        except (OSError, ValueError) as error:
            raise ValueError(f"{path}: {error}") from error
        entries.append((str(path), data))
    return generate_catalog(entries)


def main():
    if len(sys.argv) != 3:
        raise SystemExit("Usage: import_room.py rooms_directory_or_file output.h")
    source, target = map(Path, sys.argv[1:])
    try:
        content = load_catalog(source)
    except (OSError, ValueError) as error:
        raise SystemExit(f"Invalid room catalog: {error}") from error
    target.parent.mkdir(parents=True, exist_ok=True)
    if not target.exists() or target.read_text(encoding="utf-8") != content:
        with target.open("w", encoding="utf-8", newline="\n") as output:
            output.write(content)


if __name__ == "__main__":
    main()
