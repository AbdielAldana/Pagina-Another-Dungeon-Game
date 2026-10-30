
#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_seed_random.h"
#include "bn_sram.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_item.h"
#include "bn_sprite_tiles_ptr.h"
#include "bn_sprite_shape_size.h"
#include "bn_bpp_mode.h"
#include "bn_tile.h"
#include "bn_display.h"
#include "bn_blending.h"
#include "bn_bgs_mosaic.h"
#include "bn_sprites_mosaic.h"
#include "bn_sprite_affine_mat_ptr.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_sprite_items_player.h"
#include "bn_sprite_items_key.h"
#include "bn_sprite_items_posion_energy_16x16_animate.h"
#include "bn_sprite_items_posion_sprint_16x16_animate.h"
#include "bn_sprite_items_door.h"
#include "bn_sprite_items_door_special.h"
#include "bn_sprite_items_close_door.h"
#include "bn_sprite_items_close_door_special.h"
#include "bn_sprite_items_piedra1.h"
#include "bn_sprite_items_column_16x32.h"
#include "bn_sprite_items_box_16x16.h"
#include "bn_sprite_items_librery_back_16x32.h"
#include "bn_sprite_items_librery_front_16x32.h"
#include "bn_sprite_items_jarron_16x16.h"
#include "bn_sprite_items_cartel_16x16.h"
#include "bn_sprite_items_asiento_16x16_4state_down_up_left_right.h"
#include "bn_sprite_items_table_16x32.h"
#include "bn_sprite_items_table_armor_16x32.h"
#include "bn_sprite_items_cheest_16x16_animate_3frames.h"
#include "bn_sprite_items_axe_floor_16x16.h"
#include "bn_sprite_items_skeleton.h"
#include "bn_sprite_items_bat.h"
#include "bn_sprite_items_mushy.h"
#include "bn_sprite_items_plant_enemy.h"
#include "bn_sprite_items_bullet_enemy_plant.h"
#include "bn_sprite_items_stairs.h"
#include "bn_sprite_items_spikes_16x16_animate.h"
#include "bn_sprite_items_campfire.h"
#include "bn_sprite_items_label_16x16_animate.h"
#include "bn_sprite_items_map_room.h"
#include "bn_sprite_items_map_room_stairs.h"
#include "bn_sprite_items_map_room_item.h"
#include "bn_sprite_items_map_room_boss.h"
#include "bn_sprite_items_map_current.h"
#include "bn_sprite_items_map_final.h"
#include "bn_sprite_items_map_link.h"
#include "bn_sprite_items_map_link_current.h"
#include "bn_sprite_items_map_special.h"
#include "bn_sprite_items_map_link_special.h"
#include "bn_sprite_items_ui_hability.h"
#include "bn_sprite_items_ui_hability_selected.h"
#include "bn_sprite_items_hability_dash_icon.h"
#include "bn_sprite_items_hability_stoptime_icon.h"
#include "bn_sprite_items_hability_trapdoorback_icon.h"
#include "bn_sprite_items_hability_viewallmap_icon.h"
#include "bn_sprite_items_hability_masterkey_icon.h"
#include "bn_sprite_items_hability_bomb_icon.h"
#include "bn_sprite_items_hability_familiar_icon.h"
#include "bn_sprite_items_ui_stamina_bar.h"
#include "bn_sprite_items_ui_energy_bar_liquid.h"
#include "bn_sprite_items_ui_sastamina_bar_liquid.h"
#include "bn_sound_items.h"
#include "common_variable_8x16_sprite_font.h"
#include "game_regular_font.h"

#include "bn_regular_bg_ptr.h"
#include "bn_optional.h"
#include "bn_bg_palette_ptr.h"
#include "bn_regular_bg_items_land.h"
#include "bn_regular_bg_items_room2.h"
#include "bn_regular_bg_items_room3.h"
#include "bn_regular_bg_items_room_tutorial.h"
#include "bn_regular_bg_items_room_campfire.h"
#include "bn_regular_bg_items_boss_1.h"
#include "bn_regular_bg_items_inventory.h"
#include "bn_regular_bg_items_habilidad_data.h"
#include "bn_regular_bg_items_mapback.h"
#include "bn_regular_bg_items_pantalla_inicio.h"
#include "bn_regular_bg_items_menu_inicio.h"
// #include "bn_regular_bg_items_clouds.h"
#include "bn_colors.h"
#include "room_manager.h"
#include "menu_room.h"
#include "sprite_collision.h"
#include "game_save.h"
#include "imported_room.h"

enum class Pantalla
{
    MENU,
    MAPA,
    COMBATE,
    INVENTARIO
};

enum class Direccion
{
    ABAJO,
    ARRIBA,
    IZQUIERDA,
    DERECHA
};

enum class ActiveAbility
{
    DASH,
    STOP_TIME,
    TRAPDOOR_BACK,
    VIEW_ALL_MAP,
    MASTER_KEY,
    BOMB,
    FAMILIAR
};

struct AbilitySlot
{
    ActiveAbility ability = ActiveAbility::DASH;
    int level = 1;
};

void create_floor_label(int floor_number, char *label)
{
    const char prefix[] = "FLOOR ";
    int index = 0;
    for (int prefix_index = 0; prefix_index < int(sizeof(prefix) - 1); ++prefix_index)
    {
        label[index++] = prefix[prefix_index];
    }

    char digits[10];
    int digit_count = 0;
    do
    {
        digits[digit_count++] = char('0' + floor_number % 10);
        floor_number /= 10;
    } while (floor_number > 0 && digit_count < 10);

    while (digit_count > 0)
    {
        label[index++] = digits[--digit_count];
    }
    label[index] = '\0';
}

void create_seed_label(unsigned seed, char *label)
{
    const char prefix[] = "SEED ";
    int index = 0;
    for (int prefix_index = 0; prefix_index < int(sizeof(prefix) - 1); ++prefix_index)
    {
        label[index++] = prefix[prefix_index];
    }

    char digits[10];
    int digit_count = 0;
    do
    {
        digits[digit_count++] = char('0' + seed % 10);
        seed /= 10;
    } while (seed > 0 && digit_count < 10);

    while (digit_count > 0)
    {
        label[index++] = digits[--digit_count];
    }
    label[index] = '\0';
}

// 60 cuadros de salida + 60 de entrada: aproximadamente dos segundos.
constexpr int logo_transition_half_frames = 60;

void show_logo_screen()
{
    bn::regular_bg_ptr logo_bg = bn::regular_bg_items::pantalla_inicio.create_bg(0, 0);
    logo_bg.set_priority(3);

    bn::sprite_text_generator logo_text_generator(common::variable_8x16_sprite_font);
    logo_text_generator.set_center_alignment();
    logo_text_generator.set_bg_priority(0);
    bn::vector<bn::sprite_ptr, 24> logo_prompt_sprites;
    constexpr int logo_prompt_row = 8;
    constexpr int logo_prompt_y = -80 + (logo_prompt_row - 1) * 16 + 8;
    logo_text_generator.generate(0, logo_prompt_y, "START TO ENTER", logo_prompt_sprites);

    constexpr int logo_blink_interval = 60;
    int logo_blink_timer = 0;
    bool logo_prompt_visible = true;
    while(true)
    {
        bn::core::update();
        if(bn::keypad::start_pressed())
        {
            bn::sound_items::move_ui.play();
            bn::bg_palette_ptr logo_palette = logo_bg.palette();
            bn::sprite_palette_ptr prompt_palette = logo_prompt_sprites[0].palette();
            for(int step = 1; step <= logo_transition_half_frames; ++step)
            {
                bn::fixed darkness = bn::fixed(step) / logo_transition_half_frames;
                logo_palette.set_fade(bn::colors::black, darkness);
                prompt_palette.set_fade(bn::colors::black, darkness);
                bn::core::update();
            }
            break;
        }

        if(++logo_blink_timer >= logo_blink_interval)
        {
            logo_blink_timer = 0;
            logo_prompt_visible = !logo_prompt_visible;
            for(bn::sprite_ptr& sprite : logo_prompt_sprites)
            {
                sprite.set_visible(logo_prompt_visible);
            }
        }
    }
}

int main()
{
    bn::core::init();
    show_logo_screen();

    // Pantalla ++++++++++++++
    Pantalla pantalla_actual = Pantalla::MAPA;

    // Pantalla --------------

    // Fondo +++++++++++++++++
    // land del escenario
    bn::regular_bg_ptr land =
        bn::regular_bg_items::land.create_bg(0, 0);
    bn::optional<bn::regular_bg_ptr> room2_bg;
    bn::optional<bn::regular_bg_ptr> room3_bg;
    bn::optional<bn::regular_bg_ptr> inventory_bg;
    bn::optional<bn::regular_bg_ptr> ability_data_bg;
    bn::optional<bn::regular_bg_ptr> map_bg;
    bn::optional<bn::regular_bg_ptr> room_special_bg;
    bn::optional<bn::regular_bg_ptr> reward_room_bg;
    static RoomManager room_manager;
    game_save::load();
    unsigned startup_ticks = unsigned(bn::core::current_cpu_ticks());
    // La semilla propuesta para una partida nueva vive solo en RAM.
    unsigned random_seed = startup_ticks ^ 0xA511E9B3u ^
        (game_save::records().deaths * 0x9E3779B9u);
    if(!random_seed)
        random_seed = 1;

    bn::seed_random floor_random;
    bn::seed_random enemy_ambient_random;
    bn::seed_random campfire_reward_random;
    bn::seed_random reward_lever_random;
    unsigned selected_seed = random_seed;
    unsigned run_seed = selected_seed;
    bn::seed_random seed_picker(random_seed ^ 0x6D2B79F5u);
    auto select_new_seed = [&]()
    {
        unsigned previous_seed = selected_seed;
        selected_seed = seed_picker.get();
        if(!selected_seed || selected_seed == previous_seed)
        {
            selected_seed = previous_seed + 1;
            if(!selected_seed)
                selected_seed = 1;
        }
    };
    auto discover_active_item = [](RoomItemType type)
    {
        int index = int(type) - int(RoomItemType::ABILITY_DASH);
        if(index >= 0 && index < 7)
        {
            game_save::Records &records = game_save::records();
            records.active_items_discovered = uint8_t(records.active_items_discovered | (1u << index));
        }
    };
    auto reset_floor_randoms = [&](int floor_number)
    {
        unsigned floor_seed = run_seed ^ (unsigned(floor_number - 1) * 0x9E3779B9u);
        floor_random.set_seed(floor_seed);
        enemy_ambient_random.set_seed(floor_seed ^ 0xBA771E5u);
        campfire_reward_random.set_seed(floor_seed ^ 0xCA4F1AEu);
        reward_lever_random.set_seed(floor_seed ^ 0x1E7E12u);
    };

    constexpr const char *nombres_cuartos[RoomManager::max_rooms] =
        {
            "ROOM 1", "ROOM 2", "ROOM 3", "ROOM 4",
            "ROOM 5", "ROOM 6", "ROOM 7", "ROOM 8",
            "ROOM 9", "ROOM 10", "ROOM 11", "ROOM 12",
            "ROOM 13", "ROOM 14", "ROOM 15", "REWARD"};
    auto imported_room_for = [&](int room_id) -> const imported_room::Room&
    {
        return imported_room::get(room_manager.room_template(room_id));
    };
    auto current_imported_room = [&]() -> const imported_room::Room&
    {
        return imported_room_for(room_manager.current_room_id());
    };
    auto composition_room_for = [&](int room_id) -> const composition_rooms::Room*
    {
        return composition_rooms::find(room_manager.room_template(room_id),
            room_manager.is_menu_floor() && room_id == 0);
    };
    auto current_composition_room = [&]() -> const composition_rooms::Room*
    {
        return composition_room_for(room_manager.current_room_id());
    };
    auto room_display_name = [&](int room_id) -> const char*
    {
        if(imported_room::contains(room_manager.room_template(room_id)))
            return imported_room_for(room_id).name;
        if(const composition_rooms::Room* source = composition_room_for(room_id))
            return source->name;
        return room_id >= 0 && room_id < RoomManager::max_rooms ? nombres_cuartos[room_id] : "ROOM";
    };

    // Nubes encima del land
    // bn::regular_bg_ptr clouds =
    //     bn::regular_bg_items::clouds.create_bg(0, 0);

    // bn::blending::set_transparency_alpha(0.5);
    // clouds.set_blending_enabled(true);

    // clouds.set_position(
    //     clouds.x() + 0.25,
    //     clouds.y());

    // Fondo ---------------------

    // Movimiento +++++++++++++++++
    constexpr int inicio_x = 0;
    constexpr int inicio_y = 0;
    int x = inicio_x;
    int y = inicio_y;

    int movement_speed_remainder = 0;
    int animation_speed = 16;

    Direccion direccion = Direccion::ABAJO;
    Direccion dash_direction = Direccion::ABAJO;
    ActiveAbility active_ability = ActiveAbility::DASH;
    constexpr int max_carried_abilities = 4;
    AbilitySlot carried_abilities[max_carried_abilities] = {};
    int carried_ability_count = 0;
    int selected_ability_slot = 0;
    bool replacing_ability = false;
    ActiveAbility pending_ability = ActiveAbility::DASH;
    int pending_ability_level = 1;
    bool reject_reward_until_leave = false;
    bool bombed_rooms[RoomManager::max_rooms] = {};
    constexpr int bomb_flash_duration_frames = 18;
    int bomb_flash_frames = 0;
    constexpr int bomb_fuse_duration_frames = 180;
    int pending_bomb_frames = 0;
    int pending_bomb_room_id = -1;
    int pending_bomb_x = 0;
    int pending_bomb_y = 0;
    int pending_bomb_energy_cost = 0;
    int bomb_blink_timer = 0;
    bool bomb_blink_visible = true;
    bool familiar_active = false;
    int familiar_frames_remaining = 0;
    int familiar_target_kind = -1;
    int familiar_target_index = -1;
    int familiar_room_id = -1;
    int familiar_target_x = 0;
    int familiar_target_y = 0;
    int familiar_x = 0;
    int familiar_y = 0;
    bool floor_map_revealed = false;
    bool reward_room_unlocked = false;
    bool reward_map_blinking = false;
    bool reward_map_blink_visible = true;
    int reward_map_blink_timer = 0;
    int reward_map_blink_count = 0;
    bool reward_lever_activated[3] = {};
    int reward_lever_room_ids[3] = {-1, -1, -1};
    int reward_lever_x[3] = {};
    int reward_lever_y[3] = {};
    bn::vector<bn::sprite_ptr, 1> reward_lever_sprites;
    int reward_lever_sprite_index = -1;
    int reward_lever_animation_timers[3] = {};
    int reward_lever_animation_frames[3] = {};
    int time_stopped_room_id = -1;
    bool trapdoor_deployed = false;
    int trapdoor_room_id = -1;
    int trapdoor_x = 0;
    int trapdoor_y = 0;
    bool trapdoor_transitioned_this_frame = false;
    constexpr int home_room_id = 0;
    constexpr int home_trapdoor_x = -96; // Columna 1 del cuarto principal
    constexpr int home_trapdoor_y = -56; // Fila 1 del cuarto principal
    bool dash_active = false;
    constexpr int dash_pixels_per_frame = 6;

    int frame = 0;
    int animation_timer = 0;
    int displayed_frame = 0;

    bn::sprite_ptr player =
        bn::sprite_items::player.create_sprite(0, 0, 0);
    player.set_z_order(-1);
    bn::sprite_ptr ability_panel = bn::sprite_items::ui_hability.create_sprite(104, -64);
    ability_panel.set_bg_priority(0);
    ability_panel.set_z_order(-18);
    ability_panel.set_visible(false);
    bn::sprite_palette_ptr trapdoor_ability_palette =
        bn::sprite_palette_ptr::create_new(bn::sprite_items::hability_trapdoorback_icon.palette_item());
    bn::sprite_ptr ability_icon = bn::sprite_items::hability_dash_icon.create_sprite(104, -64);
    ability_icon.set_palette(bn::sprite_items::hability_dash_icon.palette_item());
    ability_icon.set_bg_priority(0);
    ability_icon.set_z_order(-19);
    ability_icon.set_visible(false);
    bn::sprite_text_generator ability_level_text_generator(common::variable_8x16_sprite_font);
    ability_level_text_generator.set_center_alignment();
    ability_level_text_generator.set_bg_priority(0);
    ability_level_text_generator.set_z_order(-20);
    bn::vector<bn::sprite_ptr, 1> ability_level_text_sprites;
    bool ability_level_text_visible = false;
    bn::sprite_ptr trapdoor_world_sprite =
        bn::sprite_items::hability_trapdoorback_icon.create_sprite(0, 0);
    trapdoor_world_sprite.set_palette(trapdoor_ability_palette);
    trapdoor_world_sprite.set_bg_priority(1);
    trapdoor_world_sprite.set_z_order(0);
    trapdoor_world_sprite.set_visible(false);
    bn::sprite_ptr familiar_world_sprite =
        bn::sprite_items::hability_familiar_icon.create_sprite(0, 0);
    familiar_world_sprite.set_bg_priority(1);
    familiar_world_sprite.set_z_order(-4);
    familiar_world_sprite.set_visible(false);
    struct NpcState
    {
        bool active = false;
        bool delivered = false;
        int room_id = -1;
        int x = 0;
        int y = 0;
    };
    constexpr int max_npcs = 4;
    static NpcState npcs[max_npcs];
    BN_DATA_EWRAM_BSS static int npc_sprite_x[max_npcs] = {};
    BN_DATA_EWRAM_BSS static int npc_sprite_y[max_npcs] = {};
    int carried_npc_index = -1;
    int rescued_adventurers = 0;
    constexpr int follow_history_capacity = 64;
    constexpr int follow_delay = 16;
    BN_DATA_EWRAM_BSS static int follow_history_x[follow_history_capacity] = {};
    BN_DATA_EWRAM_BSS static int follow_history_y[follow_history_capacity] = {};
    int follow_history_head = 0;
    int follow_history_count = 0;
    bn::vector<bn::sprite_ptr, max_npcs> npc_sprites;
    for (int index = 0; index < max_npcs; ++index)
    {
        npc_sprites.push_back(bn::sprite_items::player.create_sprite(0, 0, 0));
        npc_sprites[index].set_bg_priority(1);
        npc_sprites[index].set_visible(false);
    }
    bn::vector<bn::sprite_ptr, 26> stone_sprites;
    bn::vector<bn::sprite_ptr, menu_room::object_count> menu_object_sprites;
    int menu_chest_sprite_index = -1;
    int menu_chest_frame = 0;
    int menu_chest_target_frame = 0;
    int menu_chest_animation_timer = 0;
    bn::vector<bn::sprite_ptr, imported_room::max_decoration_count> imported_object_sprites;
    bn::vector<bn::sprite_ptr, composition_rooms::max_decoration_count> composition_object_sprites;
    int imported_chest_sprite_index = -1;
    int imported_chest_frame = 0;
    int imported_chest_target_frame = 0;
    int imported_chest_animation_timer = 0;
    constexpr int campfire_column_count = 4;
    bn::vector<bn::sprite_ptr, campfire_column_count> campfire_columns;
    bn::sprite_palette_ptr campfire_column_palette =
        bn::sprite_palette_ptr::create_new(bn::sprite_items::column_16x32.palette_item());
    constexpr int campfire_library_count = 4;
    bn::vector<bn::sprite_ptr, campfire_library_count> campfire_libraries;
    bn::sprite_palette_ptr campfire_library_front_palette =
        bn::sprite_palette_ptr::create_new(bn::sprite_items::librery_front_16x32.palette_item());
    bn::sprite_palette_ptr campfire_library_back_palette =
        bn::sprite_palette_ptr::create_new(bn::sprite_items::librery_back_16x32.palette_item());
    // Distribucion segun la reticula de 16x16: cada sprite ocupa dos filas.
    // Fila superior: librerias front; fila inferior: librerias back.
    constexpr int campfire_column_positions[campfire_column_count][2] =
        {{-80, -48}, {80, -48}, {-80, 32}, {80, 32}};
    constexpr int stairs_2_column_count = 2;
    constexpr int stairs_2_column_positions[stairs_2_column_count][2] =
        {{-80, -48}, {80, -48}};
    constexpr int campfire_library_positions[campfire_library_count][2] =
        {{-64, -48}, {64, -48}, {-64, 32}, {64, 32}};
    constexpr int composition_4_box_count = 6;
    constexpr int composition_4_box_positions[composition_4_box_count][2] =
        {{-96, -56}, {96, -56}, {0, -8}, {0, 8}, {-96, 56}, {96, 56}};
    bn::vector<bn::sprite_ptr, composition_4_box_count> composition_4_boxes;
    constexpr int max_room_spikes = 12;
    bn::vector<bn::sprite_ptr, max_room_spikes> spike_sprites;
    BN_DATA_EWRAM_BSS static int spike_counts[RoomManager::max_rooms] = {};
    BN_DATA_EWRAM_BSS static int spike_x[RoomManager::max_rooms][max_room_spikes] = {};
    BN_DATA_EWRAM_BSS static int spike_y[RoomManager::max_rooms][max_room_spikes] = {};
    BN_DATA_EWRAM_BSS static int spike_frames[RoomManager::max_rooms][max_room_spikes] = {};
    BN_DATA_EWRAM_BSS static bool spike_permanent[RoomManager::max_rooms][max_room_spikes] = {};
    BN_DATA_EWRAM_BSS static int spike_cycle_frames[RoomManager::max_rooms] = {};
    constexpr int spike_safe_duration = 300;
    constexpr int spike_transition_duration = 8;
    constexpr int spike_danger_duration = 300;
    constexpr int spike_cycle_duration = spike_safe_duration + spike_transition_duration +
        spike_danger_duration + spike_transition_duration;
    bn::sprite_palette_ptr composition_box_palette =
        bn::sprite_palette_ptr::create_new(bn::sprite_items::box_16x16.palette_item());
    constexpr int max_skeletons = 4;
    static_assert(imported_room::max_enemy_count <= max_skeletons, "Too many imported enemies");
    bn::vector<bn::sprite_ptr, max_skeletons> skeleton_sprites;
    constexpr int max_bats = 4;
    bn::vector<bn::sprite_ptr, max_bats> bat_sprites;
    constexpr int max_mushies = 4;
    bn::vector<bn::sprite_ptr, max_mushies> mushy_sprites;
    // Mantener el estado persistente fuera de la pila de main; el GBA tiene
    // una pila pequeña y estas tablas abarcan todos los cuartos del piso.
    BN_DATA_EWRAM_BSS static int mushy_counts[RoomManager::max_rooms] = {};
    BN_DATA_EWRAM_BSS static int mushy_room_x[RoomManager::max_rooms][max_mushies] = {};
    BN_DATA_EWRAM_BSS static int mushy_room_y[RoomManager::max_rooms][max_mushies] = {};
    BN_DATA_EWRAM_BSS static int mushy_direction_frames[RoomManager::max_rooms][max_mushies] = {};
    BN_DATA_EWRAM_BSS static int mushy_move_axes[RoomManager::max_rooms][max_mushies] = {};
    BN_DATA_EWRAM_BSS static bool mushy_walking[RoomManager::max_rooms][max_mushies] = {};
    BN_DATA_EWRAM_BSS static int mushy_speed_growth_timer[RoomManager::max_rooms] = {};
    BN_DATA_EWRAM_BSS static int mushy_speed_percent[RoomManager::max_rooms] = {};
    int active_mushy_count = 0;
    int mushy_move_timer = 0;
    int mushy_animation_timer = 0;
    int mushy_animation_frame = 1;
    constexpr int mushy_speed_growth_interval = 120;
    // Mushy parte en 0.4 px por cuadro; 375% equivale a 1.5 px por cuadro,
    // que es 1.2 veces la velocidad media al caminar del jugador (1.25 px/cuadro).
    constexpr int mushy_max_speed_percent = 375;
    constexpr int max_plants = 4;
    constexpr int max_plant_bullets = 16;
    constexpr int plant_shot_interval_frames = 180;
    constexpr const auto& plant_positions = composition_rooms::plant_positions;
    bn::vector<bn::sprite_ptr, max_plants> plant_sprites;
    bn::vector<bn::sprite_ptr, max_plant_bullets> plant_bullet_sprites;
    struct PlantBulletState
    {
        bool active = false;
        bn::fixed x = 0;
        bn::fixed y = 0;
        bn::fixed velocity_x = 0;
        bn::fixed velocity_y = 0;
    };
    PlantBulletState plant_bullets[max_plant_bullets];
    BN_DATA_EWRAM_BSS static int plant_shot_cooldowns[RoomManager::max_rooms][max_plants] = {};
    BN_DATA_EWRAM_BSS static int plant_counts[RoomManager::max_rooms] = {};
    BN_DATA_EWRAM_BSS static int plant_position_indices[RoomManager::max_rooms][max_plants] = {};
    int active_plant_count = 0;
    constexpr int plant_fire_frame_duration = 4;
    BN_DATA_EWRAM_BSS static int plant_fire_animation_steps[max_plants] = {};
    BN_DATA_EWRAM_BSS static int plant_fire_animation_timers[max_plants] = {};
    static int plant_displayed_frames[max_plants] = {-1, -1, -1, -1};

    // Solo existen los sprites de las puertas conectadas al cuarto actual.
    bn::vector<bn::sprite_ptr, 4> room_door_sprites;

    // Llaves del inventario: fila 2, columna 2 de la reticula de 16x16.
    constexpr int inventory_key_x = -80;
    constexpr int inventory_key_y = -40;
    constexpr int inventory_key_count_x = -64;
    // Aventureros rescatados: fila 4, columna 2.
    constexpr int inventory_rescued_x = -80;
    constexpr int inventory_rescued_y = -8;
    constexpr int inventory_rescued_count_x = -64;
    bn::sprite_palette_ptr inventory_key_palette =
        bn::sprite_palette_ptr::create_new(bn::sprite_items::key.palette_item());
    bn::sprite_palette_ptr inventory_player_palette =
        bn::sprite_palette_ptr::create_new(bn::sprite_items::player.palette_item());
    bn::optional<bn::sprite_ptr> key_icon;
    key_icon.emplace(bn::sprite_items::key.create_sprite(
        inventory_key_x, inventory_key_y));
    key_icon->set_palette(inventory_key_palette);
    key_icon->set_bg_priority(0);
    key_icon->set_z_order(-13);
    key_icon->set_visible(false);
    bn::sprite_ptr inventory_rescued_icon =
        bn::sprite_items::player.create_sprite(inventory_rescued_x, inventory_rescued_y, 0);
    inventory_rescued_icon.set_palette(inventory_player_palette);
    inventory_rescued_icon.set_bg_priority(0);
    inventory_rescued_icon.set_z_order(-13);
    inventory_rescued_icon.set_visible(false);
    bn::sprite_ptr floor_key_sprite = bn::sprite_items::key.create_sprite(0, 0);
    floor_key_sprite.set_visible(false);
    bn::sprite_ptr special_key_sprite = bn::sprite_items::key.create_sprite(0, 0);
    special_key_sprite.set_visible(false);
    bn::sprite_ptr energy_potion_sprite =
        bn::sprite_items::posion_energy_16x16_animate.create_sprite(0, 0);
    energy_potion_sprite.set_visible(false);
    bn::sprite_ptr sprint_potion_sprite =
        bn::sprite_items::posion_sprint_16x16_animate.create_sprite(0, 0);
    sprint_potion_sprite.set_visible(false);
    struct FloorPotionState
    {
        bool active = false;
        RoomItemType type = RoomItemType::ENERGY_POTION;
        int room_id = -1;
        int x = 0;
        int y = 0;
    };
    FloorPotionState floor_potions[RoomManager::max_rooms] = {};
    struct RewardRoomReward
    {
        bool active = false;
        RoomItemType type = RoomItemType::KEY;
        int x = 0;
        int y = 0;
        int level = 1;
    };
    RewardRoomReward reward_room_rewards[4] = {};
    bn::vector<bn::sprite_ptr, 4> reward_room_reward_sprites;
    int reward_room_replacement_index = -1;
    bool reward_room_ability_rejected_until_leave = false;
    int potion_animation_timer = 0;
    int potion_animation_frame = 0;
    bn::sprite_ptr stairs = bn::sprite_items::stairs.create_sprite(0, 0);
    stairs.set_bg_priority(1);
    stairs.set_position(0, -8);
    stairs.set_visible(false);
    constexpr int campfire_x = composition_rooms::campfire_position.x;
    constexpr int campfire_y = composition_rooms::campfire_position.y;
    // Punto compartido Q/& definido en el JSON de la fogata.
    constexpr int campfire_reward_x = composition_rooms::campfire_reward_position.x;
    constexpr int campfire_reward_y = composition_rooms::campfire_reward_position.y;
    bool campfire_reward_active = false;
    bool campfire_skill_reward_granted = false;
    int campfire_pending_rewards = 0;
    RoomItemType campfire_reward_type = RoomItemType::KEY;
    int campfire_reward_level = 1;
    bn::optional<bn::sprite_ptr> campfire_reward_sprite;
    bn::optional<bn::sprite_ptr> placed_bomb_sprite;
    constexpr const auto& delivered_npc_offsets = composition_rooms::delivered_npc_offsets;
    constexpr int delivered_npc_animation_interval = 180;
    int delivered_npc_animation_timer = 0;
    bool delivered_npc_animation_alt_frame = false;
    bn::sprite_ptr campfire = bn::sprite_items::campfire.create_sprite(campfire_x, campfire_y);
    campfire.set_bg_priority(1);
    campfire.set_visible(false);
    int campfire_frame = 0;
    int campfire_animation_timer = 0;
    bool campfire_used_this_floor = false;

    // Cada barra vertical se arma con tres cuadros de 16x16 y usa su propio
    // sprite liquido.
    constexpr int stamina_bar_segments = 3;
    bn::vector<bn::sprite_ptr, stamina_bar_segments> stamina_bar_background_sprites;
    bn::vector<bn::sprite_ptr, stamina_bar_segments> stamina_bar_liquid_sprites;
    bn::vector<bn::sprite_ptr, stamina_bar_segments> energy_bar_background_sprites;
    bn::vector<bn::sprite_ptr, stamina_bar_segments> energy_bar_liquid_sprites;
    const int stamina_segment_width = bn::sprite_items::ui_stamina_bar.shape_size().width();
    const int stamina_segment_height = bn::sprite_items::ui_stamina_bar.shape_size().height();
    const int stamina_bar_width = stamina_segment_width;
    const int stamina_bar_height = stamina_segment_height * stamina_bar_segments;
    constexpr int status_layout_gap = 4;
    constexpr int status_layout_margin = 0;
    const int energy_bar_x = -bn::display::width() / 2 + status_layout_margin + stamina_bar_width / 2;
    const int stamina_bar_x = bn::display::width() / 2 - status_layout_margin - stamina_bar_width / 2;
    const int stamina_bar_top_y = bn::display::height() / 2 - stamina_bar_height - 2;
    for (int index = 0; index < stamina_bar_segments; ++index)
    {
        int segment_y = stamina_bar_top_y + stamina_segment_height / 2 + index * stamina_segment_height;
        stamina_bar_background_sprites.push_back(
            bn::sprite_items::ui_stamina_bar.create_sprite(stamina_bar_x, segment_y, index));
        stamina_bar_background_sprites[index].set_bg_priority(0);
        stamina_bar_background_sprites[index].set_z_order(-12);
        stamina_bar_background_sprites[index].set_visible(false);

        stamina_bar_liquid_sprites.push_back(
            bn::sprite_items::ui_sastamina_bar_liquid.create_sprite(stamina_bar_x, segment_y, index));
        stamina_bar_liquid_sprites[index].set_bg_priority(0);
        stamina_bar_liquid_sprites[index].set_z_order(-13);
        stamina_bar_liquid_sprites[index].set_visible(false);

        energy_bar_background_sprites.push_back(
            bn::sprite_items::ui_stamina_bar.create_sprite(energy_bar_x, segment_y, index));
        energy_bar_background_sprites[index].set_bg_priority(0);
        energy_bar_background_sprites[index].set_z_order(-12);
        energy_bar_background_sprites[index].set_visible(false);

        energy_bar_liquid_sprites.push_back(
            bn::sprite_items::ui_energy_bar_liquid.create_sprite(energy_bar_x, segment_y, index));
        energy_bar_liquid_sprites[index].set_bg_priority(0);
        energy_bar_liquid_sprites[index].set_z_order(-13);
        energy_bar_liquid_sprites[index].set_visible(false);
    }

    bn::sprite_text_generator menu_text_generator(common::variable_8x16_sprite_font);
    menu_text_generator.set_center_alignment();
    menu_text_generator.set_bg_priority(0);
    menu_text_generator.set_z_order(-10);

    bn::sprite_text_generator stamina_label_generator(common::variable_8x16_sprite_font);
    stamina_label_generator.set_right_alignment();
    stamina_label_generator.set_bg_priority(0);
    stamina_label_generator.set_z_order(-13);
    bn::vector<bn::sprite_ptr, 8> stamina_label_sprites;
    stamina_label_generator.generate(stamina_bar_x - stamina_bar_width / 2 - status_layout_gap,
                                     stamina_bar_top_y + stamina_bar_height - 8,
                                     "SPIRIT", stamina_label_sprites);
    for (bn::sprite_ptr &sprite : stamina_label_sprites)
    {
        sprite.set_visible(false);
    }

    bn::sprite_text_generator energy_label_generator(common::variable_8x16_sprite_font);
    energy_label_generator.set_left_alignment();
    energy_label_generator.set_bg_priority(0);
    energy_label_generator.set_z_order(-13);
    bn::vector<bn::sprite_ptr, 8> energy_bar_label_sprites;
    energy_label_generator.generate(energy_bar_x + stamina_bar_width / 2 + status_layout_gap,
                                    stamina_bar_top_y + stamina_bar_height - 8,
                                    "ENERGY", energy_bar_label_sprites);
    for (bn::sprite_ptr &sprite : energy_bar_label_sprites)
    {
        sprite.set_visible(false);
    }

    bn::sprite_text_generator room_name_generator(common::variable_8x16_sprite_font);
    room_name_generator.set_right_alignment();
    room_name_generator.set_bg_priority(0);
    bn::vector<bn::sprite_ptr, 16> room_name_sprites;

    bool ui_abierta = false;
    bool inventory_open = false;
    bool ability_data_open = false;
    bool options_open = false;
    bool game_started = false;
    constexpr int ability_icon_blink_interval_frames = 5;
    constexpr int ability_icon_blink_times = 3;
    int ability_icon_blink_timer = 0;
    int ability_icon_blink_toggles_remaining = 0;
    bool ability_icon_blink_hidden = false;
    auto start_ability_icon_blink = [&]()
    {
        ability_icon_blink_timer = 0;
        ability_icon_blink_toggles_remaining = ability_icon_blink_times * 2;
        ability_icon_blink_hidden = false;
    };
    auto set_ability_icon = [&](bn::sprite_ptr &sprite, ActiveAbility ability)
    {
        switch (ability)
        {
        case ActiveAbility::DASH:
            sprite.set_tiles(bn::sprite_items::hability_dash_icon.tiles_item());
            sprite.set_palette(bn::sprite_items::hability_dash_icon.palette_item());
            break;
        case ActiveAbility::STOP_TIME:
            sprite.set_tiles(bn::sprite_items::hability_stoptime_icon.tiles_item());
            sprite.set_palette(bn::sprite_items::hability_stoptime_icon.palette_item());
            break;
        case ActiveAbility::TRAPDOOR_BACK:
            sprite.set_tiles(bn::sprite_items::hability_trapdoorback_icon.tiles_item());
            sprite.set_palette(bn::sprite_items::hability_trapdoorback_icon.palette_item());
            break;
        case ActiveAbility::VIEW_ALL_MAP:
            sprite.set_tiles(bn::sprite_items::hability_viewallmap_icon.tiles_item());
            sprite.set_palette(bn::sprite_items::hability_viewallmap_icon.palette_item());
            break;
        case ActiveAbility::MASTER_KEY:
            sprite.set_tiles(bn::sprite_items::hability_masterkey_icon.tiles_item());
            sprite.set_palette(bn::sprite_items::hability_masterkey_icon.palette_item());
            break;
        case ActiveAbility::BOMB:
            sprite.set_tiles(bn::sprite_items::hability_bomb_icon.tiles_item());
            sprite.set_palette(bn::sprite_items::hability_bomb_icon.palette_item());
            break;
        case ActiveAbility::FAMILIAR:
            sprite.set_tiles(bn::sprite_items::hability_familiar_icon.tiles_item());
            sprite.set_palette(bn::sprite_items::hability_familiar_icon.palette_item());
            break;
        default: break;
        }
    };
    auto find_carried_ability = [&](ActiveAbility ability)
    {
        for (int index = 0; index < carried_ability_count; ++index)
        {
            if (carried_abilities[index].ability == ability)
            {
                return index;
            }
        }
        return -1;
    };
    auto current_ability_level = [&]()
    {
        int active_slot = find_carried_ability(active_ability);
        return active_slot >= 0 ? carried_abilities[active_slot].level : 1;
    };
    auto sprint_cost_for_level = [&](ActiveAbility ability, int level)
    {
        int base_cost = 0;
        switch (ability)
        {
        case ActiveAbility::DASH: base_cost = 180; break;
        case ActiveAbility::STOP_TIME: base_cost = 200; break;
        case ActiveAbility::TRAPDOOR_BACK: base_cost = 240; break;
        case ActiveAbility::VIEW_ALL_MAP: base_cost = 400; break;
        case ActiveAbility::MASTER_KEY: base_cost = 300; break;
        case ActiveAbility::FAMILIAR: base_cost = 180; break;
        case ActiveAbility::BOMB: break;
        default: break;
        }
        return base_cost * (110 - 10 * level) / 100;
    };
    auto energy_cost_for_level = [&](ActiveAbility ability, int level)
    {
        if (ability == ActiveAbility::BOMB)
        {
            constexpr int bomb_costs[5] = {3, 2, 2, 1, 1};
            return bomb_costs[level - 1];
        }
        return 0;
    };
    auto update_ability_level_text = [&]()
    {
        ability_level_text_sprites.clear();
        if (carried_ability_count > 0 && selected_ability_slot < carried_ability_count)
        {
            char level_text[2] = {char('0' + carried_abilities[selected_ability_slot].level), '\0'};
            ability_level_text_generator.generate(95, -72, level_text, ability_level_text_sprites);
            for (bn::sprite_ptr &sprite : ability_level_text_sprites)
            {
                sprite.set_visible(ability_level_text_visible && !ability_icon_blink_hidden);
            }
        }
    };
    auto update_active_ability_icon = [&]()
    {
        set_ability_icon(ability_icon, active_ability);
        ability_icon.set_visible(carried_ability_count > 0);
        ability_panel.set_visible(carried_ability_count > 0);
        update_ability_level_text();
    };
    update_active_ability_icon();
    bn::vector<bn::sprite_ptr, 16> key_count_text_sprites;
    bn::sprite_text_generator key_count_text_generator(common::variable_8x16_sprite_font);
    key_count_text_generator.set_left_alignment();
    key_count_text_generator.set_bg_priority(0);
    key_count_text_generator.set_z_order(-13);
    key_count_text_generator.generate(inventory_key_count_x, inventory_key_y, "x0",
                                      key_count_text_sprites);
    for (bn::sprite_ptr &sprite : key_count_text_sprites)
    {
        sprite.set_visible(false);
    }
    int key_count = 0;
    bool hud_sprites_allocated = true;
    char key_count_label[12] = "x0";
    bn::vector<bn::sprite_ptr, 16> rescued_count_text_sprites;
    bn::sprite_text_generator rescued_count_text_generator(common::variable_8x16_sprite_font);
    rescued_count_text_generator.set_left_alignment();
    rescued_count_text_generator.set_bg_priority(0);
    rescued_count_text_generator.set_z_order(-13);
    char rescued_count_label[12] = "x0";
    rescued_count_text_generator.generate(inventory_rescued_count_x, inventory_rescued_y,
                                          rescued_count_label, rescued_count_text_sprites);
    for (bn::sprite_ptr &sprite : rescued_count_text_sprites)
    {
        sprite.set_visible(false);
    }
    auto update_rescued_count_label = [&]()
    {
        int value = rescued_adventurers;
        char digits[10];
        int digit_count = 0;
        do
        {
            digits[digit_count++] = char('0' + value % 10);
            value /= 10;
        } while (value > 0 && digit_count < 10);

        int index = 0;
        rescued_count_label[index++] = 'x';
        while (digit_count > 0)
        {
            rescued_count_label[index++] = digits[--digit_count];
        }
        rescued_count_label[index] = '\0';

        rescued_count_text_sprites.clear();
        rescued_count_text_generator.generate(inventory_rescued_count_x, inventory_rescued_y,
                                              rescued_count_label, rescued_count_text_sprites);
        bool visible = game_started && inventory_open && !options_open;
        for (bn::sprite_ptr &sprite : rescued_count_text_sprites)
        {
            sprite.set_visible(visible);
        }
    };
    auto update_key_count_label = [&]()
    {
        int value = key_count;
        char digits[10];
        int digit_count = 0;
        do
        {
            digits[digit_count++] = char('0' + value % 10);
            value /= 10;
        } while (value > 0 && digit_count < 10);
        int index = 0;
        key_count_label[index++] = 'x';
        while (digit_count > 0)
        {
            key_count_label[index++] = digits[--digit_count];
        }
        key_count_label[index] = '\0';
        if (!hud_sprites_allocated)
        {
            return;
        }
        key_count_text_sprites.clear();
        key_count_text_generator.generate(inventory_key_count_x, inventory_key_y, key_count_label,
                                          key_count_text_sprites);
        bool hud_visible = game_started && inventory_open && !options_open;
        for (bn::sprite_ptr &sprite : key_count_text_sprites)
        {
            sprite.set_visible(hud_visible);
        }
    };

    int piso_actual = 1;
    bn::sprite_text_generator floor_text_generator(common::variable_8x16_sprite_font);
    floor_text_generator.set_left_alignment();
    floor_text_generator.set_bg_priority(0);
    bn::vector<bn::sprite_ptr, 16> floor_text_sprites;
    char floor_label[16];
    create_floor_label(piso_actual, floor_label);
    floor_text_generator.generate(-112, -68, floor_label, floor_text_sprites);
    bn::sprite_palette_ptr secondary_text_palette =
        bn::sprite_palette_ptr::create_new(game::regular_variable_8x16_font.item().palette_item());
    for (int color_index = 1; color_index < secondary_text_palette.colors_count(); ++color_index)
    {
        secondary_text_palette.set_color(color_index, bn::color(8, 7, 5));
    }
    bn::sprite_text_generator map_seed_text_generator(game::regular_variable_8x16_font);
    map_seed_text_generator.set_right_alignment();
    map_seed_text_generator.set_bg_priority(0);
    map_seed_text_generator.set_z_order(-14);
    bn::vector<bn::sprite_ptr, 16> map_seed_sprites;
    char map_seed_label[16];
    auto update_map_seed_label = [&]()
    {
        create_seed_label(run_seed, map_seed_label);
        map_seed_sprites.clear();
        map_seed_text_generator.generate(112, 68, map_seed_label, map_seed_sprites);
        for(bn::sprite_ptr &sprite : map_seed_sprites)
        {
            sprite.set_palette(secondary_text_palette);
            sprite.set_visible(false);
        }
    };
    auto refresh_map_labels_visibility = [&]()
    {
        bool visible = game_started && ui_abierta && !inventory_open && !ability_data_open && !options_open;
        for(bn::sprite_ptr &sprite : map_seed_sprites)
        {
            sprite.set_visible(visible);
        }
        for (bn::sprite_ptr &sprite : floor_text_sprites)
        {
            sprite.set_visible(visible);
        }
        for (bn::sprite_ptr &sprite : room_name_sprites)
        {
            sprite.set_visible(visible);
        }
    };
    refresh_map_labels_visibility();

    constexpr int energy_capacity = 20;
    constexpr int starting_energy = 15;
    int energy = starting_energy;
    constexpr int stamina_capacity_frames = 600;
    constexpr int campfire_use_duration_frames = 90;
    constexpr int ability_flash_duration_frames = 8;
    constexpr int stamina_blink_interval_frames = 6;
    constexpr int stamina_blink_times = 3;
    constexpr int energy_blink_interval_frames = 6;
    int stamina = stamina_capacity_frames;
    int campfire_use_frames_remaining = 0;
    int ability_flash_frames_remaining = 0;
    auto start_ability_flash = [&]()
    {
        ability_flash_frames_remaining = ability_flash_duration_frames;
        start_ability_icon_blink();
    };
    bool stamina_exhaustion_penalty_applied = false;
    bool stamina_run_attempt_latched = false;
    int stamina_blink_timer = 0;
    int stamina_blink_toggles_remaining = 0;
    bool stamina_blink_hidden = false;
    int energy_blink_timer = 0;
    int energy_blink_toggles_remaining = 0;
    bool energy_blink_hidden = false;
    bool stamina_bar_enabled = false;
    bool energy_bar_enabled = false;
    bool stamina_liquid_segment_visible[stamina_bar_segments] = {};
    bool energy_liquid_segment_visible[stamina_bar_segments] = {};
    bn::sprite_tiles_ptr partial_liquid_tiles = bn::sprite_tiles_ptr::allocate(4, bn::bpp_mode::BPP_4);
    bn::span<bn::tile> partial_liquid_vram = *partial_liquid_tiles.vram();
    bn::sprite_tiles_ptr partial_energy_liquid_tiles = bn::sprite_tiles_ptr::allocate(4, bn::bpp_mode::BPP_4);
    bn::span<bn::tile> partial_energy_liquid_vram = *partial_energy_liquid_tiles.vram();
    auto refresh_stamina_bar_visibility = [&]()
    {
        bool visible = stamina_bar_enabled && !stamina_blink_hidden &&
                       !ui_abierta && !inventory_open && !ability_data_open && !options_open;
        for (int index = 0; index < stamina_bar_background_sprites.size(); ++index)
        {
            stamina_bar_background_sprites[index].set_visible(visible);
        }
        for (int index = 0; index < stamina_bar_liquid_sprites.size(); ++index)
        {
            stamina_bar_liquid_sprites[index].set_visible(visible && stamina_liquid_segment_visible[index]);
        }
        for (bn::sprite_ptr &sprite : stamina_label_sprites)
        {
            sprite.set_visible(visible);
        }
    };
    auto refresh_energy_bar_visibility = [&]()
    {
        bool visible = energy_bar_enabled && !energy_blink_hidden &&
                       !ui_abierta && !inventory_open && !ability_data_open && !options_open;
        for (int index = 0; index < energy_bar_background_sprites.size(); ++index)
        {
            energy_bar_background_sprites[index].set_visible(visible);
        }
        for (int index = 0; index < energy_bar_liquid_sprites.size(); ++index)
        {
            energy_bar_liquid_sprites[index].set_visible(
                visible && energy_liquid_segment_visible[index]);
        }
        for (bn::sprite_ptr &sprite : energy_bar_label_sprites)
        {
            sprite.set_visible(visible);
        }
    };
    auto refresh_main_hud_visibility = [&]()
    {
        bool ability_visible = carried_ability_count > 0 && game_started && !ui_abierta &&
            !inventory_open && !ability_data_open && !options_open;
        ability_level_text_visible = ability_visible;
        ability_panel.set_visible(ability_visible);
        ability_icon.set_visible(ability_visible && !ability_icon_blink_hidden);
        for (bn::sprite_ptr &sprite : ability_level_text_sprites)
        {
            sprite.set_visible(ability_visible && !ability_icon_blink_hidden);
        }
        bool visible = game_started && inventory_open && !options_open;
        if (key_icon.has_value())
        {
            key_icon->set_visible(visible);
        }
        for (bn::sprite_ptr &sprite : key_count_text_sprites)
        {
            sprite.set_visible(visible);
        }
        inventory_rescued_icon.set_visible(visible);
        for (bn::sprite_ptr &sprite : rescued_count_text_sprites)
        {
            sprite.set_visible(visible);
        }
        refresh_map_labels_visibility();
        refresh_stamina_bar_visibility();
        refresh_energy_bar_visibility();
    };
    auto update_ability_icon_blink = [&]()
    {
        bool ability_visible = game_started && !ui_abierta && !inventory_open &&
            !ability_data_open && !options_open;
        if (ability_visible && ability_icon_blink_toggles_remaining > 0)
        {
            ++ability_icon_blink_timer;
            if (ability_icon_blink_timer >= ability_icon_blink_interval_frames)
            {
                ability_icon_blink_timer = 0;
                ability_icon_blink_hidden = !ability_icon_blink_hidden;
                --ability_icon_blink_toggles_remaining;
                if (ability_icon_blink_toggles_remaining == 0)
                {
                    ability_icon_blink_hidden = false;
                }
            }
        }
        ability_icon.set_visible(carried_ability_count > 0 && ability_visible && !ability_icon_blink_hidden);
        for (bn::sprite_ptr &sprite : ability_level_text_sprites)
        {
            sprite.set_visible(carried_ability_count > 0 && ability_visible && !ability_icon_blink_hidden);
        }
    };
    auto update_bar_fill = [&](const bn::sprite_item &liquid_item,
                               bn::vector<bn::sprite_ptr, stamina_bar_segments> &liquid_sprites,
                               bool (&liquid_segment_visible)[stamina_bar_segments],
                               bn::sprite_tiles_ptr &partial_tiles,
                               bn::span<bn::tile> &partial_vram,
                               int value, int capacity)
    {
        if (value < 0)
        {
            value = 0;
        }
        else if (value > capacity)
        {
            value = capacity;
        }

        int visible_pixels = value * stamina_bar_height / capacity;
        for (int index = 0; index < stamina_bar_segments; ++index)
        {
            bn::span<const bn::tile> source_tiles =
                liquid_item.tiles_item().graphics_tiles_ref(index);
            int segment_from_bottom = stamina_bar_segments - index - 1;
            int segment_height = visible_pixels - segment_from_bottom * stamina_segment_height;
            if (segment_height < 0)
            {
                segment_height = 0;
            }
            else if (segment_height > stamina_segment_height)
            {
                segment_height = stamina_segment_height;
            }

            liquid_segment_visible[index] = segment_height > 0;
            if (liquid_sprites.size() != stamina_bar_segments)
            {
                continue;
            }

            if (segment_height == stamina_segment_height)
            {
                liquid_sprites[index].set_tiles(liquid_item.tiles_item(), index);
            }
            else if (segment_height > 0)
            {
                for (int tile_index = 0; tile_index < 4; ++tile_index)
                {
                    partial_vram[tile_index] = source_tiles[tile_index];
                }

                int transparent_rows = stamina_segment_height - segment_height;
                for (int row = 0; row < transparent_rows; ++row)
                {
                    int tile_row = row / 8;
                    int tile_local_row = row % 8;
                    partial_vram[tile_row * 2].data[tile_local_row] = 0;
                    partial_vram[tile_row * 2 + 1].data[tile_local_row] = 0;
                }

                liquid_sprites[index].set_tiles(
                    liquid_item.shape_size(), partial_tiles);
            }
        }
    };
    auto update_stamina_bar_fill = [&](int stamina_value)
    {
        update_bar_fill(bn::sprite_items::ui_sastamina_bar_liquid,
                        stamina_bar_liquid_sprites, stamina_liquid_segment_visible,
                        partial_liquid_tiles, partial_liquid_vram,
                        stamina_value, stamina_capacity_frames);
        refresh_stamina_bar_visibility();
    };
    auto update_energy_bar_fill = [&](int energy_value)
    {
        update_bar_fill(bn::sprite_items::ui_energy_bar_liquid,
                        energy_bar_liquid_sprites, energy_liquid_segment_visible,
                        partial_energy_liquid_tiles, partial_energy_liquid_vram,
                        energy_value, energy_capacity);
        refresh_energy_bar_visibility();
    };
    auto start_stamina_rejection_flash = [&]()
    {
        stamina_blink_timer = 0;
        stamina_blink_toggles_remaining = stamina_blink_times * 2;
        stamina_blink_hidden = false;
        refresh_stamina_bar_visibility();
    };
    auto start_stamina_recovery_flash = [&]()
    {
        stamina_blink_timer = 0;
        stamina_blink_toggles_remaining = stamina_blink_times * 2;
        stamina_blink_hidden = false;
        refresh_stamina_bar_visibility();
    };
    auto start_energy_recovery_flash = [&]()
    {
        energy_blink_timer = 0;
        energy_blink_toggles_remaining = stamina_blink_times * 2;
        energy_blink_hidden = false;
        refresh_energy_bar_visibility();
    };

    constexpr int player_blink_interval = 6;
    constexpr int player_blink_count = 4;
    int damage_invulnerability_frames = 0;
    int player_blink_timer = 0;
    int player_blink_toggles_remaining = 0;
    bool player_blink_hidden = false;
    bool enemy_contact_latched = false;
    bool spike_contact_latched = false;
    int spike_contact_room_id = -1;
    BN_DATA_EWRAM_BSS static bool scored_rooms[RoomManager::max_rooms] = {};
    int run_rooms_explored = 0;
    auto record_room_explored = [&]()
    {
        ++run_rooms_explored;
        game_save::Records &records = game_save::records();
        ++records.rooms_explored;
        if(unsigned(run_rooms_explored) > records.best_rooms)
            records.best_rooms = unsigned(run_rooms_explored);
    };
    auto record_floor_started = [&]()
    {
        game_save::Records &records = game_save::records();
        ++records.floors_explored;
        if(unsigned(piso_actual) > records.best_floor)
            records.best_floor = unsigned(piso_actual);
        record_room_explored();
    };
    bn::vector<bn::sprite_ptr, 16> menu_text_sprites;
    menu_text_generator.generate(0, -70, "MAPA", menu_text_sprites);
    for (bn::sprite_ptr &menu_sprite : menu_text_sprites)
    {
        menu_sprite.set_visible(false);
    }
    auto update_overlay_title = [&](const char *title, bool visible)
    {
        menu_text_sprites.clear();
        menu_text_generator.generate(0, -70, title, menu_text_sprites);
        for (bn::sprite_ptr &sprite : menu_text_sprites)
        {
            sprite.set_visible(visible);
        }
    };
    // La pantalla de inventario usa la reticula de tiles de 16x16:
    // fila 1 para el titulo, fila 2 para llaves, fila 4 para rescatados,
    // fila 6 para el encabezado de habilidades y filas 7-8 para los iconos.
    bn::vector<bn::sprite_ptr, 16> inventory_title_sprites;
    bn::vector<bn::sprite_ptr, 16> inventory_abilities_heading_sprites;
    bn::vector<bn::sprite_ptr, 4> inventory_ability_frames;
    bn::vector<bn::sprite_ptr, 4> inventory_ability_icons;
    bn::vector<bn::sprite_ptr, 4> inventory_ability_level_sprites;
    bn::optional<bn::sprite_ptr> ability_data_icon;
    bn::vector<bn::sprite_ptr, 128> ability_data_text_sprites;
    constexpr int inventory_ability_positions[4][2] =
        {{-72, 48}, {-24, 48}, {24, 48}, {72, 48}};
    bn::sprite_text_generator inventory_text_generator(common::variable_8x16_sprite_font);
    inventory_text_generator.set_center_alignment();
    inventory_text_generator.set_bg_priority(0);
    inventory_text_generator.set_z_order(-14);
    bn::sprite_text_generator inventory_ability_level_generator(common::variable_8x16_sprite_font);
    inventory_ability_level_generator.set_center_alignment();
    inventory_ability_level_generator.set_bg_priority(0);
    inventory_ability_level_generator.set_z_order(-18);
    bn::sprite_text_generator ability_data_text_generator(common::variable_8x16_sprite_font);
    ability_data_text_generator.set_center_alignment();
    ability_data_text_generator.set_bg_priority(0);
    ability_data_text_generator.set_z_order(-13);
    bn::sprite_text_generator ability_data_detail_generator(game::regular_variable_8x16_font);
    ability_data_detail_generator.set_center_alignment();
    ability_data_detail_generator.set_bg_priority(0);
    ability_data_detail_generator.set_z_order(-13);
    auto generate_ability_data_detail = [&](int x2, int y2, const char *text)
    {
        int first_sprite = ability_data_text_sprites.size();
        ability_data_detail_generator.generate(x2, y2, text, ability_data_text_sprites);
        for (int index = first_sprite; index < ability_data_text_sprites.size(); ++index)
        {
            ability_data_text_sprites[index].set_palette(secondary_text_palette);
        }
    };
    auto update_inventory_ability_selection = [&]()
    {
        int selected_index = selected_ability_slot;
        for (int index = 0; index < inventory_ability_frames.size(); ++index)
        {
            bn::sprite_ptr &frame_sprite = inventory_ability_frames[index];
            if (index == selected_index)
            {
                frame_sprite.set_tiles(bn::sprite_items::ui_hability_selected.tiles_item());
                frame_sprite.set_palette(bn::sprite_items::ui_hability_selected.palette_item());
            }
            else
            {
                frame_sprite.set_tiles(bn::sprite_items::ui_hability.tiles_item());
                frame_sprite.set_palette(bn::sprite_items::ui_hability.palette_item());
            }
        }
    };
    auto clear_inventory_screen = [&]()
    {
        // Oculta explícitamente los sprites antes de soltar sus handles. Esto evita
        // que los iconos, marcos o textos del inventario sigan visibles durante el
        // cambio a la pantalla de datos.
        for (bn::sprite_ptr &sprite : inventory_title_sprites) sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : inventory_abilities_heading_sprites) sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : inventory_ability_frames) sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : inventory_ability_icons) sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : inventory_ability_level_sprites) sprite.set_visible(false);
        inventory_bg.reset();
        map_bg.reset();
        inventory_title_sprites.clear();
        inventory_abilities_heading_sprites.clear();
        inventory_ability_frames.clear();
        inventory_ability_icons.clear();
        inventory_ability_level_sprites.clear();
        update_overlay_title("", false);
    };
    auto create_inventory_screen = [&]()
    {
        clear_inventory_screen();
        inventory_bg.emplace(bn::regular_bg_items::inventory.create_bg(0, 0));
        inventory_bg->set_priority(0);
        inventory_text_generator.generate(0, -56, "INVENTORY", inventory_title_sprites);
        inventory_text_generator.generate(0, 24, "HABILIDADES", inventory_abilities_heading_sprites);

        constexpr int inventory_frame_z = -16;
        for (int index = 0; index < carried_ability_count; ++index)
        {
            inventory_ability_frames.push_back(
                bn::sprite_items::ui_hability.create_sprite(
                    inventory_ability_positions[index][0], inventory_ability_positions[index][1]));
            inventory_ability_frames[index].set_bg_priority(0);
            inventory_ability_frames[index].set_z_order(inventory_frame_z);
        }

        constexpr int inventory_ability_z = -17;
        for (int index = 0; index < carried_ability_count; ++index)
        {
            inventory_ability_icons.push_back(bn::sprite_items::hability_dash_icon.create_sprite(
                inventory_ability_positions[index][0], inventory_ability_positions[index][1]));
            set_ability_icon(inventory_ability_icons[index], carried_abilities[index].ability);
            char level_text[2] = {char('0' + carried_abilities[index].level), '\0'};
            inventory_ability_level_generator.generate(
                inventory_ability_positions[index][0] - 10,
                inventory_ability_positions[index][1] - 8,
                level_text, inventory_ability_level_sprites);
        }
        for (bn::sprite_ptr &sprite : inventory_ability_icons)
        {
            sprite.set_bg_priority(0);
            sprite.set_z_order(inventory_ability_z);
        }
        update_inventory_ability_selection();
        update_overlay_title("", false);
        update_key_count_label();
        update_rescued_count_label();
    };
    auto clear_ability_data_screen = [&]()
    {
        ability_data_bg.reset();
        ability_data_icon.reset();
        ability_data_text_sprites.clear();
        ability_data_open = false;
    };
    auto create_ability_data_screen = [&]()
    {
        clear_inventory_screen();
        inventory_open = false;
        ability_data_bg.emplace(bn::regular_bg_items::habilidad_data.create_bg(0, 0));
        ability_data_bg->set_priority(0);
        ability_data_open = true;
        refresh_main_hud_visibility();

        ActiveAbility ability = carried_abilities[selected_ability_slot].ability;
        int level = carried_abilities[selected_ability_slot].level;
        constexpr const char *ability_names[7] =
            {"DASH", "DETENER TIEMPO", "TRAMPILLA", "REVELAR MAPA",
             "MASTER KEY", "BOMBA", "FAMILIAR"};
        constexpr const char *ability_descriptions[7][6] =
        {
            {"Aumenta tu", "velocidad al", "cruzar el", "cuarto.", "", ""},
            {"Detiene a los", "enemigos del", "cuarto por", "un rato.", "", ""},
            {"Coloca una", "trampilla", "para volver", "a la sala", "inicial.", ""},
            {"Revela mapa", "entero del", "piso actual.", "", "", ""},
            {"Abre todas", "las puertas", "de la sala", "con Spirit.", "", ""},
            {"Elimina a los", "enemigos del", "cuarto y", "reduce", "energia.", ""},
            {"Persigue al", "enemigo mas", "cercano y lo", "derrota.", "", ""}
        };
        constexpr const char *ability_costs[7][5] =
        {
            {"N1: 30%", "N2: 27%", "N3: 24%", "N4: 21%", "N5: 18%"},
            {"N1: 33.33%", "N2: 30%", "N3: 26.67%", "N4: 23.33%", "N5: 20%"},
            {"N1: 40%", "N2: 36%", "N3: 32%", "N4: 28%", "N5: 24%"},
            {"N1: 66.67%", "N2: 60%", "N3: 53.33%", "N4: 46.67%", "N5: 40%"},
            {"N1: 50%", "N2: 45%", "N3: 40%", "N4: 35%", "N5: 30%"},
            {"N1: 3 EN", "N2: 2 EN", "N3: 2 EN", "N4: 1 EN", "N5: 1 EN"},
            {"N1: 30%", "N2: 27%", "N3: 24%", "N4: 21%", "N5: 18%"}
        };
        int ability_index = int(ability);
        ability_data_icon.emplace(bn::sprite_items::hability_dash_icon.create_sprite(-60, -48));
        set_ability_icon(*ability_data_icon, ability);
        ability_data_icon->set_bg_priority(0);
        ability_data_icon->set_z_order(-12);

        ability_data_text_generator.generate(56, -56, "COST SPIRIT", ability_data_text_sprites);
        ability_data_text_generator.generate(-56, -24, ability_names[ability_index],
                                             ability_data_text_sprites);
        char level_label[] = "Lvl. 1";
        level_label[5] = char('0' + level);
        generate_ability_data_detail(-24, -48, level_label);
        for (int index = 0; index < 6; ++index)
        {
            if (ability_descriptions[ability_index][index][0])
            {
                generate_ability_data_detail(-64, -8 + index * 16,
                    ability_descriptions[ability_index][index]);
            }
        }
        for (int index = 0; index < 5; ++index)
        {
            generate_ability_data_detail(54, -24 + index * 16,
                ability_costs[ability_index][index]);
        }
    };
    auto open_ability_replacement = [&]()
    {
        replacing_ability = true;
        selected_ability_slot = 0;
        active_ability = carried_abilities[selected_ability_slot].ability;
        update_active_ability_icon();
        inventory_open = true;
        pantalla_actual = Pantalla::INVENTARIO;
        ui_abierta = false;
        create_inventory_screen();
        start_ability_icon_blink();
        refresh_main_hud_visibility();
    };

    bn::sprite_text_generator title_text_generator(common::variable_8x16_sprite_font);
    title_text_generator.set_center_alignment();
    title_text_generator.set_bg_priority(0);
    bn::vector<bn::sprite_ptr, 20> title_name_sprites;
    bn::vector<bn::sprite_ptr, 20> title_credit_sprites;
    bn::vector<bn::sprite_ptr, 16> title_seed_sprites;
    bn::vector<bn::sprite_ptr, 20> title_edit_seed_sprites;
    bn::vector<bn::sprite_ptr, 24> title_enter_sprites;
    bn::vector<bn::sprite_ptr, 16> seed_editor_heading_sprites;
    bn::vector<bn::sprite_ptr, 12> seed_editor_static_digits_sprites;
    bn::vector<bn::sprite_ptr, 2> seed_editor_selected_digit_sprites;
    bn::vector<bn::sprite_ptr, 24> seed_editor_move_help_sprites;
    bn::vector<bn::sprite_ptr, 24> seed_editor_change_help_sprites;
    bn::vector<bn::sprite_ptr, 20> seed_editor_accept_help_sprites;
    bn::vector<bn::sprite_ptr, 20> seed_editor_range_sprites;
    bn::vector<bn::sprite_ptr, 24> seed_editor_random_help_sprites;
    bn::vector<bn::sprite_ptr, 12> options_heading_sprites;
    bn::vector<bn::sprite_ptr, 12> options_continue_sprites;
    bn::vector<bn::sprite_ptr, 20> options_reset_sprites;
    bn::vector<bn::sprite_ptr, 16> options_title_sprites;
    bn::vector<bn::sprite_ptr, 2> options_cursor_sprites;
    bn::vector<bn::sprite_ptr, 40> game_over_text_sprites;
    bn::vector<bn::sprite_ptr, 16> floor_transition_text_sprites;
    bool game_over = false;
    bn::fixed title_scale = bn::fixed(1.75);
    bn::fixed credit_scale = bn::fixed(0.75);
    bn::fixed seed_scale = bn::fixed(1.3);
    bn::sprite_affine_mat_ptr title_affine_mat = bn::sprite_affine_mat_ptr::create();
    title_affine_mat.set_scale(title_scale);
    bn::sprite_affine_mat_ptr credit_affine_mat = bn::sprite_affine_mat_ptr::create();
    credit_affine_mat.set_scale(credit_scale);
    bn::sprite_affine_mat_ptr seed_affine_mat = bn::sprite_affine_mat_ptr::create();
    seed_affine_mat.set_scale(seed_scale);
    char title_seed_label[16];
    bool seed_editor_open = false;
    int seed_digits[10] = {};
    int seed_digit_cursor = 0;
    int seed_blink_timer = 0;
    bool seed_digit_visible = true;
    int start_prompt_blink_timer = 0;
    bool start_prompt_visible = true;
    int options_selection = 0;
    auto create_main_title = [&]()
    {
        title_name_sprites.clear();
        title_credit_sprites.clear();
        title_seed_sprites.clear();
        title_edit_seed_sprites.clear();
        title_enter_sprites.clear();

        title_text_generator.generate(0, -46, "ANOTHERDUNGEON", title_name_sprites);
        for (bn::sprite_ptr &sprite : title_name_sprites)
        {
            bn::fixed old_x = sprite.x();
            sprite.set_affine_mat(title_affine_mat);
            sprite.set_x(old_x * title_scale);
        }
        title_text_generator.generate(0, -19, "BY ABDIELALDANA", title_credit_sprites);
        for (bn::sprite_ptr &sprite : title_credit_sprites)
        {
            bn::fixed old_x = sprite.x();
            sprite.set_affine_mat(credit_affine_mat);
            sprite.set_x(old_x * credit_scale);
        }
        create_seed_label(selected_seed, title_seed_label);
        title_text_generator.generate(0, 2, title_seed_label, title_seed_sprites);
        for (bn::sprite_ptr &sprite : title_seed_sprites)
        {
            bn::fixed old_x = sprite.x();
            sprite.set_affine_mat(seed_affine_mat);
            sprite.set_x(old_x * seed_scale);
        }
        title_text_generator.generate(0, 23, "SELECT: EDIT SEED", title_edit_seed_sprites);
        title_text_generator.generate(0, 53, "PRESS START TO ENTER", title_enter_sprites);
        start_prompt_blink_timer = 0;
        start_prompt_visible = true;
    };
    create_main_title();

    bn::vector<bn::sprite_ptr, RoomManager::max_rooms> map_room_sprites;
    bn::vector<bn::sprite_ptr, RoomManager::max_rooms - 1> map_link_sprites;
    bn::vector<bn::sprite_ptr, 1> map_link_current_sprites;
    bn::vector<bn::sprite_ptr, 1> map_link_special_sprites;
    constexpr int map_current_blink_interval = 10;
    int map_current_blink_timer = 0;
    int map_current_blink_toggles_remaining = 0;
    bool map_current_marker_hidden = false;

    // Copias de las paletas para oscurecer el mundo sin alterar la fuente del texto.
    bn::bg_palette_ptr land_palette = land.palette();
    bn::sprite_palette_ptr player_palette = player.palette();
    bn::sprite_palette_ptr stairs_palette = stairs.palette();
    bn::sprite_palette_ptr title_text_palette = title_name_sprites[0].palette();

    auto set_scene_darkness = [&](bn::fixed intensity)
    {
        // Los cálculos de transición con bn::fixed pueden redondear ligeramente
        // por encima de 1; Butano exige que set_fade reciba un valor entre 0 y 1.
        bn::fixed safe_intensity = intensity;
        if (safe_intensity < bn::fixed(0))
        {
            safe_intensity = bn::fixed(0);
        }
        else if (safe_intensity > bn::fixed(1))
        {
            safe_intensity = bn::fixed(1);
        }

        // Cada color dura dos pisos: normal, azul, gris, morado y guindo.
        // El mapa, el inventario y sus elementos de interfaz quedan excluidos.
        int tint_group = ((piso_actual - 1) / 2) % 5;
        bool apply_floor_tint = tint_group != 0 && !ui_abierta && !inventory_open;
        bn::color floor_tint_color = bn::colors::black;
        switch (tint_group)
        {
        case 1: floor_tint_color = bn::colors::blue; break;   // Pisos 3-4
        case 2: floor_tint_color = bn::colors::gray; break;   // Pisos 5-6
        case 3: floor_tint_color = bn::colors::purple; break; // Pisos 7-8
        case 4: floor_tint_color = bn::colors::maroon; break; // Pisos 9-10
        default: break;                                       // Pisos 1-2: normal
        }
        auto set_room_palette_tint = [&](auto palette)
        {
            if (apply_floor_tint)
            {
                const bn::fixed tint_strength = bn::fixed(0.18);
                bn::fixed remaining_color = bn::fixed(1) - safe_intensity;
                bn::fixed combined_intensity = safe_intensity + tint_strength * remaining_color;
                if (combined_intensity > bn::fixed(1))
                {
                    combined_intensity = bn::fixed(1);
                }
                else if (combined_intensity < bn::fixed(0))
                {
                    combined_intensity = bn::fixed(0);
                }
                bn::fixed tint_weight = tint_strength * remaining_color;
                bn::fixed target_weight = combined_intensity == bn::fixed(0) ?
                    bn::fixed(0) : tint_weight / combined_intensity;
                bn::color target_color(
                    int(bn::fixed(floor_tint_color.red()) * target_weight),
                    int(bn::fixed(floor_tint_color.green()) * target_weight),
                    int(bn::fixed(floor_tint_color.blue()) * target_weight));
                palette.set_fade(target_color, combined_intensity);
            }
            else
            {
                palette.set_fade(bn::colors::black, safe_intensity);
            }
        };

        set_room_palette_tint(land_palette);
        if (room2_bg.has_value())
        {
            bn::bg_palette_ptr palette = room2_bg->palette();
            set_room_palette_tint(palette);
        }
        if (room3_bg.has_value())
        {
            bn::bg_palette_ptr palette = room3_bg->palette();
            set_room_palette_tint(palette);
        }
        if (room_special_bg.has_value())
        {
            bn::bg_palette_ptr special_room_palette = room_special_bg->palette();
            set_room_palette_tint(special_room_palette);
        }
        set_room_palette_tint(player_palette);
        set_room_palette_tint(campfire.palette());
        set_room_palette_tint(floor_key_sprite.palette());
        set_room_palette_tint(special_key_sprite.palette());
        set_room_palette_tint(energy_potion_sprite.palette());
        set_room_palette_tint(sprint_potion_sprite.palette());
        set_room_palette_tint(campfire_column_palette);
        set_room_palette_tint(composition_box_palette);
        set_room_palette_tint(campfire_library_front_palette);
        set_room_palette_tint(campfire_library_back_palette);
        set_room_palette_tint(trapdoor_ability_palette);
        set_room_palette_tint(stairs_palette);
        if (campfire_reward_sprite.has_value())
        {
            bn::sprite_palette_ptr reward_palette = campfire_reward_sprite->palette();
            set_room_palette_tint(reward_palette);
        }
        if (placed_bomb_sprite.has_value())
        {
            bn::sprite_palette_ptr bomb_palette = placed_bomb_sprite->palette();
            set_room_palette_tint(bomb_palette);
        }
        for(bn::sprite_ptr &sprite : menu_object_sprites)
        {
            bn::sprite_palette_ptr palette = sprite.palette();
            set_room_palette_tint(palette);
        }
        for(bn::sprite_ptr &sprite : composition_object_sprites)
        {
            bn::sprite_palette_ptr palette = sprite.palette();
            set_room_palette_tint(palette);
        }
        for(bn::sprite_ptr &sprite : imported_object_sprites)
        {
            bn::sprite_palette_ptr palette = sprite.palette();
            set_room_palette_tint(palette);
        }
        for (bn::sprite_ptr &door : room_door_sprites)
        {
            bn::sprite_palette_ptr palette = door.palette();
            set_room_palette_tint(palette);
        }
        for (bn::sprite_ptr &stone : stone_sprites)
        {
            bn::sprite_palette_ptr palette = stone.palette();
            set_room_palette_tint(palette);
        }
        for (bn::sprite_ptr &box : composition_4_boxes)
        {
            bn::sprite_palette_ptr palette = box.palette();
            set_room_palette_tint(palette);
        }
        for (bn::sprite_ptr &column : campfire_columns)
        {
            bn::sprite_palette_ptr palette = column.palette();
            set_room_palette_tint(palette);
        }
        for (bn::sprite_ptr &npc : npc_sprites)
        {
            bn::sprite_palette_ptr palette = npc.palette();
            set_room_palette_tint(palette);
        }
        for (bn::sprite_ptr &skeleton : skeleton_sprites)
        {
            bn::sprite_palette_ptr palette = skeleton.palette();
            set_room_palette_tint(palette);
        }
        for (bn::sprite_ptr &bat : bat_sprites)
        {
            bn::sprite_palette_ptr palette = bat.palette();
            set_room_palette_tint(palette);
        }
        for (bn::sprite_ptr &mushy : mushy_sprites)
        {
            bn::sprite_palette_ptr palette = mushy.palette();
            set_room_palette_tint(palette);
        }
        for (bn::sprite_ptr &spike : spike_sprites)
        {
            bn::sprite_palette_ptr palette = spike.palette();
            set_room_palette_tint(palette);
        }
        for (bn::sprite_ptr &plant : plant_sprites)
        {
            bn::sprite_palette_ptr palette = plant.palette();
            set_room_palette_tint(palette);
        }
        for (bn::sprite_ptr &bullet : plant_bullet_sprites)
        {
            bn::sprite_palette_ptr palette = bullet.palette();
            set_room_palette_tint(palette);
        }
    };

    auto restore_status_bar_colors = [&]()
    {
        auto restore_sprite_colors = [](auto &sprites)
        {
            for (bn::sprite_ptr &sprite : sprites)
            {
                bn::sprite_palette_ptr palette = sprite.palette();
                palette.set_fade(bn::colors::black, 0);
            }
        };
        restore_sprite_colors(stamina_bar_background_sprites);
        restore_sprite_colors(stamina_bar_liquid_sprites);
        restore_sprite_colors(stamina_label_sprites);
        restore_sprite_colors(energy_bar_background_sprites);
        restore_sprite_colors(energy_bar_liquid_sprites);
        restore_sprite_colors(energy_bar_label_sprites);
    };

    auto set_start_transition_darkness = [&](bn::fixed intensity)
    {
        set_scene_darkness(intensity);
        bn::fixed safe_intensity = intensity;
        if (safe_intensity < bn::fixed(0))
        {
            safe_intensity = bn::fixed(0);
        }
        else if (safe_intensity > bn::fixed(1))
        {
            safe_intensity = bn::fixed(1);
        }
        title_text_palette.set_fade(bn::colors::black, safe_intensity);
        secondary_text_palette.set_fade(bn::colors::black, safe_intensity);
    };

    auto set_overlay_blackness = [&](bn::fixed intensity)
    {
        bn::fixed safe_intensity = intensity;
        if (safe_intensity < bn::fixed(0))
        {
            safe_intensity = bn::fixed(0);
        }
        else if (safe_intensity > bn::fixed(1))
        {
            safe_intensity = bn::fixed(1);
        }

        if (map_bg.has_value())
        {
            bn::bg_palette_ptr palette = map_bg->palette();
            palette.set_fade(bn::colors::black, safe_intensity);
        }
        if (inventory_bg.has_value())
        {
            bn::bg_palette_ptr palette = inventory_bg->palette();
            palette.set_fade(bn::colors::black, safe_intensity);
        }
        if (ability_data_bg.has_value())
        {
            bn::bg_palette_ptr palette = ability_data_bg->palette();
            palette.set_fade(bn::colors::black, safe_intensity);
        }

        auto fade_sprites = [&](auto &sprites)
        {
            for (bn::sprite_ptr &sprite : sprites)
            {
                bn::sprite_palette_ptr palette = sprite.palette();
                palette.set_fade(bn::colors::black, safe_intensity);
            }
        };
        fade_sprites(map_room_sprites);
        fade_sprites(map_link_sprites);
        fade_sprites(map_link_current_sprites);
        fade_sprites(map_link_special_sprites);
        fade_sprites(floor_text_sprites);
        fade_sprites(map_seed_sprites);
        fade_sprites(room_name_sprites);
        fade_sprites(menu_text_sprites);
        fade_sprites(inventory_title_sprites);
        fade_sprites(inventory_abilities_heading_sprites);
        fade_sprites(inventory_ability_frames);
        fade_sprites(inventory_ability_icons);
        fade_sprites(inventory_ability_level_sprites);
        fade_sprites(ability_data_text_sprites);
        if (ability_data_icon.has_value())
        {
            bn::sprite_palette_ptr palette = ability_data_icon->palette();
            palette.set_fade(bn::colors::black, safe_intensity);
        }
        fade_sprites(key_count_text_sprites);
        fade_sprites(rescued_count_text_sprites);
        if (key_icon.has_value())
        {
            bn::sprite_palette_ptr palette = key_icon->palette();
            palette.set_fade(bn::colors::black, safe_intensity);
        }
        bn::sprite_palette_ptr rescued_palette = inventory_rescued_icon.palette();
        rescued_palette.set_fade(bn::colors::black, safe_intensity);
    };

    auto set_hud_blackness = [&](bn::fixed intensity)
    {
        bn::fixed safe_intensity = intensity;
        if (safe_intensity < bn::fixed(0))
        {
            safe_intensity = bn::fixed(0);
        }
        else if (safe_intensity > bn::fixed(1))
        {
            safe_intensity = bn::fixed(1);
        }
        auto fade_sprites = [&](auto &sprites)
        {
            for (bn::sprite_ptr &sprite : sprites)
            {
                bn::sprite_palette_ptr palette = sprite.palette();
                palette.set_fade(bn::colors::black, safe_intensity);
            }
        };
        fade_sprites(stamina_bar_background_sprites);
        fade_sprites(stamina_bar_liquid_sprites);
        fade_sprites(stamina_label_sprites);
        fade_sprites(energy_bar_background_sprites);
        fade_sprites(energy_bar_liquid_sprites);
        fade_sprites(energy_bar_label_sprites);
        fade_sprites(ability_level_text_sprites);
        bn::sprite_palette_ptr panel_palette = ability_panel.palette();
        panel_palette.set_fade(bn::colors::black, safe_intensity);
        bn::sprite_palette_ptr ability_palette = ability_icon.palette();
        ability_palette.set_fade(bn::colors::black, safe_intensity);
    };

    auto set_complete_screen_blackness = [&](bn::fixed intensity)
    {
        set_scene_darkness(intensity);
        set_overlay_blackness(intensity);
        set_hud_blackness(intensity);
    };

    auto fade_screen_to_black = [&]()
    {
        for (int step = 1; step <= 12; ++step)
        {
            set_complete_screen_blackness(bn::fixed(step) / 12);
            bn::core::update();
        }
    };

    auto fade_screen_from_black = [&]()
    {
        set_complete_screen_blackness(1);
        bn::core::update();
        for (int step = 11; step >= 0; --step)
        {
            set_complete_screen_blackness(bn::fixed(step) / 12);
            bn::core::update();
        }
        set_complete_screen_blackness(0);
    };

    auto fade_room_transition = [&](bool fade_out)
    {
        if(fade_out)
        {
            reward_lever_sprites.clear();
            reward_lever_sprite_index = -1;
        }
        bn::fixed base_darkness = ui_abierta ? 0.8 : 0;
        for (int step = 1; step <= 12; ++step)
        {
            bn::fixed progress = step / 12.0;
            bn::fixed darkness = fade_out ? base_darkness + (1 - base_darkness) * progress : 1 - (1 - base_darkness) * progress;
            set_scene_darkness(darkness);
            // Mantener las barras visibles y con sus colores durante todo el fade;
            // algunas paletas del mundo pueden estar compartidas con el HUD.
            restore_status_bar_colors();
            refresh_stamina_bar_visibility();
            refresh_energy_bar_visibility();
            bn::fixed text_darkness = fade_out ? progress : 1 - progress;
            title_text_palette.set_fade(bn::colors::black, text_darkness);
            bn::core::update();
        }
        if (! fade_out)
        {
            update_stamina_bar_fill(stamina);
            update_energy_bar_fill(energy);
        }
    };

    auto update_door_sprites = [&]()
    {
        room_door_sprites.clear();
        const RoomDirection directions[] =
            {
                RoomDirection::LEFT, RoomDirection::RIGHT,
                RoomDirection::UP, RoomDirection::DOWN};
        const int door_x[] = {-112, 111, 0, 0};
        const int door_y[] = {0, -1, -72, 71};
        const int door_angles[] = {0, 180, 270, 90};
        for (int index = 0; index < 4; ++index)
        {
            const imported_room::Object *authored_door = imported_room::contains(room_manager.current_template()) ?
                current_imported_room().door(directions[index]) : nullptr;
            const composition_rooms::Room* composition = current_composition_room();
            const composition_rooms::Door* native_door = composition ? &composition->doors[index] : nullptr;
            bool exists = room_manager.has_door(directions[index]) || authored_door ||
                (native_door && native_door->rule == 2);
            if (!exists || !room_manager.door_is_visible(
                    room_manager.current_room_id(), directions[index]))
            {
                continue;
            }

            bool open = room_manager.door_is_open(directions[index]);
            bool special = room_manager.door_is_special(directions[index]);
            bn::sprite_ptr door = special ?
                (open ? bn::sprite_items::door_special.create_sprite(door_x[index], door_y[index]) :
                        bn::sprite_items::close_door_special.create_sprite(door_x[index], door_y[index])) :
                (open ? bn::sprite_items::door.create_sprite(door_x[index], door_y[index]) :
                        bn::sprite_items::close_door.create_sprite(door_x[index], door_y[index]));
            if(authored_door)
            {
                door.set_position(authored_door->x, authored_door->y);
                door.set_horizontal_flip(authored_door->flip_x);
            }
            if(native_door && native_door->rule)
                door.set_position(native_door->x, native_door->y);
            door.set_rotation_angle(authored_door ? authored_door->rotation :
                (native_door && native_door->rule ? native_door->rotation : door_angles[index]));
            door.set_bg_priority(1);
            room_door_sprites.push_back(door);
        }
    };

    auto update_room_background = [&]()
    {
        RoomBackgroundId background_id = room_manager.room_background(room_manager.current_room_id());
        room2_bg.reset();
        room3_bg.reset();
        room_special_bg.reset();
        reward_room_bg.reset();
        land.set_visible(background_id == RoomBackgroundId::LAND);
        if (background_id == RoomBackgroundId::ROOM2)
        {
            room2_bg.emplace(bn::regular_bg_items::room2.create_bg(0, 0));
            room2_bg->set_priority(3);
        }
        else if (background_id == RoomBackgroundId::ROOM3)
        {
            room3_bg.emplace(bn::regular_bg_items::room3.create_bg(0, 0));
            room3_bg->set_priority(3);
        }
        else if (background_id == RoomBackgroundId::MENU_START)
        {
            room_special_bg.emplace(bn::regular_bg_items::menu_inicio.create_bg(0, 0));
            room_special_bg->set_priority(3);
        }
        else if (background_id == RoomBackgroundId::TUTORIAL)
        {
            room_special_bg.emplace(bn::regular_bg_items::room_tutorial.create_bg(0, 0));
            room_special_bg->set_priority(3);
        }
        else if (background_id == RoomBackgroundId::CAMPFIRE)
        {
            room_special_bg.emplace(bn::regular_bg_items::room_campfire.create_bg(0, 0));
            room_special_bg->set_priority(3);
        }
        else if (background_id == RoomBackgroundId::BOSS_ROOM)
        {
            reward_room_bg.emplace(bn::regular_bg_items::boss_1.create_bg(0, 0));
            reward_room_bg->set_priority(3);
        }

        // El fondo nuevo nace durante el fade a negro; aplicar la paleta aqui
        // evita que el piso 2 aparezca con sus colores originales antes del fade in.
        set_scene_darkness(1);
    };

    constexpr int stone_layout_1[4][2] =
        {
            {-32, -32}, {0, -32}, {32, -32}, {0, 32}};
    constexpr int stone_layout_2[4][2] =
        {
            {-32, -16}, {32, -16}, {-32, 16}, {32, 16}};
    constexpr int stone_layout_3[5][2] =
        {
            {0, 0}, {-16, 0}, {16, 0}, {0, -16}, {0, 16}};
    constexpr int stone_layout_4[5][2] =
        {
            {-80, -48}, {80, -48}, {-80, 48}, {80, 48}, {0, 0}};
    constexpr int stone_layout_5[12][2] =
        {
            {-80, -48}, {-64, -48}, {-80, -32}, {80, -48}, {64, -48}, {80, -32}, {-80, 48}, {-64, 48}, {-80, 32}, {80, 48}, {64, 48}, {80, 32}};
    constexpr int composition_1_stones[18][2] =
        {{-80,-40},{-64,-40},{64,-40},{80,-40},{-80,-24},{80,-24},
         {-16,-8},{0,-8},{16,-8},{-16,8},{0,8},{16,8},
         {-80,24},{80,24},
         {-80,40},{-64,40},{64,40},{80,40}};
    constexpr int composition_2_stones[14][2] =
        {{0,-40},{-32,-24},{-16,-24},{0,-24},{16,-24},{32,-24},{-16,-8},{16,-8},
         {-32,8},{-16,8},{0,8},{16,8},{32,8},{0,24}};
    constexpr int composition_3_stones[26][2] =
        {{-80,-40},{-64,-40},{-48,-40},{-32,-40},{-16,-40},{0,-40},{16,-40},{32,-40},{48,-40},{64,-40},{80,-40},
         {-80,-24},{80,-24},{-80,24},{80,24},
         {-80,40},{-64,40},{-48,40},{-32,40},{-16,40},{0,40},{16,40},{32,40},{48,40},{64,40},{80,40}};
    constexpr int stairs_2_stones[17][2] =
        {{-32,8},{-16,8},{0,8},{16,8},{32,8},
         {-96,24},{96,24},{-96,40},{-80,40},{80,40},{96,40},
         {-96,56},{-80,56},{-64,56},{64,56},{80,56},{96,56}};
    constexpr int plant_garden_stones[18][2] =
        {{-96,-56},{-80,-56},{80,-56},{96,-56}, {-96,-40},{96,-40},
         {-16,-8},{0,-8},{16,-8}, {-16,8},{0,8},{16,8},
         {-96,40},{96,40}, {-96,56},{-80,56},{80,56},{96,56}};
    constexpr int plant_center_stones[14][2] =
        {{0,-40}, {-32,-24},{-16,-24},{0,-24},{16,-24},{32,-24},
         {-16,-8},{16,-8}, {-32,8},{-16,8},{0,8},{16,8},{32,8}, {0,24}};
    constexpr int skeleton_min_x = -92;
    constexpr int skeleton_max_x = 92;
    constexpr int skeleton_min_y = -52;
    constexpr int skeleton_max_y = 52;
    constexpr int skeleton_path_width = skeleton_max_x - skeleton_min_x;
    constexpr int skeleton_path_height = skeleton_max_y - skeleton_min_y;
    constexpr int skeleton_path_length = 2 * (skeleton_path_width + skeleton_path_height);
    constexpr int skeleton_corner_path_positions[4] =
        {0, skeleton_path_width, skeleton_path_width + skeleton_path_height,
         2 * skeleton_path_width + skeleton_path_height};
    constexpr int bat_min_x = -96;
    constexpr int bat_max_x = 96;
    constexpr int bat_min_y = -56;
    constexpr int bat_max_y = 56;
    BN_DATA_EWRAM_BSS static int bat_counts[RoomManager::max_rooms] = {};
    BN_DATA_EWRAM_BSS static int bat_start_x[RoomManager::max_rooms][max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_start_y[RoomManager::max_rooms][max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_start_dx[RoomManager::max_rooms][max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_start_dy[RoomManager::max_rooms][max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_start_animation_frames[RoomManager::max_rooms][max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_start_animation_timers[RoomManager::max_rooms][max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_room_x[RoomManager::max_rooms][max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_room_y[RoomManager::max_rooms][max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_room_dx[RoomManager::max_rooms][max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_room_dy[RoomManager::max_rooms][max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_room_animation_frames[RoomManager::max_rooms][max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_room_animation_timers[RoomManager::max_rooms][max_bats] = {};
    int active_bat_count = 0;
    BN_DATA_EWRAM_BSS static int bat_x[max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_y[max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_dx[max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_dy[max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_animation_frames[max_bats] = {};
    BN_DATA_EWRAM_BSS static int bat_animation_timers[max_bats] = {};
    static int bat_displayed_frames[max_bats] = {-1, -1, -1, -1};
    int bat_sound_cooldown_frames = 0;
    int skeleton_sound_cooldown_frames = 0;
    BN_DATA_EWRAM_BSS static int skeleton_counts[RoomManager::max_rooms] = {};
    BN_DATA_EWRAM_BSS static int skeleton_start_corners[RoomManager::max_rooms][max_skeletons] = {};
    BN_DATA_EWRAM_BSS static int skeleton_room_path_positions[RoomManager::max_rooms][max_skeletons] = {};
    BN_DATA_EWRAM_BSS static bool skeleton_room_path_initialized[RoomManager::max_rooms][max_skeletons] = {};
    int active_skeleton_count = 0;
    BN_DATA_EWRAM_BSS static int skeleton_path_positions[max_skeletons] = {};
    BN_DATA_EWRAM_BSS static int skeleton_x[max_skeletons] = {};
    BN_DATA_EWRAM_BSS static int skeleton_y[max_skeletons] = {};
    BN_DATA_EWRAM_BSS static int skeleton_walk_frames[max_skeletons] = {};
    BN_DATA_EWRAM_BSS static int skeleton_animation_timers[max_skeletons] = {};
    static int skeleton_displayed_frames[max_skeletons] = {-1, -1, -1, -1};
    auto skeleton_path_coordinates = [&](int path_position, int &position_x, int &position_y)
    {
        path_position %= skeleton_path_length;
        if (path_position < 0)
        {
            path_position += skeleton_path_length;
        }
        if (path_position < skeleton_path_width)
        {
            position_x = skeleton_min_x + path_position;
            position_y = skeleton_min_y;
        }
        else if (path_position < skeleton_path_width + skeleton_path_height)
        {
            position_x = skeleton_max_x;
            position_y = skeleton_min_y + path_position - skeleton_path_width;
        }
        else if (path_position < 2 * skeleton_path_width + skeleton_path_height)
        {
            position_x = skeleton_max_x - (path_position - skeleton_path_width - skeleton_path_height);
            position_y = skeleton_max_y;
        }
        else
        {
            position_x = skeleton_min_x;
            position_y = skeleton_max_y - (path_position - 2 * skeleton_path_width - skeleton_path_height);
        }
    };
    auto set_skeleton_path_position = [&](int skeleton_index, int path_position)
    {
        if(imported_room::contains(room_manager.current_template()))
        {
            int room_id = room_manager.current_room_id();
            const imported_room::Object &source = current_imported_room().enemy(skeleton_start_corners[room_id][skeleton_index]);
            constexpr int patrol_distance = 32;
            constexpr int patrol_length = patrol_distance * 2;
            path_position %= patrol_length;
            if(path_position < 0) path_position += patrol_length;
            int offset = path_position <= patrol_distance ? path_position : patrol_length - path_position;
            int heading = source.frame / 4;
            int step_x = heading == 3 ? 1 : (heading == 2 ? -1 : 0);
            int step_y = heading == 0 ? 1 : (heading == 1 ? -1 : 0);
            skeleton_x[skeleton_index] = source.x + step_x * offset;
            skeleton_y[skeleton_index] = source.y + step_y * offset;
            if(skeleton_x[skeleton_index] < -102) skeleton_x[skeleton_index] = -102;
            if(skeleton_x[skeleton_index] > 102) skeleton_x[skeleton_index] = 102;
            if(skeleton_y[skeleton_index] < -64) skeleton_y[skeleton_index] = -64;
            if(skeleton_y[skeleton_index] > 58) skeleton_y[skeleton_index] = 58;
            skeleton_path_positions[skeleton_index] = path_position;
            skeleton_room_path_positions[room_id][skeleton_index] = path_position;
            skeleton_room_path_initialized[room_id][skeleton_index] = true;
            skeleton_sprites[skeleton_index].set_position(skeleton_x[skeleton_index], skeleton_y[skeleton_index]);
            return;
        }
        const composition_rooms::Room* composition = current_composition_room();
        if(composition && composition->skeletons.size > 0)
        {
            int room_id = room_manager.current_room_id();
            const composition_rooms::Route& route = composition->skeletons[skeleton_start_corners[room_id][skeleton_index]];
            int length = (route.end_x - route.x) + (route.end_y - route.y);
            int period = length * 2;
            path_position %= period;
            if(path_position < 0) path_position += period;
            int offset = path_position <= length ? path_position : period - path_position;
            skeleton_x[skeleton_index] = route.x + (route.end_x != route.x ? offset : 0);
            skeleton_y[skeleton_index] = route.y + (route.end_y != route.y ? offset : 0);
            skeleton_path_positions[skeleton_index] = path_position;
            skeleton_room_path_positions[room_id][skeleton_index] = path_position;
            skeleton_room_path_initialized[room_id][skeleton_index] = true;
            skeleton_sprites[skeleton_index].set_position(skeleton_x[skeleton_index], skeleton_y[skeleton_index]);
            return;
        }
        path_position %= skeleton_path_length;
        if (path_position < 0)
        {
            path_position += skeleton_path_length;
        }
        skeleton_path_positions[skeleton_index] = path_position;
        int room_id = room_manager.current_room_id();
        skeleton_room_path_positions[room_id][skeleton_index] = path_position;
        skeleton_room_path_initialized[room_id][skeleton_index] = true;
        skeleton_path_coordinates(path_position, skeleton_x[skeleton_index], skeleton_y[skeleton_index]);
        skeleton_sprites[skeleton_index].set_position(skeleton_x[skeleton_index], skeleton_y[skeleton_index]);
    };
    auto skeleton_frame_for_path = [&](int skeleton_index)
    {
        if(imported_room::contains(room_manager.current_template()))
        {
            int room_id = room_manager.current_room_id();
            int direction_frame = current_imported_room().enemy(skeleton_start_corners[room_id][skeleton_index]).frame / 4 * 4;
            if(skeleton_path_positions[skeleton_index] >= 32)
                direction_frame = direction_frame == 0 ? 4 : (direction_frame == 4 ? 0 : (direction_frame == 8 ? 12 : 8));
            return direction_frame + skeleton_walk_frames[skeleton_index];
        }
        const composition_rooms::Room* composition = current_composition_room();
        if(composition && composition->skeletons.size > 0)
        {
            int room_id = room_manager.current_room_id();
            const composition_rooms::Route& route = composition->skeletons[skeleton_start_corners[room_id][skeleton_index]];
            int length = (route.end_x - route.x) + (route.end_y - route.y);
            bool forward = skeleton_path_positions[skeleton_index] < length;
            int direction_frame = route.end_x != route.x ? (forward ? 12 : 8) : (forward ? 0 : 4);
            return direction_frame + skeleton_walk_frames[skeleton_index];
        }
        int direction_frame = 0;
        int path_position = skeleton_path_positions[skeleton_index];
        if (path_position < skeleton_path_width)
        {
            direction_frame = 12; // derecha
        }
        else if (path_position < skeleton_path_width + skeleton_path_height)
        {
            direction_frame = 0; // abajo
        }
        else if (path_position < 2 * skeleton_path_width + skeleton_path_height)
        {
            direction_frame = 8; // izquierda
        }
        else
        {
            direction_frame = 4; // arriba
        }
        return direction_frame + skeleton_walk_frames[skeleton_index];
    };
    auto setup_floor_skeletons = [&]()
    {
        for (int room_id = 0; room_id < RoomManager::max_rooms; ++room_id)
        {
            skeleton_counts[room_id] = 0;
            for (int index = 0; index < max_skeletons; ++index)
            {
                skeleton_start_corners[room_id][index] = 0;
                skeleton_room_path_positions[room_id][index] = 0;
                skeleton_room_path_initialized[room_id][index] = false;
            }
        }

        for (int room_id = 0; room_id < room_manager.room_count(); ++room_id)
        {
            if(imported_room::contains(room_manager.room_template(room_id)))
            {
                skeleton_counts[room_id] = imported_room_for(room_id).enemy_count;
                for(int index = 0; index < imported_room_for(room_id).enemy_count; ++index)
                    skeleton_start_corners[room_id][index] = index;
                continue;
            }
            if (room_manager.room_template(room_id) == RoomTemplateId::RUINS ||
                room_manager.room_template(room_id) == RoomTemplateId::COMPOSITION_3 ||
                room_manager.room_template(room_id) == RoomTemplateId::COMPOSITION_4)
            {
                skeleton_counts[room_id] = floor_random.get_int(1, max_skeletons + 1);
                int corners[4] = {0, 1, 2, 3};
                for (int index = 3; index > 0; --index)
                {
                    int swap_index = floor_random.get_int(index + 1);
                    int corner = corners[index];
                    corners[index] = corners[swap_index];
                    corners[swap_index] = corner;
                }

                for (int index = 0; index < skeleton_counts[room_id]; ++index)
                {
                    skeleton_start_corners[room_id][index] = corners[index];
                }
            }
        }
    };
    auto setup_floor_bats = [&]()
    {
        for (int room_id = 0; room_id < RoomManager::max_rooms; ++room_id)
        {
            bat_counts[room_id] = 0;
            for (int index = 0; index < max_bats; ++index)
            {
                bat_room_x[room_id][index] = 0;
                bat_room_y[room_id][index] = 0;
                bat_room_dx[room_id][index] = 0;
                bat_room_dy[room_id][index] = 0;
                bat_room_animation_frames[room_id][index] = 0;
                bat_room_animation_timers[room_id][index] = 0;
                bat_start_x[room_id][index] = 0;
                bat_start_y[room_id][index] = 0;
                bat_start_dx[room_id][index] = 0;
                bat_start_dy[room_id][index] = 0;
                bat_start_animation_frames[room_id][index] = 0;
                bat_start_animation_timers[room_id][index] = 0;
            }
        }

        for (int room_id = 0; room_id < room_manager.room_count(); ++room_id)
        {
            RoomTemplateId room_template = room_manager.room_template(room_id);
            if (room_template != RoomTemplateId::COMPOSITION_1 &&
                room_template != RoomTemplateId::COMPOSITION_2 &&
                room_template != RoomTemplateId::STAIRS && room_template != RoomTemplateId::STONE)
            {
                continue;
            }

            bool final_room = room_template == RoomTemplateId::STAIRS;
            bool room_one = room_template == RoomTemplateId::COMPOSITION_1;
            int route_count = room_template == RoomTemplateId::COMPOSITION_2 ||
                room_template == RoomTemplateId::STONE ? 4 : 2;
            int bat_count = room_template == RoomTemplateId::COMPOSITION_2 ||
                room_template == RoomTemplateId::STONE ? floor_random.get_int(1, 5) :
                floor_random.get_int(1, 3);
            bat_counts[room_id] = bat_count;

            int start_positions[4][2] = {};
            if (final_room)
            {
                start_positions[0][0] = -64; start_positions[0][1] = -40;
                start_positions[1][0] = 64; start_positions[1][1] = 24;
            }
            else if (room_one)
            {
                start_positions[0][0] = 64; start_positions[0][1] = -24;
                start_positions[1][0] = -64; start_positions[1][1] = 24;
            }
            else if (room_template == RoomTemplateId::COMPOSITION_2)
            {
                start_positions[0][0] = -16; start_positions[0][1] = -40;
                start_positions[1][0] = -32; start_positions[1][1] = -8;
                start_positions[2][0] = 32; start_positions[2][1] = -8;
                start_positions[3][0] = 16; start_positions[3][1] = 24;
            }
            else
            {
                start_positions[0][0] = -96; start_positions[0][1] = -56;
                start_positions[1][0] = 96; start_positions[1][1] = -56;
                start_positions[2][0] = -96; start_positions[2][1] = 56;
                start_positions[3][0] = 96; start_positions[3][1] = 56;
            }

            if(const composition_rooms::Room* source = composition_room_for(room_id))
                for(int slot = 0; slot < route_count; ++slot)
                {
                    start_positions[slot][0] = source->bats[slot].x;
                    start_positions[slot][1] = source->bats[slot].y;
                }
            int slots[4] = {0, 1, 2, 3};
            for (int index = route_count - 1; index > 0; --index)
            {
                int swap_index = floor_random.get_int(index + 1);
                int temp = slots[index];
                slots[index] = slots[swap_index];
                slots[swap_index] = temp;
            }

            for (int index = 0; index < bat_count; ++index)
            {
                int slot = slots[index];
                bat_start_x[room_id][index] = start_positions[slot][0];
                bat_start_y[room_id][index] = start_positions[slot][1];
                bat_start_dx[room_id][index] = floor_random.get_int(2) == 0 ? -1 : 1;
                bat_start_dy[room_id][index] = floor_random.get_int(2) == 0 ? -1 : 1;
            }
            for (int index = 0; index < bat_count; ++index)
            {
                bat_start_animation_frames[room_id][index] = floor_random.get_int(4);
                bat_start_animation_timers[room_id][index] = floor_random.get_int(8);
                bat_room_x[room_id][index] = bat_start_x[room_id][index];
                bat_room_y[room_id][index] = bat_start_y[room_id][index];
                bat_room_dx[room_id][index] = bat_start_dx[room_id][index];
                bat_room_dy[room_id][index] = bat_start_dy[room_id][index];
                bat_room_animation_frames[room_id][index] = bat_start_animation_frames[room_id][index];
                bat_room_animation_timers[room_id][index] = bat_start_animation_timers[room_id][index];
            }
        }
    };
    auto setup_floor_mushies = [&]()
    {
        for (int room_id = 0; room_id < RoomManager::max_rooms; ++room_id)
        {
            mushy_counts[room_id] = 0;
            mushy_speed_growth_timer[room_id] = 0;
            mushy_speed_percent[room_id] = 100;
            for (int index = 0; index < max_mushies; ++index)
            {
                mushy_room_x[room_id][index] = 0;
                mushy_room_y[room_id][index] = 0;
                mushy_direction_frames[room_id][index] = 0;
                mushy_move_axes[room_id][index] = 0;
                mushy_walking[room_id][index] = false;
            }
        }

        for (int room_id = 0; room_id < room_manager.room_count(); ++room_id)
        {
            RoomTemplateId room_template = room_manager.room_template(room_id);
            int spawn_positions[4][2] = {};
            int spawn_position_count = 0;
            if (room_template == RoomTemplateId::COMPOSITION_1_2)
            {
                spawn_positions[0][0] = 64; spawn_positions[0][1] = -24;
                spawn_positions[1][0] = -64; spawn_positions[1][1] = 24;
                spawn_position_count = 2;
            }
            else if (room_template == RoomTemplateId::COMPOSITION_2_2)
            {
                spawn_positions[0][0] = -96; spawn_positions[0][1] = -56;
                spawn_positions[1][0] = 96; spawn_positions[1][1] = -56;
                spawn_positions[2][0] = -96; spawn_positions[2][1] = 56;
                spawn_positions[3][0] = 96; spawn_positions[3][1] = 56;
                spawn_position_count = 4;
            }

            if(const composition_rooms::Room* source = composition_room_for(room_id))
                for(int slot = 0; slot < spawn_position_count; ++slot)
                {
                    spawn_positions[slot][0] = source->mushies[slot].x;
                    spawn_positions[slot][1] = source->mushies[slot].y;
                }
            if (spawn_position_count > 0)
            {
                int order[max_mushies] = {0, 1, 2, 3};
                for (int index = spawn_position_count - 1; index > 0; --index)
                {
                    int swap_index = floor_random.get_int(index + 1);
                    int slot = order[index];
                    order[index] = order[swap_index];
                    order[swap_index] = slot;
                }
                int mushy_count = floor_random.get_int(1, spawn_position_count + 1);
                mushy_counts[room_id] = mushy_count;
                for (int index = 0; index < mushy_count; ++index)
                {
                    int spawn_index = order[index];
                    mushy_room_x[room_id][index] = spawn_positions[spawn_index][0];
                    mushy_room_y[room_id][index] = spawn_positions[spawn_index][1];
                }
            }
        }
    };
    auto setup_floor_plants = [&]()
    {
        for (int room_id = 0; room_id < RoomManager::max_rooms; ++room_id)
        {
            plant_counts[room_id] = 0;
            for (int index = 0; index < max_plants; ++index)
            {
                plant_shot_cooldowns[room_id][index] = 0;
                plant_position_indices[room_id][index] = index;
            }
        }

        for (int room_id = 0; room_id < room_manager.room_count(); ++room_id)
        {
            RoomTemplateId room_template = room_manager.room_template(room_id);
            if (room_template == RoomTemplateId::PLANT_GARDEN)
            {
                // La semilla determina si este cuarto tiene de una a cuatro plantas
                // y en qué esquinas aparecen.
                int plant_count = floor_random.get_int(1, max_plants + 1);
                plant_counts[room_id] = plant_count;
                int position_order[max_plants] = {0, 1, 2, 3};
                for (int index = max_plants - 1; index > 0; --index)
                {
                    int swap_index = floor_random.get_int(index + 1);
                    int position = position_order[index];
                    position_order[index] = position_order[swap_index];
                    position_order[swap_index] = position;
                }

                for (int index = 0; index < plant_count; ++index)
                {
                    // La semilla separa los primeros disparos para que no salgan juntos.
                    plant_shot_cooldowns[room_id][index] = floor_random.get_int(
                        plant_shot_interval_frames);
                    plant_position_indices[room_id][index] = position_order[index];
                }
            }
            else if (room_template == RoomTemplateId::PLANT_CENTER_ROOM)
            {
                plant_counts[room_id] = 1;
                plant_position_indices[room_id][0] = max_plants;
                plant_shot_cooldowns[room_id][0] = floor_random.get_int(
                    plant_shot_interval_frames);
            }
            else if (room_template == RoomTemplateId::STAIRS_2)
            {
                int plant_count = floor_random.get_int(1, 3);
                plant_counts[room_id] = plant_count;
                int position_order[2] = {max_plants + 1, max_plants + 2};
                if (floor_random.get_int(2) == 0)
                {
                    int position = position_order[0];
                    position_order[0] = position_order[1];
                    position_order[1] = position;
                }
                for (int index = 0; index < plant_count; ++index)
                {
                    plant_position_indices[room_id][index] = position_order[index];
                    plant_shot_cooldowns[room_id][index] = floor_random.get_int(
                        plant_shot_interval_frames);
                }
            }
        }
    };
    auto setup_floor_spikes = [&]()
    {
        for (int room_id = 0; room_id < RoomManager::max_rooms; ++room_id)
        {
            spike_counts[room_id] = 0;
            spike_cycle_frames[room_id] = 0;
            for (int index = 0; index < max_room_spikes; ++index)
            {
                spike_x[room_id][index] = 0;
                spike_y[room_id][index] = 0;
                spike_frames[room_id][index] = 0;
                spike_permanent[room_id][index] = false;
            }
        }

        for (int room_id = 0; room_id < room_manager.room_count(); ++room_id)
        {
            const composition_rooms::Room* source = composition_room_for(room_id);
            if(!source || source->spikes.size == 0)
                continue;
            const composition_rooms::Spike* placements = source->spikes.data;
            int placement_count = source->spikes.size;
            spike_counts[room_id] = placement_count;
            for (int index = 0; index < placement_count; ++index)
            {
                spike_x[room_id][index] = placements[index].x;
                spike_y[room_id][index] = placements[index].y;
                spike_permanent[room_id][index] = placements[index].always_active;
                spike_frames[room_id][index] = placements[index].always_active ? 2 : 0;
            }
        }
    };
    int active_stone_count = 0;
    BN_DATA_EWRAM_BSS static int active_stone_x[26] = {};
    static_assert(composition_rooms::max_stones <= 26, "Too many composition stones");
    BN_DATA_EWRAM_BSS static int active_stone_y[26] = {};
    auto create_composition_objects = [&]()
    {
        const composition_rooms::Room* source = current_composition_room();
        if(!source || source->menu) return;
        for(const composition_rooms::Object& object : source->objects)
        {
            if(object.type == 'p' || object.type == 'c' || object.type == 'e') continue;
            const bn::sprite_item* item = nullptr;
            switch(object.type)
            {
            case 'x': item = &bn::sprite_items::column_16x32; break;
            case 'f': item = &bn::sprite_items::librery_front_16x32; break;
            case 'b': item = &bn::sprite_items::librery_back_16x32; break;
            case 'o': item = &bn::sprite_items::box_16x16; break;
            case 'v': item = &bn::sprite_items::jarron_16x16; break;
            case 't': item = &bn::sprite_items::cartel_16x16; break;
            case 'i': item = &bn::sprite_items::asiento_16x16_4state_down_up_left_right; break;
            case 'n': item = &bn::sprite_items::table_16x32; break;
            case 'w': item = &bn::sprite_items::table_armor_16x32; break;
            case 'g': item = &bn::sprite_items::cheest_16x16_animate_3frames; break;
            case 'z': item = &bn::sprite_items::axe_floor_16x16; break;
            default: break;
            }
            BN_ASSERT(item, "Unsupported composition decoration");
            composition_object_sprites.push_back(item->create_sprite(object.x, object.y, object.frame));
            bn::sprite_ptr& sprite = composition_object_sprites.back();
            sprite.set_bg_priority(1);
            sprite.set_horizontal_flip(object.flip_x);
            if(object.type == 'x') sprite.set_palette(campfire_column_palette);
            else if(object.type == 'f') sprite.set_palette(campfire_library_front_palette);
            else if(object.type == 'b') sprite.set_palette(campfire_library_back_palette);
            else if(object.type == 'o') sprite.set_palette(composition_box_palette);
        }
    };
    auto create_imported_objects = [&]()
    {
        for(const imported_room::Object &object : current_imported_room().objects)
        {
            if(object.role != imported_room::Role::DECORATION)
                continue;
            const bn::sprite_item *item = nullptr;
            switch(object.asset)
            {
            case imported_room::Asset::TABLE: item = &bn::sprite_items::table_16x32; break;
            case imported_room::Asset::LIBRARY_FRONT: item = &bn::sprite_items::librery_front_16x32; break;
            case imported_room::Asset::CHEST: item = &bn::sprite_items::cheest_16x16_animate_3frames; break;
            case imported_room::Asset::TABLE_ARMOR: item = &bn::sprite_items::table_armor_16x32; break;
            case imported_room::Asset::BOX: item = &bn::sprite_items::box_16x16; break;
            case imported_room::Asset::PLAYER: item = &bn::sprite_items::player; break;
            default: break;
            }
            BN_ASSERT(item, "Unsupported imported decoration");
            imported_object_sprites.push_back(item->create_sprite(object.x, object.y, object.frame));
            bn::sprite_ptr &sprite = imported_object_sprites.back();
            sprite.set_bg_priority(1);
            sprite.set_rotation_angle(object.rotation);
            sprite.set_horizontal_flip(object.flip_x);
            if(object.asset == imported_room::Asset::LIBRARY_FRONT)
                sprite.set_palette(campfire_library_front_palette);
            if(object.asset == imported_room::Asset::CHEST)
            {
                imported_chest_sprite_index = imported_object_sprites.size() - 1;
                imported_chest_frame = object.frame;
            }
        }
    };

    auto update_imported_chest = [&]()
    {
        if(imported_chest_sprite_index < 0 || current_imported_room().chest_index < 0)
            return;
        const imported_room::Object &chest = current_imported_room().objects[current_imported_room().chest_index];
        int delta_x = x - chest.x;
        int delta_y = y - chest.y;
        int radius = imported_chest_target_frame == 2 ? 40 : 32;
        int target = delta_x * delta_x + delta_y * delta_y <= radius * radius ? 2 : 0;
        if(target != imported_chest_target_frame)
        {
            imported_chest_target_frame = target;
            imported_chest_animation_timer = 0;
        }
        if(imported_chest_frame == imported_chest_target_frame)
        {
            imported_chest_animation_timer = 0;
            return;
        }
        if(++imported_chest_animation_timer >= 8)
        {
            imported_chest_animation_timer = 0;
            imported_chest_frame += imported_chest_target_frame > imported_chest_frame ? 1 : -1;
            imported_object_sprites[imported_chest_sprite_index].set_tiles(
                bn::sprite_items::cheest_16x16_animate_3frames.tiles_item(), imported_chest_frame);
        }
    };

    auto create_menu_room_objects = [&]()
    {
        for(const menu_room::Object &object : menu_room::objects)
        {
            const bn::sprite_item *item = nullptr;
            switch(object.type)
            {
            case 'x': item = &bn::sprite_items::column_16x32; break;
            case 'f': item = &bn::sprite_items::librery_front_16x32; break;
            case 'b': item = &bn::sprite_items::librery_back_16x32; break;
            case 'v': item = &bn::sprite_items::jarron_16x16; break;
            case 't': item = &bn::sprite_items::cartel_16x16; break;
            case 'i': item = &bn::sprite_items::asiento_16x16_4state_down_up_left_right; break;
            case 'n': item = &bn::sprite_items::table_16x32; break;
            case 'w': item = &bn::sprite_items::table_armor_16x32; break;
            case 'g': item = &bn::sprite_items::cheest_16x16_animate_3frames; break;
            case 'z': item = &bn::sprite_items::axe_floor_16x16; break;
            default: break;
            }
            BN_ASSERT(item, "Invalid menu object");
            menu_object_sprites.push_back(item->create_sprite(object.x, object.y, object.frame));
            bn::sprite_ptr &sprite = menu_object_sprites.back();
            sprite.set_bg_priority(1);
            if(object.type == 'x')
                sprite.set_palette(campfire_column_palette);
            else if(object.type == 'f')
                sprite.set_palette(campfire_library_front_palette);
            else if(object.type == 'b')
                sprite.set_palette(campfire_library_back_palette);
            else if(object.type == 'g')
                menu_chest_sprite_index = menu_object_sprites.size() - 1;
        }
    };

    auto update_menu_chest = [&]()
    {
        if(menu_chest_sprite_index < 0 || menu_object_sprites.empty())
            return;
        const menu_room::Object &chest = menu_room::objects[menu_chest_sprite_index];
        int delta_x = x - chest.x;
        int delta_y = y - chest.y;
        // El margen de salida evita alternar estados al caminar justo en el limite.
        int proximity_radius = menu_chest_target_frame == 2 ? 40 : 32;
        int target_frame = delta_x * delta_x + delta_y * delta_y <=
            proximity_radius * proximity_radius ? 2 : 0;
        if(target_frame != menu_chest_target_frame)
        {
            menu_chest_target_frame = target_frame;
            menu_chest_animation_timer = 0;
        }
        if(menu_chest_frame == menu_chest_target_frame)
        {
            menu_chest_animation_timer = 0;
            return;
        }
        constexpr int transition_frame_duration = 8;
        if(++menu_chest_animation_timer >= transition_frame_duration)
        {
            menu_chest_animation_timer = 0;
            menu_chest_frame += menu_chest_target_frame > menu_chest_frame ? 1 : -1;
            menu_object_sprites[menu_chest_sprite_index].set_tiles(
                bn::sprite_items::cheest_16x16_animate_3frames.tiles_item(), menu_chest_frame);
        }
    };

    auto update_room_obstacles = [&]()
    {
        composition_object_sprites.clear();
        imported_object_sprites.clear();
        imported_chest_sprite_index = -1;
        imported_chest_frame = 0;
        imported_chest_target_frame = 0;
        imported_chest_animation_timer = 0;
        menu_object_sprites.clear();
        menu_chest_sprite_index = -1;
        menu_chest_frame = 0;
        menu_chest_target_frame = 0;
        menu_chest_animation_timer = 0;
        stone_sprites.clear();
        campfire_columns.clear();
        campfire_libraries.clear();
        skeleton_sprites.clear();
        bat_sprites.clear();
        mushy_sprites.clear();
        spike_sprites.clear();
        plant_sprites.clear();
        plant_bullet_sprites.clear();
        composition_4_boxes.clear();
        active_stone_count = 0;
        active_plant_count = 0;
        active_mushy_count = 0;
        mushy_move_timer = 0;
        mushy_animation_timer = 0;
        mushy_animation_frame = 1;
        for (PlantBulletState &bullet : plant_bullets)
        {
            bullet.active = false;
            bullet.x = 0;
            bullet.y = 0;
            bullet.velocity_x = 0;
            bullet.velocity_y = 0;
        }

        const int (*layout)[2] = nullptr;
        int layout_count = 0;
        RoomTemplateId room_template = room_manager.current_template();
        int obstacle_room_id = room_manager.current_room_id();
        if(room_manager.room_background(obstacle_room_id) == RoomBackgroundId::MENU_START)
        {
            active_skeleton_count = 0;
            active_bat_count = 0;
            campfire.set_visible(false);
            create_menu_room_objects();
            return;
        }
        for (int index = 0; index < spike_counts[obstacle_room_id]; ++index)
        {
            spike_sprites.push_back(bn::sprite_items::spikes_16x16_animate.create_sprite(
                spike_x[obstacle_room_id][index], spike_y[obstacle_room_id][index],
                spike_frames[obstacle_room_id][index]));
            spike_sprites[index].set_bg_priority(1);
            spike_sprites[index].set_z_order(0);
            spike_sprites[index].set_visible(true);
        }
        campfire.set_visible(room_template == RoomTemplateId::SPECIAL);
        const composition_rooms::Room* composition = current_composition_room();
        composition_rooms::Point stairs_position = composition ? composition->position('e') : composition_rooms::Point{0, -8};
        stairs.set_position(stairs_position.x, stairs_position.y);
        const int (*active_column_positions)[2] = room_template == RoomTemplateId::STAIRS_2 ?
            stairs_2_column_positions : campfire_column_positions;
        int active_column_count = room_template == RoomTemplateId::STAIRS_2 ?
            stairs_2_column_count : campfire_column_count;
        bool needs_columns = room_template == RoomTemplateId::SPECIAL ||
            room_template == RoomTemplateId::STAIRS ||
            room_template == RoomTemplateId::STAIRS_2 ||
            room_template == RoomTemplateId::COMPOSITION_4;
        if (needs_columns && !composition)
        {
            for (int index = 0; index < active_column_count; ++index)
            {
                campfire_columns.push_back(
                    bn::sprite_items::column_16x32.create_sprite(
                        active_column_positions[index][0], active_column_positions[index][1]));
                campfire_columns[index].set_palette(campfire_column_palette);
                campfire_columns[index].set_bg_priority(1);
            }
        }
        if (room_template == RoomTemplateId::SPECIAL && !composition)
        {
            for (int index = 0; index < campfire_library_count; ++index)
            {
                bn::sprite_ptr library = index < 2 ?
                    bn::sprite_items::librery_front_16x32.create_sprite(
                        campfire_library_positions[index][0], campfire_library_positions[index][1]) :
                    bn::sprite_items::librery_back_16x32.create_sprite(
                        campfire_library_positions[index][0], campfire_library_positions[index][1]);
                library.set_palette(index < 2 ? campfire_library_front_palette :
                                                campfire_library_back_palette);
                campfire_libraries.push_back(library);
                campfire_libraries[index].set_bg_priority(1);
                // Las estanterias de la derecha miran hacia el centro.
                campfire_libraries[index].set_horizontal_flip(index % 2 == 1);
            }
        }
        active_skeleton_count = !bombed_rooms[room_manager.current_room_id()] &&
            (room_template == RoomTemplateId::RUINS ||
                                 room_template == RoomTemplateId::COMPOSITION_3 ||
                                 room_template == RoomTemplateId::COMPOSITION_4 ||
                                 imported_room::contains(room_template)) ?
            skeleton_counts[room_manager.current_room_id()] : 0;
        skeleton_sound_cooldown_frames = active_skeleton_count > 0 ?
            30 + enemy_ambient_random.get_int(0, 121) : 0;
        for (int index = 0; index < active_skeleton_count; ++index)
        {
            skeleton_sprites.push_back(
                bn::sprite_items::skeleton.create_sprite(0, 0));
            skeleton_sprites[index].set_bg_priority(1);
            skeleton_animation_timers[index] = 0;
            skeleton_walk_frames[index] = 1;
            int corner = skeleton_start_corners[room_manager.current_room_id()][index];
            int start_position = corner == 1 ? 128 : (corner == 2 || corner == 3 ? 48 : 0);
            if(imported_room::contains(room_template))
                start_position = 0;
            else if(composition && composition->skeletons.size > 0)
                start_position = composition->skeletons[corner].initial_offset;
            else if (room_template == RoomTemplateId::COMPOSITION_4)
            {
                start_position = corner == 2 ? 80 : (corner == 3 ? 128 : 0);
            }
            else if (room_template == RoomTemplateId::COMPOSITION_3)
            {
                start_position = corner % 2 == 1 ? 48 : 0;
            }
            else
            {
                start_position = skeleton_corner_path_positions[corner];
            }
            int room_id = room_manager.current_room_id();
            if (skeleton_room_path_initialized[room_id][index])
            {
                start_position = skeleton_room_path_positions[room_id][index];
            }
            bool first_spawn = !skeleton_room_path_initialized[room_id][index];
            set_skeleton_path_position(index, start_position);
            if(imported_room::contains(room_template))
                skeleton_sprites[index].set_tiles(bn::sprite_items::skeleton.tiles_item(), first_spawn ?
                    current_imported_room().enemy(corner).frame : skeleton_frame_for_path(index));
            skeleton_displayed_frames[index] = -1;
            skeleton_sprites[index].set_z_order(y < skeleton_y[index] ? -2 : 0);
            skeleton_sprites[index].set_visible(true);
        }
        active_bat_count = !bombed_rooms[room_manager.current_room_id()] &&
            (room_template == RoomTemplateId::STONE ||
                            room_template == RoomTemplateId::COMPOSITION_1 ||
                            room_template == RoomTemplateId::COMPOSITION_2 ||
                            room_template == RoomTemplateId::STAIRS) ?
            bat_counts[room_manager.current_room_id()] : 0;
        bat_sound_cooldown_frames = active_bat_count > 0 ?
            30 + enemy_ambient_random.get_int(0, 121) : 0;
        for (int index = 0; index < active_bat_count; ++index)
        {
            bat_sprites.push_back(bn::sprite_items::bat.create_sprite(0, 0));
            bat_sprites[index].set_bg_priority(1);
            int room_id = room_manager.current_room_id();
            bat_x[index] = bat_room_x[room_id][index];
            bat_y[index] = bat_room_y[room_id][index];
            bat_dx[index] = bat_room_dx[room_id][index];
            bat_dy[index] = bat_room_dy[room_id][index];
            bat_animation_frames[index] = bat_room_animation_frames[room_id][index];
            bat_animation_timers[index] = bat_room_animation_timers[room_id][index];
            int direction_frame = bat_dy[index] > 0 ? 0 : (bat_dy[index] < 0 ? 4 :
                (bat_dx[index] < 0 ? 8 : 12));
            bat_displayed_frames[index] = direction_frame + bat_animation_frames[index];
            bat_sprites[index].set_tiles(bn::sprite_items::bat.tiles_item(),
                                         bat_displayed_frames[index]);
            bat_sprites[index].set_position(bat_x[index], bat_y[index]);
            // z_order menor se dibuja delante; mantenerlos sobre las piedras.
            bat_sprites[index].set_z_order(-3);
        }
        if (room_template == RoomTemplateId::COMPOSITION_1_2 ||
            room_template == RoomTemplateId::COMPOSITION_2_2)
        {
            int room_id = room_manager.current_room_id();
            active_mushy_count = bombed_rooms[room_id] ? 0 : mushy_counts[room_id];
            for (int index = 0; index < active_mushy_count; ++index)
            {
                mushy_sprites.push_back(bn::sprite_items::mushy.create_sprite(
                    mushy_room_x[room_id][index], mushy_room_y[room_id][index]));
                mushy_sprites[index].set_bg_priority(1);
                mushy_sprites[index].set_z_order(-3);
                mushy_sprites[index].set_tiles(bn::sprite_items::mushy.tiles_item(),
                    mushy_direction_frames[room_id][index]);
                mushy_sprites[index].set_visible(true);
            }
        }
        if (room_template == RoomTemplateId::PLANT_GARDEN ||
            room_template == RoomTemplateId::PLANT_CENTER_ROOM ||
            room_template == RoomTemplateId::STAIRS_2)
        {
            int room_id = room_manager.current_room_id();
            active_plant_count = bombed_rooms[room_id] ? 0 : plant_counts[room_id];
            for (int index = 0; index < active_plant_count; ++index)
            {
                int position_index = plant_position_indices[room_id][index];
                plant_sprites.push_back(bn::sprite_items::plant_enemy.create_sprite(
                    plant_positions[position_index][0], plant_positions[position_index][1]));
                plant_sprites[index].set_bg_priority(1);
                plant_sprites[index].set_z_order(-3);
                plant_fire_animation_steps[index] = 0;
                plant_fire_animation_timers[index] = 0;
                plant_displayed_frames[index] = -1;
            }
            for (int index = 0; index < max_plant_bullets; ++index)
            {
                plant_bullet_sprites.push_back(
                    bn::sprite_items::bullet_enemy_plant.create_sprite(0, 0));
                plant_bullet_sprites[index].set_bg_priority(1);
                plant_bullet_sprites[index].set_z_order(-3);
                plant_bullet_sprites[index].set_visible(false);
            }
        }
        if (room_template == RoomTemplateId::FIELD_STONES_1)
        {
            layout = stone_layout_1;
            layout_count = 4;
        }
        else if (room_template == RoomTemplateId::FIELD_STONES_2)
        {
            layout = stone_layout_2;
            layout_count = 4;
        }
        else if (room_template == RoomTemplateId::FIELD_STONES_3)
        {
            layout = stone_layout_3;
            layout_count = 5;
        }
        else if (room_template == RoomTemplateId::FIELD_STONES_4)
        {
            layout = stone_layout_4;
            layout_count = 5;
        }
        else if (room_template == RoomTemplateId::FIELD_STONES_5)
        {
            layout = stone_layout_5;
            layout_count = 12;
        }
        else if (room_template == RoomTemplateId::COMPOSITION_1 ||
                 room_template == RoomTemplateId::COMPOSITION_1_2)
        {
            layout = composition_1_stones;
            layout_count = 18;
        }
        else if (room_template == RoomTemplateId::COMPOSITION_2 ||
                 room_template == RoomTemplateId::COMPOSITION_2_2)
        {
            layout = composition_2_stones;
            layout_count = 14;
        }
        else if (room_template == RoomTemplateId::COMPOSITION_3)
        {
            layout = composition_3_stones;
            layout_count = 26;
        }
        else if (room_template == RoomTemplateId::STAIRS_2)
        {
            layout = stairs_2_stones;
            layout_count = 17;
        }
        else if (room_template == RoomTemplateId::PLANT_GARDEN)
        {
            layout = plant_garden_stones;
            layout_count = 18;
        }
        else if (room_template == RoomTemplateId::PLANT_CENTER_ROOM)
        {
            layout = plant_center_stones;
            layout_count = 14;
        }

        if (!composition && (room_template == RoomTemplateId::SPECIAL || room_template == RoomTemplateId::STAIRS ||
            room_template == RoomTemplateId::STAIRS_2 ||
            room_template == RoomTemplateId::COMPOSITION_4))
        {
            for (int index = 0; index < active_column_count; ++index)
            {
                campfire_columns[index].set_position(active_column_positions[index][0],
                                                     active_column_positions[index][1]);
                campfire_columns[index].set_visible(true);
            }
        }

        if (room_template == RoomTemplateId::COMPOSITION_4 && !composition)
        {
            for (int index = 0; index < composition_4_box_count; ++index)
            {
                composition_4_boxes.push_back(
                    bn::sprite_items::box_16x16.create_sprite(0, 0));
                composition_4_boxes[index].set_palette(composition_box_palette);
                composition_4_boxes[index].set_bg_priority(1);
                composition_4_boxes[index].set_position(composition_4_box_positions[index][0],
                                                        composition_4_box_positions[index][1]);
                composition_4_boxes[index].set_visible(true);
            }
        }

        if(composition)
        {
            active_stone_count = composition->stones.size;
            for(int index = 0; index < active_stone_count; ++index)
            {
                active_stone_x[index] = composition->stones[index].x;
                active_stone_y[index] = composition->stones[index].y;
                stone_sprites.push_back(bn::sprite_items::piedra1.create_sprite(active_stone_x[index], active_stone_y[index]));
                stone_sprites[index].set_bg_priority(1);
            }
            create_composition_objects();
        }
        else if (layout)
        {
            active_stone_count = layout_count;
            for (int index = 0; index < active_stone_count; ++index)
            {
                active_stone_x[index] = layout[index][0];
                active_stone_y[index] = layout[index][1];
                stone_sprites.push_back(bn::sprite_items::piedra1.create_sprite(
                    active_stone_x[index], active_stone_y[index]));
                stone_sprites[index].set_bg_priority(1);
                stone_sprites[index].set_position(active_stone_x[index], active_stone_y[index]);
            }
        }
        if(imported_room::contains(room_template))
            create_imported_objects();
        if(!room_manager.is_menu_floor())
        {
            uint8_t seen = game_save::records().enemies_seen;
            if(active_skeleton_count > 0) seen = uint8_t(seen | 1u);
            if(active_bat_count > 0) seen = uint8_t(seen | 2u);
            if(active_mushy_count > 0) seen = uint8_t(seen | 4u);
            if(active_plant_count > 0) seen = uint8_t(seen | 8u);
            game_save::records().enemies_seen = seen;
        }
    };
    auto update_spikes = [&](int player_x, int player_y)
    {
        int room_id = room_manager.current_room_id();
        if (spike_contact_room_id != room_id)
        {
            spike_contact_room_id = room_id;
            spike_contact_latched = false;
        }
        if (spike_counts[room_id] == 0)
        {
            return false;
        }

        int phase = spike_cycle_frames[room_id];
        bool touching_dangerous_spikes = false;
        for (int index = 0; index < spike_counts[room_id]; ++index)
        {
            int spike_frame;
            if (spike_permanent[room_id][index])
            {
                spike_frame = 2;
            }
            else if (phase < spike_safe_duration)
            {
                spike_frame = 0;
            }
            else if (phase < spike_safe_duration + spike_transition_duration)
            {
                spike_frame = 1;
            }
            else if (phase < spike_safe_duration + spike_transition_duration + spike_danger_duration)
            {
                spike_frame = 2;
            }
            else
            {
                spike_frame = 1;
            }

            if (spike_frames[room_id][index] != spike_frame)
            {
                spike_frames[room_id][index] = spike_frame;
                spike_sprites[index].set_tiles(
                    bn::sprite_items::spikes_16x16_animate.tiles_item(), spike_frame);
            }

            if (spike_frame >= 1)
            {
                int delta_x = player_x - spike_x[room_id][index];
                int delta_y = player_y - spike_y[room_id][index];
                if (game::sprite16_overlaps_player(delta_x, delta_y))
                {
                    touching_dangerous_spikes = true;
                }
            }
        }

        spike_cycle_frames[room_id] = (phase + 1) % spike_cycle_duration;
        return touching_dangerous_spikes;
    };
    auto overlaps_stone = [&](int player_x, int player_y)
    {
        if(current_composition_room()) return false;
        for (int index = 0; index < active_stone_count; ++index)
        {
            int delta_x = player_x - active_stone_x[index];
            // Caja de 10x8 en la mitad superior del sprite de 16x16.
            int delta_y = player_y - active_stone_y[index];
            if (game::sprite16_overlaps_player(delta_x, delta_y))
            {
                return true;
            }
        }
        return false;
    };
    auto overlaps_campfire_furniture = [&](int player_x, int player_y)
    {
        if(current_composition_room()) return false;
        RoomTemplateId room_template = room_manager.current_template();
        if (room_template != RoomTemplateId::SPECIAL && room_template != RoomTemplateId::STAIRS &&
            room_template != RoomTemplateId::STAIRS_2 &&
            room_template != RoomTemplateId::COMPOSITION_4)
        {
            return false;
        }

        // La hitbox mide 12x8 y se pega al borde superior de la mitad inferior
        // del sprite de 16x32: su centro queda 4 px debajo del centro del sprite.
        const int (*column_positions)[2] = room_template == RoomTemplateId::STAIRS_2 ?
            stairs_2_column_positions : campfire_column_positions;
        int column_count = room_template == RoomTemplateId::STAIRS_2 ?
            stairs_2_column_count : campfire_column_count;
        for (int index = 0; index < column_count; ++index)
        {
            int delta_x = player_x - column_positions[index][0];
            int delta_y = player_y - (column_positions[index][1] + 4);
            if (game::sprite_collision_overlaps(delta_x, delta_y, 14, -12, 12))
            {
                return true;
            }
        }

        if (room_template == RoomTemplateId::SPECIAL)
        {
            for (int index = 0; index < campfire_library_count; ++index)
            {
                int delta_x = player_x - campfire_library_positions[index][0];
                int delta_y = player_y - (campfire_library_positions[index][1] + 4);
                if (game::sprite_collision_overlaps(delta_x, delta_y, 14, -12, 12))
                {
                    return true;
                }
            }
        }
        return false;
    };
    auto overlaps_composition_4_box = [&](int player_x, int player_y)
    {
        if(current_composition_room()) return false;
        if (room_manager.current_template() != RoomTemplateId::COMPOSITION_4)
        {
            return false;
        }
        for (int index = 0; index < composition_4_boxes.size(); ++index)
        {
            int delta_x = player_x - composition_4_box_positions[index][0];
            int delta_y = player_y - composition_4_box_positions[index][1];
            if (game::sprite16_overlaps_player(delta_x, delta_y))
            {
                return true;
            }
        }
        return false;
    };
    auto overlaps_skeleton = [&](int skeleton_index, int player_x, int player_y)
    {
        if ((room_manager.current_template() != RoomTemplateId::RUINS &&
             room_manager.current_template() != RoomTemplateId::COMPOSITION_3 &&
             room_manager.current_template() != RoomTemplateId::COMPOSITION_4 &&
             !imported_room::contains(room_manager.current_template())) ||
            skeleton_index >= active_skeleton_count)
        {
            return false;
        }

        int delta_x = player_x - skeleton_x[skeleton_index];
        int delta_y = player_y - skeleton_y[skeleton_index];
        if(imported_room::contains(room_manager.current_template()))
        {
            const imported_room::Object &source = current_imported_room().enemy(
                skeleton_start_corners[room_manager.current_room_id()][skeleton_index]);
            return game::hitboxes_overlap(delta_x, delta_y, game::player_hitbox, source.hitbox);
        }
        // Hitbox del esqueleto: 10x8, centrada arriba de su sprite de 16x16.
        return game::sprite16_overlaps_player(delta_x, delta_y);
    };
    auto overlaps_bat = [&](int bat_index, int player_x, int player_y)
    {
        if ((room_manager.current_template() != RoomTemplateId::STONE &&
             room_manager.current_template() != RoomTemplateId::COMPOSITION_1 &&
             room_manager.current_template() != RoomTemplateId::COMPOSITION_2 &&
             room_manager.current_template() != RoomTemplateId::STAIRS) ||
            bat_index >= active_bat_count)
        {
            return false;
        }

        int delta_x = player_x - bat_x[bat_index];
        int delta_y = player_y - bat_y[bat_index];
        // Hitbox del murcielago: 10x8, centrada arriba de su sprite de 16x16.
        return game::sprite16_overlaps_player(delta_x, delta_y);
    };
    auto update_skeleton = [&]()
    {
        if (room_manager.current_template() != RoomTemplateId::RUINS &&
            room_manager.current_template() != RoomTemplateId::COMPOSITION_3 &&
            room_manager.current_template() != RoomTemplateId::COMPOSITION_4 &&
            !imported_room::contains(room_manager.current_template()))
        {
            for (bn::sprite_ptr &sprite : skeleton_sprites)
            {
                sprite.set_visible(false);
            }
            return;
        }

        for (int index = 0; index < active_skeleton_count; ++index)
        {
            set_skeleton_path_position(index, skeleton_path_positions[index] + 1);
            ++skeleton_animation_timers[index];
            if (skeleton_animation_timers[index] >= 8)
            {
                skeleton_animation_timers[index] = 0;
                skeleton_walk_frames[index] = skeleton_walk_frames[index] % 3 + 1;
            }

            int target_frame = skeleton_frame_for_path(index);
            if (target_frame != skeleton_displayed_frames[index])
            {
                skeleton_sprites[index].set_tiles(bn::sprite_items::skeleton.tiles_item(), target_frame);
                skeleton_displayed_frames[index] = target_frame;
            }

            skeleton_sprites[index].set_z_order(y < skeleton_y[index] ? -2 : 0);
            skeleton_sprites[index].set_visible(true);
        }
    };
    auto update_bats = [&]()
    {
        if (room_manager.current_template() != RoomTemplateId::STONE &&
            room_manager.current_template() != RoomTemplateId::COMPOSITION_1 &&
            room_manager.current_template() != RoomTemplateId::COMPOSITION_2 &&
            room_manager.current_template() != RoomTemplateId::STAIRS)
        {
            for (bn::sprite_ptr &sprite : bat_sprites)
            {
                sprite.set_visible(false);
            }
            return;
        }

        for (int index = 0; index < active_bat_count; ++index)
        {
            // Los murcielagos rebotan en diagonal contra los limites del cuarto.
            auto advance_axis = [](int &position, int &direction, int minimum, int maximum)
            {
                int next_position = position + direction;
                if (next_position >= maximum)
                {
                    position = maximum;
                    direction = -1;
                }
                else if (next_position <= minimum)
                {
                    position = minimum;
                    direction = 1;
                }
                else
                {
                    position = next_position;
                }
            };
            int minimum_x = bat_min_x;
            int maximum_x = bat_max_x;
            int minimum_y = bat_min_y;
            int maximum_y = bat_max_y;
            if (bat_dx[index] != 0)
            {
                advance_axis(bat_x[index], bat_dx[index], minimum_x, maximum_x);
            }
            if (bat_dy[index] != 0)
            {
                advance_axis(bat_y[index], bat_dy[index], minimum_y, maximum_y);
            }

            ++bat_animation_timers[index];
            if (bat_animation_timers[index] >= 6)
            {
                bat_animation_timers[index] = 0;
                bat_animation_frames[index] = (bat_animation_frames[index] + 1) % 4;
            }

            int direction_frame = bat_dy[index] > 0 ? 0 : (bat_dy[index] < 0 ? 4 :
                (bat_dx[index] < 0 ? 8 : 12));
            int target_frame = direction_frame + bat_animation_frames[index];
            if (target_frame != bat_displayed_frames[index])
            {
                bat_sprites[index].set_tiles(bn::sprite_items::bat.tiles_item(), target_frame);
                bat_displayed_frames[index] = target_frame;
            }

            bat_sprites[index].set_position(bat_x[index], bat_y[index]);
            // Los murcielagos deben quedar por encima de cualquier piedra.
            bat_sprites[index].set_z_order(-3);
            bat_sprites[index].set_visible(true);

            int room_id = room_manager.current_room_id();
            bat_room_x[room_id][index] = bat_x[index];
            bat_room_y[room_id][index] = bat_y[index];
            bat_room_dx[room_id][index] = bat_dx[index];
            bat_room_dy[room_id][index] = bat_dy[index];
            bat_room_animation_frames[room_id][index] = bat_animation_frames[index];
            bat_room_animation_timers[room_id][index] = bat_animation_timers[index];
        }
    };
    auto update_mushies = [&]()
    {
        RoomTemplateId room_template = room_manager.current_template();
        if (room_template != RoomTemplateId::COMPOSITION_1_2 &&
            room_template != RoomTemplateId::COMPOSITION_2_2)
        {
            for (bn::sprite_ptr &sprite : mushy_sprites)
            {
                sprite.set_visible(false);
            }
            return;
        }

        int room_id = room_manager.current_room_id();
        if (active_mushy_count > 0 &&
            ++mushy_speed_growth_timer[room_id] >= mushy_speed_growth_interval)
        {
            mushy_speed_growth_timer[room_id] = 0;
            mushy_speed_percent[room_id] = mushy_speed_percent[room_id] * 110 / 100;
            if (mushy_speed_percent[room_id] > mushy_max_speed_percent)
            {
                mushy_speed_percent[room_id] = mushy_max_speed_percent;
            }
        }
        auto can_move_to = [&](int moving_index, int candidate_x, int candidate_y)
        {
            constexpr int mushy_min_x = -96;
            constexpr int mushy_max_x = 96;
            constexpr int mushy_min_y = -56;
            constexpr int mushy_max_y = 56;
            if (candidate_x < mushy_min_x || candidate_x > mushy_max_x ||
                candidate_y < mushy_min_y || candidate_y > mushy_max_y)
            {
                return false;
            }

            for (int stone_index = 0; stone_index < active_stone_count; ++stone_index)
            {
                int delta_x = candidate_x - active_stone_x[stone_index];
                int delta_y = candidate_y - active_stone_y[stone_index];
                if (game::sprites16_overlap(delta_x, delta_y))
                {
                    return false;
                }
            }
            for (int other_index = 0; other_index < active_mushy_count; ++other_index)
            {
                if (other_index == moving_index)
                {
                    continue;
                }
                int delta_x = candidate_x - mushy_room_x[room_id][other_index];
                int delta_y = candidate_y - mushy_room_y[room_id][other_index];
                if (game::sprites16_overlap(delta_x, delta_y))
                {
                    return false;
                }
            }
            return true;
        };
        mushy_move_timer += 2 * mushy_speed_percent[room_id];
        int move_steps_this_frame = mushy_move_timer / 500;
        mushy_move_timer %= 500;
        bool animate_this_frame = ++mushy_animation_timer >= 8;
        if (animate_this_frame)
        {
            mushy_animation_timer = 0;
            mushy_animation_frame = mushy_animation_frame >= 3 ? 1 : mushy_animation_frame + 1;
        }
        for (int index = 0; index < active_mushy_count; ++index)
        {
            for (int move_step = 0; move_step < move_steps_this_frame; ++move_step)
            {
                mushy_walking[room_id][index] = false;
                int dx = x - mushy_room_x[room_id][index];
                int dy = y - mushy_room_y[room_id][index];
                int step_x = dx > 0 ? 1 : (dx < 0 ? -1 : 0);
                int step_y = dy > 0 ? 1 : (dy < 0 ? -1 : 0);
                int candidate_x = mushy_room_x[room_id][index];
                int candidate_y = mushy_room_y[room_id][index];
                int &move_axis = mushy_move_axes[room_id][index];
                if ((move_axis == 1 && step_x == 0) || (move_axis == 2 && step_y == 0))
                {
                    move_axis = 0;
                }
                if (move_axis == 0)
                {
                    int abs_x = dx < 0 ? -dx : dx;
                    int abs_y = dy < 0 ? -dy : dy;
                    move_axis = step_x == 0 ? 2 : (step_y == 0 ? 1 :
                        (abs_x >= abs_y ? 1 : 2));
                }

                if (move_axis == 1 && step_x != 0)
                {
                    if (can_move_to(index, candidate_x + step_x, candidate_y))
                    {
                        mushy_room_x[room_id][index] += step_x;
                        mushy_direction_frames[room_id][index] = step_x > 0 ? 12 : 8;
                        mushy_walking[room_id][index] = true;
                    }
                    else if (step_y != 0 && can_move_to(index, candidate_x, candidate_y + step_y))
                    {
                        move_axis = 2;
                        mushy_room_y[room_id][index] += step_y;
                        mushy_direction_frames[room_id][index] = step_y > 0 ? 0 : 4;
                        mushy_walking[room_id][index] = true;
                    }
                }
                else if (move_axis == 2 && step_y != 0)
                {
                    if (can_move_to(index, candidate_x, candidate_y + step_y))
                    {
                        mushy_room_y[room_id][index] += step_y;
                        mushy_direction_frames[room_id][index] = step_y > 0 ? 0 : 4;
                        mushy_walking[room_id][index] = true;
                    }
                    else if (step_x != 0 && can_move_to(index, candidate_x + step_x, candidate_y))
                    {
                        move_axis = 1;
                        mushy_room_x[room_id][index] += step_x;
                        mushy_direction_frames[room_id][index] = step_x > 0 ? 12 : 8;
                        mushy_walking[room_id][index] = true;
                    }
                }
            }

            int target_frame = mushy_direction_frames[room_id][index] +
                (mushy_walking[room_id][index] ? mushy_animation_frame : 0);
            if (animate_this_frame || move_steps_this_frame > 0)
            {
                mushy_sprites[index].set_tiles(bn::sprite_items::mushy.tiles_item(), target_frame);
            }
            mushy_sprites[index].set_position(mushy_room_x[room_id][index],
                                               mushy_room_y[room_id][index]);
            mushy_sprites[index].set_z_order(-3);
            mushy_sprites[index].set_visible(true);
        }
    };
    auto update_plants_and_bullets = [&]()
    {
        if (room_manager.current_template() != RoomTemplateId::PLANT_GARDEN &&
            room_manager.current_template() != RoomTemplateId::PLANT_CENTER_ROOM &&
            room_manager.current_template() != RoomTemplateId::STAIRS_2)
        {
            return false;
        }

        int room_id = room_manager.current_room_id();
        for (int index = 0; index < active_plant_count; ++index)
        {
            int position_index = plant_position_indices[room_id][index];
            int plant_x = plant_positions[position_index][0];
            int plant_y = plant_positions[position_index][1];
            // La planta mira al jugador en todo momento. Cada dirección ocupa
            // cuatro cuadros: uno quieto y tres de animación de disparo.
            int delta_x = x - plant_x;
            int delta_y = y - plant_y;
            int abs_x = delta_x < 0 ? -delta_x : delta_x;
            int abs_y = delta_y < 0 ? -delta_y : delta_y;
            int direction_frame = abs_x >= abs_y ? (delta_x < 0 ? 8 : 12) :
                (delta_y < 0 ? 4 : 0);

            if(plant_fire_animation_steps[index] > 0)
            {
                ++plant_fire_animation_timers[index];
                if(plant_fire_animation_timers[index] >= plant_fire_frame_duration)
                {
                    plant_fire_animation_timers[index] = 0;
                    ++plant_fire_animation_steps[index];
                    if(plant_fire_animation_steps[index] > 3)
                    {
                        plant_fire_animation_steps[index] = 0;
                    }
                }
            }

            int target_frame = direction_frame + plant_fire_animation_steps[index];
            if(target_frame != plant_displayed_frames[index])
            {
                plant_sprites[index].set_tiles(
                    bn::sprite_items::plant_enemy.tiles_item(), target_frame);
                plant_displayed_frames[index] = target_frame;
            }

            int &cooldown = plant_shot_cooldowns[room_id][index];
            ++cooldown;
            if (cooldown >= plant_shot_interval_frames)
            {
                cooldown = 0;
                int bullet_slot = -1;
                for (int bullet_index = 0; bullet_index < max_plant_bullets; ++bullet_index)
                {
                    if (! plant_bullets[bullet_index].active)
                    {
                        bullet_slot = bullet_index;
                        break;
                    }
                }

                if (bullet_slot >= 0)
                {
                    int normalizer = abs_x > abs_y ? abs_x : abs_y;
                    if (normalizer == 0)
                    {
                        normalizer = 1;
                        delta_y = 1;
                    }

                    PlantBulletState &bullet = plant_bullets[bullet_slot];
                    constexpr bn::fixed bullet_speed = bn::fixed(1.05);
                    bullet.x = bn::fixed(plant_x);
                    bullet.y = bn::fixed(plant_y);
                    bullet.velocity_x = bn::fixed(delta_x) * bullet_speed / normalizer;
                    bullet.velocity_y = bn::fixed(delta_y) * bullet_speed / normalizer;
                    bullet.active = true;
                    plant_bullet_sprites[bullet_slot].set_position(bullet.x, bullet.y);
                    plant_bullet_sprites[bullet_slot].set_visible(true);
                    plant_fire_animation_steps[index] = 1;
                    plant_fire_animation_timers[index] = 0;
                    int firing_frame = direction_frame + 1;
                    plant_sprites[index].set_tiles(
                        bn::sprite_items::plant_enemy.tiles_item(), firing_frame);
                    plant_displayed_frames[index] = firing_frame;
                }
            }
        }

        bool bullet_hit_player = false;
        for (int index = 0; index < max_plant_bullets; ++index)
        {
            PlantBulletState &bullet = plant_bullets[index];
            if (! bullet.active)
            {
                continue;
            }

            bullet.x += bullet.velocity_x;
            bullet.y += bullet.velocity_y;
            if (bullet.x < bn::fixed(-120) || bullet.x > bn::fixed(120) ||
                bullet.y < bn::fixed(-72) || bullet.y > bn::fixed(72))
            {
                bullet.active = false;
                plant_bullet_sprites[index].set_visible(false);
                continue;
            }

            plant_bullet_sprites[index].set_position(bullet.x, bullet.y);
            bn::fixed delta_x = bn::fixed(x) - bullet.x;
            bn::fixed delta_y = bn::fixed(y) - bullet.y;
            // Bullet 8x8 contra jugador 16x16.
            if (game::sprite_collision_overlaps(delta_x, delta_y, 12, -12, 12))
            {
                bullet_hit_player = true;
                bullet.active = false;
                plant_bullet_sprites[index].set_visible(false);
            }
        }
        return bullet_hit_player;
    };
    auto overlaps_campfire = [&](int player_x, int player_y)
    {
        if (room_manager.current_template() != RoomTemplateId::SPECIAL)
        {
            return false;
        }

        // Hitbox de la fogata: 16x16. El personaje tambien mide 16x16.
        int delta_x = player_x - campfire_x;
        int delta_y = player_y - campfire_y;
        return game::sprite_collision_overlaps(delta_x, delta_y, 12, -4, 12);
    };
    auto update_stone_layers = [&]()
    {
        for (int index = 0; index < active_stone_count; ++index)
        {
            // Un z_order menor coloca la piedra delante del personaje.
            stone_sprites[index].set_z_order(y < active_stone_y[index] ? -2 : 0);
        }
        for (int index = 0; index < campfire_columns.size(); ++index)
        {
            int furniture_base_y = campfire_column_positions[index][1] + 8;
            campfire_columns[index].set_z_order(y < furniture_base_y ? -2 : 0);
        }
        for (int index = 0; index < composition_4_boxes.size(); ++index)
        {
            int box_base_y = composition_4_box_positions[index][1] + 8;
            composition_4_boxes[index].set_z_order(y < box_base_y ? -2 : 0);
        }
        for (int index = 0; index < campfire_libraries.size(); ++index)
        {
            int furniture_base_y = campfire_library_positions[index][1] + 8;
            campfire_libraries[index].set_z_order(y < furniture_base_y ? -2 : 0);
        }
        for(int index = 0; index < menu_object_sprites.size(); ++index)
        {
            const menu_room::Object &object = menu_room::objects[index];
            int base_y = object.y + (object.height == 32 ? 8 : 0);
            bool in_front_of_player = object.type != 'i' && y < base_y;
            menu_object_sprites[index].set_z_order(in_front_of_player ? -2 : 0);
        }
        int composition_index = 0;
        if(const composition_rooms::Room* source = current_composition_room())
            for(const composition_rooms::Object& object : source->objects)
            {
                if(object.type == 'p' || object.type == 'c' || object.type == 'e') continue;
                if(composition_index < composition_object_sprites.size())
                    composition_object_sprites[composition_index].set_z_order(
                        object.type != 'i' && y < object.y + object.hitbox.bottom ? -2 : 0);
                ++composition_index;
            }
        int imported_index = 0;
        for(const imported_room::Object &object : current_imported_room().objects)
        {
            if(object.role != imported_room::Role::DECORATION)
                continue;
            if(imported_index < imported_object_sprites.size())
                imported_object_sprites[imported_index].set_z_order(y < object.y + object.hitbox.bottom ? -2 : 0);
            ++imported_index;
        }
        // La fogata se dibuja delante si el personaje esta detras de ella.
        campfire.set_z_order(y < campfire_y ? -2 : 0);
    };
    auto append_follow_position = [&](int player_x, int player_y)
    {
        follow_history_x[follow_history_head] = player_x;
        follow_history_y[follow_history_head] = player_y;
        follow_history_head = (follow_history_head + 1) % follow_history_capacity;
        if (follow_history_count < follow_history_capacity)
        {
            ++follow_history_count;
        }
    };
    auto reset_follow_history = [&]()
    {
        follow_history_head = 0;
        follow_history_count = 0;
        int direction_x = 0;
        int direction_y = 0;
        switch (direccion)
        {
        case Direccion::ABAJO:
            direction_y = 1;
            break;
        case Direccion::ARRIBA:
            direction_y = -1;
            break;
        case Direccion::IZQUIERDA:
            direction_x = -1;
            break;
        case Direccion::DERECHA:
            direction_x = 1;
            break;
        default:
            direction_y = 1;
            break;
        }

        for (int distance = follow_delay; distance >= 0; --distance)
        {
            append_follow_position(x - direction_x * distance, y - direction_y * distance);
        }
    };
    auto reset_follow_history_at_player = [&]()
    {
        follow_history_head = 0;
        follow_history_count = 0;
        for (int index = 0; index <= follow_delay; ++index)
        {
            append_follow_position(x, y);
        }
    };
    auto record_follow_movement = [&](int previous_x, int previous_y, int player_x, int player_y)
    {
        int delta_x = player_x - previous_x;
        int delta_y = player_y - previous_y;
        int step_x = delta_x < 0 ? -1 : (delta_x > 0 ? 1 : 0);
        int step_y = delta_y < 0 ? -1 : (delta_y > 0 ? 1 : 0);
        int distance = (delta_x < 0 ? -delta_x : delta_x) + (delta_y < 0 ? -delta_y : delta_y);
        for (int step = 1; step <= distance; ++step)
        {
            append_follow_position(previous_x + step_x * step, previous_y + step_y * step);
        }
    };
    auto q_spawn_count = [&](RoomTemplateId room_template)
    {
        const composition_rooms::Room* source = composition_rooms::find(room_template);
        return source && source->ordinary ? source->q_points.size : 0;
    };
    auto q_spawn_position = [&](RoomTemplateId room_template, int slot, int &spawn_x, int &spawn_y)
    {
        const composition_rooms::Room* source = composition_rooms::find(room_template);
        if(!source || !source->ordinary || slot < 0 || slot >= source->q_points.size) return false;
        spawn_x = source->q_points[slot].x;
        spawn_y = source->q_points[slot].y;
        return true;
    };
    auto reset_floor_npcs = [&](bool keep_carried_npc)
    {
        NpcState previous_carried_npc;
        bool preserve = keep_carried_npc && carried_npc_index >= 0;
        if (preserve)
        {
            previous_carried_npc = npcs[carried_npc_index];
        }

        for (int index = 0; index < max_npcs; ++index)
        {
            npcs[index] = NpcState();
            npc_sprites[index].set_visible(false);
        }
        carried_npc_index = -1;
        if (preserve)
        {
            npcs[max_npcs - 1] = previous_carried_npc;
            carried_npc_index = max_npcs - 1;
        }

        int available_room_count = room_manager.room_count() - 4;
        if (available_room_count <= 0)
        {
            return;
        }

        bool used_rooms[RoomManager::max_rooms] = {};
        int npc_count = floor_random.get_int(1, 3);
        for (int index = 0; index < npc_count; ++index)
        {
            int room_id = -1;
            for (int attempt = 0; attempt < 32 && room_id < 0; ++attempt)
            {
                int candidate = 1 + floor_random.get_int(available_room_count);
                if (!used_rooms[candidate] && q_spawn_count(room_manager.room_template(candidate)) > 0)
                {
                    room_id = candidate;
                }
            }
            if (room_id < 0) break;
            used_rooms[room_id] = true;

            int position_index = floor_random.get_int(q_spawn_count(room_manager.room_template(room_id)));
            npcs[index].active = true;
            npcs[index].room_id = room_id;
            q_spawn_position(room_manager.room_template(room_id), position_index,
                             npcs[index].x, npcs[index].y);
        }
    };
    auto update_npc_sprites = [&](int player_frame)
    {
        int current_room_id = room_manager.current_room_id();
        int delivered_count = 0;
        for (int index = 0; index < max_npcs; ++index)
        {
            NpcState &npc = npcs[index];
            bn::sprite_ptr &sprite = npc_sprites[index];
            if (!npc.active)
            {
                sprite.set_visible(false);
                continue;
            }

            int npc_x = npc.x;
            int npc_y = npc.y;
            int idle_npc_frame = 0;
            bool visible = false;
            bool carried = index == carried_npc_index;
            if (carried)
            {
                if (follow_history_count > follow_delay)
                {
                    int history_index = follow_history_head - 1 - follow_delay;
                    while (history_index < 0)
                    {
                        history_index += follow_history_capacity;
                    }
                    npc_x = follow_history_x[history_index];
                    npc_y = follow_history_y[history_index];
                }
                visible = true;
            }
            else if (npc.delivered)
            {
                if (current_room_id == npc.room_id)
                {
                    if (delivered_count == 0)
                    {
                        // El NPC de la izquierda mira a la derecha hacia la fogata.
                        idle_npc_frame = 12 + int(delivered_npc_animation_alt_frame);
                    }
                    else if (delivered_count == 1)
                    {
                        // El NPC de la derecha mira a la izquierda hacia la fogata.
                        idle_npc_frame = 8 + int(delivered_npc_animation_alt_frame);
                    }
                    if (delivered_count < 3)
                    {
                        npc_x = campfire_x + delivered_npc_offsets[delivered_count][0];
                        npc_y = campfire_y + delivered_npc_offsets[delivered_count][1];
                        ++delivered_count;
                        visible = true;
                    }
                }
            }
            else if (current_room_id == npc.room_id)
            {
                visible = true;
            }

            sprite.set_position(npc_x, npc_y);
            npc_sprite_x[index] = npc_x;
            npc_sprite_y[index] = npc_y;
            sprite.set_tiles(bn::sprite_items::player.tiles_item(),
                             carried ? player_frame : idle_npc_frame);
            // El seguidor va por delante de piedras y puertas; los NPC de la fogata
            // se ordenan con el jugador segun su posicion vertical.
            sprite.set_z_order(carried ? -3 : (y < npc_y ? -2 : 0));
            sprite.set_visible(visible);
        }
    };
    auto overlaps_delivered_npc = [&](int player_x, int player_y)
    {
        int current_room_id = room_manager.current_room_id();
        for (int index = 0; index < max_npcs; ++index)
        {
            const NpcState &npc = npcs[index];
            if (!npc.active || !npc.delivered || npc.room_id != current_room_id)
            {
                continue;
            }

            int npc_x = npc_sprite_x[index];
            int npc_y = npc_sprite_y[index];

            int delta_x = player_x - npc_x;
            int delta_y = player_y - npc_y;
            if (game::sprite16_overlaps_player(delta_x, delta_y))
            {
                return true;
            }
        }
        return false;
    };
    auto overlaps_menu_furniture = [&](int player_x, int player_y)
    {
        constexpr int player_half_size = 8;
        if(menu_object_sprites.empty())
            return false;
        for(const menu_room::Object &object : menu_room::objects)
        {
            if(object.type == 'i')
                continue;
            if(object.width == 16 && object.height == 16)
            {
                if(game::sprite16_overlaps_player(player_x - object.x, player_y - object.y))
                    return true;
                continue;
            }
            int collision_y = object.y;
            int half_width = object.width / 2 + player_half_size;
            int half_height = object.height / 2 + player_half_size;
            if(object.height == 32)
            {
                // Igual que las columnas y libreros de los otros cuartos: solo la base es solida.
                collision_y += 4;
                half_width = 6 + player_half_size;
                half_height = 4 + player_half_size;
            }
            int delta_x = player_x - object.x;
            int delta_y = player_y - collision_y;
            if(game::sprite_collision_overlaps(delta_x, delta_y, half_width,
                    -half_height, half_height))
                return true;
        }
        return false;
    };

    auto overlaps_composition_objects = [&](int player_x, int player_y)
    {
        const composition_rooms::Room* source = current_composition_room();
        if(!source || source->menu) return false;
        for(const composition_rooms::Object& object : source->objects)
        {
            if(!object.solid) continue;
            int delta_x = player_x - object.x;
            int delta_y = player_y - object.y;
            if(object.legacy_base)
            {
                if(game::sprite_collision_overlaps(delta_x, delta_y - 4, 14, -12, 12)) return true;
            }
            else if(game::hitboxes_overlap(delta_x, delta_y, game::player_hitbox, object.hitbox)) return true;
        }
        for(const composition_rooms::Rect& rect : source->collisions)
            if(player_x + 8 > rect.x && player_x - 8 < rect.x + rect.width &&
               player_y + 8 > rect.y && player_y - 8 < rect.y + rect.height) return true;
        return false;
    };
    auto overlaps_imported_objects = [&](int player_x, int player_y)
    {
        if(!imported_room::contains(room_manager.current_template()))
            return false;
        for(const imported_room::Object &object : current_imported_room().objects)
        {
            if(object.role == imported_room::Role::DECORATION && object.solid &&
               game::hitboxes_overlap(player_x - object.x, player_y - object.y,
                   game::player_hitbox, object.hitbox))
                return true;
        }
        return false;
    };

    auto player_position_blocked = [&](int player_x, int player_y)
    {
        bool overlaps_reward_wall = !current_composition_room() && room_manager.current_template() == RoomTemplateId::REWARD &&
            player_x >= -48 && player_x <= -16 && player_y >= -72 && player_y <= 72;
        bool overlaps_reward_lever = false;
        int current_room_id = room_manager.current_room_id();
        for(int index = 0; index < 3; ++index)
        {
            bool prerequisites_met = index != 2 ||
                (reward_lever_activated[0] && reward_lever_activated[1]);
            if(prerequisites_met && reward_lever_room_ids[index] == current_room_id)
            {
                int delta_x = player_x - reward_lever_x[index];
                int delta_y = player_y - reward_lever_y[index];
                if(game::sprite16_overlaps_player(delta_x, delta_y))
                {
                    overlaps_reward_lever = true;
                    break;
                }
            }
        }
        return overlaps_composition_objects(player_x, player_y) ||
               overlaps_imported_objects(player_x, player_y) ||
               overlaps_menu_furniture(player_x, player_y) ||
               overlaps_stone(player_x, player_y) ||
               overlaps_campfire_furniture(player_x, player_y) ||
               overlaps_composition_4_box(player_x, player_y) ||
               overlaps_campfire(player_x, player_y) ||
               overlaps_delivered_npc(player_x, player_y) || overlaps_reward_wall ||
               overlaps_reward_lever;
    };

    int floor_key_room = -1;
    int floor_key_x = 0;
    int floor_key_y = 0;
    bool floor_key_collected = false;
    int special_key_room = -1;
    int special_key_x = 0;
    int special_key_y = 0;
    bool special_key_collected = false;
    auto q_spawn_is_occupied = [&](int room_id, int spawn_x, int spawn_y)
    {
        constexpr int minimum_item_spacing = 20;
        auto positions_overlap = [&](int other_x, int other_y)
        {
            int delta_x = spawn_x - other_x;
            int delta_y = spawn_y - other_y;
            return delta_x > -minimum_item_spacing && delta_x < minimum_item_spacing &&
                   delta_y > -minimum_item_spacing && delta_y < minimum_item_spacing;
        };

        for (int index = 0; index < max_npcs; ++index)
        {
            const NpcState &npc = npcs[index];
            if (npc.active && !npc.delivered && index != carried_npc_index &&
                npc.room_id == room_id &&
                positions_overlap(npc.x, npc.y))
            {
                return true;
            }
        }

        if ((! floor_key_collected && floor_key_room == room_id &&
             positions_overlap(floor_key_x, floor_key_y)) ||
            (! special_key_collected && special_key_room == room_id &&
             positions_overlap(special_key_x, special_key_y)))
        {
            return true;
        }

        for (int index = 0; index < RoomManager::max_rooms; ++index)
        {
            const FloorPotionState &potion = floor_potions[index];
            if (potion.active && potion.room_id == room_id &&
                positions_overlap(potion.x, potion.y))
            {
                return true;
            }
        }
        return false;
    };

    auto setup_reward_levers = [&]()
    {
        reward_lever_sprites.clear();
        reward_lever_sprite_index = -1;
        reward_room_unlocked = false;
        reward_map_blinking = false;
        reward_map_blink_visible = true;
        reward_map_blink_timer = 0;
        reward_map_blink_count = 0;
        for(int index = 0; index < 3; ++index)
        {
            reward_lever_animation_timers[index] = 0;
            reward_lever_animation_frames[index] = 0;
            reward_lever_activated[index] = false;
            reward_lever_room_ids[index] = -1;
            reward_lever_x[index] = 0;
            reward_lever_y[index] = 0;
        }

        int campfire_room_id = -1;
        for(int room_id = 0; room_id < room_manager.room_count(); ++room_id)
        {
            if(room_manager.room_template(room_id) == RoomTemplateId::SPECIAL)
            {
                campfire_room_id = room_id;
                break;
            }
        }
        if(campfire_room_id >= 0)
        {
            reward_lever_room_ids[0] = campfire_room_id;
            const auto& positions = composition_room_for(campfire_room_id)->levers;
            int slot = floor_random.get_int(positions.size);
            reward_lever_x[0] = positions[slot].x;
            reward_lever_y[0] = positions[slot].y;
        }

        reward_lever_room_ids[1] = room_manager.stairs_room_id();
        const auto& positions = composition_room_for(reward_lever_room_ids[1])->levers;
        int slot = floor_random.get_int(positions.size);
        reward_lever_x[1] = positions[slot].x;
        reward_lever_y[1] = positions[slot].y;

    };

    auto choose_third_reward_lever_room = [&]()
    {
        if(reward_lever_room_ids[2] >= 0 ||
           !reward_lever_activated[0] || !reward_lever_activated[1])
        {
            return;
        }

        int candidate_rooms[RoomManager::max_rooms] = {};
        int candidate_count = 0;
        int stairs_room_id = room_manager.stairs_room_id();
        for(int room_id = 0; room_id < stairs_room_id; ++room_id)
        {
            if(room_id == reward_lever_room_ids[0] ||
               room_id == reward_lever_room_ids[1] ||
               q_spawn_count(room_manager.room_template(room_id)) <= 0)
            {
                continue;
            }
            candidate_rooms[candidate_count++] = room_id;
        }

        if(candidate_count <= 0)
        {
            return;
        }

        int selected = reward_lever_random.get_int(candidate_count);
        int room_id = candidate_rooms[selected];
        RoomTemplateId room_template = room_manager.room_template(room_id);
        int q_count = q_spawn_count(room_template);
        int start_slot = reward_lever_random.get_int(q_count);
        int fallback_x = 0;
        int fallback_y = 0;
        bool found_fallback = false;
        for(int offset = 0; offset < q_count; ++offset)
        {
            int slot = (start_slot + offset) % q_count;
            int spawn_x = 0;
            int spawn_y = 0;
            if(!q_spawn_position(room_template, slot, spawn_x, spawn_y))
            {
                continue;
            }
            if(!found_fallback)
            {
                fallback_x = spawn_x;
                fallback_y = spawn_y;
                found_fallback = true;
            }
            if(!q_spawn_is_occupied(room_id, spawn_x, spawn_y))
            {
                reward_lever_x[2] = spawn_x;
                reward_lever_y[2] = spawn_y;
                reward_lever_room_ids[2] = room_id;
                return;
            }
        }

        // Si todos los puntos Q ya tienen un objeto, conserva igualmente un
        // cuarto elegido y deja la palanca en uno de sus puntos de aparicion.
        if(found_fallback)
        {
            reward_lever_x[2] = fallback_x;
            reward_lever_y[2] = fallback_y;
            reward_lever_room_ids[2] = room_id;
        }
    };

    auto update_reward_lever_visibility = [&]()
    {
        int room_id = room_manager.current_room_id();
        int visible_index = -1;
        for(int index = 0; index < 3; ++index)
        {
            bool prerequisites_met = index != 2 ||
                (reward_lever_activated[0] && reward_lever_activated[1]);
            if(prerequisites_met && reward_lever_room_ids[index] == room_id &&
               game_started && !game_over)
            {
                visible_index = index;
                break;
            }
        }

        if(visible_index < 0)
        {
            reward_lever_sprites.clear();
            reward_lever_sprite_index = -1;
            return;
        }

        if(reward_lever_sprites.empty() || reward_lever_sprite_index != visible_index)
        {
            reward_lever_sprites.clear();
            reward_lever_sprites.push_back(
                bn::sprite_items::label_16x16_animate.create_sprite(
                    reward_lever_x[visible_index], reward_lever_y[visible_index],
                    reward_lever_animation_frames[visible_index]));
            reward_lever_sprites[0].set_bg_priority(1);
            reward_lever_sprite_index = visible_index;
        }
        reward_lever_sprites[0].set_position(
            reward_lever_x[visible_index], reward_lever_y[visible_index]);
        reward_lever_sprites[0].set_visible(true);
    };

    auto setup_reward_room_rewards = [&]()
    {
        reward_room_reward_sprites.clear();
        reward_room_replacement_index = -1;
        reward_room_ability_rejected_until_leave = false;
        for(RewardRoomReward &reward : reward_room_rewards)
        {
            reward = RewardRoomReward();
        }

        int reward_room_id = room_manager.reward_room_id();
        if(reward_room_id < 0)
        {
            return;
        }

        // Las tres Q de Boss 1 Ant siempre entregan un objeto, y & siempre
        // entrega una habilidad aleatoria.
        const composition_rooms::Room& source = *composition_room_for(reward_room_id);
        for(int index = 0; index < 3; ++index)
        {
            RewardRoomReward &reward = reward_room_rewards[index];
            reward.active = true;
            int reward_choice = floor_random.get_int(3);
            reward.type = reward_choice == 0 ? RoomItemType::KEY :
                (reward_choice == 1 ? RoomItemType::ENERGY_POTION :
                                      RoomItemType::SPRINT_POTION);
            reward.x = source.q_points[index].x;
            reward.y = source.q_points[index].y;
            if(!room_manager.add_item(reward_room_id, reward.type, reward.x, reward.y))
            {
                reward.active = false;
                continue;
            }

            if(reward.type == RoomItemType::KEY)
            {
                reward_room_reward_sprites.push_back(
                    bn::sprite_items::key.create_sprite(reward.x, reward.y));
            }
            else if(reward.type == RoomItemType::ENERGY_POTION)
            {
                reward_room_reward_sprites.push_back(
                    bn::sprite_items::posion_energy_16x16_animate.create_sprite(
                        reward.x, reward.y, potion_animation_frame));
            }
            else
            {
                reward_room_reward_sprites.push_back(
                    bn::sprite_items::posion_sprint_16x16_animate.create_sprite(
                        reward.x, reward.y, potion_animation_frame));
            }
            reward_room_reward_sprites.back().set_bg_priority(1);
            reward_room_reward_sprites.back().set_z_order(-2);
            reward_room_reward_sprites.back().set_visible(false);
        }

        RewardRoomReward &ability_reward = reward_room_rewards[3];
        ability_reward.active = true;
        ability_reward.type = static_cast<RoomItemType>(
            int(RoomItemType::ABILITY_DASH) + floor_random.get_int(7));
        ability_reward.x = source.ability_points[0].x;
        ability_reward.y = source.ability_points[0].y;
        if(room_manager.add_item(reward_room_id, ability_reward.type,
                                 ability_reward.x, ability_reward.y))
        {
            reward_room_reward_sprites.push_back(
                bn::sprite_items::hability_dash_icon.create_sprite(
                    ability_reward.x, ability_reward.y));
            set_ability_icon(reward_room_reward_sprites.back(), static_cast<ActiveAbility>(
                int(ability_reward.type) - int(RoomItemType::ABILITY_DASH)));
            reward_room_reward_sprites.back().set_bg_priority(1);
            reward_room_reward_sprites.back().set_z_order(-2);
            reward_room_reward_sprites.back().set_visible(false);
        }
        else
        {
            ability_reward.active = false;
        }
    };

    auto update_reward_room_reward_visibility = [&]()
    {
        int reward_room_id = room_manager.reward_room_id();
        bool room_visible = game_started && !game_over && reward_room_unlocked &&
            room_manager.current_room_id() == reward_room_id;
        for(int index = 0; index < reward_room_reward_sprites.size(); ++index)
        {
            bool visible = room_visible && reward_room_rewards[index].active;
            reward_room_reward_sprites[index].set_visible(visible);
            if(visible)
                discover_active_item(reward_room_rewards[index].type);
        }
    };

    auto place_floor_key = [&]()
    {
        floor_key_collected = false;
        floor_key_room = -1;
        if (room_manager.room_count() < 3)
        {
            floor_key_sprite.set_visible(false);
            return;
        }

        // Las Q del documento son puntos asignados para objetos y aventureros.
        for (int attempt = 0; attempt < 32 && floor_key_room < 0; ++attempt)
        {
            int candidate = floor_random.get_int(room_manager.room_count() - 3);
            int count = q_spawn_count(room_manager.room_template(candidate));
            if (count > 0)
            {
                int start_slot = floor_random.get_int(count);
                for (int offset = 0; offset < count; ++offset)
                {
                    int slot = (start_slot + offset) % count;
                    int candidate_x = 0;
                    int candidate_y = 0;
                    if (q_spawn_position(room_manager.room_template(candidate), slot,
                                         candidate_x, candidate_y) &&
                        !q_spawn_is_occupied(candidate, candidate_x, candidate_y))
                    {
                        floor_key_room = candidate;
                        floor_key_x = candidate_x;
                        floor_key_y = candidate_y;
                        break;
                    }
                }
            }
        }
        if (floor_key_room < 0)
        {
            floor_key_sprite.set_visible(false);
            return;
        }
        room_manager.add_item(floor_key_room, RoomItemType::KEY, floor_key_x, floor_key_y);
        floor_key_sprite.set_position(floor_key_x, floor_key_y);
        floor_key_sprite.set_visible(room_manager.current_room_id() == floor_key_room);
    };

    auto place_special_key = [&]()
    {
        special_key_collected = false;
        special_key_room = -1;
        int eligible_room_count = room_manager.room_count() - 3;
        if(eligible_room_count < 2)
        {
            special_key_sprite.set_visible(false);
            return;
        }

        // Las llaves aparecen en cuartos distintos, accesibles antes de abrir ambos cuartos especiales.
        for (int attempt = 0; attempt < 32 && special_key_room < 0; ++attempt)
        {
            int candidate = floor_random.get_int(eligible_room_count);
            int count = q_spawn_count(room_manager.room_template(candidate));
            if (candidate != floor_key_room && count > 0)
            {
                int start_slot = floor_random.get_int(count);
                for (int offset = 0; offset < count; ++offset)
                {
                    int slot = (start_slot + offset) % count;
                    int candidate_x = 0;
                    int candidate_y = 0;
                    if (q_spawn_position(room_manager.room_template(candidate), slot,
                                         candidate_x, candidate_y) &&
                        !q_spawn_is_occupied(candidate, candidate_x, candidate_y))
                    {
                        special_key_room = candidate;
                        special_key_x = candidate_x;
                        special_key_y = candidate_y;
                        break;
                    }
                }
            }
        }
        if (special_key_room < 0)
        {
            special_key_sprite.set_visible(false);
            return;
        }
        room_manager.add_item(special_key_room, RoomItemType::KEY, special_key_x, special_key_y);
        special_key_sprite.set_position(special_key_x, special_key_y);
        special_key_sprite.set_visible(room_manager.current_room_id() == special_key_room);
    };

    auto place_floor_potions = [&]()
    {
        energy_potion_sprite.set_visible(false);
        sprint_potion_sprite.set_visible(false);
        for (FloorPotionState &potion : floor_potions)
        {
            potion = FloorPotionState();
        }

        int candidate_room_ids[RoomManager::max_rooms] = {};
        int candidate_count = 0;
        for (int room_id = 0; room_id < room_manager.room_count() - 3; ++room_id)
        {
            RoomTemplateId room_template = room_manager.room_template(room_id);
            int q_count = q_spawn_count(room_template);
            bool has_free_q = false;
            for (int slot = 0; slot < q_count; ++slot)
            {
                int spawn_x = 0;
                int spawn_y = 0;
                if (q_spawn_position(room_template, slot, spawn_x, spawn_y) &&
                    !q_spawn_is_occupied(room_id, spawn_x, spawn_y))
                {
                    has_free_q = true;
                    break;
                }
            }
            if (has_free_q)
            {
                candidate_room_ids[candidate_count++] = room_id;
            }
        }

        if (candidate_count == 0)
        {
            return;
        }

        // La semilla elige cualquier cantidad desde cero hasta todos los cuartos elegibles.
        int selected_count = floor_random.get_int(candidate_count + 1);
        for (int index = 0; index < selected_count; ++index)
        {
            int selected = index + floor_random.get_int(candidate_count - index);
            int room_id = candidate_room_ids[selected];
            candidate_room_ids[selected] = candidate_room_ids[index];

            RoomTemplateId room_template = room_manager.room_template(room_id);
            int q_count = q_spawn_count(room_template);
            int start_slot = floor_random.get_int(q_count);
            int spawn_x = 0;
            int spawn_y = 0;
            bool found_free_q = false;
            for (int offset = 0; offset < q_count; ++offset)
            {
                int slot = (start_slot + offset) % q_count;
                int candidate_x = 0;
                int candidate_y = 0;
                if (q_spawn_position(room_template, slot, candidate_x, candidate_y) &&
                    !q_spawn_is_occupied(room_id, candidate_x, candidate_y))
                {
                    spawn_x = candidate_x;
                    spawn_y = candidate_y;
                    found_free_q = true;
                    break;
                }
            }
            if (!found_free_q)
            {
                continue;
            }

            RoomItemType item_type = floor_random.get_int(2) == 0 ?
                RoomItemType::ENERGY_POTION : RoomItemType::SPRINT_POTION;
            if (!room_manager.add_item(room_id, item_type, spawn_x, spawn_y))
            {
                continue;
            }

            FloorPotionState &potion = floor_potions[room_id];
            potion.active = true;
            potion.type = item_type;
            potion.room_id = room_id;
            potion.x = spawn_x;
            potion.y = spawn_y;

            if (item_type == RoomItemType::ENERGY_POTION)
            {
                energy_potion_sprite.set_position(spawn_x, spawn_y);
            }
            else
            {
                sprint_potion_sprite.set_position(spawn_x, spawn_y);
            }
        }
    };

    auto update_floor_item_visibility = [&]()
    {
        int current_room_id = room_manager.current_room_id();
        bool show_energy_potion = false;
        bool show_sprint_potion = false;
        if (current_room_id >= 0 && current_room_id < RoomManager::max_rooms)
        {
            const FloorPotionState &potion = floor_potions[current_room_id];
            bool visible = game_started && !game_over && potion.active;
            show_energy_potion = visible && potion.type == RoomItemType::ENERGY_POTION;
            show_sprint_potion = visible && potion.type == RoomItemType::SPRINT_POTION;
            if (visible)
            {
                bn::sprite_ptr &sprite = show_energy_potion ?
                    energy_potion_sprite : sprint_potion_sprite;
                sprite.set_position(potion.x, potion.y);
            }
        }
        energy_potion_sprite.set_visible(show_energy_potion);
        sprint_potion_sprite.set_visible(show_sprint_potion);
    };

    auto remove_room_item = [&](int room_id, RoomItemType item_type, int item_x, int item_y)
    {
        for (int item_index = 0; item_index < RoomManager::max_items_per_room; ++item_index)
        {
            const RoomItem *item = room_manager.item_at(room_id, item_index);
            if (item && item->type == item_type && item->x == item_x && item->y == item_y)
            {
                room_manager.remove_item(room_id, item_index);
                return;
            }
        }
    };

    auto move_player_outside_pickup = [&](int item_x, int item_y)
    {
        constexpr int pickup_half_width = 10;
        constexpr int pickup_half_height = 12;
        int left_distance = x - (item_x - pickup_half_width) + 1;
        int right_distance = (item_x + pickup_half_width) - x + 1;
        int top_distance = y - (item_y - pickup_half_height) + 1;
        int bottom_distance = (item_y + pickup_half_height) - y + 1;

        if(left_distance <= right_distance && left_distance <= top_distance &&
           left_distance <= bottom_distance)
        {
            x = item_x - pickup_half_width - 1;
        }
        else if(right_distance <= top_distance && right_distance <= bottom_distance)
        {
            x = item_x + pickup_half_width + 1;
        }
        else if(top_distance <= bottom_distance)
        {
            y = item_y - pickup_half_height - 1;
        }
        else
        {
            y = item_y + pickup_half_height + 1;
        }
    };

    auto lock_stairs_room_door = [&]()
    {
        int last_room_id = room_manager.stairs_room_id();
        const RoomDirection directions[] =
            {
                RoomDirection::LEFT, RoomDirection::RIGHT,
                RoomDirection::UP, RoomDirection::DOWN};
        for (RoomDirection direction : directions)
        {
            if (room_manager.connected_room_id(last_room_id, direction) >= 0)
            {
                room_manager.set_door_open(last_room_id, direction, false);
                break;
            }
        }
    };

    auto update_map = [&]()
    {
        if (!ui_abierta)
        {
            // Libera los sprites del mapa cuando no se muestra para dejar espacio
            // al menu y a otros textos temporales.
            map_room_sprites.clear();
            map_link_sprites.clear();
            map_link_current_sprites.clear();
            map_link_special_sprites.clear();
            return;
        }

        if (map_room_sprites.empty())
        {
            for (int index = 0; index < RoomManager::max_rooms; ++index)
            {
                map_room_sprites.push_back(bn::sprite_items::map_room.create_sprite(0, 0));
                map_room_sprites[index].set_visible(false);
                map_room_sprites[index].set_bg_priority(0);
                map_room_sprites[index].set_z_order(-8);
            }
            for (int index = 0; index < RoomManager::max_rooms - 1; ++index)
            {
                map_link_sprites.push_back(bn::sprite_items::map_link.create_sprite(0, 0));
                map_link_sprites[index].set_visible(false);
                map_link_sprites[index].set_bg_priority(0);
                map_link_sprites[index].set_z_order(-8);
            }
            map_link_current_sprites.push_back(
                bn::sprite_items::map_link_current.create_sprite(0, 0));
            map_link_current_sprites[0].set_visible(false);
            map_link_current_sprites[0].set_bg_priority(0);
            map_link_current_sprites[0].set_z_order(-8);
            map_link_special_sprites.push_back(
                bn::sprite_items::map_link_special.create_sprite(0, 0));
            map_link_special_sprites[0].set_visible(false);
            map_link_special_sprites[0].set_bg_priority(0);
            map_link_special_sprites[0].set_z_order(-8);
        }

        for (bn::sprite_ptr &sprite : map_room_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : map_link_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : map_link_current_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : map_link_special_sprites)
            sprite.set_visible(false);
        constexpr int map_step = 16;
        constexpr int map_offset_y = 5;
        int current_id = room_manager.current_room_id();
        int stairs_room_id = room_manager.stairs_room_id();
        int reward_room_id = room_manager.reward_room_id();
        int normal_link_index = 0;
        int stairs_link_index = 0;
        int special_link_index = 0;
        for (int room_id = 0; room_id < room_manager.room_count(); ++room_id)
        {
            if(room_id == reward_room_id && !reward_room_unlocked)
            {
                continue;
            }
            if (!floor_map_revealed && !room_manager.room_discovered(room_id))
                continue;
            int room_x = room_manager.room_grid_x(room_id) * map_step;
            int room_y = room_manager.room_grid_y(room_id) * map_step + map_offset_y;
            const RoomDirection directions[] =
                {
                    RoomDirection::LEFT, RoomDirection::RIGHT,
                    RoomDirection::UP, RoomDirection::DOWN};
            for (RoomDirection direction : directions)
            {
                int other_id = room_manager.connected_room_id(room_id, direction);
                if (other_id < 0 || !room_manager.door_is_visible(room_id, direction))
                    continue;

                bool other_discovered = floor_map_revealed || room_manager.room_discovered(other_id);
                // Las salidas quedan en el mapa desde que se descubre su cuarto de origen,
                // aunque todavia no se haya visitado el destino.
                if (other_discovered && room_id > other_id)
                    continue;

                bool special_link = room_manager.door_is_special(room_id, direction);
                bool stairs_link = room_id == stairs_room_id || other_id == stairs_room_id;
                bn::sprite_ptr *link;
                if(special_link)
                {
                    link = &map_link_special_sprites[special_link_index++];
                }
                else if(stairs_link)
                {
                    link = &map_link_current_sprites[stairs_link_index++];
                }
                else
                {
                    link = &map_link_sprites[normal_link_index++];
                }
                int other_x = room_manager.room_grid_x(other_id) * map_step;
                int other_y = room_manager.room_grid_y(other_id) * map_step + map_offset_y;
                int link_x = (room_x + other_x) / 2;
                int link_y = (room_y + other_y) / 2;
                bool vertical_link = direction == RoomDirection::UP || direction == RoomDirection::DOWN;
                // El grafico rotado para conexiones verticales se ve un pixel mas abajo.
                if (vertical_link)
                {
                    --link_y;
                }
                link->set_position(link_x, link_y);
                link->set_rotation_angle(vertical_link ? 90 : 0);
                link->set_visible(true);
            }
            RoomTemplateId room_template = room_manager.room_template(room_id);
            bool room_has_item = false;
            for (int item_index = 0; item_index < RoomManager::max_items_per_room; ++item_index)
            {
                const RoomItem* item = room_manager.item_at(room_id, item_index);
                if (item)
                {
                    room_has_item = true;
                    break;
                }
            }

            bool room_has_adventurer = false;
            for (int npc_index = 0; npc_index < max_npcs; ++npc_index)
            {
                const NpcState &npc = npcs[npc_index];
                if (npc.active && !npc.delivered && npc_index != carried_npc_index &&
                    npc.room_id == room_id)
                {
                    room_has_adventurer = true;
                    break;
                }
            }

            // El indicador de ubicacion tiene prioridad sobre el icono de objeto.
            if(room_id == current_id && room_id != stairs_room_id)
            {
                map_room_sprites[room_id].set_tiles(bn::sprite_items::map_current.tiles_item());
            }
            else if(room_id == stairs_room_id)
            {
                map_room_sprites[room_id].set_tiles(bn::sprite_items::map_room_stairs.tiles_item());
            }
            else if(room_id == reward_room_id && reward_room_unlocked)
            {
                map_room_sprites[room_id].set_tiles(bn::sprite_items::map_room_boss.tiles_item());
            }
            else if(room_has_item || room_has_adventurer)
            {
                map_room_sprites[room_id].set_tiles(bn::sprite_items::map_room_item.tiles_item());
            }
            else if(room_template == RoomTemplateId::SPECIAL)
            {
                map_room_sprites[room_id].set_tiles(bn::sprite_items::map_special.tiles_item());
            }
            else
            {
                map_room_sprites[room_id].set_tiles(bn::sprite_items::map_room.tiles_item());
            }
            map_room_sprites[room_id].set_position(room_x, room_y);
            map_room_sprites[room_id].set_visible(true);
        }
    };

    auto unlock_reward_room = [&]()
    {
        if(reward_room_unlocked || !reward_lever_activated[0] ||
           !reward_lever_activated[1] || !reward_lever_activated[2])
        {
            return;
        }
        reward_room_unlocked = true;
        int reward_room_id = room_manager.reward_room_id();
        int reward_parent_id = room_manager.connected_room_id(
            reward_room_id, RoomDirection::RIGHT);
        if(reward_parent_id >= 0)
        {
            room_manager.set_door_visible(reward_room_id, RoomDirection::RIGHT, true);
            room_manager.set_door_open(reward_room_id, RoomDirection::RIGHT, true);
            room_manager.set_door_visible(reward_parent_id, RoomDirection::LEFT, true);
            room_manager.set_door_open(reward_parent_id, RoomDirection::LEFT, true);
        }
        update_door_sprites();
        floor_map_revealed = true;
        ui_abierta = true;
        pantalla_actual = Pantalla::MAPA;
        map_bg.emplace(bn::regular_bg_items::mapback.create_bg(0, 0));
        map_bg->set_priority(0);
        map_current_blink_timer = 0;
        map_current_blink_toggles_remaining = 6;
        map_current_marker_hidden = false;
        update_overlay_title("MAPA", true);
        reward_map_blinking = true;
        reward_map_blink_visible = true;
        reward_map_blink_timer = 0;
        reward_map_blink_count = 10;
        update_map();
        update_reward_room_reward_visibility();
        if(reward_room_id >= 0 && reward_room_id < map_room_sprites.size())
        {
            map_room_sprites[reward_room_id].set_tiles(
                bn::sprite_items::map_room_boss.tiles_item());
        }
    };

    auto update_reward_map_blink = [&]()
    {
        if(!reward_map_blinking)
        {
            return;
        }
        ++reward_map_blink_timer;
        if(reward_map_blink_timer >= 12)
        {
            reward_map_blink_timer = 0;
            reward_map_blink_visible = !reward_map_blink_visible;
            if(reward_map_blink_count > 0)
            {
                --reward_map_blink_count;
            }
            if(reward_map_blink_count <= 0)
            {
                reward_map_blinking = false;
                reward_map_blink_visible = true;
            }
        }
        int reward_room_id = room_manager.reward_room_id();
        if(reward_room_id >= 0 && reward_room_id < map_room_sprites.size())
        {
            map_room_sprites[reward_room_id].set_visible(ui_abierta && reward_map_blink_visible);
        }
    };

    auto clear_placed_bomb = [&]()
    {
        placed_bomb_sprite.reset();
        pending_bomb_frames = 0;
        pending_bomb_room_id = -1;
        pending_bomb_energy_cost = 0;
        bomb_blink_timer = 0;
        bomb_blink_visible = true;
    };

    auto detonate_placed_bomb = [&]()
    {
        int room_id = pending_bomb_room_id;
        int energy_cost = pending_bomb_energy_cost;
        clear_placed_bomb();
        if (room_id < 0 || room_id >= RoomManager::max_rooms)
        {
            return;
        }

        energy -= energy_cost;
        update_energy_bar_fill(energy);
        if (energy < 5)
        {
            start_energy_recovery_flash();
        }

        bombed_rooms[room_id] = true;
        skeleton_counts[room_id] = 0;
        bat_counts[room_id] = 0;
        mushy_counts[room_id] = 0;
        plant_counts[room_id] = 0;
        update_map();
        if (room_manager.current_room_id() == room_id)
        {
            update_room_obstacles();
            update_stone_layers();
            bomb_flash_frames = bomb_flash_duration_frames;
            bn::blending::set_white_fade_color();
            bn::blending::set_fade_alpha(1);
        }
        bn::sound_items::dash.play();
        start_ability_flash();
    };

    auto update_placed_bomb = [&]()
    {
        if (pending_bomb_frames <= 0)
        {
            return;
        }

        --pending_bomb_frames;
        int elapsed_frames = bomb_fuse_duration_frames - pending_bomb_frames;
        int blink_interval = 30 - elapsed_frames * 28 / bomb_fuse_duration_frames;
        if (blink_interval < 2)
        {
            blink_interval = 2;
        }
        ++bomb_blink_timer;
        if (bomb_blink_timer >= blink_interval)
        {
            bomb_blink_timer = 0;
            bomb_blink_visible = !bomb_blink_visible;
        }

        if (placed_bomb_sprite.has_value())
        {
            placed_bomb_sprite->set_visible(
                room_manager.current_room_id() == pending_bomb_room_id && bomb_blink_visible);
        }
        if (pending_bomb_frames == 0)
        {
            detonate_placed_bomb();
        }
    };

    auto update_map_current_blink = [&]()
    {
        if (map_current_blink_toggles_remaining > 0)
        {
            ++map_current_blink_timer;
            if (map_current_blink_timer >= map_current_blink_interval)
            {
                map_current_blink_timer = 0;
                map_current_marker_hidden = !map_current_marker_hidden;
                --map_current_blink_toggles_remaining;
                if (map_current_blink_toggles_remaining == 0)
                {
                    map_current_marker_hidden = false;
                }
            }
        }

        int current_room_id = room_manager.current_room_id();
        if (current_room_id >= 0 && current_room_id < map_room_sprites.size())
        {
            map_room_sprites[current_room_id].set_visible(!map_current_marker_hidden);
        }
    };

    auto update_trapdoor_visibility = [&]()
    {
        bool visible = game_started && !game_over && trapdoor_deployed &&
            (room_manager.current_room_id() == home_room_id ||
             room_manager.current_room_id() == trapdoor_room_id);
        trapdoor_world_sprite.set_visible(visible);
        if (visible)
        {
            bool in_home_room = room_manager.current_room_id() == home_room_id;
            trapdoor_world_sprite.set_position(in_home_room ? home_trapdoor_x : trapdoor_x,
                                               in_home_room ? home_trapdoor_y : trapdoor_y);
        }
    };

    auto update_campfire_reward_visibility = [&]()
    {
        if (campfire_reward_sprite.has_value())
        {
            bool visible = campfire_reward_active && game_started && !game_over &&
                room_manager.current_template() == RoomTemplateId::SPECIAL;
            campfire_reward_sprite->set_visible(visible);
            if(visible)
                discover_active_item(campfire_reward_type);
        }
    };

    auto clear_campfire_reward = [&]()
    {
        campfire_reward_active = false;
        campfire_reward_level = 1;
        if (campfire_reward_sprite.has_value())
        {
            campfire_reward_sprite.reset();
        }
    };

    auto spawn_campfire_reward = [&](bool from_queue = false)
    {
        // Los premios esperan en cola para no apilarse en el mismo tile.
        if (campfire_reward_active || (campfire_pending_rewards > 0 && !from_queue))
        {
            ++campfire_pending_rewards;
            return;
        }

        bool gives_ability = ! campfire_skill_reward_granted;
        campfire_reward_level = 1;
        if (gives_ability)
        {
            campfire_reward_type = static_cast<RoomItemType>(
                int(RoomItemType::ABILITY_DASH) + campfire_reward_random.get_int(7));
        }
        else
        {
            int pickup = campfire_reward_random.get_int(3);
            campfire_reward_type = pickup == 0 ? RoomItemType::KEY :
                (pickup == 1 ? RoomItemType::ENERGY_POTION : RoomItemType::SPRINT_POTION);
        }

        int room_id = room_manager.current_room_id();
        if (!room_manager.add_item(room_id, campfire_reward_type,
                                   campfire_reward_x, campfire_reward_y))
        {
            return;
        }

        if (gives_ability)
        {
            campfire_reward_sprite.emplace(bn::sprite_items::hability_dash_icon.create_sprite(
                campfire_reward_x, campfire_reward_y));
            ActiveAbility reward_ability = static_cast<ActiveAbility>(
                int(campfire_reward_type) - int(RoomItemType::ABILITY_DASH));
            set_ability_icon(*campfire_reward_sprite, reward_ability);
            campfire_skill_reward_granted = true;
        }
        else if (campfire_reward_type == RoomItemType::KEY)
        {
            campfire_reward_sprite.emplace(
                bn::sprite_items::key.create_sprite(campfire_reward_x, campfire_reward_y));
        }
        else if (campfire_reward_type == RoomItemType::ENERGY_POTION)
        {
            campfire_reward_sprite.emplace(bn::sprite_items::posion_energy_16x16_animate.create_sprite(
                campfire_reward_x, campfire_reward_y, potion_animation_frame));
        }
        else
        {
            campfire_reward_sprite.emplace(bn::sprite_items::posion_sprint_16x16_animate.create_sprite(
                campfire_reward_x, campfire_reward_y, potion_animation_frame));
        }

        campfire_reward_active = true;
        campfire_reward_sprite->set_bg_priority(1);
        campfire_reward_sprite->set_z_order(-2);
        update_campfire_reward_visibility();
        update_map();
    };

    auto clear_options_menu = [&]()
    {
        options_heading_sprites.clear();
        options_continue_sprites.clear();
        options_reset_sprites.clear();
        options_title_sprites.clear();
        options_cursor_sprites.clear();
    };

    auto update_options_cursor = [&]()
    {
        if (!options_cursor_sprites.empty())
        {
            options_cursor_sprites[0].set_y(-8 + options_selection * 24);
        }
    };

    auto open_options_menu = [&]()
    {
        options_open = true;
        inventory_open = false;
        clear_inventory_screen();
        options_selection = 0;
        ui_abierta = false;
        for (bn::sprite_ptr &sprite : menu_text_sprites)
            sprite.set_visible(false);
        refresh_main_hud_visibility();
        set_scene_darkness(0.8);
        update_map();
        clear_options_menu();
        title_text_generator.generate(0, -40, "OPTIONS", options_heading_sprites);
        title_text_generator.generate(0, -8, "CONTINUE", options_continue_sprites);
        title_text_generator.generate(0, 16, "NEW SEED", options_reset_sprites);
        title_text_generator.generate(0, 40, "MAIN MENU", options_title_sprites);
        title_text_generator.generate(-64, -8, ">", options_cursor_sprites);
    };

    auto close_options_menu = [&]()
    {
        options_open = false;
        clear_options_menu();
        set_scene_darkness(0);
        update_map();
        refresh_main_hud_visibility();
    };

    auto update_floor_label = [&]()
    {
        create_floor_label(piso_actual, floor_label);
        floor_text_sprites.clear();
        floor_text_generator.generate(-112, -68, floor_label, floor_text_sprites);
        refresh_map_labels_visibility();
    };

    auto update_energy_label = [&]()
    {
        update_energy_bar_fill(energy);
    };

    auto release_hud_sprites = [&]()
    {
        hud_sprites_allocated = false;
        if (key_icon.has_value())
        {
            key_icon.reset();
        }
        key_count_text_sprites.clear();
        stamina_bar_background_sprites.clear();
        stamina_bar_liquid_sprites.clear();
        energy_bar_background_sprites.clear();
        energy_bar_liquid_sprites.clear();
        stamina_label_sprites.clear();
        energy_bar_label_sprites.clear();
    };

    auto create_hud_sprites = [&]()
    {
        if (hud_sprites_allocated)
        {
            return;
        }

        key_icon.emplace(bn::sprite_items::key.create_sprite(
            inventory_key_x, inventory_key_y));
        key_icon->set_palette(inventory_key_palette);
        key_icon->set_bg_priority(0);
        key_icon->set_z_order(-13);

        for (int index = 0; index < stamina_bar_segments; ++index)
        {
            int segment_y = stamina_bar_top_y + stamina_segment_height / 2 + index * stamina_segment_height;
            stamina_bar_background_sprites.push_back(
                bn::sprite_items::ui_stamina_bar.create_sprite(stamina_bar_x, segment_y, index));
            stamina_bar_background_sprites[index].set_bg_priority(0);
            stamina_bar_background_sprites[index].set_z_order(-12);

            stamina_bar_liquid_sprites.push_back(
                bn::sprite_items::ui_sastamina_bar_liquid.create_sprite(stamina_bar_x, segment_y, index));
            stamina_bar_liquid_sprites[index].set_bg_priority(0);
            stamina_bar_liquid_sprites[index].set_z_order(-13);

            energy_bar_background_sprites.push_back(
                bn::sprite_items::ui_stamina_bar.create_sprite(energy_bar_x, segment_y, index));
            energy_bar_background_sprites[index].set_bg_priority(0);
            energy_bar_background_sprites[index].set_z_order(-12);

            energy_bar_liquid_sprites.push_back(
                bn::sprite_items::ui_energy_bar_liquid.create_sprite(energy_bar_x, segment_y, index));
            energy_bar_liquid_sprites[index].set_bg_priority(0);
            energy_bar_liquid_sprites[index].set_z_order(-13);
        }

        stamina_label_generator.generate(stamina_bar_x - stamina_bar_width / 2 - status_layout_gap,
                                         stamina_bar_top_y + stamina_bar_height - 8,
                                        "SPIRIT", stamina_label_sprites);
        energy_label_generator.generate(energy_bar_x + stamina_bar_width / 2 + status_layout_gap,
                                        stamina_bar_top_y + stamina_bar_height - 8,
                                        "ENERGY", energy_bar_label_sprites);
        key_count_text_generator.generate(inventory_key_count_x, inventory_key_y, key_count_label,
                                          key_count_text_sprites);
        floor_text_sprites.clear();
        floor_text_generator.generate(-112, -68, floor_label, floor_text_sprites);

        hud_sprites_allocated = true;
        update_stamina_bar_fill(stamina);
        update_energy_bar_fill(energy);
        refresh_main_hud_visibility();
    };

    auto damage_player = [&]()
    {
        if (damage_invulnerability_frames > 0)
        {
            return;
        }

        if (energy > 0)
        {
            --energy;
        }
        bn::sound_items::hit.play();
        update_energy_label();
        update_energy_bar_fill(energy);
        if (energy < 5)
        {
            start_energy_recovery_flash();
        }
        damage_invulnerability_frames = player_blink_interval * player_blink_count * 2;
        player_blink_timer = 0;
        player_blink_toggles_remaining = player_blink_count * 2;
        player_blink_hidden = false;
        player.set_visible(true);
    };

    auto update_player_damage_blink = [&]()
    {
        if (damage_invulnerability_frames > 0)
        {
            --damage_invulnerability_frames;
        }

        if (player_blink_toggles_remaining > 0)
        {
            ++player_blink_timer;
            if (player_blink_timer >= player_blink_interval)
            {
                player_blink_timer = 0;
                player_blink_hidden = !player_blink_hidden;
                --player_blink_toggles_remaining;
                player.set_visible(!player_blink_hidden);
            }
        }
        else if (damage_invulnerability_frames <= 0)
        {
            player.set_visible(true);
        }
    };

    auto read_edited_seed = [&](unsigned &seed_value)
    {
        unsigned value = 0;
        for(int digit : seed_digits)
        {
            if(value > (0xFFFFFFFFu - unsigned(digit)) / 10u)
                return false;
            value = value * 10u + unsigned(digit);
        }
        seed_value = value;
        return true;
    };

    auto update_seed_editor_selection = [&]()
    {
        for (bn::sprite_ptr &sprite : seed_editor_static_digits_sprites)
        {
            sprite.set_visible(true);
        }
        if (!seed_editor_selected_digit_sprites.empty())
        {
            seed_editor_selected_digit_sprites[0].set_visible(seed_digit_visible);
        }
    };

    auto update_seed_editor_digits = [&]()
    {
        seed_editor_static_digits_sprites.clear();
        seed_editor_selected_digit_sprites.clear();
        char digits_label[11];
        for (int index = 0; index < 10; ++index)
        {
            digits_label[index] = char('0' + seed_digits[index]);
        }
        digits_label[10] = '\0';

        int text_x = -title_text_generator.width(digits_label) / 2;
        char prefix[11];
        char suffix[11];
        for (int index = 0; index < seed_digit_cursor; ++index)
        {
            prefix[index] = digits_label[index];
        }
        prefix[seed_digit_cursor] = '\0';
        int suffix_start = seed_digit_cursor + 1;
        int suffix_length = 10 - suffix_start;
        for (int index = 0; index < suffix_length; ++index)
        {
            suffix[index] = digits_label[suffix_start + index];
        }
        suffix[suffix_length] = '\0';

        char selected_digit[2] = {digits_label[seed_digit_cursor], '\0'};
        int selected_x = text_x + title_text_generator.width(prefix);
        int suffix_x = selected_x + title_text_generator.width(selected_digit);
        title_text_generator.set_left_alignment();

        auto generate_scaled_text = [&](int x_position, const char *text,
                                        bn::vector<bn::sprite_ptr, 12> &sprites)
        {
            if (text[0])
            {
                int first_sprite = sprites.size();
                title_text_generator.generate(x_position, -28, text, sprites);
                for (int index = first_sprite; index < sprites.size(); ++index)
                {
                    bn::fixed old_x = sprites[index].x();
                    sprites[index].set_affine_mat(seed_affine_mat);
                    sprites[index].set_x(old_x * seed_scale);
                }
            }
        };

        generate_scaled_text(text_x, prefix, seed_editor_static_digits_sprites);
        title_text_generator.generate(selected_x, -28, selected_digit, seed_editor_selected_digit_sprites);
        bn::fixed selected_old_x = seed_editor_selected_digit_sprites[0].x();
        seed_editor_selected_digit_sprites[0].set_affine_mat(seed_affine_mat);
        seed_editor_selected_digit_sprites[0].set_x(selected_old_x * seed_scale);
        generate_scaled_text(suffix_x, suffix, seed_editor_static_digits_sprites);
        title_text_generator.set_center_alignment();

        unsigned edited_seed = 0;
        bool seed_valid = read_edited_seed(edited_seed);
        seed_editor_accept_help_sprites.clear();
        title_text_generator.generate(0, 46,
            seed_valid ? "START: ACCEPT" : "SEED OUT OF RANGE", seed_editor_accept_help_sprites);
        update_seed_editor_selection();
    };

    auto set_main_title_visible = [&](bool visible)
    {
        for (bn::sprite_ptr &sprite : title_name_sprites)
            sprite.set_visible(visible);
        for (bn::sprite_ptr &sprite : title_credit_sprites)
            sprite.set_visible(visible);
        for (bn::sprite_ptr &sprite : title_seed_sprites)
            sprite.set_visible(visible);
        for (bn::sprite_ptr &sprite : title_edit_seed_sprites)
            sprite.set_visible(visible);
        for (bn::sprite_ptr &sprite : title_enter_sprites)
            sprite.set_visible(visible);
    };

    auto open_seed_editor = [&]()
    {
        seed_editor_open = true;
        seed_digit_cursor = 0;
        seed_blink_timer = 0;
        seed_digit_visible = true;
        unsigned seed_value = selected_seed;
        for (int index = 9; index >= 0; --index)
        {
            seed_digits[index] = int(seed_value % 10);
            seed_value /= 10;
        }

        set_main_title_visible(false);
        title_name_sprites.clear();
        title_credit_sprites.clear();
        title_seed_sprites.clear();
        title_edit_seed_sprites.clear();
        title_enter_sprites.clear();
        title_text_generator.generate(0, -55, "ENTER SEED", seed_editor_heading_sprites);
        for (bn::sprite_ptr &sprite : seed_editor_heading_sprites)
        {
            bn::fixed old_x = sprite.x();
            sprite.set_affine_mat(title_affine_mat);
            sprite.set_x(old_x * title_scale);
        }
        title_text_generator.generate(0, -8, "MAX: 4294967295", seed_editor_range_sprites);
        title_text_generator.generate(0, 12, "LEFT/RIGHT: SELECT", seed_editor_move_help_sprites);
        title_text_generator.generate(0, 29, "UP/DOWN: EDIT", seed_editor_change_help_sprites);
        title_text_generator.generate(0, 63, "SELECT: RANDOM + BACK", seed_editor_random_help_sprites);
        update_seed_editor_digits();
    };

    auto close_seed_editor = [&]()
    {
        seed_editor_open = false;
        seed_editor_heading_sprites.clear();
        seed_editor_static_digits_sprites.clear();
        seed_editor_selected_digit_sprites.clear();
        seed_editor_move_help_sprites.clear();
        seed_editor_change_help_sprites.clear();
        seed_editor_accept_help_sprites.clear();
        seed_editor_range_sprites.clear();
        seed_editor_random_help_sprites.clear();
        create_main_title();
    };

    // Cambiar esta version si se modifica el orden o significado de los campos guardados.
    constexpr unsigned run_snapshot_version_v1 = 0x00010001u;
    constexpr unsigned run_snapshot_version = 0x00010002u;
    constexpr int run_snapshot_bytes_v1 = int(sizeof(unsigned) * 5 + sizeof(room_manager)
        + sizeof(run_seed)
        + sizeof(x)
        + sizeof(y)
        + sizeof(movement_speed_remainder)
        + sizeof(animation_speed)
        + sizeof(direccion)
        + sizeof(dash_direction)
        + sizeof(active_ability)
        + sizeof(carried_abilities)
        + sizeof(carried_ability_count)
        + sizeof(selected_ability_slot)
        + sizeof(replacing_ability)
        + sizeof(pending_ability)
        + sizeof(pending_ability_level)
        + sizeof(reject_reward_until_leave)
        + sizeof(bombed_rooms)
        + sizeof(bomb_flash_frames)
        + sizeof(pending_bomb_frames)
        + sizeof(pending_bomb_room_id)
        + sizeof(pending_bomb_x)
        + sizeof(pending_bomb_y)
        + sizeof(pending_bomb_energy_cost)
        + sizeof(bomb_blink_timer)
        + sizeof(bomb_blink_visible)
        + sizeof(familiar_active)
        + sizeof(familiar_frames_remaining)
        + sizeof(familiar_target_kind)
        + sizeof(familiar_target_index)
        + sizeof(familiar_room_id)
        + sizeof(familiar_target_x)
        + sizeof(familiar_target_y)
        + sizeof(familiar_x)
        + sizeof(familiar_y)
        + sizeof(floor_map_revealed)
        + sizeof(reward_room_unlocked)
        + sizeof(reward_map_blinking)
        + sizeof(reward_map_blink_visible)
        + sizeof(reward_map_blink_timer)
        + sizeof(reward_map_blink_count)
        + sizeof(reward_lever_activated)
        + sizeof(reward_lever_room_ids)
        + sizeof(reward_lever_x)
        + sizeof(reward_lever_y)
        + sizeof(reward_lever_animation_timers)
        + sizeof(reward_lever_animation_frames)
        + sizeof(time_stopped_room_id)
        + sizeof(trapdoor_deployed)
        + sizeof(trapdoor_room_id)
        + sizeof(trapdoor_x)
        + sizeof(trapdoor_y)
        + sizeof(dash_active)
        + sizeof(frame)
        + sizeof(animation_timer)
        + sizeof(displayed_frame)
        + sizeof(npcs)
        + sizeof(npc_sprite_x)
        + sizeof(npc_sprite_y)
        + sizeof(carried_npc_index)
        + sizeof(rescued_adventurers)
        + sizeof(follow_history_x)
        + sizeof(follow_history_y)
        + sizeof(follow_history_head)
        + sizeof(follow_history_count)
        + sizeof(spike_counts)
        + sizeof(spike_x)
        + sizeof(spike_y)
        + sizeof(spike_frames)
        + sizeof(spike_permanent)
        + sizeof(spike_cycle_frames)
        + sizeof(mushy_counts)
        + sizeof(mushy_room_x)
        + sizeof(mushy_room_y)
        + sizeof(mushy_direction_frames)
        + sizeof(mushy_move_axes)
        + sizeof(mushy_walking)
        + sizeof(mushy_speed_growth_timer)
        + sizeof(mushy_speed_percent)
        + sizeof(active_mushy_count)
        + sizeof(mushy_move_timer)
        + sizeof(mushy_animation_timer)
        + sizeof(mushy_animation_frame)
        + sizeof(plant_bullets)
        + sizeof(plant_shot_cooldowns)
        + sizeof(plant_counts)
        + sizeof(plant_position_indices)
        + sizeof(active_plant_count)
        + sizeof(plant_fire_animation_steps)
        + sizeof(plant_fire_animation_timers)
        + sizeof(plant_displayed_frames)
        + sizeof(floor_potions)
        + sizeof(reward_room_rewards)
        + sizeof(reward_room_replacement_index)
        + sizeof(reward_room_ability_rejected_until_leave)
        + sizeof(potion_animation_timer)
        + sizeof(potion_animation_frame)
        + sizeof(campfire_reward_active)
        + sizeof(campfire_skill_reward_granted)
        + sizeof(campfire_pending_rewards)
        + sizeof(campfire_reward_type)
        + sizeof(campfire_reward_level)
        + sizeof(delivered_npc_animation_timer)
        + sizeof(delivered_npc_animation_alt_frame)
        + sizeof(campfire_frame)
        + sizeof(campfire_animation_timer)
        + sizeof(campfire_used_this_floor)
        + sizeof(ability_icon_blink_timer)
        + sizeof(ability_icon_blink_toggles_remaining)
        + sizeof(ability_icon_blink_hidden)
        + sizeof(key_count)
        + sizeof(piso_actual)
        + sizeof(energy)
        + sizeof(stamina)
        + sizeof(campfire_use_frames_remaining)
        + sizeof(ability_flash_frames_remaining)
        + sizeof(stamina_exhaustion_penalty_applied)
        + sizeof(stamina_run_attempt_latched)
        + sizeof(stamina_blink_timer)
        + sizeof(stamina_blink_toggles_remaining)
        + sizeof(stamina_blink_hidden)
        + sizeof(energy_blink_timer)
        + sizeof(energy_blink_toggles_remaining)
        + sizeof(energy_blink_hidden)
        + sizeof(damage_invulnerability_frames)
        + sizeof(player_blink_timer)
        + sizeof(player_blink_toggles_remaining)
        + sizeof(player_blink_hidden)
        + sizeof(enemy_contact_latched)
        + sizeof(spike_contact_latched)
        + sizeof(spike_contact_room_id)
        + sizeof(scored_rooms)
        + sizeof(bat_counts)
        + sizeof(bat_start_x)
        + sizeof(bat_start_y)
        + sizeof(bat_start_dx)
        + sizeof(bat_start_dy)
        + sizeof(bat_start_animation_frames)
        + sizeof(bat_start_animation_timers)
        + sizeof(bat_room_x)
        + sizeof(bat_room_y)
        + sizeof(bat_room_dx)
        + sizeof(bat_room_dy)
        + sizeof(bat_room_animation_frames)
        + sizeof(bat_room_animation_timers)
        + sizeof(active_bat_count)
        + sizeof(bat_x)
        + sizeof(bat_y)
        + sizeof(bat_dx)
        + sizeof(bat_dy)
        + sizeof(bat_animation_frames)
        + sizeof(bat_animation_timers)
        + sizeof(bat_displayed_frames)
        + sizeof(bat_sound_cooldown_frames)
        + sizeof(skeleton_sound_cooldown_frames)
        + sizeof(skeleton_counts)
        + sizeof(skeleton_start_corners)
        + sizeof(skeleton_room_path_positions)
        + sizeof(skeleton_room_path_initialized)
        + sizeof(active_skeleton_count)
        + sizeof(skeleton_path_positions)
        + sizeof(skeleton_x)
        + sizeof(skeleton_y)
        + sizeof(skeleton_walk_frames)
        + sizeof(skeleton_animation_timers)
        + sizeof(skeleton_displayed_frames)
        + sizeof(floor_key_room)
        + sizeof(floor_key_x)
        + sizeof(floor_key_y)
        + sizeof(floor_key_collected)
        + sizeof(special_key_room)
        + sizeof(special_key_x)
        + sizeof(special_key_y)
        + sizeof(special_key_collected)
        + sizeof(run_rooms_explored));
    constexpr int run_snapshot_bytes = run_snapshot_bytes_v1 + int(sizeof(imported_chest_frame) +
        sizeof(imported_chest_target_frame) + sizeof(imported_chest_animation_timer));
    static_assert(run_snapshot_bytes <= game_save::max_run_bytes, "Active run exceeds SRAM budget");
    auto transfer_run_state = [&](auto &archive)
    {
        unsigned version = run_snapshot_version;
        archive.value(version);
        if(version != run_snapshot_version_v1 && version != run_snapshot_version)
            return false;
        archive.value(room_manager);
        archive.value(run_seed);
        archive.value(x);
        archive.value(y);
        archive.value(movement_speed_remainder);
        archive.value(animation_speed);
        archive.value(direccion);
        archive.value(dash_direction);
        archive.value(active_ability);
        archive.value(carried_abilities);
        archive.value(carried_ability_count);
        archive.value(selected_ability_slot);
        archive.value(replacing_ability);
        archive.value(pending_ability);
        archive.value(pending_ability_level);
        archive.value(reject_reward_until_leave);
        archive.value(bombed_rooms);
        archive.value(bomb_flash_frames);
        archive.value(pending_bomb_frames);
        archive.value(pending_bomb_room_id);
        archive.value(pending_bomb_x);
        archive.value(pending_bomb_y);
        archive.value(pending_bomb_energy_cost);
        archive.value(bomb_blink_timer);
        archive.value(bomb_blink_visible);
        archive.value(familiar_active);
        archive.value(familiar_frames_remaining);
        archive.value(familiar_target_kind);
        archive.value(familiar_target_index);
        archive.value(familiar_room_id);
        archive.value(familiar_target_x);
        archive.value(familiar_target_y);
        archive.value(familiar_x);
        archive.value(familiar_y);
        archive.value(floor_map_revealed);
        archive.value(reward_room_unlocked);
        archive.value(reward_map_blinking);
        archive.value(reward_map_blink_visible);
        archive.value(reward_map_blink_timer);
        archive.value(reward_map_blink_count);
        archive.value(reward_lever_activated);
        archive.value(reward_lever_room_ids);
        archive.value(reward_lever_x);
        archive.value(reward_lever_y);
        archive.value(reward_lever_animation_timers);
        archive.value(reward_lever_animation_frames);
        archive.value(time_stopped_room_id);
        archive.value(trapdoor_deployed);
        archive.value(trapdoor_room_id);
        archive.value(trapdoor_x);
        archive.value(trapdoor_y);
        archive.value(dash_active);
        archive.value(frame);
        archive.value(animation_timer);
        archive.value(displayed_frame);
        archive.value(npcs);
        archive.value(npc_sprite_x);
        archive.value(npc_sprite_y);
        archive.value(carried_npc_index);
        archive.value(rescued_adventurers);
        archive.value(follow_history_x);
        archive.value(follow_history_y);
        archive.value(follow_history_head);
        archive.value(follow_history_count);
        archive.value(spike_counts);
        archive.value(spike_x);
        archive.value(spike_y);
        archive.value(spike_frames);
        archive.value(spike_permanent);
        archive.value(spike_cycle_frames);
        archive.value(mushy_counts);
        archive.value(mushy_room_x);
        archive.value(mushy_room_y);
        archive.value(mushy_direction_frames);
        archive.value(mushy_move_axes);
        archive.value(mushy_walking);
        archive.value(mushy_speed_growth_timer);
        archive.value(mushy_speed_percent);
        archive.value(active_mushy_count);
        archive.value(mushy_move_timer);
        archive.value(mushy_animation_timer);
        archive.value(mushy_animation_frame);
        archive.value(plant_bullets);
        archive.value(plant_shot_cooldowns);
        archive.value(plant_counts);
        archive.value(plant_position_indices);
        archive.value(active_plant_count);
        archive.value(plant_fire_animation_steps);
        archive.value(plant_fire_animation_timers);
        archive.value(plant_displayed_frames);
        archive.value(floor_potions);
        archive.value(reward_room_rewards);
        archive.value(reward_room_replacement_index);
        archive.value(reward_room_ability_rejected_until_leave);
        archive.value(potion_animation_timer);
        archive.value(potion_animation_frame);
        archive.value(campfire_reward_active);
        archive.value(campfire_skill_reward_granted);
        archive.value(campfire_pending_rewards);
        archive.value(campfire_reward_type);
        archive.value(campfire_reward_level);
        archive.value(delivered_npc_animation_timer);
        archive.value(delivered_npc_animation_alt_frame);
        archive.value(campfire_frame);
        archive.value(campfire_animation_timer);
        archive.value(campfire_used_this_floor);
        archive.value(ability_icon_blink_timer);
        archive.value(ability_icon_blink_toggles_remaining);
        archive.value(ability_icon_blink_hidden);
        archive.value(key_count);
        archive.value(piso_actual);
        archive.value(energy);
        archive.value(stamina);
        archive.value(campfire_use_frames_remaining);
        archive.value(ability_flash_frames_remaining);
        archive.value(stamina_exhaustion_penalty_applied);
        archive.value(stamina_run_attempt_latched);
        archive.value(stamina_blink_timer);
        archive.value(stamina_blink_toggles_remaining);
        archive.value(stamina_blink_hidden);
        archive.value(energy_blink_timer);
        archive.value(energy_blink_toggles_remaining);
        archive.value(energy_blink_hidden);
        archive.value(damage_invulnerability_frames);
        archive.value(player_blink_timer);
        archive.value(player_blink_toggles_remaining);
        archive.value(player_blink_hidden);
        archive.value(enemy_contact_latched);
        archive.value(spike_contact_latched);
        archive.value(spike_contact_room_id);
        archive.value(scored_rooms);
        archive.value(bat_counts);
        archive.value(bat_start_x);
        archive.value(bat_start_y);
        archive.value(bat_start_dx);
        archive.value(bat_start_dy);
        archive.value(bat_start_animation_frames);
        archive.value(bat_start_animation_timers);
        archive.value(bat_room_x);
        archive.value(bat_room_y);
        archive.value(bat_room_dx);
        archive.value(bat_room_dy);
        archive.value(bat_room_animation_frames);
        archive.value(bat_room_animation_timers);
        archive.value(active_bat_count);
        archive.value(bat_x);
        archive.value(bat_y);
        archive.value(bat_dx);
        archive.value(bat_dy);
        archive.value(bat_animation_frames);
        archive.value(bat_animation_timers);
        archive.value(bat_displayed_frames);
        archive.value(bat_sound_cooldown_frames);
        archive.value(skeleton_sound_cooldown_frames);
        archive.value(skeleton_counts);
        archive.value(skeleton_start_corners);
        archive.value(skeleton_room_path_positions);
        archive.value(skeleton_room_path_initialized);
        archive.value(active_skeleton_count);
        archive.value(skeleton_path_positions);
        archive.value(skeleton_x);
        archive.value(skeleton_y);
        archive.value(skeleton_walk_frames);
        archive.value(skeleton_animation_timers);
        archive.value(skeleton_displayed_frames);
        archive.value(floor_key_room);
        archive.value(floor_key_x);
        archive.value(floor_key_y);
        archive.value(floor_key_collected);
        archive.value(special_key_room);
        archive.value(special_key_x);
        archive.value(special_key_y);
        archive.value(special_key_collected);
        archive.value(run_rooms_explored);
        unsigned random_states[4] = {floor_random.seed(), enemy_ambient_random.seed(),
            campfire_reward_random.seed(), reward_lever_random.seed()};
        archive.value(random_states);
        floor_random.set_seed(random_states[0]);
        enemy_ambient_random.set_seed(random_states[1]);
        campfire_reward_random.set_seed(random_states[2]);
        reward_lever_random.set_seed(random_states[3]);
        if(version == run_snapshot_version)
        {
            archive.value(imported_chest_frame);
            archive.value(imported_chest_target_frame);
            archive.value(imported_chest_animation_timer);
        }
        else
        {
            imported_chest_frame = 0;
            imported_chest_target_frame = 0;
            imported_chest_animation_timer = 0;
        }
        return archive.good();
    };

    auto loaded_run_valid = [&]()
    {
        int rooms = room_manager.room_count();
        if(!room_manager.saved_floor_valid() || piso_actual < 1 || run_rooms_explored < piso_actual ||
           x < -102 || x > 102 || y < -64 || y > 58 || energy <= 0 || energy > energy_capacity ||
           stamina < 0 || stamina > stamina_capacity_frames || key_count < 0 ||
           carried_ability_count < 0 || carried_ability_count > max_carried_abilities ||
           selected_ability_slot < 0 || selected_ability_slot >= max_carried_abilities ||
           carried_npc_index < -1 || carried_npc_index >= max_npcs ||
           follow_history_head < 0 || follow_history_head >= follow_history_capacity ||
           follow_history_count < 0 || follow_history_count > follow_history_capacity ||
           int(direccion) < 0 || int(direccion) > 3 || displayed_frame < 0 || displayed_frame >= 16 ||
           active_skeleton_count < 0 || active_skeleton_count > max_skeletons ||
           active_bat_count < 0 || active_bat_count > max_bats ||
           active_mushy_count < 0 || active_mushy_count > max_mushies ||
           active_plant_count < 0 || active_plant_count > max_plants ||
           imported_chest_frame < 0 || imported_chest_frame > 2 ||
           (imported_chest_target_frame != 0 && imported_chest_target_frame != 2) ||
           imported_chest_animation_timer < 0 || imported_chest_animation_timer >= 8)
            return false;
        for(int index = 0; index < carried_ability_count; ++index)
        {
            if(int(carried_abilities[index].ability) < 0 || int(carried_abilities[index].ability) >= 7 ||
               carried_abilities[index].level < 1 || carried_abilities[index].level > 5)
                return false;
        }
        for(int room_id = 0; room_id < rooms; ++room_id)
        {
            if(skeleton_counts[room_id] < 0 || skeleton_counts[room_id] > max_skeletons ||
               bat_counts[room_id] < 0 || bat_counts[room_id] > max_bats ||
               mushy_counts[room_id] < 0 || mushy_counts[room_id] > max_mushies ||
               plant_counts[room_id] < 0 || plant_counts[room_id] > max_plants ||
               spike_counts[room_id] < 0 || spike_counts[room_id] > max_room_spikes)
                return false;
            if(imported_room::contains(room_manager.room_template(room_id)))
            {
                if(skeleton_counts[room_id] > imported_room_for(room_id).enemy_count)
                    return false;
                for(int index = 0; index < skeleton_counts[room_id]; ++index)
                    if(skeleton_start_corners[room_id][index] < 0 ||
                       skeleton_start_corners[room_id][index] >= imported_room_for(room_id).enemy_count)
                        return false;
            }
            for(int index = 0; index < plant_counts[room_id]; ++index)
                if(plant_position_indices[room_id][index] < 0 || plant_position_indices[room_id][index] >= max_plants + 3)
                    return false;
            for(int index = 0; index < spike_counts[room_id]; ++index)
                if(spike_frames[room_id][index] < 0 || spike_frames[room_id][index] > 2)
                    return false;
        }
        for(int index = 0; index < 3; ++index)
            if(reward_lever_room_ids[index] < -1 || reward_lever_room_ids[index] >= rooms ||
               reward_lever_animation_frames[index] < 0 || reward_lever_animation_frames[index] > 2)
                return false;
        return true;
    };

    auto save_active_game = [&]()
    {
        if(room_manager.is_menu_floor() || energy <= 0 || !loaded_run_valid())
            return false;
        game_save::Writer writer;
        bool copied = transfer_run_state(writer);
        BN_ASSERT(copied && writer.size() == run_snapshot_bytes, "Incomplete active save");
        if(!copied || writer.size() != run_snapshot_bytes)
            return false;
        game_save::commit_run(writer.size());
        return true;
    };

    auto start_game = [&]()
    {
        bomb_flash_frames = 0;
        clear_placed_bomb();
        mushy_move_timer = 0;
        bn::bgs_mosaic::set_stretch(0);
        bn::sprites_mosaic::set_stretch(0);
        carried_ability_count = 0;
        selected_ability_slot = 0;
        active_ability = ActiveAbility::DASH;
        replacing_ability = false;
        familiar_active = false;
        familiar_world_sprite.set_visible(false);
        for (bool &room_bombed : bombed_rooms)
        {
            room_bombed = false;
        }
        update_active_ability_icon();
        run_seed = selected_seed;
        piso_actual = 1;
        run_rooms_explored = 0;
        reset_floor_randoms(piso_actual);
        campfire_use_frames_remaining = 0;
        bn::blending::set_fade_alpha(0);
        int generated_room_count = floor_random.get_int(5, RoomManager::max_rooms);
        if (!room_manager.generate_random_floor(floor_random, generated_room_count, true))
        {
            return false;
        }
        clear_campfire_reward();
        campfire_pending_rewards = 0;
        campfire_skill_reward_granted = false;
        setup_floor_skeletons();
        setup_floor_bats();
        setup_floor_mushies();
        setup_floor_plants();
        setup_floor_spikes();

        x = inicio_x;
        y = inicio_y;
        dash_active = false;
        movement_speed_remainder = 0;
        floor_map_revealed = false;
        time_stopped_room_id = -1;
        trapdoor_deployed = false;
        trapdoor_room_id = -1;
        trapdoor_world_sprite.set_visible(false);
        energy = starting_energy;
        stamina = stamina_capacity_frames;
        stamina_exhaustion_penalty_applied = false;
        stamina_run_attempt_latched = false;
        stamina_blink_timer = 0;
        stamina_blink_toggles_remaining = 0;
        stamina_blink_hidden = false;
        energy_blink_timer = 0;
        energy_blink_toggles_remaining = 0;
        energy_blink_hidden = false;
        update_stamina_bar_fill(stamina);
        stamina_bar_enabled = true;
        energy_bar_enabled = true;
        update_energy_bar_fill(energy);
        // Al iniciar o reiniciar el piso, restaura las dos barras juntas.
        // Esto tambien cubre el primer cuarto (tutorial), antes de actualizar
        // la visibilidad general del HUD desde el bucle principal.
        refresh_stamina_bar_visibility();
        refresh_energy_bar_visibility();
        damage_invulnerability_frames = 0;
        player_blink_timer = 0;
        player_blink_toggles_remaining = 0;
        player_blink_hidden = false;
        enemy_contact_latched = false;
        spike_contact_latched = false;
        spike_contact_room_id = -1;
        reset_floor_npcs(false);
        campfire_used_this_floor = false;
        campfire_frame = 0;
        campfire_animation_timer = 0;
        campfire.set_tiles(bn::sprite_items::campfire.tiles_item(), campfire_frame);
        key_count = 0;
        rescued_adventurers = 0;
        update_rescued_count_label();
        for (bool &room_scored : scored_rooms)
        {
            room_scored = false;
        }
        scored_rooms[room_manager.current_room_id()] = true;
        record_floor_started();

        lock_stairs_room_door();
        update_room_background();
        update_room_obstacles();
        update_stone_layers();
        place_floor_key();
        place_special_key();
        place_floor_potions();
        setup_reward_room_rewards();
        setup_reward_levers();
        update_floor_item_visibility();
        update_door_sprites();
        stairs.set_visible(room_manager.current_room_id() == room_manager.stairs_room_id());

        room_name_sprites.clear();
        room_name_generator.generate(112, -68, room_display_name(room_manager.current_room_id()),
                                     room_name_sprites);
        refresh_map_labels_visibility();
        update_floor_label();
        update_map_seed_label();
        update_energy_label();
        update_key_count_label();
        update_map();

        player.set_position(x, y);
        update_npc_sprites(displayed_frame);
        player.set_visible(true);
        if (key_icon.has_value())
        {
            key_icon->set_visible(false);
        }
        for (bn::sprite_ptr &sprite : title_name_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : title_credit_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : title_seed_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : title_edit_seed_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : title_enter_sprites)
            sprite.set_visible(false);
        // La pantalla de titulo ya no se usa durante la partida. Liberar sus
        // sprites deja espacio para el mapa y sus conexiones.
        title_name_sprites.clear();
        title_credit_sprites.clear();
        title_seed_sprites.clear();
        title_edit_seed_sprites.clear();
        title_enter_sprites.clear();
        return save_active_game();
    };

    auto update_player_animation = [&](bool moving, bool dashing)
    {
        // Primer fotograma de cada dirección:
        // quieto: 0, 4, 8, 12.
        int idle_frame = 0;
        int walk_start = 1;

        switch (direccion)
        {
        case Direccion::ABAJO:
            idle_frame = 0;
            walk_start = 1;
            break;

        case Direccion::ARRIBA:
            idle_frame = 4;
            walk_start = 5;
            break;

        case Direccion::IZQUIERDA:
            idle_frame = 8;
            walk_start = 9;
            break;

        case Direccion::DERECHA:
            idle_frame = 12;
            walk_start = 13;
            break;

        default:
            // Respaldo por si direccion recibe un valor inesperado.
            idle_frame = 0;
            walk_start = 1;
            break;
        }

        int target_frame = idle_frame;

        if (moving)
        {
            animation_timer++;

            if (animation_timer >= animation_speed)
            {
                animation_timer = 0;
                // Usa solo los cuadros de caminata 1 y 2 de cada direccion;
                // el tercero se omite en las cuatro filas de animacion.
                frame = (frame + 1) % 2;
                if (! dashing)
                {
                    bn::sound_items::step.play();
                }
            }

            target_frame = walk_start + frame;
        }
        else
        {
            animation_timer = 0;
            frame = 0;
        }

        // Animacion ===================
        // Cambiar el gráfico solo cuando sea necesario.
        if (target_frame != displayed_frame)
        {
            player.set_tiles(
                bn::sprite_items::player.tiles_item(),
                target_frame);

            displayed_frame = target_frame;
        }

    };

    auto enter_menu_floor = [&]()
    {
        clear_inventory_screen();
        clear_options_menu();
        clear_placed_bomb();
        clear_campfire_reward();
        map_bg.reset();
        map_room_sprites.clear();
        map_link_sprites.clear();
        map_link_current_sprites.clear();
        map_link_special_sprites.clear();
        map_seed_sprites.clear();
        reward_room_reward_sprites.clear();
        reward_lever_sprites.clear();
        reward_lever_sprite_index = -1;
        reward_room_unlocked = false;
        reward_map_blinking = false;
        for(int index = 0; index < 3; ++index)
        {
            reward_lever_room_ids[index] = -1;
            reward_lever_activated[index] = false;
        }
        for(RewardRoomReward &reward : reward_room_rewards)
            reward = RewardRoomReward();
        for(FloorPotionState &potion : floor_potions)
            potion = FloorPotionState();
        for(NpcState &npc : npcs)
            npc = NpcState();
        for(bn::sprite_ptr &sprite : npc_sprites)
            sprite.set_visible(false);
        for(int room_id = 0; room_id < RoomManager::max_rooms; ++room_id)
        {
            skeleton_counts[room_id] = 0;
            bat_counts[room_id] = 0;
            mushy_counts[room_id] = 0;
            plant_counts[room_id] = 0;
            spike_counts[room_id] = 0;
            bombed_rooms[room_id] = false;
        }
        carried_npc_index = -1;
        carried_ability_count = 0;
        replacing_ability = false;
        familiar_active = false;
        familiar_world_sprite.set_visible(false);
        trapdoor_deployed = false;
        trapdoor_world_sprite.set_visible(false);
        floor_key_collected = true;
        special_key_collected = true;
        floor_key_room = -1;
        special_key_room = -1;
        floor_key_sprite.set_visible(false);
        special_key_sprite.set_visible(false);
        energy_potion_sprite.set_visible(false);
        sprint_potion_sprite.set_visible(false);
        campfire_pending_rewards = 0;
        campfire_use_frames_remaining = 0;
        ability_flash_frames_remaining = 0;
        bomb_flash_frames = 0;
        bn::blending::set_fade_alpha(0);
        bn::bgs_mosaic::set_stretch(0);
        bn::sprites_mosaic::set_stretch(0);
        game_started = true;
        game_over = false;
        ui_abierta = false;
        inventory_open = false;
        ability_data_open = false;
        options_open = false;
        seed_editor_open = false;
        pantalla_actual = Pantalla::MENU;
        piso_actual = 1;
        energy = starting_energy;
        stamina = stamina_capacity_frames;
        stamina_bar_enabled = false;
        energy_bar_enabled = false;
        release_hud_sprites();
        set_main_title_visible(false);
        title_name_sprites.clear();
        title_credit_sprites.clear();
        title_seed_sprites.clear();
        title_edit_seed_sprites.clear();
        title_enter_sprites.clear();
        menu_text_sprites.clear();
        room_name_sprites.clear();
        game_over_text_sprites.clear();
        inventory_rescued_icon.set_visible(false);
        refresh_main_hud_visibility();

        room_manager.generate_menu_floor();
        x = inicio_x;
        y = inicio_y;
        dash_active = false;
        movement_speed_remainder = 0;
        direccion = Direccion::ABAJO;
        frame = 0;
        animation_timer = 0;
        animation_speed = 16;
        displayed_frame = 0;
        player.set_tiles(bn::sprite_items::player.tiles_item(), 0);
        update_room_background();
        update_room_obstacles();
        update_stone_layers();
        update_door_sprites();
        stairs.set_visible(false);
        player.set_position(x, y);
        player.set_visible(true);
        set_start_transition_darkness(0);
    };

    auto update_menu_floor = [&]()
    {
        int previous_x = x;
        int previous_y = y;
        bool sprinting = bn::keypad::b_held();
        int movement_units = sprinting ? 8 : 5;
        int accumulated_units = movement_speed_remainder + movement_units;
        int movement_pixels = accumulated_units / 4;
        animation_speed = sprinting ? 8 : 16;
        if(bn::keypad::down_held())
        {
            direccion = Direccion::ABAJO;
            y += movement_pixels;
        }
        else if(bn::keypad::up_held())
        {
            direccion = Direccion::ARRIBA;
            y -= movement_pixels;
        }
        else if(bn::keypad::left_held())
        {
            direccion = Direccion::IZQUIERDA;
            x -= movement_pixels;
        }
        else if(bn::keypad::right_held())
        {
            direccion = Direccion::DERECHA;
            x += movement_pixels;
        }
        if(x < -102) x = -102;
        if(x > 102) x = 102;
        if(y < -64) y = -64;
        if(y > 58) y = 58;
        if(player_position_blocked(x, y))
        {
            x = previous_x;
            y = previous_y;
        }
        bool moving = x != previous_x || y != previous_y;
        if(moving)
            movement_speed_remainder = accumulated_units % 4;
        update_player_animation(moving, false);
        player.set_position(x, y);
        update_menu_chest();
        update_stone_layers();

        const RoomDirection directions[] = {
            RoomDirection::LEFT, RoomDirection::RIGHT, RoomDirection::UP, RoomDirection::DOWN};
        for(RoomDirection direction : directions)
        {
            if(room_manager.door_is_open(direction) && room_manager.touching_door(x, y, direction))
            {
                bn::sound_items::enter_door.play();
                fade_room_transition(true);
                int spawn_x = x;
                int spawn_y = y;
                if(room_manager.try_transition(x, y, spawn_x, spawn_y))
                {
                    x = spawn_x;
                    y = spawn_y;
                    update_room_background();
                    update_room_obstacles();
                    update_stone_layers();
                    update_door_sprites();
                    stairs.set_visible(room_manager.current_room_id() == room_manager.stairs_room_id());
                    player.set_position(x, y);
                }
                fade_room_transition(false);
                break;
            }
        }
        return room_manager.current_room_id() == room_manager.stairs_room_id() &&
            x >= -10 && x <= 10 && y >= -18 && y <= 2;
    };

    auto create_saved_reward_sprite = [&](RoomItemType type, int sprite_x, int sprite_y)
    {
        const bn::sprite_item *item = &bn::sprite_items::key;
        int sprite_frame = 0;
        if(type == RoomItemType::ENERGY_POTION)
        {
            item = &bn::sprite_items::posion_energy_16x16_animate;
            sprite_frame = potion_animation_frame;
        }
        else if(type == RoomItemType::SPRINT_POTION)
        {
            item = &bn::sprite_items::posion_sprint_16x16_animate;
            sprite_frame = potion_animation_frame;
        }
        else if(int(type) >= int(RoomItemType::ABILITY_DASH))
            item = &bn::sprite_items::hability_dash_icon;
        bn::sprite_ptr sprite = item->create_sprite(sprite_x, sprite_y, sprite_frame);
        if(int(type) >= int(RoomItemType::ABILITY_DASH))
            set_ability_icon(sprite, static_cast<ActiveAbility>(int(type) - int(RoomItemType::ABILITY_DASH)));
        sprite.set_bg_priority(1);
        sprite.set_z_order(-2);
        sprite.set_visible(false);
        return sprite;
    };

    auto restore_game = [&]()
    {
        if(!game_save::has_run() ||
           (game_save::run_size() != run_snapshot_bytes && game_save::run_size() != run_snapshot_bytes_v1))
        {
            game_save::clear_run();
            return false;
        }
        game_save::Reader first_pass;
        if(!transfer_run_state(first_pass) || !first_pass.finished() || !loaded_run_valid())
        {
            game_save::clear_run();
            return false;
        }
        game_started = true;
        game_over = false;
        ui_abierta = false;
        inventory_open = false;
        ability_data_open = false;
        options_open = false;
        map_bg.reset();
        update_room_background();
        update_room_obstacles();
        // El constructor de sprites reinicia timers, proyectiles y algunas tablas.
        // Una segunda lectura recupera el estado exacto sin volver a sortear el piso.
        game_save::Reader second_pass;
        if(!transfer_run_state(second_pass) || !second_pass.finished() || !loaded_run_valid())
        {
            game_save::clear_run();
            return false;
        }
        selected_seed = run_seed;
        pantalla_actual = Pantalla::MAPA;
        game_started = true;
        game_over = false;
        ui_abierta = false;
        inventory_open = false;
        ability_data_open = false;
        options_open = false;
        replacing_ability = false;
        reward_room_replacement_index = -1;
        trapdoor_transitioned_this_frame = false;
        stamina_bar_enabled = true;
        energy_bar_enabled = true;
        map_current_marker_hidden = false;
        map_current_blink_toggles_remaining = 0;
        bn::blending::set_fade_alpha(0);
        bn::bgs_mosaic::set_stretch(0);
        bn::sprites_mosaic::set_stretch(0);

        for(int index = 0; index < skeleton_sprites.size(); ++index)
        {
            skeleton_sprites[index].set_position(skeleton_x[index], skeleton_y[index]);
            if(skeleton_displayed_frames[index] >= 0)
                skeleton_sprites[index].set_tiles(bn::sprite_items::skeleton.tiles_item(), skeleton_displayed_frames[index]);
        }
        for(int index = 0; index < bat_sprites.size(); ++index)
        {
            bat_sprites[index].set_position(bat_x[index], bat_y[index]);
            if(bat_displayed_frames[index] >= 0)
                bat_sprites[index].set_tiles(bn::sprite_items::bat.tiles_item(), bat_displayed_frames[index]);
        }
        int current_room = room_manager.current_room_id();
        for(int index = 0; index < mushy_sprites.size(); ++index)
        {
            mushy_sprites[index].set_position(mushy_room_x[current_room][index], mushy_room_y[current_room][index]);
            int sprite_frame = mushy_direction_frames[current_room][index] +
                (mushy_walking[current_room][index] ? mushy_animation_frame : 0);
            mushy_sprites[index].set_tiles(bn::sprite_items::mushy.tiles_item(), sprite_frame);
        }
        for(int index = 0; index < plant_sprites.size(); ++index)
            if(plant_displayed_frames[index] >= 0)
                plant_sprites[index].set_tiles(bn::sprite_items::plant_enemy.tiles_item(), plant_displayed_frames[index]);
        for(int index = 0; index < plant_bullet_sprites.size(); ++index)
        {
            plant_bullet_sprites[index].set_position(plant_bullets[index].x, plant_bullets[index].y);
            plant_bullet_sprites[index].set_visible(plant_bullets[index].active);
        }
        campfire.set_tiles(bn::sprite_items::campfire.tiles_item(), campfire_frame);
        if(imported_chest_sprite_index >= 0)
            imported_object_sprites[imported_chest_sprite_index].set_tiles(
                bn::sprite_items::cheest_16x16_animate_3frames.tiles_item(), imported_chest_frame);
        reward_room_reward_sprites.clear();
        for(const RewardRoomReward &reward : reward_room_rewards)
            reward_room_reward_sprites.push_back(create_saved_reward_sprite(reward.type, reward.x, reward.y));
        campfire_reward_sprite.reset();
        if(campfire_reward_active)
            campfire_reward_sprite.emplace(create_saved_reward_sprite(campfire_reward_type,
                campfire_reward_x, campfire_reward_y));
        placed_bomb_sprite.reset();
        if(pending_bomb_frames > 0)
        {
            placed_bomb_sprite.emplace(bn::sprite_items::hability_bomb_icon.create_sprite(pending_bomb_x, pending_bomb_y));
            placed_bomb_sprite->set_bg_priority(1);
            placed_bomb_sprite->set_z_order(-2);
            placed_bomb_sprite->set_visible(pending_bomb_room_id == current_room && bomb_blink_visible);
        }
        familiar_world_sprite.set_position(familiar_x, familiar_y);
        familiar_world_sprite.set_visible(familiar_active && familiar_room_id == current_room);
        floor_key_sprite.set_position(floor_key_x, floor_key_y);
        floor_key_sprite.set_visible(!floor_key_collected && floor_key_room == current_room);
        special_key_sprite.set_position(special_key_x, special_key_y);
        special_key_sprite.set_visible(!special_key_collected && special_key_room == current_room);
        player.set_tiles(bn::sprite_items::player.tiles_item(), displayed_frame);
        player.set_position(x, y);
        player.set_visible(!player_blink_hidden);
        update_door_sprites();
        stairs.set_visible(current_room == room_manager.stairs_room_id());
        update_floor_item_visibility();
        update_reward_room_reward_visibility();
        update_campfire_reward_visibility();
        update_reward_lever_visibility();
        update_trapdoor_visibility();
        update_npc_sprites(displayed_frame);
        update_stone_layers();
        create_hud_sprites();
        update_active_ability_icon();
        update_floor_label();
        update_map_seed_label();
        update_key_count_label();
        update_rescued_count_label();
        update_energy_bar_fill(energy);
        update_stamina_bar_fill(stamina);
        room_name_sprites.clear();
        room_name_generator.generate(112, -68, room_display_name(current_room), room_name_sprites);
        update_map();
        refresh_main_hud_visibility();
        return true;
    };

    auto return_to_title = [&]()
    {
        clear_inventory_screen();
        map_seed_sprites.clear();
        bomb_flash_frames = 0;
        clear_placed_bomb();
        bn::bgs_mosaic::set_stretch(0);
        bn::sprites_mosaic::set_stretch(0);
        campfire_use_frames_remaining = 0;
        bn::blending::set_fade_alpha(0);
        clear_campfire_reward();
        campfire_pending_rewards = 0;
        campfire_skill_reward_granted = false;
        game_started = false;
        options_open = false;
        ui_abierta = false;
        inventory_open = false;
        stamina_bar_enabled = false;
        energy_bar_enabled = false;
        inventory_rescued_icon.set_visible(false);
        inventory_rescued_icon.set_position(inventory_rescued_x, inventory_rescued_y);
        refresh_stamina_bar_visibility();
        refresh_energy_bar_visibility();
        refresh_main_hud_visibility();
        clear_options_menu();
        set_scene_darkness(0);
        title_text_palette.set_fade(bn::colors::black, 0);
        update_map();
        room_special_bg.reset();
        reward_room_bg.reset();
        room2_bg.reset();
        room3_bg.reset();
        land.set_visible(true);
        player.set_visible(false);
        for (bn::sprite_ptr &sprite : skeleton_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : bat_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : mushy_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : spike_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : stone_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : campfire_columns)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : campfire_libraries)
            sprite.set_visible(false);
        stone_sprites.clear();
        campfire_columns.clear();
        campfire_libraries.clear();
        skeleton_sprites.clear();
        bat_sprites.clear();
        mushy_sprites.clear();
        spike_sprites.clear();
        composition_4_boxes.clear();
        room_door_sprites.clear();
        if (key_icon.has_value())
        {
            key_icon->set_visible(false);
        }
        floor_key_sprite.set_visible(false);
        special_key_sprite.set_visible(false);
        update_floor_item_visibility();
        stairs.set_visible(false);
        campfire.set_visible(false);
        for (bn::sprite_ptr &sprite : npc_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : room_name_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : key_count_text_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : floor_text_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : menu_text_sprites)
            sprite.set_visible(false);
        enter_menu_floor();
    };

    auto show_game_over = [&]()
    {
        clear_inventory_screen();
        clear_placed_bomb();
        campfire_use_frames_remaining = 0;
        bn::blending::set_fade_alpha(0);
        clear_campfire_reward();
        campfire_pending_rewards = 0;
        game_over = true;
        ++game_save::records().deaths;
        game_save::clear_run();
        select_new_seed();
        map_seed_sprites.clear();
        ui_abierta = false;
        inventory_open = false;
        options_open = false;
        ability_panel.set_visible(false);
        ability_icon.set_visible(false);
        trapdoor_world_sprite.set_visible(false);
        stamina_bar_enabled = false;
        energy_bar_enabled = false;
        refresh_stamina_bar_visibility();
        refresh_energy_bar_visibility();
        clear_options_menu();
        update_map();
        player.set_visible(false);
        for (bn::sprite_ptr &sprite : skeleton_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : bat_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : mushy_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : spike_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : stone_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : campfire_columns)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : campfire_libraries)
            sprite.set_visible(false);
        stone_sprites.clear();
        campfire_columns.clear();
        campfire_libraries.clear();
        skeleton_sprites.clear();
        bat_sprites.clear();
        mushy_sprites.clear();
        spike_sprites.clear();
        composition_4_boxes.clear();
        room_door_sprites.clear();
        if (key_icon.has_value())
        {
            key_icon->set_visible(false);
        }
        floor_key_sprite.set_visible(false);
        special_key_sprite.set_visible(false);
        update_floor_item_visibility();
        stairs.set_visible(false);
        campfire.set_visible(false);
        for (bn::sprite_ptr &sprite : npc_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : room_name_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : key_count_text_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : floor_text_sprites)
            sprite.set_visible(false);
        for (bn::sprite_ptr &sprite : menu_text_sprites)
            sprite.set_visible(false);
        set_main_title_visible(false);
        title_text_palette.set_fade(bn::colors::black, 0);
        game_over_text_sprites.clear();
        title_text_generator.generate(0, -36, "GAME OVER", game_over_text_sprites);

        inventory_rescued_icon.set_position(-32, 2);
        inventory_rescued_icon.set_z_order(-13);
        inventory_player_palette.set_fade(bn::colors::black, 0);
        inventory_rescued_icon.set_visible(true);

        char rescued_label[12] = "x";
        int rescued_digits[10];
        int rescued_digit_count = 0;
        int rescued_value = rescued_adventurers;
        do
        {
            rescued_digits[rescued_digit_count++] = rescued_value % 10;
            rescued_value /= 10;
        } while (rescued_value > 0 && rescued_digit_count < 10);
        int rescued_label_index = 1;
        while (rescued_digit_count > 0)
        {
            rescued_label[rescued_label_index++] = char('0' + rescued_digits[--rescued_digit_count]);
        }
        rescued_label[rescued_label_index] = '\0';
        title_text_generator.generate(0, 2, rescued_label, game_over_text_sprites);

        char floors_label[20];
        const char floors_prefix[] = "FLOORS ";
        int floors_label_index = 0;
        while (floors_prefix[floors_label_index] != '\0')
        {
            floors_label[floors_label_index] = floors_prefix[floors_label_index];
            ++floors_label_index;
        }
        int floor_digits[10];
        int floor_digit_count = 0;
        int floor_value = piso_actual;
        do
        {
            floor_digits[floor_digit_count++] = floor_value % 10;
            floor_value /= 10;
        } while (floor_value > 0 && floor_digit_count < 10);
        while (floor_digit_count > 0)
        {
            floors_label[floors_label_index++] = char('0' + floor_digits[--floor_digit_count]);
        }
        floors_label[floors_label_index] = '\0';
        title_text_generator.generate(0, 30, floors_label, game_over_text_sprites);

        for (int step = 1; step <= 12; ++step)
        {
            set_scene_darkness(step / 12.0);
            bn::core::update();
        }
    };

    auto generate_next_floor = [&]()
    {
        clear_placed_bomb();
        constexpr int fade_frames = 30;
        constexpr int title_frames = 60;
        reset_floor_randoms(piso_actual + 1);
        int generated_room_count = floor_random.get_int(5, RoomManager::max_rooms);
        for (int step = 1; step <= fade_frames; ++step)
        {
            set_scene_darkness(step / float(fade_frames));
            restore_status_bar_colors();
            bn::core::update();
        }

        if (!room_manager.generate_random_floor(floor_random, generated_room_count))
        {
            bn::fixed base_darkness = ui_abierta ? bn::fixed(0.8) : bn::fixed(0);
            for (int step = fade_frames - 1; step >= 0; --step)
            {
                bn::fixed progress = bn::fixed(step) / fade_frames;
                set_scene_darkness(base_darkness + (1 - base_darkness) * progress);
                restore_status_bar_colors();
                bn::core::update();
            }
            return;
        }
        floor_map_revealed = false;
        time_stopped_room_id = -1;
        trapdoor_deployed = false;
        trapdoor_room_id = -1;
        trapdoor_world_sprite.set_visible(false);
        clear_campfire_reward();
        campfire_pending_rewards = 0;
        campfire_skill_reward_granted = false;

        setup_floor_skeletons();
        setup_floor_bats();
        setup_floor_mushies();
        setup_floor_plants();
        setup_floor_spikes();

        stamina = stamina_capacity_frames;
        dash_active = false;
        movement_speed_remainder = 0;
        stamina_exhaustion_penalty_applied = false;
        stamina_run_attempt_latched = false;
        stamina_blink_timer = 0;
        stamina_blink_toggles_remaining = 0;
        stamina_blink_hidden = false;
        energy_blink_timer = 0;
        energy_blink_toggles_remaining = 0;
        energy_blink_hidden = false;
        update_stamina_bar_fill(stamina);

        reset_floor_npcs(true);
        if (carried_npc_index >= 0)
        {
            reset_follow_history();
        }

        for (FloorPotionState &potion : floor_potions)
        {
            potion = FloorPotionState();
        }

        lock_stairs_room_door();
        update_room_obstacles();
        update_stone_layers();
        place_floor_key();
        place_special_key();
        place_floor_potions();
        setup_reward_room_rewards();
        setup_reward_levers();
        update_floor_item_visibility();

        ++piso_actual;
        campfire_used_this_floor = false;
        campfire_frame = 0;
        campfire_animation_timer = 0;
        campfire.set_tiles(bn::sprite_items::campfire.tiles_item(), campfire_frame);
        for (bool &room_scored : scored_rooms)
        {
            room_scored = false;
        }
        scored_rooms[room_manager.current_room_id()] = true;
        record_floor_started();
        if (energy > 0)
        {
            --energy;
        }
        update_energy_label();
        update_energy_bar_fill(energy);
        if (energy < 5) start_energy_recovery_flash();
        direccion = Direccion::ABAJO;
        frame = 0;
        animation_timer = 0;

        update_room_background();
        update_door_sprites();
        stairs.set_visible(room_manager.current_room_id() == room_manager.stairs_room_id());
        update_npc_sprites(displayed_frame);

        room_name_sprites.clear();
        room_name_generator.generate(112, -68, room_display_name(room_manager.current_room_id()),
                                     room_name_sprites);
        refresh_map_labels_visibility();
        update_floor_label();
        update_map();

        char transition_floor_label[16];
        create_floor_label(piso_actual, transition_floor_label);
        title_text_palette.set_fade(bn::colors::black, 0);
        floor_transition_text_sprites.clear();
        title_text_generator.generate(0, 0, transition_floor_label, floor_transition_text_sprites);
        for (int step = 0; step < title_frames; ++step)
        {
            bn::core::update();
        }

        bn::fixed base_darkness = ui_abierta ? bn::fixed(0.8) : bn::fixed(0);
        for (int step = fade_frames - 1; step >= 0; --step)
        {
            bn::fixed progress = bn::fixed(step) / fade_frames;
            set_scene_darkness(base_darkness + (1 - base_darkness) * progress);
            restore_status_bar_colors();
            bn::core::update();
        }
        floor_transition_text_sprites.clear();
        title_text_palette.set_fade(bn::colors::black, 0);
        update_stamina_bar_fill(stamina);
        update_energy_bar_fill(energy);
        refresh_stamina_bar_visibility();
        refresh_energy_bar_visibility();
        save_active_game();
    };

    auto teleport_through_trapdoor = [&](int destination_room_id, int destination_x, int destination_y)
    {
        int previous_room_id = room_manager.current_room_id();
        bn::sound_items::enter_door.play();
        fade_room_transition(true);
        if (!room_manager.set_current_room(destination_room_id))
        {
            update_reward_lever_visibility();
            fade_room_transition(false);
            return;
        }
        trapdoor_transitioned_this_frame = true;

        if (time_stopped_room_id == previous_room_id && previous_room_id != destination_room_id)
        {
            time_stopped_room_id = -1;
        }
        int destination_room = room_manager.current_room_id();
        bool first_visit = !scored_rooms[destination_room];
        if (first_visit)
        {
            scored_rooms[destination_room] = true;
            record_room_explored();
        }
        if (first_visit || carried_npc_index >= 0)
        {
            if (energy > 0)
            {
                --energy;
            }
            update_energy_label();
            update_energy_bar_fill(energy);
            if (energy < 5) start_energy_recovery_flash();
        }
        x = destination_x;
        y = destination_y;
        dash_active = false;
        movement_speed_remainder = 0;
        if (carried_npc_index >= 0)
        {
            reset_follow_history_at_player();
        }

        if (room_manager.current_template() == RoomTemplateId::SPECIAL && carried_npc_index >= 0)
        {
            NpcState &npc = npcs[carried_npc_index];
            npc.delivered = true;
            npc.room_id = destination_room;
            carried_npc_index = -1;
            ++rescued_adventurers;
            ++game_save::records().adventurers_saved;
            update_rescued_count_label();
            spawn_campfire_reward();
            energy += 3;
            if (energy > energy_capacity)
            {
                energy = energy_capacity;
            }
            update_energy_label();
            start_energy_recovery_flash();
        }

        update_room_background();
        update_room_obstacles();
        update_stone_layers();
        update_door_sprites();
        floor_key_sprite.set_visible(!floor_key_collected &&
                                     room_manager.current_room_id() == floor_key_room);
        special_key_sprite.set_visible(!special_key_collected &&
                                       room_manager.current_room_id() == special_key_room);
        update_floor_item_visibility();
        update_reward_room_reward_visibility();
        update_campfire_reward_visibility();
        stairs.set_visible(room_manager.current_room_id() == room_manager.stairs_room_id());
        update_npc_sprites(displayed_frame);
        room_name_sprites.clear();
        room_name_generator.generate(112, -68, room_display_name(room_manager.current_room_id()),
                                     room_name_sprites);
        refresh_map_labels_visibility();
        update_trapdoor_visibility();
        update_map();
        player.set_position(x, y);
        update_reward_lever_visibility();
        fade_room_transition(false);
        start_ability_flash();
    };

    // Movimiento ---------------------

    // Prioridades
    land.set_priority(3);
    // clouds.set_priority(2);

    // Prioridad 1 igual que las puertas; z_order menor lo dibuja delante de ellas.
    player.set_bg_priority(1);
    player.set_z_order(-1);
    key_icon->set_bg_priority(0);
    key_icon->set_z_order(-12);
    floor_key_sprite.set_bg_priority(1);
    special_key_sprite.set_bg_priority(1);
    energy_potion_sprite.set_bg_priority(1);
    sprint_potion_sprite.set_bg_priority(1);
    energy_potion_sprite.set_z_order(-2);
    sprint_potion_sprite.set_z_order(-2);

    land.set_visible(true);
    player.set_visible(false);
    for (bn::sprite_ptr &sprite : skeleton_sprites)
        sprite.set_visible(false);
    for (bn::sprite_ptr &sprite : bat_sprites)
        sprite.set_visible(false);
    for (bn::sprite_ptr &sprite : mushy_sprites)
        sprite.set_visible(false);
    for (bn::sprite_ptr &sprite : stone_sprites)
        sprite.set_visible(false);
    for (bn::sprite_ptr &sprite : campfire_columns)
        sprite.set_visible(false);
    for (bn::sprite_ptr &sprite : campfire_libraries)
        sprite.set_visible(false);
    for (bn::sprite_ptr &sprite : composition_4_boxes)
        sprite.set_visible(false);
    stone_sprites.clear();
    campfire_columns.clear();
    campfire_libraries.clear();
    skeleton_sprites.clear();
    bat_sprites.clear();
    mushy_sprites.clear();
    composition_4_boxes.clear();
    room_door_sprites.clear();
    if (key_icon.has_value())
    {
        key_icon->set_visible(false);
    }
    floor_key_sprite.set_visible(false);
    special_key_sprite.set_visible(false);
    update_floor_item_visibility();
    stairs.set_visible(false);
    for (bn::sprite_ptr &sprite : room_name_sprites)
        sprite.set_visible(false);
    for (bn::sprite_ptr &sprite : key_count_text_sprites)
        sprite.set_visible(false);
    for (bn::sprite_ptr &sprite : floor_text_sprites)
        sprite.set_visible(false);
    enter_menu_floor();
    for(int step = logo_transition_half_frames - 1; step >= 0; --step)
    {
        set_start_transition_darkness(bn::fixed(step) / logo_transition_half_frames);
        bn::core::update();
    }
    while (true)
    {
        // El piso de menu solo permite caminar, cambiar de cuarto y usar las escaleras.
        if(room_manager.is_menu_floor())
        {
            if(update_menu_floor())
            {
                bn::sound_items::stairs.play();
                fade_room_transition(true);
                create_hud_sprites();
                if(game_save::has_run() ? restore_game() : start_game())
                {
                    pantalla_actual = Pantalla::MAPA;
                    update_floor_item_visibility();
                    refresh_main_hud_visibility();
                }
                else
                {
                    enter_menu_floor();
                }
                fade_room_transition(false);
            }
            bn::core::update();
            continue;
        }

        trapdoor_transitioned_this_frame = false;
        bool campfire_effect_active_this_frame = campfire_use_frames_remaining > 0;
        if (campfire_effect_active_this_frame)
        {
            int elapsed_frames = campfire_use_duration_frames - campfire_use_frames_remaining;
            bn::blending::set_white_fade_color();
            bn::blending::set_fade_alpha((elapsed_frames / 5) % 2 == 0 ? bn::fixed(1) : bn::fixed(0));
            --campfire_use_frames_remaining;
            if (campfire_use_frames_remaining == 0)
            {
                bn::blending::set_fade_alpha(0);
            }
        }
        else if (bomb_flash_frames > 0)
        {
            bn::blending::set_white_fade_color();
            bn::blending::set_fade_alpha((bomb_flash_frames / 2) % 2 == 0 ? bn::fixed(1) : bn::fixed(0));
            int elapsed_frames = bomb_flash_duration_frames - bomb_flash_frames;
            int mosaic_step = elapsed_frames % 8;
            int mosaic_size = mosaic_step <= 4 ? mosaic_step : 8 - mosaic_step;
            bn::fixed mosaic_stretch = bn::fixed(mosaic_size) / 6;
            bn::bgs_mosaic::set_stretch(mosaic_stretch);
            bn::sprites_mosaic::set_stretch(mosaic_stretch);
            --bomb_flash_frames;
            if (bomb_flash_frames == 0)
            {
                bn::blending::set_fade_alpha(0);
                bn::bgs_mosaic::set_stretch(0);
                bn::sprites_mosaic::set_stretch(0);
            }
        }
        else if (ability_flash_frames_remaining > 0)
        {
            bn::blending::set_white_fade_color();
            bn::blending::set_fade_alpha(
                ability_flash_frames_remaining > ability_flash_duration_frames / 2 ?
                    bn::fixed(1) : bn::fixed(0));
            --ability_flash_frames_remaining;
            if (ability_flash_frames_remaining == 0)
            {
                bn::blending::set_fade_alpha(0);
            }
        }

        if (!game_started)
        {
            if (!seed_editor_open)
            {
                ++start_prompt_blink_timer;
                if (start_prompt_blink_timer >= 30)
                {
                    start_prompt_blink_timer = 0;
                    start_prompt_visible = !start_prompt_visible;
                    for (bn::sprite_ptr &sprite : title_enter_sprites)
                    {
                        sprite.set_visible(start_prompt_visible);
                    }
                }

                if (bn::keypad::select_pressed())
                {
                    open_seed_editor();
                }
                else if (bn::keypad::start_pressed())
                {
                    for (int step = 1; step <= 12; ++step)
                    {
                        set_start_transition_darkness(step / 12.0);
                        bn::core::update();
                    }

                    create_hud_sprites();
                    if (start_game())
                    {
                        game_started = true;
                        update_floor_item_visibility();
                        refresh_main_hud_visibility();
                    }

                    for (int step = 11; step >= 0; --step)
                    {
                        set_start_transition_darkness(step / 12.0);
                        bn::core::update();
                    }
                }
            }
            else
            {
                ++seed_blink_timer;
                if (seed_blink_timer >= 30)
                {
                    seed_blink_timer = 0;
                    seed_digit_visible = !seed_digit_visible;
                    update_seed_editor_selection();
                }

                if (bn::keypad::left_pressed())
                {
                    seed_digit_cursor = (seed_digit_cursor + 9) % 10;
                    seed_blink_timer = 0;
                    seed_digit_visible = true;
                    update_seed_editor_digits();
                }
                else if (bn::keypad::right_pressed())
                {
                    seed_digit_cursor = (seed_digit_cursor + 1) % 10;
                    seed_blink_timer = 0;
                    seed_digit_visible = true;
                    update_seed_editor_digits();
                }

                if (bn::keypad::up_pressed())
                {
                    seed_digits[seed_digit_cursor] = (seed_digits[seed_digit_cursor] + 1) % 10;
                    update_seed_editor_digits();
                }
                else if (bn::keypad::down_pressed())
                {
                    seed_digits[seed_digit_cursor] = (seed_digits[seed_digit_cursor] + 9) % 10;
                    update_seed_editor_digits();
                }

                if (bn::keypad::select_pressed())
                {
                    select_new_seed();
                    close_seed_editor();
                }
                else if (bn::keypad::start_pressed())
                {
                    unsigned edited_seed = 0;
                    if(read_edited_seed(edited_seed))
                    {
                        selected_seed = edited_seed;
                        close_seed_editor();
                    }
                }
            }

            bn::core::update();
            continue;
        }

        if (game_over)
        {
            if (bn::keypad::a_pressed() || bn::keypad::b_pressed() ||
                bn::keypad::start_pressed() || bn::keypad::select_pressed() ||
                bn::keypad::l_pressed() || bn::keypad::r_pressed() ||
                bn::keypad::up_pressed() || bn::keypad::down_pressed() ||
                bn::keypad::left_pressed() || bn::keypad::right_pressed())
            {
                game_over = false;
                game_over_text_sprites.clear();
                return_to_title();
                set_start_transition_darkness(1);
                for (int step = 11; step >= 0; --step)
                {
                    set_start_transition_darkness(step / 12.0);
                    bn::core::update();
                }
            }

            bn::core::update();
            continue;
        }

        if (options_open)
        {
            if (bn::keypad::b_pressed() || bn::keypad::start_pressed())
            {
                close_options_menu();
                create_hud_sprites();
            }
            else if (bn::keypad::up_pressed())
            {
                options_selection = (options_selection + 2) % 3;
                update_options_cursor();
            }
            else if (bn::keypad::down_pressed())
            {
                options_selection = (options_selection + 1) % 3;
                update_options_cursor();
            }
            else if (bn::keypad::a_pressed())
            {
                if (options_selection == 0)
                {
                    close_options_menu();
                    create_hud_sprites();
                }
                else if (options_selection == 1)
                {
                    select_new_seed();

                    close_options_menu();
                    create_hud_sprites();
                    for (int step = 1; step <= 12; ++step)
                    {
                        set_start_transition_darkness(step / 12.0);
                        bn::core::update();
                    }
                    if(!start_game())
                        enter_menu_floor();
                    for (int step = 11; step >= 0; --step)
                    {
                        set_start_transition_darkness(step / 12.0);
                        bn::core::update();
                    }
                }
                else if(save_active_game())
                {
                    close_options_menu();
                    for (int step = 1; step <= 12; ++step)
                    {
                        set_start_transition_darkness(step / 12.0);
                        bn::core::update();
                    }
                    return_to_title();
                    for (int step = 11; step >= 0; --step)
                    {
                        set_start_transition_darkness(step / 12.0);
                        bn::core::update();
                    }
                }
            }

            bn::core::update();
            continue;
        }

        if (inventory_open || ability_data_open)
        {
            if (ability_data_open)
            {
                if (bn::keypad::b_pressed() || bn::keypad::a_pressed() ||
                    bn::keypad::r_pressed() || bn::keypad::select_pressed())
                {
                    fade_screen_to_black();
                    clear_ability_data_screen();
                    inventory_open = true;
                    create_inventory_screen();
                    refresh_main_hud_visibility();
                    set_complete_screen_blackness(1);
                    fade_screen_from_black();
                    bn::sound_items::move_ui.play();
                }
                else if (bn::keypad::l_pressed())
                {
                    fade_screen_to_black();
                    clear_ability_data_screen();
                    inventory_open = false;
                    ui_abierta = true;
                    pantalla_actual = Pantalla::MAPA;
                    map_bg.emplace(bn::regular_bg_items::mapback.create_bg(0, 0));
                    map_bg->set_priority(0);
                    map_current_blink_timer = 0;
                    map_current_blink_toggles_remaining = 6;
                    map_current_marker_hidden = false;
                    release_hud_sprites();
                    bn::sound_items::move_ui.play();
                    update_overlay_title("MAPA", true);
                    update_map();
                    refresh_main_hud_visibility();
                    set_complete_screen_blackness(1);
                    fade_screen_from_black();
                }

                bn::core::update();
                continue;
            }

            if (replacing_ability && ability_icon_blink_toggles_remaining > 0)
            {
                ++ability_icon_blink_timer;
                if (ability_icon_blink_timer >= ability_icon_blink_interval_frames)
                {
                    ability_icon_blink_timer = 0;
                    ability_icon_blink_hidden = !ability_icon_blink_hidden;
                    --ability_icon_blink_toggles_remaining;
                    if (ability_icon_blink_toggles_remaining == 0)
                    {
                        ability_icon_blink_hidden = false;
                        if (selected_ability_slot < inventory_ability_frames.size())
                        {
                            inventory_ability_frames[selected_ability_slot].set_visible(true);
                            inventory_ability_icons[selected_ability_slot].set_visible(true);
                            inventory_ability_level_sprites[selected_ability_slot].set_visible(true);
                        }
                    }
                    if (selected_ability_slot < inventory_ability_frames.size())
                    {
                        inventory_ability_frames[selected_ability_slot].set_visible(!ability_icon_blink_hidden);
                        inventory_ability_icons[selected_ability_slot].set_visible(!ability_icon_blink_hidden);
                        inventory_ability_level_sprites[selected_ability_slot].set_visible(!ability_icon_blink_hidden);
                    }
                }
            }
            if (carried_ability_count > 0 && bn::keypad::left_pressed())
            {
                selected_ability_slot = (selected_ability_slot + carried_ability_count - 1) % carried_ability_count;
                active_ability = carried_abilities[selected_ability_slot].ability;
                update_active_ability_icon();
                update_inventory_ability_selection();
                start_ability_icon_blink();
                for (bn::sprite_ptr &sprite : inventory_ability_frames) sprite.set_visible(true);
                for (bn::sprite_ptr &sprite : inventory_ability_icons) sprite.set_visible(true);
                for (bn::sprite_ptr &sprite : inventory_ability_level_sprites) sprite.set_visible(true);
                bn::sound_items::move_ui.play();
            }
            else if (carried_ability_count > 0 && bn::keypad::right_pressed())
            {
                selected_ability_slot = (selected_ability_slot + 1) % carried_ability_count;
                active_ability = carried_abilities[selected_ability_slot].ability;
                update_active_ability_icon();
                update_inventory_ability_selection();
                start_ability_icon_blink();
                for (bn::sprite_ptr &sprite : inventory_ability_frames) sprite.set_visible(true);
                for (bn::sprite_ptr &sprite : inventory_ability_icons) sprite.set_visible(true);
                for (bn::sprite_ptr &sprite : inventory_ability_level_sprites) sprite.set_visible(true);
                bn::sound_items::move_ui.play();
            }
            else if (carried_ability_count > 0 && bn::keypad::select_pressed())
            {
                fade_screen_to_black();
                create_ability_data_screen();
                set_complete_screen_blackness(1);
                fade_screen_from_black();
                bn::sound_items::move_ui.play();
            }
            else if (!replacing_ability && bn::keypad::l_pressed())
            {
                fade_screen_to_black();
                inventory_open = false;
                clear_inventory_screen();
                ui_abierta = true;
                pantalla_actual = Pantalla::MAPA;
                map_bg.emplace(bn::regular_bg_items::mapback.create_bg(0, 0));
                map_bg->set_priority(0);
                map_current_blink_timer = 0;
                map_current_blink_toggles_remaining = 6;
                map_current_marker_hidden = false;
                release_hud_sprites();
                bn::sound_items::move_ui.play();
                update_overlay_title("MAPA", true);
                update_map();
                refresh_main_hud_visibility();
                set_complete_screen_blackness(1);
                fade_screen_from_black();
            }
            else if (replacing_ability && bn::keypad::a_pressed() && carried_ability_count == max_carried_abilities)
            {
                RoomItemType outgoing_type = static_cast<RoomItemType>(
                    int(RoomItemType::ABILITY_DASH) + int(carried_abilities[selected_ability_slot].ability));
                int outgoing_level = carried_abilities[selected_ability_slot].level;
                if(reward_room_replacement_index >= 0)
                {
                    int index = reward_room_replacement_index;
                    RewardRoomReward &reward = reward_room_rewards[index];
                    remove_room_item(room_manager.current_room_id(), reward.type, reward.x, reward.y);
                    reward.type = outgoing_type;
                    reward.level = outgoing_level;
                    room_manager.add_item(room_manager.current_room_id(), reward.type,
                                          reward.x, reward.y, outgoing_level);
                    set_ability_icon(reward_room_reward_sprites[index],
                                     carried_abilities[selected_ability_slot].ability);
                    move_player_outside_pickup(reward.x, reward.y);
                    reward_room_replacement_index = -1;
                    reward_room_ability_rejected_until_leave = true;
                }
                else
                {
                    remove_room_item(room_manager.current_room_id(), campfire_reward_type,
                                     campfire_reward_x, campfire_reward_y);
                    room_manager.add_item(room_manager.current_room_id(), outgoing_type,
                                          campfire_reward_x, campfire_reward_y, outgoing_level);
                    campfire_reward_type = outgoing_type;
                    campfire_reward_level = outgoing_level;
                    move_player_outside_pickup(campfire_reward_x, campfire_reward_y);
                    if(campfire_reward_sprite.has_value())
                    {
                        set_ability_icon(*campfire_reward_sprite,
                                         carried_abilities[selected_ability_slot].ability);
                    }
                }
                carried_abilities[selected_ability_slot] = {pending_ability, pending_ability_level};
                active_ability = pending_ability;
                replacing_ability = false;
                inventory_open = false;
                reject_reward_until_leave = true;
                clear_inventory_screen();
                create_hud_sprites();
                refresh_main_hud_visibility();
                update_active_ability_icon();
                update_campfire_reward_visibility();
                update_reward_room_reward_visibility();
                bn::sound_items::pickup_item.play();
                update_map();
                pantalla_actual = Pantalla::INVENTARIO;
                fade_screen_to_black();
                create_ability_data_screen();
                set_complete_screen_blackness(1);
                fade_screen_from_black();
            }
            else if (replacing_ability && bn::keypad::b_pressed())
            {
                replacing_ability = false;
                reward_room_replacement_index = -1;
                inventory_open = false;
                reject_reward_until_leave = true;
                clear_inventory_screen();
                create_hud_sprites();
                refresh_main_hud_visibility();
            }
            else if (!replacing_ability &&
                     (bn::keypad::a_pressed() || bn::keypad::r_pressed() || bn::keypad::b_pressed()))
            {
                fade_screen_to_black();
                bn::sound_items::move_ui.play();
                inventory_open = false;
                pantalla_actual = Pantalla::MAPA;
                clear_inventory_screen();
                create_hud_sprites();
                refresh_main_hud_visibility();
                set_complete_screen_blackness(1);
                fade_screen_from_black();
            }
            else if (!replacing_ability && bn::keypad::start_pressed())
            {
                open_options_menu();
            }

            bn::core::update();
            continue;
        }

        // Start abre la pausa, L el mapa y R el inventario.
        if (bn::keypad::start_pressed())
        {
            open_options_menu();
        }
        else if (bn::keypad::l_pressed())
        {
            fade_screen_to_black();
            pantalla_actual = Pantalla::MAPA;
            ui_abierta = !ui_abierta;
            if (ui_abierta)
            {
                map_current_blink_timer = 0;
                map_current_blink_toggles_remaining = 6;
                map_current_marker_hidden = false;
                if (!map_bg.has_value())
                {
                    map_bg.emplace(bn::regular_bg_items::mapback.create_bg(0, 0));
                }
                map_bg->set_priority(0);
                release_hud_sprites();
                bn::sound_items::move_ui.play();
            }
            else
            {
                map_bg.reset();
                bn::sound_items::move_ui.play();
                update_map();
                create_hud_sprites();
            }
            if (!ui_abierta)
            {
                // Reaplica el aspecto normal del piso al cerrar el mapa.
                set_scene_darkness(0);
            }
            update_overlay_title("MAPA", ui_abierta);
            update_map();
            refresh_main_hud_visibility();
            set_complete_screen_blackness(1);
            fade_screen_from_black();
        }
        else if (bn::keypad::r_pressed())
        {
            fade_screen_to_black();
            ui_abierta = false;
            create_hud_sprites();
            pantalla_actual = Pantalla::INVENTARIO;
            inventory_open = true;
            create_inventory_screen();
            bn::sound_items::move_ui.play();
            update_map();
            inventory_key_palette.set_fade(bn::colors::black, 0);
            inventory_player_palette.set_fade(bn::colors::black, 0);
            key_icon->set_bg_priority(0);
            key_icon->set_z_order(-13);
            inventory_rescued_icon.set_bg_priority(0);
            inventory_rescued_icon.set_z_order(-13);
            refresh_main_hud_visibility();
            set_complete_screen_blackness(1);
            fade_screen_from_black();
        }

        if (inventory_open)
        {
            bn::core::update();
            continue;
        }

        // El mapa detiene el mundo igual que el inventario; L lo vuelve a cerrar.
        if (ui_abierta)
        {
            update_map_current_blink();
            update_reward_map_blink();
            bn::core::update();
            continue;
        }

        update_ability_icon_blink();
        update_placed_bomb();

        // L genera un piso nuevo de entre 5 y 16 cuartos.
        // if (bn::keypad::l_pressed() && !ui_abierta)
        // {
        //     generate_next_floor();
        // }

        // Movimiento ++++++++++++++++++
        int previous_x = x;
        int previous_y = y;
        bool moving = false;

        // Actualiza primero la direccion, para que A pueda usar la flecha pulsada
        // en el mismo cuadro; sin flechas conserva la ultima direccion.
        if (campfire_effect_active_this_frame)
        {
            dash_active = false;
        }
        else if (dash_active)
        {
            direccion = dash_direction;
        }
        else if (bn::keypad::down_held())
        {
            direccion = Direccion::ABAJO;
        }
        else if (bn::keypad::up_held())
        {
            direccion = Direccion::ARRIBA;
        }
        else if (bn::keypad::left_held())
        {
            direccion = Direccion::IZQUIERDA;
        }
        else if (bn::keypad::right_held())
        {
            direccion = Direccion::DERECHA;
        }

        bool reward_lever_interaction_consumed = false;
        if(bn::keypad::a_pressed() && !dash_active)
        {
            int current_room_id = room_manager.current_room_id();
            for(int index = 0; index < 3; ++index)
            {
                if(!reward_lever_activated[index] &&
                   (index != 2 || (reward_lever_activated[0] && reward_lever_activated[1])) &&
                   reward_lever_room_ids[index] == current_room_id &&
                   x >= reward_lever_x[index] - 20 && x <= reward_lever_x[index] + 20 &&
                   y >= reward_lever_y[index] - 20 && y <= reward_lever_y[index] + 20)
                {
                    reward_lever_activated[index] = true;
                    reward_lever_animation_timers[index] = 0;
                    reward_lever_animation_frames[index] = 0;
                    choose_third_reward_lever_room();
                    reward_lever_interaction_consumed = true;
                    bn::sound_items::open_door.play();
                    unlock_reward_room();
                    if(reward_room_unlocked)
                    {
                        release_hud_sprites();
                    }
                    break;
                }
            }
        }

        int ability_level = current_ability_level();
        int dash_stamina_cost = sprint_cost_for_level(ActiveAbility::DASH, ability_level);
        int stop_time_stamina_cost = sprint_cost_for_level(ActiveAbility::STOP_TIME, ability_level);
        int trapdoor_stamina_cost = sprint_cost_for_level(ActiveAbility::TRAPDOOR_BACK, ability_level);
        int reveal_map_stamina_cost = sprint_cost_for_level(ActiveAbility::VIEW_ALL_MAP, ability_level);
        int master_key_stamina_cost = sprint_cost_for_level(ActiveAbility::MASTER_KEY, ability_level);
        int familiar_stamina_cost = sprint_cost_for_level(ActiveAbility::FAMILIAR, ability_level);
        int bomb_energy_cost = energy_cost_for_level(ActiveAbility::BOMB, ability_level);
        if (!reward_lever_interaction_consumed && !campfire_effect_active_this_frame && carried_ability_count > 0 &&
            bn::keypad::a_pressed() && !dash_active)
        {
            if (active_ability == ActiveAbility::DASH)
            {
                if (stamina >= dash_stamina_cost)
                {
                    stamina -= dash_stamina_cost;
                    update_stamina_bar_fill(stamina);
                    dash_direction = direccion;
                    dash_active = true;
                    movement_speed_remainder = 0;
                    bn::sound_items::dash.play();
                    start_ability_flash();
                }
                else
                {
                    start_stamina_rejection_flash();
                }
            }
            else if (active_ability == ActiveAbility::STOP_TIME &&
                     time_stopped_room_id != room_manager.current_room_id())
            {
                if (stamina >= stop_time_stamina_cost)
                {
                    stamina -= stop_time_stamina_cost;
                    update_stamina_bar_fill(stamina);
                    time_stopped_room_id = room_manager.current_room_id();
                    bn::sound_items::dash.play();
                    start_ability_flash();
                }
                else
                {
                    start_stamina_rejection_flash();
                }
            }
            else if (active_ability == ActiveAbility::TRAPDOOR_BACK)
            {
                int current_room_id = room_manager.current_room_id();
                bool near_home_trapdoor = x >= home_trapdoor_x - 12 && x <= home_trapdoor_x + 12 &&
                    y >= home_trapdoor_y - 12 && y <= home_trapdoor_y + 12;
                bool near_remote_trapdoor = x >= trapdoor_x - 12 && x <= trapdoor_x + 12 &&
                    y >= trapdoor_y - 12 && y <= trapdoor_y + 12;

                if (trapdoor_deployed && current_room_id == home_room_id && near_home_trapdoor)
                {
                    teleport_through_trapdoor(trapdoor_room_id, trapdoor_x, trapdoor_y);
                }
                else if (trapdoor_deployed && current_room_id == trapdoor_room_id &&
                         current_room_id != home_room_id && near_remote_trapdoor)
                {
                    teleport_through_trapdoor(home_room_id, home_trapdoor_x, home_trapdoor_y);
                }
                else if (current_room_id != home_room_id)
                {
                    if (stamina >= trapdoor_stamina_cost)
                    {
                        stamina -= trapdoor_stamina_cost;
                        update_stamina_bar_fill(stamina);
                        trapdoor_room_id = current_room_id;
                        trapdoor_x = x;
                        trapdoor_y = y;
                        trapdoor_deployed = true;
                        update_trapdoor_visibility();
                        bn::sound_items::open_door.play();
                        start_ability_flash();
                    }
                    else
                    {
                        start_stamina_rejection_flash();
                    }
                }
            }
            else if (active_ability == ActiveAbility::VIEW_ALL_MAP && !floor_map_revealed)
            {
                if (stamina >= reveal_map_stamina_cost)
                {
                    stamina -= reveal_map_stamina_cost;
                    update_stamina_bar_fill(stamina);
                    floor_map_revealed = true;
                    update_map();
                    bn::sound_items::dash.play();
                    start_ability_flash();
                }
                else
                {
                    start_stamina_rejection_flash();
                }
            }
            else if (active_ability == ActiveAbility::MASTER_KEY)
            {
                const RoomDirection master_key_directions[] =
                    {RoomDirection::LEFT, RoomDirection::RIGHT,
                     RoomDirection::UP, RoomDirection::DOWN};
                int closed_door_count = 0;
                for (RoomDirection direction : master_key_directions)
                {
                    if (room_manager.has_door(direction) &&
                        room_manager.door_is_visible(room_manager.current_room_id(), direction) &&
                        !room_manager.door_is_open(direction))
                    {
                        ++closed_door_count;
                    }
                }
                if (closed_door_count > 0 && stamina >= master_key_stamina_cost)
                {
                    for (RoomDirection direction : master_key_directions)
                    {
                        if (room_manager.has_door(direction) &&
                            room_manager.door_is_visible(room_manager.current_room_id(), direction) &&
                            !room_manager.door_is_open(direction))
                        {
                            room_manager.set_door_open(room_manager.current_room_id(), direction, true);
                        }
                    }
                    stamina -= master_key_stamina_cost;
                    update_stamina_bar_fill(stamina);
                    update_door_sprites();
                    bn::sound_items::open_door.play();
                    start_ability_flash();
                }
                else if (closed_door_count > 0)
                {
                    start_stamina_rejection_flash();
                }
            }
            else if (active_ability == ActiveAbility::BOMB && energy >= bomb_energy_cost &&
                     pending_bomb_frames == 0)
            {
                pending_bomb_room_id = room_manager.current_room_id();
                pending_bomb_x = x;
                pending_bomb_y = y;
                pending_bomb_energy_cost = bomb_energy_cost;
                pending_bomb_frames = bomb_fuse_duration_frames;
                bomb_blink_timer = 0;
                bomb_blink_visible = true;
                placed_bomb_sprite.emplace(bn::sprite_items::hability_bomb_icon.create_sprite(
                    pending_bomb_x, pending_bomb_y));
                placed_bomb_sprite->set_bg_priority(1);
                placed_bomb_sprite->set_z_order(-2);
            }
            else if (active_ability == ActiveAbility::FAMILIAR && !familiar_active)
            {
                int nearest_kind = -1;
                int nearest_index = -1;
                int nearest_distance = 0x7fffffff;
                auto consider_enemy = [&](int kind, int index, int enemy_x, int enemy_y)
                {
                    int dx = enemy_x - x;
                    int dy = enemy_y - y;
                    int distance = dx * dx + dy * dy;
                    if (distance < nearest_distance)
                    {
                        nearest_distance = distance;
                        nearest_kind = kind;
                        nearest_index = index;
                    }
                };
                int room_id = room_manager.current_room_id();
                for (int index = 0; index < active_skeleton_count; ++index)
                    consider_enemy(0, index, skeleton_x[index], skeleton_y[index]);
                for (int index = 0; index < active_bat_count; ++index)
                    consider_enemy(1, index, bat_x[index], bat_y[index]);
                for (int index = 0; index < active_mushy_count; ++index)
                    consider_enemy(3, index, mushy_room_x[room_id][index],
                                   mushy_room_y[room_id][index]);
                for (int index = 0; index < active_plant_count; ++index)
                {
                    int position = plant_position_indices[room_id][index];
                    consider_enemy(2, index, plant_positions[position][0], plant_positions[position][1]);
                }
                if (nearest_kind >= 0)
                {
                    if (stamina >= familiar_stamina_cost)
                    {
                        stamina -= familiar_stamina_cost;
                        update_stamina_bar_fill(stamina);
                        familiar_active = true;
                        familiar_frames_remaining = 120;
                        familiar_target_kind = nearest_kind;
                        familiar_target_index = nearest_index;
                        familiar_room_id = room_id;
                        if (nearest_kind == 0)
                        {
                            familiar_target_x = skeleton_x[nearest_index];
                            familiar_target_y = skeleton_y[nearest_index];
                        }
                        else if (nearest_kind == 1)
                        {
                            familiar_target_x = bat_x[nearest_index];
                            familiar_target_y = bat_y[nearest_index];
                        }
                        else if (nearest_kind == 2)
                        {
                            int position = plant_position_indices[room_id][nearest_index];
                            familiar_target_x = plant_positions[position][0];
                            familiar_target_y = plant_positions[position][1];
                        }
                        else
                        {
                            familiar_target_x = mushy_room_x[room_id][nearest_index];
                            familiar_target_y = mushy_room_y[room_id][nearest_index];
                        }
                        familiar_x = x;
                        familiar_y = y;
                        familiar_world_sprite.set_position(familiar_x, familiar_y);
                        familiar_world_sprite.set_visible(true);
                        bn::sound_items::dash.play();
                        start_ability_flash();
                    }
                    else
                    {
                        start_stamina_rejection_flash();
                    }
                }
            }
        }

        if (familiar_active)
        {
            if (familiar_room_id != room_manager.current_room_id())
            {
                familiar_active = false;
                familiar_world_sprite.set_visible(false);
            }
            else
            {
                bool familiar_target_valid =
                    (familiar_target_kind == 0 && familiar_target_index < active_skeleton_count) ||
                    (familiar_target_kind == 1 && familiar_target_index < active_bat_count) ||
                    (familiar_target_kind == 2 && familiar_target_index < active_plant_count) ||
                    (familiar_target_kind == 3 && familiar_target_index < active_mushy_count);
                if (!familiar_target_valid)
                {
                    // No cambies a otro enemigo si el objetivo original desaparecio.
                    familiar_target_kind = -1;
                    familiar_frames_remaining = 0;
                }

                if (familiar_target_kind == 0 && familiar_target_index < active_skeleton_count)
                {
                    familiar_target_x = skeleton_x[familiar_target_index];
                    familiar_target_y = skeleton_y[familiar_target_index];
                }
                else if (familiar_target_kind == 1 && familiar_target_index < active_bat_count)
                {
                    familiar_target_x = bat_x[familiar_target_index];
                    familiar_target_y = bat_y[familiar_target_index];
                }
                else if (familiar_target_kind == 2 && familiar_target_index < active_plant_count)
                {
                    int position = plant_position_indices[familiar_room_id][familiar_target_index];
                    familiar_target_x = plant_positions[position][0];
                    familiar_target_y = plant_positions[position][1];
                }
                else if (familiar_target_kind == 3 &&
                         familiar_target_index < active_mushy_count)
                {
                    familiar_target_x = mushy_room_x[familiar_room_id][familiar_target_index];
                    familiar_target_y = mushy_room_y[familiar_room_id][familiar_target_index];
                }
                int dx = familiar_target_x - familiar_x;
                int dy = familiar_target_y - familiar_y;
                int step_x = dx > 2 ? 2 : (dx < -2 ? -2 : dx);
                int step_y = dy > 2 ? 2 : (dy < -2 ? -2 : dy);
                familiar_x += step_x;
                familiar_y += step_y;
                familiar_world_sprite.set_position(familiar_x, familiar_y);
                int remaining_dx = familiar_target_x - familiar_x;
                int remaining_dy = familiar_target_y - familiar_y;
                bool familiar_reached_target =
                    remaining_dx * remaining_dx + remaining_dy * remaining_dy <= 4;
                if (familiar_reached_target || --familiar_frames_remaining <= 0)
                {
                    if (familiar_reached_target && familiar_target_kind == 0 &&
                        skeleton_counts[familiar_room_id] > 0 &&
                        familiar_target_index < skeleton_counts[familiar_room_id])
                    {
                        int last = skeleton_counts[familiar_room_id] - 1;
#define SWAP_SKELETON_STATE(field) \
                        { int value = field[familiar_target_index]; \
                          field[familiar_target_index] = field[last]; field[last] = value; }
                        SWAP_SKELETON_STATE(skeleton_path_positions);
                        SWAP_SKELETON_STATE(skeleton_x);
                        SWAP_SKELETON_STATE(skeleton_y);
                        SWAP_SKELETON_STATE(skeleton_walk_frames);
                        SWAP_SKELETON_STATE(skeleton_animation_timers);
                        SWAP_SKELETON_STATE(skeleton_displayed_frames);
#undef SWAP_SKELETON_STATE
                        int corner = skeleton_start_corners[familiar_room_id][familiar_target_index];
                        skeleton_start_corners[familiar_room_id][familiar_target_index] = skeleton_start_corners[familiar_room_id][last];
                        skeleton_start_corners[familiar_room_id][last] = corner;
                        int saved_path = skeleton_room_path_positions[familiar_room_id][familiar_target_index];
                        skeleton_room_path_positions[familiar_room_id][familiar_target_index] =
                            skeleton_room_path_positions[familiar_room_id][last];
                        skeleton_room_path_positions[familiar_room_id][last] = saved_path;
                        bool path_initialized = skeleton_room_path_initialized[familiar_room_id][familiar_target_index];
                        skeleton_room_path_initialized[familiar_room_id][familiar_target_index] =
                            skeleton_room_path_initialized[familiar_room_id][last];
                        skeleton_room_path_initialized[familiar_room_id][last] = path_initialized;
                        skeleton_displayed_frames[familiar_target_index] = -1;
                        --skeleton_counts[familiar_room_id];
                        --active_skeleton_count;
                        skeleton_sprites[last].set_visible(false);
                    }
                    else if (familiar_reached_target && familiar_target_kind == 1 &&
                             bat_counts[familiar_room_id] > 0 &&
                             familiar_target_index < bat_counts[familiar_room_id])
                    {
                        int last = bat_counts[familiar_room_id] - 1;
#define SWAP_BAT_STATE(field) \
                        { auto value = field[familiar_room_id][familiar_target_index]; \
                          field[familiar_room_id][familiar_target_index] = field[familiar_room_id][last]; \
                          field[familiar_room_id][last] = value; }
                        SWAP_BAT_STATE(bat_start_x);
                        SWAP_BAT_STATE(bat_start_y);
                        SWAP_BAT_STATE(bat_start_dx);
                        SWAP_BAT_STATE(bat_start_dy);
                        SWAP_BAT_STATE(bat_start_animation_frames);
                        SWAP_BAT_STATE(bat_start_animation_timers);
                        SWAP_BAT_STATE(bat_room_x);
                        SWAP_BAT_STATE(bat_room_y);
                        SWAP_BAT_STATE(bat_room_dx);
                        SWAP_BAT_STATE(bat_room_dy);
                        SWAP_BAT_STATE(bat_room_animation_frames);
                        SWAP_BAT_STATE(bat_room_animation_timers);
#undef SWAP_BAT_STATE
#define SWAP_BAT_RUNTIME(field) \
                        { int value = field[familiar_target_index]; \
                          field[familiar_target_index] = field[last]; field[last] = value; }
                        SWAP_BAT_RUNTIME(bat_x);
                        SWAP_BAT_RUNTIME(bat_y);
                        SWAP_BAT_RUNTIME(bat_dx);
                        SWAP_BAT_RUNTIME(bat_dy);
                        SWAP_BAT_RUNTIME(bat_animation_frames);
                        SWAP_BAT_RUNTIME(bat_animation_timers);
                        SWAP_BAT_RUNTIME(bat_displayed_frames);
#undef SWAP_BAT_RUNTIME
                        bat_displayed_frames[familiar_target_index] = -1;
                        --bat_counts[familiar_room_id];
                        --active_bat_count;
                        bat_sprites[last].set_visible(false);
                    }
                    else if (familiar_reached_target && familiar_target_kind == 2 &&
                             plant_counts[familiar_room_id] > 0 &&
                             familiar_target_index < plant_counts[familiar_room_id])
                    {
                        int last = plant_counts[familiar_room_id] - 1;
                        int position = plant_position_indices[familiar_room_id][familiar_target_index];
                        plant_position_indices[familiar_room_id][familiar_target_index] =
                            plant_position_indices[familiar_room_id][last];
                        plant_position_indices[familiar_room_id][last] = position;
                        int cooldown = plant_shot_cooldowns[familiar_room_id][familiar_target_index];
                        plant_shot_cooldowns[familiar_room_id][familiar_target_index] = plant_shot_cooldowns[familiar_room_id][last];
                        plant_shot_cooldowns[familiar_room_id][last] = cooldown;
                        int fire_step = plant_fire_animation_steps[familiar_target_index];
                        plant_fire_animation_steps[familiar_target_index] = plant_fire_animation_steps[last];
                        plant_fire_animation_steps[last] = fire_step;
                        int fire_timer = plant_fire_animation_timers[familiar_target_index];
                        plant_fire_animation_timers[familiar_target_index] = plant_fire_animation_timers[last];
                        plant_fire_animation_timers[last] = fire_timer;
                        plant_displayed_frames[familiar_target_index] = -1;
                        --plant_counts[familiar_room_id];
                        --active_plant_count;
                        plant_sprites[last].set_visible(false);
                    }
                    else if (familiar_reached_target && familiar_target_kind == 3 &&
                             mushy_counts[familiar_room_id] > 0 &&
                             familiar_target_index < mushy_counts[familiar_room_id])
                    {
                        int last = mushy_counts[familiar_room_id] - 1;
                        mushy_room_x[familiar_room_id][familiar_target_index] =
                            mushy_room_x[familiar_room_id][last];
                        mushy_room_y[familiar_room_id][familiar_target_index] =
                            mushy_room_y[familiar_room_id][last];
                        mushy_direction_frames[familiar_room_id][familiar_target_index] =
                            mushy_direction_frames[familiar_room_id][last];
                        mushy_move_axes[familiar_room_id][familiar_target_index] =
                            mushy_move_axes[familiar_room_id][last];
                        mushy_walking[familiar_room_id][familiar_target_index] =
                            mushy_walking[familiar_room_id][last];
                        --mushy_counts[familiar_room_id];
                        --active_mushy_count;
                        mushy_sprites[familiar_target_index].set_position(
                            mushy_room_x[familiar_room_id][familiar_target_index],
                            mushy_room_y[familiar_room_id][familiar_target_index]);
                        mushy_sprites[familiar_target_index].set_tiles(
                            bn::sprite_items::mushy.tiles_item(),
                            mushy_direction_frames[familiar_room_id][familiar_target_index] +
                                (mushy_walking[familiar_room_id][familiar_target_index] ?
                                    mushy_animation_frame : 0));
                        mushy_sprites[last].set_visible(false);
                    }
                    familiar_active = false;
                    familiar_world_sprite.set_visible(false);
                    update_map();
                }
            }
        }

        bool dashing = dash_active;
        bool direction_held = bn::keypad::down_held() || bn::keypad::up_held() ||
                              bn::keypad::left_held() || bn::keypad::right_held();
        bool sprint_requested = ! campfire_effect_active_this_frame && bn::keypad::b_held() && direction_held;
        bool sprint_this_frame = !dashing && sprint_requested && stamina > 0;
        animation_speed = dashing ? 4 : (sprint_this_frame ? 8 : 16);
        int movement_units = sprint_this_frame ? 8 : 5;
        int accumulated_movement_units = movement_speed_remainder + movement_units;
        int frame_movement = accumulated_movement_units / 4;
        int next_movement_speed_remainder = dashing ? 0 : accumulated_movement_units % 4;

        if (campfire_effect_active_this_frame)
        {
            moving = false;
        }
        else if (dashing)
        {
            for (int step = 0; step < dash_pixels_per_frame; ++step)
            {
                int candidate_x = x;
                int candidate_y = y;
                switch (dash_direction)
                {
                case Direccion::ABAJO: ++candidate_y; break;
                case Direccion::ARRIBA: --candidate_y; break;
                case Direccion::IZQUIERDA: --candidate_x; break;
                case Direccion::DERECHA: ++candidate_x; break;
                default: break;
                }

                bool outside_room = candidate_x < -102 || candidate_x > 102 ||
                    candidate_y < -64 || candidate_y > 58;
                if (outside_room || player_position_blocked(candidate_x, candidate_y))
                {
                    dash_active = false;
                    break;
                }

                x = candidate_x;
                y = candidate_y;
            }
            moving = x != previous_x || y != previous_y;
        }
        else
        {
            if (bn::keypad::down_held())
            {
                y += frame_movement;
                moving = true;
            }
            else if (bn::keypad::up_held())
            {
                y -= frame_movement;
                moving = true;
            }
            else if (bn::keypad::left_held())
            {
                x -= frame_movement;
                moving = true;
            }
            else if (bn::keypad::right_held())
            {
                x += frame_movement;
                moving = true;
            }

            // Hitbox de paredes: 2 px mas hacia dentro a la izquierda y 4 px abajo.
            if (x < -102)
                x = -102;
            if (x > 102)
                x = 102;
            if (y < -64)
                y = -64;
            if (y > 58)
                y = 58;

            if (player_position_blocked(x, y))
            {
                x = previous_x;
                y = previous_y;
                moving = false;
            }
        }

        if (x != previous_x || y != previous_y)
        {
            movement_speed_remainder = next_movement_speed_remainder;
        }

        // Una iteracion corresponde a un cuadro de juego; el consumo por cuadro
        // mantiene la duracion constante y solo cuenta mientras el jugador avanza.
        bool stamina_emptied_this_frame = false;
        if (sprint_this_frame && (x != previous_x || y != previous_y))
        {
            --stamina;
            stamina_emptied_this_frame = stamina == 0;
            update_stamina_bar_fill(stamina);
        }

        if (stamina_emptied_this_frame && !stamina_exhaustion_penalty_applied)
        {
            if (energy > 0)
            {
                --energy;
            }
            update_energy_label();
            update_energy_bar_fill(energy);
            if (energy < 5) start_energy_recovery_flash();
            stamina_exhaustion_penalty_applied = true;
        }

        if (!sprint_requested)
        {
            stamina_run_attempt_latched = false;
        }
        else if (stamina == 0 && !stamina_run_attempt_latched)
        {
            start_stamina_rejection_flash();
            stamina_run_attempt_latched = true;
        }

        if (stamina_blink_toggles_remaining > 0)
        {
            ++stamina_blink_timer;
            if (stamina_blink_timer >= stamina_blink_interval_frames)
            {
                stamina_blink_timer = 0;
                stamina_blink_hidden = !stamina_blink_hidden;
                --stamina_blink_toggles_remaining;
                if (stamina_blink_toggles_remaining == 0)
                {
                    stamina_blink_hidden = false;
                }
                refresh_stamina_bar_visibility();
            }
        }

        if (energy_blink_toggles_remaining > 0)
        {
            ++energy_blink_timer;
            if (energy_blink_timer >= energy_blink_interval_frames)
            {
                energy_blink_timer = 0;
                energy_blink_hidden = !energy_blink_hidden;
                --energy_blink_toggles_remaining;
                if (energy_blink_toggles_remaining == 0)
                {
                    energy_blink_hidden = false;
                }
                refresh_energy_bar_visibility();
            }
        }

        if (carried_npc_index >= 0 && (x != previous_x || y != previous_y))
        {
            record_follow_movement(previous_x, previous_y, x, y);
        }

        if (carried_npc_index < 0)
        {
            for (int index = 0; index < max_npcs; ++index)
            {
                NpcState &npc = npcs[index];
                int delta_x = x - npc.x;
                int delta_y = y - npc.y;
                if (npc.active && !npc.delivered && npc.room_id == room_manager.current_room_id() &&
                    delta_x > -16 && delta_x < 16 && delta_y > -16 && delta_y < 16)
                {
                    carried_npc_index = index;
                    reset_follow_history();
                    update_map();
                    break;
                }
            }
        }

        // Recoger la llave al acercarse; permanece visible en el cuarto donde aparecio.
        if (!floor_key_collected && room_manager.current_room_id() == floor_key_room &&
            x >= floor_key_x - 10 && x <= floor_key_x + 10 &&
            y >= floor_key_y - 12 && y <= floor_key_y + 12)
        {
            floor_key_collected = true;
            ++key_count;
            bn::sound_items::pickup_item.play();
            floor_key_sprite.set_visible(false);
            remove_room_item(floor_key_room, RoomItemType::KEY, floor_key_x, floor_key_y);
            update_key_count_label();
            update_map();
        }

    if(!special_key_collected && room_manager.current_room_id() == special_key_room &&
       x >= special_key_x - 10 && x <= special_key_x + 10 &&
       y >= special_key_y - 12 && y <= special_key_y + 12)
    {
        special_key_collected = true;
        ++key_count;
        bn::sound_items::pickup_item.play();
        special_key_sprite.set_visible(false);
        remove_room_item(special_key_room, RoomItemType::KEY, special_key_x, special_key_y);
        update_key_count_label();
        update_map();
    }

        bool near_reward_for_block = x >= campfire_reward_x - 10 && x <= campfire_reward_x + 10 &&
            y >= campfire_reward_y - 12 && y <= campfire_reward_y + 12;
        if (!near_reward_for_block)
        {
            reject_reward_until_leave = false;
        }
        if (campfire_reward_active && !reject_reward_until_leave &&
            room_manager.current_template() == RoomTemplateId::SPECIAL &&
            x >= campfire_reward_x - 10 && x <= campfire_reward_x + 10 &&
            y >= campfire_reward_y - 12 && y <= campfire_reward_y + 12)
        {
            bool is_ability = int(campfire_reward_type) >= int(RoomItemType::ABILITY_DASH);
            if (is_ability)
            {
                pending_ability = static_cast<ActiveAbility>(
                    int(campfire_reward_type) - int(RoomItemType::ABILITY_DASH));
                pending_ability_level = campfire_reward_level;
                int existing_slot = find_carried_ability(pending_ability);
                if (existing_slot >= 0 || carried_ability_count < max_carried_abilities)
                {
                    remove_room_item(room_manager.current_room_id(), campfire_reward_type,
                                     campfire_reward_x, campfire_reward_y);
                    clear_campfire_reward();
                    if (existing_slot >= 0)
                    {
                        carried_abilities[existing_slot].level += pending_ability_level;
                        if(carried_abilities[existing_slot].level > 5)
                            carried_abilities[existing_slot].level = 5;
                        selected_ability_slot = existing_slot;
                    }
                    else
                    {
                        carried_abilities[carried_ability_count] =
                            {pending_ability, pending_ability_level};
                        selected_ability_slot = carried_ability_count++;
                    }
                    active_ability = carried_abilities[selected_ability_slot].ability;
                    update_active_ability_icon();
                    update_inventory_ability_selection();
                    bn::sound_items::pickup_item.play();
                    pantalla_actual = Pantalla::INVENTARIO;
                    fade_screen_to_black();
                    create_ability_data_screen();
                    set_complete_screen_blackness(1);
                    fade_screen_from_black();
                }
                else
                {
                    open_ability_replacement();
                }
            }
            else
            {
                RoomItemType reward_type = campfire_reward_type;
                remove_room_item(room_manager.current_room_id(), reward_type,
                                 campfire_reward_x, campfire_reward_y);
                clear_campfire_reward();
                bn::sound_items::pickup_item.play();
                if (reward_type == RoomItemType::KEY)
                {
                    ++key_count;
                    update_key_count_label();
                }
                else if (reward_type == RoomItemType::ENERGY_POTION)
                {
                    energy += 5;
                    if (energy > energy_capacity) energy = energy_capacity;
                    update_energy_label();
                    update_energy_bar_fill(energy);
                    start_energy_recovery_flash();
                }
                else
                {
                    stamina += stamina_capacity_frames / 3;
                    if (stamina > stamina_capacity_frames) stamina = stamina_capacity_frames;
                    update_stamina_bar_fill(stamina);
                    start_stamina_recovery_flash();
                }
            }
            update_map();
        }

        bool player_near_campfire_reward =
            x >= campfire_reward_x - 10 && x <= campfire_reward_x + 10 &&
            y >= campfire_reward_y - 12 && y <= campfire_reward_y + 12;
        if (campfire_pending_rewards > 0 && !campfire_reward_active &&
            room_manager.current_template() == RoomTemplateId::SPECIAL &&
            !player_near_campfire_reward)
        {
            --campfire_pending_rewards;
            spawn_campfire_reward(true);
        }

        int current_room_id = room_manager.current_room_id();
        if (current_room_id >= 0 && current_room_id < RoomManager::max_rooms)
        {
            FloorPotionState &potion = floor_potions[current_room_id];
            if (potion.active && x >= potion.x - 10 && x <= potion.x + 10 &&
                y >= potion.y - 12 && y <= potion.y + 12)
            {
                bn::sound_items::pickup_item.play();
                if (potion.type == RoomItemType::ENERGY_POTION)
                {
                    energy += 5;
                    if (energy > energy_capacity)
                    {
                        energy = energy_capacity;
                    }
                    update_energy_label();
                    update_energy_bar_fill(energy);
                    start_energy_recovery_flash();
                }
                else
                {
                    stamina += stamina_capacity_frames / 3;
                    if (stamina > stamina_capacity_frames)
                    {
                        stamina = stamina_capacity_frames;
                    }
                    update_stamina_bar_fill(stamina);
                    start_stamina_recovery_flash();
                }

                remove_room_item(potion.room_id, potion.type, potion.x, potion.y);
                potion.active = false;
                update_floor_item_visibility();
                update_map();
            }
        }

        int reward_room_id = room_manager.reward_room_id();
        bool near_boss_ability_reward = current_room_id == reward_room_id &&
            x >= reward_room_rewards[3].x - 10 && x <= reward_room_rewards[3].x + 10 &&
            y >= reward_room_rewards[3].y - 12 && y <= reward_room_rewards[3].y + 12;
        if(!near_boss_ability_reward)
        {
            reward_room_ability_rejected_until_leave = false;
        }
        if(reward_room_unlocked && current_room_id == reward_room_id)
        {
            for(int index = 0; index < 4; ++index)
            {
                RewardRoomReward &reward = reward_room_rewards[index];
                if(!reward.active || (index == 3 && reward_room_ability_rejected_until_leave) ||
                   x < reward.x - 10 || x > reward.x + 10 ||
                   y < reward.y - 12 || y > reward.y + 12)
                {
                    continue;
                }

                if(index < 3)
                {
                    remove_room_item(reward_room_id, reward.type, reward.x, reward.y);
                    reward.active = false;
                    reward_room_reward_sprites[index].set_visible(false);
                    bn::sound_items::pickup_item.play();
                    if(reward.type == RoomItemType::KEY)
                    {
                        ++key_count;
                        update_key_count_label();
                    }
                    else if(reward.type == RoomItemType::ENERGY_POTION)
                    {
                        energy += 5;
                        if(energy > energy_capacity) energy = energy_capacity;
                        update_energy_label();
                        update_energy_bar_fill(energy);
                        start_energy_recovery_flash();
                    }
                    else
                    {
                        stamina += stamina_capacity_frames / 3;
                        if(stamina > stamina_capacity_frames)
                            stamina = stamina_capacity_frames;
                        update_stamina_bar_fill(stamina);
                        start_stamina_recovery_flash();
                    }
                    update_map();
                    continue;
                }

                pending_ability = static_cast<ActiveAbility>(
                    int(reward.type) - int(RoomItemType::ABILITY_DASH));
                pending_ability_level = reward.level;
                int existing_slot = find_carried_ability(pending_ability);
                if(existing_slot < 0 && carried_ability_count >= max_carried_abilities)
                {
                    reward_room_replacement_index = index;
                    reward_room_ability_rejected_until_leave = false;
                    open_ability_replacement();
                    break;
                }

                remove_room_item(reward_room_id, reward.type, reward.x, reward.y);
                reward.active = false;
                reward_room_reward_sprites[index].set_visible(false);
                if(existing_slot >= 0)
                {
                    carried_abilities[existing_slot].level += pending_ability_level;
                    if(carried_abilities[existing_slot].level > 5)
                        carried_abilities[existing_slot].level = 5;
                    selected_ability_slot = existing_slot;
                }
                else
                {
                    carried_abilities[carried_ability_count] =
                        {pending_ability, pending_ability_level};
                    selected_ability_slot = carried_ability_count++;
                }
                active_ability = carried_abilities[selected_ability_slot].ability;
                update_active_ability_icon();
                update_inventory_ability_selection();
                bn::sound_items::pickup_item.play();
                pantalla_actual = Pantalla::INVENTARIO;
                fade_screen_to_black();
                create_ability_data_screen();
                set_complete_screen_blackness(1);
                fade_screen_from_black();
                update_map();
                break;
            }
        }

        // Las llaves normales abren una puerta al contacto; MasterKey se activa con A.
        const RoomDirection door_directions[] =
            {
                RoomDirection::LEFT, RoomDirection::RIGHT,
                RoomDirection::UP, RoomDirection::DOWN};
        if (key_count > 0)
        {
            for (RoomDirection direction : door_directions)
            {
                if (room_manager.has_door(direction) &&
                    room_manager.door_is_visible(room_manager.current_room_id(), direction) &&
                    !room_manager.door_is_open(direction) &&
                    room_manager.touching_door(x, y, direction))
                {
                    room_manager.set_door_open(room_manager.current_room_id(), direction, true);
                    --key_count;
                    update_key_count_label();
                    bn::sound_items::open_door.play();
                    update_door_sprites();
                    break;
                }
            }
        }

        // RoomManager detecta las puertas y devuelve la entrada del cuarto destino.
        bool transition_fade_started = false;
        for (RoomDirection direction : door_directions)
        {
            if (!trapdoor_transitioned_this_frame && room_manager.has_door(direction) &&
                room_manager.door_is_open(direction) &&
                room_manager.touching_door(x, y, direction))
            {
                transition_fade_started = true;
                bn::sound_items::enter_door.play();
                fade_room_transition(true);
                break;
            }
        }

        int spawn_x = x;
        int spawn_y = y;
        if (!trapdoor_transitioned_this_frame && room_manager.try_transition(x, y, spawn_x, spawn_y))
        {
            if (time_stopped_room_id >= 0 &&
                time_stopped_room_id != room_manager.current_room_id())
            {
                time_stopped_room_id = -1;
            }
            // Conserva la posicion de entrada junto a la puerta correspondiente.
            x = spawn_x;
            y = spawn_y;
            dash_active = false;
            if (carried_npc_index >= 0)
            {
                reset_follow_history();
            }

            int entered_room_id = room_manager.current_room_id();
            bool first_visit = !scored_rooms[entered_room_id];
            if (first_visit)
            {
                scored_rooms[entered_room_id] = true;
                record_room_explored();
            }

            // Explorar un cuarto nuevo cuesta energia; si llevas un NPC,
            // cada puerta tambien cuesta energia aunque ya conozcas el cuarto.
            if (first_visit || carried_npc_index >= 0)
            {
                if (energy > 0)
                {
                    --energy;
                }
                update_energy_label();
                update_energy_bar_fill(energy);
                if (energy < 5) start_energy_recovery_flash();
            }

            if (room_manager.current_template() == RoomTemplateId::SPECIAL && carried_npc_index >= 0)
            {
                NpcState &npc = npcs[carried_npc_index];
                npc.delivered = true;
                npc.room_id = entered_room_id;
                carried_npc_index = -1;
                ++rescued_adventurers;
                ++game_save::records().adventurers_saved;
                update_rescued_count_label();
                spawn_campfire_reward();
                energy += 3;
                if (energy > energy_capacity)
                {
                    energy = energy_capacity;
                }
                update_energy_label();
                start_energy_recovery_flash();
            }

            update_room_background();
            update_room_obstacles();
            update_stone_layers();

            update_door_sprites();
            floor_key_sprite.set_visible(!floor_key_collected &&
                                         room_manager.current_room_id() == floor_key_room);
            special_key_sprite.set_visible(!special_key_collected &&
                room_manager.current_room_id() == special_key_room);
            update_floor_item_visibility();
            update_reward_room_reward_visibility();
            update_campfire_reward_visibility();
            stairs.set_visible(room_manager.current_room_id() == room_manager.stairs_room_id());

            room_name_sprites.clear();
            room_name_generator.generate(112, -68, room_display_name(room_manager.current_room_id()),
                                         room_name_sprites);
            refresh_map_labels_visibility();
            update_trapdoor_visibility();
            update_map();
            update_npc_sprites(displayed_frame);
            if (transition_fade_started)
            {
                player.set_position(x, y);
                update_reward_lever_visibility();
                fade_room_transition(false);
            }
        }
        else if (transition_fade_started)
        {
            update_reward_lever_visibility();
            set_scene_darkness(ui_abierta ? 0.8 : 0);
        }

        // Tocar las escaleras del ultimo cuarto genera el siguiente piso.
        const composition_rooms::Room* stairs_source = current_composition_room();
        int stairs_x = stairs_source ? stairs_source->position('e').x : 0;
        int stairs_y = stairs_source ? stairs_source->position('e').y +
            (room_manager.current_template() == RoomTemplateId::STAIRS_2 ? 0 : 8) : 0;
        bool near_stairs = x >= stairs_x - 10 && x <= stairs_x + 10 &&
            y >= stairs_y - 10 && y <= stairs_y + 10;
        if (room_manager.current_room_id() == room_manager.stairs_room_id() && near_stairs)
        {
            bn::sound_items::stairs.play();
            generate_next_floor();
        }

        bool enemies_stopped = time_stopped_room_id == room_manager.current_room_id();
        bool plant_bullet_hit_player = false;
        if (!enemies_stopped)
        {
            update_skeleton();
            update_bats();
            update_mushies();
            plant_bullet_hit_player = update_plants_and_bullets();
            if (active_bat_count > 0)
            {
                if (bat_sound_cooldown_frames > 0)
                {
                    --bat_sound_cooldown_frames;
                }
                if (bat_sound_cooldown_frames == 0)
                {
                    if (enemy_ambient_random.get_int(2) == 0)
                    {
                        bn::sound_items::bat1.play();
                    }
                    else
                    {
                        bn::sound_items::bat2.play();
                    }
                    bat_sound_cooldown_frames = 30 + enemy_ambient_random.get_int(0, 121);
                }
            }
            else
            {
                bat_sound_cooldown_frames = 0;
            }
            if (active_skeleton_count > 0)
            {
                if (skeleton_sound_cooldown_frames > 0)
                {
                    --skeleton_sound_cooldown_frames;
                }
                if (skeleton_sound_cooldown_frames == 0)
                {
                    bn::sound_items::skeleton.play();
                    skeleton_sound_cooldown_frames = 30 + enemy_ambient_random.get_int(0, 121);
                }
            }
            else
            {
                skeleton_sound_cooldown_frames = 0;
            }
        }
        bool touching_active_spikes = update_spikes(x, y);
        bool touching_enemy = false;
        for (int index = 0; index < active_skeleton_count; ++index)
        {
            touching_enemy = touching_enemy || overlaps_skeleton(index, x, y);
        }
        for (int index = 0; index < active_bat_count; ++index)
        {
            touching_enemy = touching_enemy || overlaps_bat(index, x, y);
        }

        int touching_mushy_index = -1;
        int current_mushy_room = room_manager.current_room_id();
        for (int index = 0; index < active_mushy_count; ++index)
        {
            int delta_x = x - mushy_room_x[current_mushy_room][index];
            int delta_y = y - mushy_room_y[current_mushy_room][index];
            if (game::sprite16_overlaps_player(delta_x, delta_y))
            {
                touching_mushy_index = index;
                break;
            }
        }
        if (touching_mushy_index >= 0)
        {
            int last = mushy_counts[current_mushy_room] - 1;
            if (familiar_active && familiar_room_id == current_mushy_room &&
                familiar_target_kind == 3)
            {
                if (familiar_target_index == touching_mushy_index)
                {
                    // El objetivo murio por contacto con el jugador; no persigas al que ocupa su indice.
                    familiar_active = false;
                    familiar_world_sprite.set_visible(false);
                }
                else if (familiar_target_index == last)
                {
                    // Al compactar la lista, conserva la identidad del Mushy que iba siguiendo.
                    familiar_target_index = touching_mushy_index;
                }
            }
            mushy_room_x[current_mushy_room][touching_mushy_index] =
                mushy_room_x[current_mushy_room][last];
            mushy_room_y[current_mushy_room][touching_mushy_index] =
                mushy_room_y[current_mushy_room][last];
            mushy_direction_frames[current_mushy_room][touching_mushy_index] =
                mushy_direction_frames[current_mushy_room][last];
            mushy_move_axes[current_mushy_room][touching_mushy_index] =
                mushy_move_axes[current_mushy_room][last];
            mushy_walking[current_mushy_room][touching_mushy_index] =
                mushy_walking[current_mushy_room][last];
            --mushy_counts[current_mushy_room];
            --active_mushy_count;
            if (touching_mushy_index < active_mushy_count)
            {
                mushy_sprites[touching_mushy_index].set_position(
                    mushy_room_x[current_mushy_room][touching_mushy_index],
                    mushy_room_y[current_mushy_room][touching_mushy_index]);
                mushy_sprites[touching_mushy_index].set_tiles(bn::sprite_items::mushy.tiles_item(),
                    mushy_direction_frames[current_mushy_room][touching_mushy_index] +
                        (mushy_walking[current_mushy_room][touching_mushy_index] ?
                            mushy_animation_frame : 0));
            }
            mushy_sprites[last].set_visible(false);
            stamina -= stamina_capacity_frames / 5;
            if (stamina < 0) stamina = 0;
            update_stamina_bar_fill(stamina);
            start_stamina_rejection_flash();
            bn::sound_items::hit.play();
        }

        if (plant_bullet_hit_player && !dashing)
        {
            damage_player();
        }
        if (touching_enemy && !enemy_contact_latched && !dashing)
        {
            damage_player();
        }
        if (touching_active_spikes && !spike_contact_latched && !dashing)
        {
            damage_player();
        }
        spike_contact_latched = touching_active_spikes && !dashing;
        // Si el dash atraviesa a un enemigo, no registres el contacto como
        // atendido: al terminar el dash, seguir encima del enemigo debe herir.
        enemy_contact_latched = touching_enemy && !dashing;
        update_player_damage_blink();

        if (!campfire_used_this_floor && room_manager.current_template() == RoomTemplateId::SPECIAL &&
            x > campfire_x - 16 && x < campfire_x + 16 &&
            y > campfire_y - 16 && y < campfire_y + 16)
        {
            campfire_used_this_floor = true;
            bn::sound_items::campfire.play();
            campfire_use_frames_remaining = campfire_use_duration_frames;
            dash_active = false;
            energy += 7;
            if (energy > energy_capacity)
            {
                energy = energy_capacity;
            }
            stamina += stamina_capacity_frames * 3 / 10;
            if (stamina > stamina_capacity_frames)
            {
                stamina = stamina_capacity_frames;
            }
            update_energy_label();
            update_stamina_bar_fill(stamina);
            start_energy_recovery_flash();
            start_stamina_recovery_flash();
        }

        ++campfire_animation_timer;
        if (campfire_animation_timer >= 8)
        {
            campfire_animation_timer = 0;
            campfire_frame = (campfire_frame + 1) % 4;
            campfire.set_tiles(bn::sprite_items::campfire.tiles_item(), campfire_frame);
        }

        ++potion_animation_timer;
        if (potion_animation_timer >= 12)
        {
            potion_animation_timer = 0;
            potion_animation_frame = (potion_animation_frame + 1) % 3;
            energy_potion_sprite.set_tiles(
                bn::sprite_items::posion_energy_16x16_animate.tiles_item(), potion_animation_frame);
            sprint_potion_sprite.set_tiles(
                bn::sprite_items::posion_sprint_16x16_animate.tiles_item(), potion_animation_frame);
            if (campfire_reward_active && campfire_reward_sprite.has_value())
            {
                if (campfire_reward_type == RoomItemType::ENERGY_POTION)
                {
                    campfire_reward_sprite->set_tiles(
                        bn::sprite_items::posion_energy_16x16_animate.tiles_item(),
                        potion_animation_frame);
                }
                else if (campfire_reward_type == RoomItemType::SPRINT_POTION)
                {
                    campfire_reward_sprite->set_tiles(
                        bn::sprite_items::posion_sprint_16x16_animate.tiles_item(),
                        potion_animation_frame);
                }
            }
        }

        update_reward_lever_visibility();
        update_reward_room_reward_visibility();
        for(int index = 0; index < 3 && index < reward_room_reward_sprites.size(); ++index)
        {
            if(reward_room_rewards[index].active &&
               reward_room_rewards[index].type == RoomItemType::ENERGY_POTION)
            {
                reward_room_reward_sprites[index].set_tiles(
                    bn::sprite_items::posion_energy_16x16_animate.tiles_item(),
                    potion_animation_frame);
            }
            else if(reward_room_rewards[index].active &&
                    reward_room_rewards[index].type == RoomItemType::SPRINT_POTION)
            {
                reward_room_reward_sprites[index].set_tiles(
                    bn::sprite_items::posion_sprint_16x16_animate.tiles_item(),
                    potion_animation_frame);
            }
        }
        for(int index = 0; index < 3; ++index)
        {
            if(reward_lever_activated[index] && reward_lever_animation_frames[index] < 2)
            {
                ++reward_lever_animation_timers[index];
                if(reward_lever_animation_timers[index] >= 8)
                {
                    reward_lever_animation_timers[index] = 0;
                    ++reward_lever_animation_frames[index];
                    if(reward_lever_sprite_index == index && !reward_lever_sprites.empty())
                    {
                        reward_lever_sprites[0].set_tiles(
                            bn::sprite_items::label_16x16_animate.tiles_item(),
                            reward_lever_animation_frames[index]);
                    }
                }
            }
        }

        if (energy == 0)
        {
            show_game_over();
            bn::core::update();
            continue;
        }

        update_player_animation(moving, dashing);

        update_stone_layers();
        player.set_position(x, y);
        update_imported_chest();

        if (room_manager.current_template() == RoomTemplateId::SPECIAL)
        {
            ++delivered_npc_animation_timer;
            if (delivered_npc_animation_timer >= delivered_npc_animation_interval)
            {
                delivered_npc_animation_timer = 0;
                delivered_npc_animation_alt_frame = ! delivered_npc_animation_alt_frame;
            }
        }
        else
        {
            delivered_npc_animation_timer = 0;
            delivered_npc_animation_alt_frame = false;
        }

        update_npc_sprites(displayed_frame);
        update_reward_lever_visibility();
        // Movimiento -----------------------

        // Pantallas +++++++++++++++++++++
        switch (pantalla_actual)
        {
        case Pantalla::MENU:
            // Mostrar el menú.
            break;

        case Pantalla::MAPA:
            // Mover al personaje.
            // Mostrar fondo y nubes.
            break;

        case Pantalla::COMBATE:
            // Ejecutar la lógica del combate.
            break;

        case Pantalla::INVENTARIO:
            // Mostrar los objetos.
            break;

        default:

            break;
        }

        // Pantallas ----------------------

        bn::core::update();
    }
}
