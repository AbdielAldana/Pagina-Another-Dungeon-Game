#include "room_manager.h"
#include "bn_seed_random.h"
#include "imported_room.h"

int RoomManager::direction_index(RoomDirection direction)
{
    switch(direction)
    {
    case RoomDirection::LEFT:
        return 0;
    case RoomDirection::RIGHT:
        return 1;
    case RoomDirection::UP:
        return 2;
    case RoomDirection::DOWN:
        return 3;

    default:
        return 0;
    }

    return 0;
}

RoomDirection RoomManager::opposite_direction(RoomDirection direction)
{
    switch(direction)
    {
    case RoomDirection::LEFT:
        return RoomDirection::RIGHT;
    case RoomDirection::RIGHT:
        return RoomDirection::LEFT;
    case RoomDirection::UP:
        return RoomDirection::DOWN;
    case RoomDirection::DOWN:
        return RoomDirection::UP;

    default:
        return RoomDirection::LEFT;
    }

    return RoomDirection::LEFT;
}

int RoomManager::add_room(RoomTemplateId template_id)
{
    if(_room_count >= max_rooms)
    {
        return -1;
    }

    int room_id = _room_count++;
    _rooms[room_id].template_id = template_id;

    if(_current_room_id < 0)
    {
        _current_room_id = room_id;
        _rooms[room_id].position_assigned = true;
        _rooms[room_id].discovered = true;
    }

    return room_id;
}

void RoomManager::generate_menu_floor()
{
    for(RoomInstance& room : _rooms)
        room = RoomInstance();
    _room_count = 0;
    _current_room_id = -1;
    _menu_floor = true;

    add_room(RoomTemplateId::TUTORIAL);
    _rooms[0].background_id = RoomBackgroundId::MENU_START;
    add_room(RoomTemplateId::STAIRS);
    _rooms[1].background_id = RoomBackgroundId::CAMPFIRE;
    connect_rooms(0, RoomDirection::UP, 1);
}

bool RoomManager::is_menu_floor() const
{
    return _menu_floor;
}

bool RoomManager::saved_floor_valid() const
{
    if(_menu_floor || _room_count < 5 || _room_count > max_rooms ||
       _current_room_id < 0 || _current_room_id >= _room_count)
    {
        return false;
    }

    if(_rooms[_room_count - 1].template_id != RoomTemplateId::REWARD)
    {
        return false;
    }
    RoomTemplateId stairs_template = _rooms[_room_count - 2].template_id;
    if(stairs_template != RoomTemplateId::STAIRS && stairs_template != RoomTemplateId::STAIRS_2)
    {
        return false;
    }

    constexpr int opposite_slots[4] = {1, 0, 3, 2};
    for(int room_id = 0; room_id < _room_count; ++room_id)
    {
        const RoomInstance& room = _rooms[room_id];
        if(int(room.template_id) < int(RoomTemplateId::FIELD) ||
           (int(room.template_id) > int(RoomTemplateId::REWARD) &&
            !imported_room::contains(room.template_id)) ||
           int(room.background_id) < int(RoomBackgroundId::LAND) ||
           int(room.background_id) > int(RoomBackgroundId::BOSS_ROOM) ||
           !room.position_assigned || room.grid_x < -6 || room.grid_x > 6 ||
           room.grid_y < -6 || room.grid_y > 6)
        {
            return false;
        }

        for(int slot = 0; slot < 4; ++slot)
        {
            int target_room_id = room.doors[slot].target_room_id;
            if(target_room_id < -1 || target_room_id >= _room_count || target_room_id == room_id)
            {
                return false;
            }
            if(imported_room::contains(room.template_id) && target_room_id >= 0 &&
               !imported_room::get(room.template_id).door(static_cast<RoomDirection>(slot)))
                return false;
            if(target_room_id >= 0 && !composition_rooms::allows(room.template_id, static_cast<RoomDirection>(slot)))
                return false;
            if(target_room_id >= 0 &&
               _rooms[target_room_id].doors[opposite_slots[slot]].target_room_id != room_id)
            {
                return false;
            }
        }

        for(const RoomItem& item : room.items)
        {
            if(item.active &&
               (int(item.type) < int(RoomItemType::KEY) ||
                int(item.type) > int(RoomItemType::ABILITY_FAMILIAR) ||
                item.level < 1 || item.level > 5 ||
                item.x < -120 || item.x > 120 || item.y < -80 || item.y > 80))
            {
                return false;
            }
        }
    }
    return true;
}

bool RoomManager::generate_random_floor(bn::seed_random& random, int room_count, bool tutorial_start)
{
    // Cada piso reserva un espacio para la sala de recompensa adicional.
    if(room_count < 1 || room_count >= max_rooms)
    {
        return false;
    }

    _menu_floor = false;
    for(RoomInstance& room : _rooms)
    {
        room = RoomInstance();
    }

    _room_count = 0;
    _current_room_id = -1;
    add_room(tutorial_start ? RoomTemplateId::TUTORIAL : RoomTemplateId::FIELD);
    _rooms[0].background_id = tutorial_start ? RoomBackgroundId::TUTORIAL : RoomBackgroundId::LAND;
    int special_room_id = room_count >= 3 ? room_count - 2 : -1;
    int item_room_count = 0;

    // Estos diseños dejan libre el centro para las escaleras del último cuarto.

    for(int room_id = 1; room_id < room_count; ++room_id)
    {
        bool final_room = room_id == room_count - 1;
        bool special_room = room_id == special_room_id;
        int composition = random.get_int(9);
        RoomTemplateId template_id = final_room ?
            (random.get_int(2) == 0 ? RoomTemplateId::STAIRS : RoomTemplateId::STAIRS_2) :
            (special_room ? RoomTemplateId::SPECIAL :
             (composition == 0 ? RoomTemplateId::COMPOSITION_1 :
              (composition == 1 ? RoomTemplateId::COMPOSITION_1_2 :
              (composition == 2 ? RoomTemplateId::COMPOSITION_2 :
              (composition == 3 ? RoomTemplateId::COMPOSITION_2_2 :
              (composition == 4 ? RoomTemplateId::COMPOSITION_3 :
              (composition == 5 ? RoomTemplateId::COMPOSITION_4 :
              (composition == 6 ? RoomTemplateId::PLANT_GARDEN :
                                  RoomTemplateId::PLANT_CENTER_ROOM))))))));
        if(!final_room && !special_room)
        {
            // El JSON no declara puntos Q: conserva dos cuartos para llaves y aventureros.
            int remaining_ordinary_rooms = special_room_id - room_id - 1;
            if(composition == 8 && imported_room::room_count > 0 &&
               item_room_count + remaining_ordinary_rooms >= 2)
            {
                int catalog_index = imported_room::room_count == 1 ? 0 :
                    random.get_int(imported_room::room_count);
                template_id = imported_room::template_at(catalog_index);
            }
            if(!imported_room::contains(template_id))
                ++item_room_count;
        }
        add_room(template_id);
        RoomInstance& new_room = _rooms[room_id];
        const composition_rooms::Room* composition_source = composition_rooms::find(template_id);
        if(composition_source)
        {
            int count = composition_source->backgrounds.size;
            new_room.background_id = composition_source->backgrounds[count == 1 ? 0 : random.get_int(count)];
        }
        else if(final_room)
        {
            int background_choice = random.get_int(3);
            new_room.background_id = background_choice == 0 ? RoomBackgroundId::CAMPFIRE :
                (background_choice == 1 ? RoomBackgroundId::ROOM2 : RoomBackgroundId::ROOM3);
        }
        else if(special_room)
        {
            new_room.background_id = RoomBackgroundId::CAMPFIRE;
        }
        else if(imported_room::contains(template_id))
        {
            new_room.background_id = imported_room::get(template_id).background;
        }
        else if(template_id == RoomTemplateId::COMPOSITION_1)
        {
            new_room.background_id = RoomBackgroundId::CAMPFIRE;
        }
        else if(template_id == RoomTemplateId::COMPOSITION_2 ||
                template_id == RoomTemplateId::COMPOSITION_2_2)
        {
            new_room.background_id = random.get_int(2) == 0 ? RoomBackgroundId::LAND :
                RoomBackgroundId::CAMPFIRE;
        }
        else if(template_id == RoomTemplateId::PLANT_GARDEN ||
                template_id == RoomTemplateId::PLANT_CENTER_ROOM)
        {
            int background_choice = random.get_int(3);
            new_room.background_id = background_choice == 0 ? RoomBackgroundId::CAMPFIRE :
                (background_choice == 1 ? RoomBackgroundId::ROOM2 : RoomBackgroundId::ROOM3);
        }
        else
        {
            int background_choice = random.get_int(3);
            new_room.background_id = background_choice == 0 ? RoomBackgroundId::LAND :
                (background_choice == 1 ? RoomBackgroundId::ROOM2 : RoomBackgroundId::ROOM3);
        }

        bool connected = false;
        int parent_limit = final_room && special_room_id >= 0 ? room_id - 1 : room_id;

        // STAIRS_2 solo tiene la puerta obligatoria del borde inferior. Conecta
        // su vecino por arriba para que el lado visible coincida con la reticula.
        if(final_room && new_room.template_id == RoomTemplateId::STAIRS_2)
        {
            for(int attempt = 0; attempt < max_rooms * 4 && ! connected; ++attempt)
            {
                int parent_id = random.get_int(parent_limit);
                if(! has_door(parent_id, RoomDirection::UP))
                {
                    connected = connect_rooms(parent_id, RoomDirection::UP, room_id);
                }
            }

            for(int parent_id = 0; parent_id < parent_limit && ! connected; ++parent_id)
            {
                if(! has_door(parent_id, RoomDirection::UP))
                {
                    connected = connect_rooms(parent_id, RoomDirection::UP, room_id);
                }
            }

            if(! connected)
            {
                // Si el mapa no tiene lugar para esa orientación, usa la variante
                // con puertas opcionales para no perder la conexión del piso.
                new_room.template_id = RoomTemplateId::STAIRS;
            }
        }

        // Cada cuarto nuevo se conecta a uno anterior por una salida libre.
        for(int attempt = 0; attempt < max_rooms * 4 && ! connected; ++attempt)
        {
            int parent_id = random.get_int(parent_limit);
            RoomDirection direction = static_cast<RoomDirection>(random.get_int(4));

            if(! has_door(parent_id, direction))
            {
                connected = connect_rooms(parent_id, direction, room_id);
            }
        }

        // Garantiza que el piso siga conectado si los intentos aleatorios no encontraron salida.
        for(int parent_id = 0; parent_id < parent_limit && ! connected; ++parent_id)
        {
            for(int direction_index_value = 0; direction_index_value < 4 && ! connected;
                ++direction_index_value)
            {
                RoomDirection direction = static_cast<RoomDirection>(direction_index_value);

                if(! has_door(parent_id, direction))
                {
                    connected = connect_rooms(parent_id, direction, room_id);
                }
            }
        }

        // A restrictive authored door layout may not fit this floor. Use a
        // standard room rather than adding doors that the JSON did not declare.
        if(!connected && imported_room::contains(new_room.template_id))
        {
            new_room.template_id = RoomTemplateId::PLANT_CENTER_ROOM;
            ++item_room_count;
            for(int parent_id = 0; parent_id < parent_limit && !connected; ++parent_id)
            {
                for(int slot = 0; slot < 4 && !connected; ++slot)
                {
                    RoomDirection direction = static_cast<RoomDirection>(slot);
                    if(!has_door(parent_id, direction))
                        connected = connect_rooms(parent_id, direction, room_id);
                }
            }
        }
        if(! connected)
        {
            return false;
        }
    }

    if(special_room_id >= 0)
    {
        for(int direction_index_value = 0; direction_index_value < 4; ++direction_index_value)
        {
            RoomDirection direction = static_cast<RoomDirection>(direction_index_value);
            RoomInstance::Door& door =
                _rooms[special_room_id].doors[direction_index(direction)];
            if(door.target_room_id >= 0)
            {
                int other_room_id = door.target_room_id;
                int opposite_slot = direction_index(opposite_direction(direction));
                door.special = true;
                door.open = false;
                _rooms[other_room_id].doors[opposite_slot].special = true;
                _rooms[other_room_id].doors[opposite_slot].open = false;
                break;
            }
        }
    }

    int reward_room_id = add_room(RoomTemplateId::REWARD);
    if(reward_room_id < 0)
    {
        return false;
    }
    _rooms[reward_room_id].background_id = RoomBackgroundId::BOSS_ROOM;

    // Boss 1 Ant tiene la puerta obligatoria a la derecha; queda a la
    // izquierda del cuarto existente al que se conecta.
    bool reward_connected = false;
    for(int attempt = 0; attempt < max_rooms * 8 && ! reward_connected; ++attempt)
    {
        int parent_id = random.get_int(reward_room_id - 1);
        if(! has_door(parent_id, RoomDirection::LEFT))
        {
            reward_connected = connect_rooms(parent_id, RoomDirection::LEFT, reward_room_id);
        }
    }
    for(int parent_id = 0; parent_id < reward_room_id && ! reward_connected; ++parent_id)
    {
        if(! has_door(parent_id, RoomDirection::LEFT))
        {
            reward_connected = connect_rooms(parent_id, RoomDirection::LEFT, reward_room_id);
        }
    }
    if(! reward_connected)
    {
        return false;
    }
    int reward_parent_id = connected_room_id(reward_room_id, RoomDirection::RIGHT);
    set_door_open(reward_room_id, RoomDirection::RIGHT, false);
    set_door_visible(reward_room_id, RoomDirection::RIGHT, false);
    set_door_open(reward_parent_id, RoomDirection::LEFT, false);
    set_door_visible(reward_parent_id, RoomDirection::LEFT, false);

    return true;
}

bool RoomManager::connect_rooms(int room_id, RoomDirection direction, int other_room_id)
{
    if(room_id < 0 || room_id >= _room_count || other_room_id < 0 || other_room_id >= _room_count ||
       room_id == other_room_id)
    {
        return false;
    }

    if((imported_room::contains(_rooms[room_id].template_id) && !imported_room::get(_rooms[room_id].template_id).door(direction)) ||
       (imported_room::contains(_rooms[other_room_id].template_id) &&
        !imported_room::get(_rooms[other_room_id].template_id).door(opposite_direction(direction))))
        return false;

    if(!composition_rooms::allows(_rooms[room_id].template_id, direction, _menu_floor && room_id == 0) ||
       !composition_rooms::allows(_rooms[other_room_id].template_id, opposite_direction(direction),
           _menu_floor && other_room_id == 0)) return false;

    int direction_slot = direction_index(direction);
    int opposite_slot = direction_index(opposite_direction(direction));

    RoomInstance& room = _rooms[room_id];
    RoomInstance& other_room = _rooms[other_room_id];
    int target_x = room.grid_x;
    int target_y = room.grid_y;

    switch(direction)
    {
    case RoomDirection::LEFT: --target_x; break;
    case RoomDirection::RIGHT: ++target_x; break;
    case RoomDirection::UP: --target_y; break;
    case RoomDirection::DOWN: ++target_y; break;
    }

    if(! room.position_assigned || target_x < -6 || target_x > 6 || target_y < -6 || target_y > 6)
    {
        return false;
    }

    if(other_room.position_assigned && (other_room.grid_x != target_x || other_room.grid_y != target_y))
    {
        return false;
    }

    for(int index = 0; index < _room_count; ++index)
    {
        if(index != other_room_id && _rooms[index].position_assigned &&
           _rooms[index].grid_x == target_x && _rooms[index].grid_y == target_y)
        {
            return false;
        }
    }

    RoomInstance::Door& door = _rooms[room_id].doors[direction_slot];
    RoomInstance::Door& opposite_door = _rooms[other_room_id].doors[opposite_slot];

    if((door.target_room_id >= 0 && door.target_room_id != other_room_id) ||
       (opposite_door.target_room_id >= 0 && opposite_door.target_room_id != room_id))
    {
        return false;
    }

    door.target_room_id = other_room_id;
    other_room.grid_x = target_x;
    other_room.grid_y = target_y;
    other_room.position_assigned = true;
    door.open = true;
    door.visible = true;
    opposite_door.target_room_id = room_id;
    opposite_door.open = true;
    opposite_door.visible = true;
    return true;
}

bool RoomManager::has_door(RoomDirection direction) const
{
    if(_current_room_id < 0)
    {
        return false;
    }

    return has_door(_current_room_id, direction);
}

bool RoomManager::has_door(int room_id, RoomDirection direction) const
{
    if(room_id < 0 || room_id >= _room_count)
    {
        return false;
    }

    // Las tres puertas sin destino del menu permanecen cerradas.
    return (_menu_floor && room_id == 0) ||
        _rooms[room_id].doors[direction_index(direction)].target_room_id >= 0;
}

bool RoomManager::door_is_open(RoomDirection direction) const
{
    if(_current_room_id < 0)
    {
        return false;
    }

    const RoomInstance::Door& door = _rooms[_current_room_id].doors[direction_index(direction)];
    return door.target_room_id >= 0 && door.open;
}

bool RoomManager::door_is_special(RoomDirection direction) const
{
    if(_current_room_id < 0)
    {
        return false;
    }

    return _rooms[_current_room_id].doors[direction_index(direction)].special;
}

bool RoomManager::door_is_special(int room_id, RoomDirection direction) const
{
    return room_id >= 0 && room_id < _room_count &&
        _rooms[room_id].doors[direction_index(direction)].special;
}

bool RoomManager::door_is_visible(int room_id, RoomDirection direction) const
{
    if(room_id < 0 || room_id >= _room_count) return false;
    const composition_rooms::Room* source = composition_rooms::find(_rooms[room_id].template_id, _menu_floor && room_id == 0);
    return (has_door(room_id, direction) ||
        (source && source->doors[direction_index(direction)].rule == 2) ||
        (imported_room::contains(_rooms[room_id].template_id) && imported_room::get(_rooms[room_id].template_id).door(direction))) &&
        _rooms[room_id].doors[direction_index(direction)].visible;
}

bool RoomManager::set_door_visible(int room_id, RoomDirection direction, bool visible)
{
    if(room_id < 0 || room_id >= _room_count)
    {
        return false;
    }
    RoomInstance::Door& door = _rooms[room_id].doors[direction_index(direction)];
    if(door.target_room_id < 0)
    {
        return false;
    }
    int other_room_id = door.target_room_id;
    int opposite_slot = direction_index(opposite_direction(direction));
    door.visible = visible;
    _rooms[other_room_id].doors[opposite_slot].visible = visible;
    return true;
}

bool RoomManager::touching_door(int player_x, int player_y, RoomDirection direction) const
{
    const composition_rooms::Room* source = composition_rooms::find(current_template(), _menu_floor && _current_room_id == 0);
    if(source)
    {
        const composition_rooms::Door& door = source->doors[direction_index(direction)];
        if(!door.rule) return false;
        switch(direction)
        {
        case RoomDirection::LEFT: return player_x <= door.x + 10 && player_y >= door.y - 12 && player_y <= door.y + 12;
        case RoomDirection::RIGHT: return player_x >= door.x - 10 && player_y >= door.y - 12 && player_y <= door.y + 12;
        case RoomDirection::UP: return player_y <= door.y + 8 && player_x >= door.x - 12 && player_x <= door.x + 12;
        case RoomDirection::DOWN: return player_y >= door.y - 14 && player_x >= door.x - 12 && player_x <= door.x + 12;
        }
    }
    if(imported_room::contains(current_template()))
    {
        const imported_room::Object *door = imported_room::get(current_template()).door(direction);
        if(!door) return false;
        switch(direction)
        {
        case RoomDirection::LEFT:
            return player_x <= door->x + 10 && player_y >= door->y - 12 && player_y <= door->y + 12;
        case RoomDirection::RIGHT:
            return player_x >= door->x - 10 && player_y >= door->y - 12 && player_y <= door->y + 12;
        case RoomDirection::UP:
            return player_y <= door->y + 8 && player_x >= door->x - 12 && player_x <= door->x + 12;
        case RoomDirection::DOWN:
            return player_y >= door->y - 14 && player_x >= door->x - 12 && player_x <= door->x + 12;
        }
    }
    switch(direction)
    {
    case RoomDirection::LEFT:
        return player_x <= -102 && player_y >= -12 && player_y <= 12;
    case RoomDirection::RIGHT:
        return player_x >= 102 && player_y >= -13 && player_y <= 11;
    case RoomDirection::UP:
        return player_y <= -64 && player_x >= -12 && player_x <= 12;
    case RoomDirection::DOWN:
        return player_y >= 58 && player_x >= -12 && player_x <= 12;
    }

    return false;
}

bool RoomManager::set_door_open(int room_id, RoomDirection direction, bool open)
{
    if(room_id < 0 || room_id >= _room_count)
    {
        return false;
    }

    int direction_slot = direction_index(direction);
    RoomInstance::Door& door = _rooms[room_id].doors[direction_slot];

    if(door.target_room_id < 0)
    {
        return false;
    }

    int other_room_id = door.target_room_id;
    int opposite_slot = direction_index(opposite_direction(direction));
    door.open = open;
    _rooms[other_room_id].doors[opposite_slot].open = open;
    return true;
}

int RoomManager::current_room_id() const
{
    return _current_room_id;
}

bool RoomManager::set_current_room(int room_id)
{
    if(room_id < 0 || room_id >= _room_count)
    {
        return false;
    }

    _current_room_id = room_id;
    _rooms[room_id].discovered = true;
    return true;
}

int RoomManager::room_count() const { return _room_count; }
int RoomManager::stairs_room_id() const { return _menu_floor ? 1 : (_room_count >= 2 ? _room_count - 2 : -1); }
int RoomManager::reward_room_id() const { return !_menu_floor && _room_count >= 1 ? _room_count - 1 : -1; }

bool RoomManager::room_discovered(int room_id) const
{
    return room_id >= 0 && room_id < _room_count && _rooms[room_id].discovered;
}

int RoomManager::room_grid_x(int room_id) const
{
    return room_id >= 0 && room_id < _room_count ? _rooms[room_id].grid_x : 0;
}

int RoomManager::room_grid_y(int room_id) const
{
    return room_id >= 0 && room_id < _room_count ? _rooms[room_id].grid_y : 0;
}

int RoomManager::connected_room_id(int room_id, RoomDirection direction) const
{
    if(room_id < 0 || room_id >= _room_count) return -1;
    return _rooms[room_id].doors[direction_index(direction)].target_room_id;
}

RoomTemplateId RoomManager::current_template() const
{
    return room_template(_current_room_id);
}

RoomTemplateId RoomManager::room_template(int room_id) const
{
    return room_id >= 0 && room_id < _room_count ?
        _rooms[room_id].template_id : RoomTemplateId::FIELD;
}

RoomBackgroundId RoomManager::room_background(int room_id) const
{
    return room_id >= 0 && room_id < _room_count ?
        _rooms[room_id].background_id : RoomBackgroundId::LAND;
}

bool RoomManager::add_item(int room_id, RoomItemType type, int x, int y, int level)
{
    if(room_id < 0 || room_id >= _room_count)
    {
        return false;
    }

    for(RoomItem& item : _rooms[room_id].items)
    {
        if(! item.active)
        {
            item.type = type;
            item.x = x;
            item.y = y;
            item.level = level;
            item.active = true;
            return true;
        }
    }

    return false;
}

const RoomItem* RoomManager::item_at(int room_id, int item_index) const
{
    if(room_id < 0 || room_id >= _room_count || item_index < 0 || item_index >= max_items_per_room)
    {
        return nullptr;
    }

    const RoomItem& item = _rooms[room_id].items[item_index];
    return item.active ? &item : nullptr;
}

bool RoomManager::remove_item(int room_id, int item_index)
{
    if(room_id < 0 || room_id >= _room_count || item_index < 0 || item_index >= max_items_per_room)
    {
        return false;
    }

    RoomItem& item = _rooms[room_id].items[item_index];

    if(! item.active)
    {
        return false;
    }

    item.active = false;
    return true;
}

bool RoomManager::try_transition(int player_x, int player_y, int& spawn_x, int& spawn_y)
{
    if(_current_room_id < 0)
    {
        return false;
    }

    RoomDirection direction;
    bool entered_door = false;

    // Los limites coinciden con el borde interior de los sprites de puerta en main.cpp.
    if(touching_door(player_x, player_y, RoomDirection::LEFT) && door_is_open(RoomDirection::LEFT))
    {
        direction = RoomDirection::LEFT;
        entered_door = true;
        spawn_x = 95;
        spawn_y = 0;
    }
    else if(touching_door(player_x, player_y, RoomDirection::RIGHT) && door_is_open(RoomDirection::RIGHT))
    {
        direction = RoomDirection::RIGHT;
        entered_door = true;
        spawn_x = -96;
        spawn_y = 0;
    }
    else if(touching_door(player_x, player_y, RoomDirection::UP) && door_is_open(RoomDirection::UP))
    {
        direction = RoomDirection::UP;
        entered_door = true;
        spawn_x = 0;
        spawn_y = 55;
    }
    else if(touching_door(player_x, player_y, RoomDirection::DOWN) && door_is_open(RoomDirection::DOWN))
    {
        direction = RoomDirection::DOWN;
        entered_door = true;
        spawn_x = 0;
        spawn_y = -56;
    }

    if(! entered_door)
    {
        return false;
    }

    _current_room_id = _rooms[_current_room_id].doors[direction_index(direction)].target_room_id;
    if(imported_room::contains(current_template()))
    {
        const imported_room::Object *door = imported_room::get(current_template()).door(opposite_direction(direction));
        if(door)
        {
            spawn_x = door->x + (direction == RoomDirection::RIGHT ? 16 :
                (direction == RoomDirection::LEFT ? -16 : 0));
            spawn_y = door->y + (direction == RoomDirection::DOWN ? 16 :
                (direction == RoomDirection::UP ? -16 : 0));
            if(spawn_x < -96) spawn_x = -96;
            if(spawn_x > 95) spawn_x = 95;
            if(spawn_y < -56) spawn_y = -56;
            if(spawn_y > 55) spawn_y = 55;
        }
    }
    const composition_rooms::Room* source = composition_rooms::find(current_template(), _menu_floor && _current_room_id == 0);
    if(source)
    {
        const composition_rooms::Door& door = source->doors[direction_index(opposite_direction(direction))];
        spawn_x = door.x + (direction == RoomDirection::RIGHT ? 16 : (direction == RoomDirection::LEFT ? -16 : 0));
        spawn_y = door.y + (direction == RoomDirection::DOWN ? 16 : (direction == RoomDirection::UP ? -16 : 0));
        if(spawn_x < -96) spawn_x = -96;
        if(spawn_x > 95) spawn_x = 95;
        if(spawn_y < -56) spawn_y = -56;
        if(spawn_y > 55) spawn_y = 55;
    }
    _rooms[_current_room_id].discovered = true;
    return true;
}
