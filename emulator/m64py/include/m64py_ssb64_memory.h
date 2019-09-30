#ifndef M64PY_SSB64_MEMORY_H
#define M64PY_SSB64_MEMORY_H

#include <stdint.h>

typedef struct m64py_Emulator_corelib_interface m64py_Emulator_corelib_interface;

typedef enum m64py_region_e
{
    REGION_JAPAN = 0,
    REGION_AUSTRALIA,
    REGION_EUROPE,
    REGION_USA,
    REGION_IQUE
} m64py_region_e;

typedef enum m64py_character_e
{
    FIGHTER_MARIO                 = 0x00,
    FIGHTER_FOX                   = 0x01,
    FIGHTER_DONKEY_KONG           = 0x02,
    FIGHTER_SAMUS                 = 0x03,
    FIGHTER_LUIGI                 = 0x04,
    FIGHTER_LINK                  = 0x05,
    FIGHTER_YOSHI                 = 0x06,
    FIGHTER_FALCON                = 0x07,
    FIGHTER_KIRBY                 = 0x08,
    FIGHTER_PIKACHU               = 0x09,
    FIGHTER_JIGGLYPUFF            = 0x0A,
    FIGHTER_NESS                  = 0x0B,
    FIGHTER_MASTER_HAND           = 0x0C,
    FIGHTER_METAL_MARIO           = 0x0D,
    FIGHTER_POLYGON_MARIO         = 0x0E,
    FIGHTER_POLYGON_FOX           = 0x0F,
    FIGHTER_POLYGON_DONKEY_KONG   = 0X10,
    FIGHTER_POLYGON_SAMUS         = 0x11,
    FIGHTER_POLYGON_LUIGI         = 0x12,
    FIGHTER_POLYGON_LINK          = 0x13,
    FIGHTER_POLYGON_YOSHI         = 0x14,
    FIGHTER_POLYGON_FALCON        = 0x15,
    FIGHTER_POLYGON_KIRBY         = 0x16,
    FIGHTER_POLYGON_PIKACHU       = 0x17,
    FIGHTER_POLYGON_JIGGLYPUFF    = 0x18,
    FIGHTER_POLYGON_NESS          = 0x19,
    FIGHTER_GIANT_DONKEY_KONG     = 0x1A,
    /* FIGHTER_CRASH              = 0x1B, */
    FIGHTER_NONE                  = 0x1C
} m64py_character_e;

typedef enum m64py_stage_e
{
    STAGE_PEACHS_CASTLE           = 0x00,
    STAGE_SECTOR_Z                = 0x01,
    STAGE_KONGO_JUNGLE            = 0x02,
    STAGE_PLANET_ZEBES            = 0x03,
    STAGE_HYRULE_CASTLE           = 0x04,
    STAGE_YOSHIS_ISLAND           = 0x05,
    STAGE_DREAM_LAND              = 0x06,
    STAGE_SAFFRON_CITY            = 0x07,
    STAGE_MUSHROOM_KINGDOM        = 0x08,
    STAGE_DREAM_LAND_BETA_1       = 0x09,
    STAGE_DREAM_LAND_BETA_2       = 0x0A,
    STAGE_DEMO_STAGE              = 0x0B,
    STAGE_YOSHIS_ISLAND_NO_CLOUDS = 0x0C,
    STAGE_METAL_MARIO             = 0x0D,
    STAGE_POLYGON_TEAM            = 0x0E,
    STAGE_RACE_TO_THE_FINISH      = 0x0F,
    STAGE_FINAL_DESTINATION       = 0x10,
    STAGE_BTT_MARIO               = 0x11,
    STAGE_BTT_FOX                 = 0x12,
    STAGE_BTT_DONKEY_KONG         = 0x13,
    STAGE_BTT_SAMUS               = 0x14,
    STAGE_BTT_LUIGI               = 0x15,
    STAGE_BTT_LINK                = 0x16,
    STAGE_BTT_YOSHI               = 0x17,
    STAGE_BTT_FALCON              = 0x18,
    STAGE_BTT_KIRBY               = 0x19,
    STAGE_BTT_PIKACHU             = 0x1A,
    STAGE_BTT_JIGGLYPUFF          = 0x1B,
    STAGE_BTT_NESS                = 0x1C,
    STAGE_BTP_MARIO               = 0x1D,
    STAGE_BTP_FOX                 = 0x1E,
    STAGE_BTP_DONKEY_KONG         = 0x1F,
    STAGE_BTP_SAMUS               = 0x20,
    STAGE_BTP_LUIGI               = 0x21,
    STAGE_BTP_LINK                = 0x22,
    STAGE_BTP_YOSHI               = 0x23,
    STAGE_BTP_FALCON              = 0x24,
    STAGE_BTP_KIRBY               = 0x25,
    STAGE_BTP_PIKACHU             = 0x26,
    STAGE_BTP_JIGGLYPUFF          = 0x27,
    STAGE_BTP_NESS                = 0x28
} m64py_stage_e;

typedef enum m64py_fighter_controlled_by_e
{
    CONTROLLED_BY_HUMAN = 0,
    CONTROLLED_BY_AI = 1,
    CONTROLLED_BY_NONE = 2
} m64py_fighter_controlled_by_e;

typedef enum m64py_screen_e
{
    SCREEN_MODE_SELECT = 3,
    SCREEN_TITLE = 4,
    SCREEN_VS_MODE = 5,
    SCREEN_INTRO_CUTSCENE = 6,
    SCREEN_GAME = 8,
    SCREEN_RESULTS = 11,
    SCREEN_CHARACTER_SELECT = 14,

} m64py_screen_e;

typedef struct m64py_memory_interface_t
{
    m64py_Emulator_corelib_interface* corelib;
    m64py_region_e region;
} m64py_memory_interface_t;

m64py_memory_interface_t* m64py_memory_interface_create(m64py_Emulator_corelib_interface* corelib, m64py_region_e region);
void m64py_memory_interface_destroy(m64py_memory_interface_t* memory);

void m64py_memory_set_items(m64py_memory_interface_t* memory, int enable);
void m64py_memory_unlock_characters(m64py_memory_interface_t* memory);
int m64py_memory_match_settings_get_time(m64py_memory_interface_t* memory, uint8_t* time_in_minutes, const char** error_msg);
int m64py_memory_match_settings_set_time(m64py_memory_interface_t* memory, uint8_t time_in_minutes, const char** error_msg);
int m64py_memory_match_settings_get_stocks(m64py_memory_interface_t* memory, uint8_t* stockcount, const char** error_msg);
int m64py_memory_match_settings_set_stocks(m64py_memory_interface_t* memory, uint8_t stockcount, const char** error_msg);
int m64py_memory_match_settings_get_stocks_timed(m64py_memory_interface_t* memory, int* stockcount, int* time_in_minutes, const char** error_msg);
int m64py_memory_match_settings_set_stocks_timed(m64py_memory_interface_t* memory, int stockcount, int time_in_minutes, const char** error_msg);
int m64py_memory_match_settings_get_fighter_character(m64py_memory_interface_t* memory, int player_slot, m64py_character_e* character, const char** error_msg);
int m64py_memory_match_settings_set_fighter_character(m64py_memory_interface_t* memory, int player_slot, m64py_character_e character, const char** error_msg);
int m64py_memory_match_settings_get_stage(m64py_memory_interface_t* memory, m64py_stage_e* stage, const char** error_msg);
int m64py_memory_match_settings_set_stage(m64py_memory_interface_t* memory, m64py_stage_e stage, const char** error_msg);

void m64py_memory_call_start_game(m64py_memory_interface_t* memory);

int m64py_memory_get_fighter_address(m64py_memory_interface_t* memory, int fighter_memory_index, uint32_t* fighter_address, const char** error_msg);
void m64py_memory_read_fighter_character(m64py_memory_interface_t* memory, uint32_t fighter_address, m64py_character_e* fighter);
int m64py_memory_read_fighter_position(m64py_memory_interface_t* memory, uint32_t fighter_address, float* xpos, float* ypos, const char** error_msg);
void m64py_memory_read_fighter_velocity(m64py_memory_interface_t* memory, uint32_t fighter_address, float* xvel, float* yvel);
void m64py_memory_read_fighter_acceleration(m64py_memory_interface_t* memory, uint32_t fighter_address, float* xacc, float* yacc);
void m64py_memory_read_fighter_orientation(m64py_memory_interface_t* memory, uint32_t fighter_address, int32_t* orientation);
void m64py_memory_read_fighter_movement_frame(m64py_memory_interface_t* memory, uint32_t fighter_address, uint32_t* frame);
void m64py_memory_read_fighter_movement_state(m64py_memory_interface_t* memory, uint32_t fighter_address, int16_t* state);
void m64py_memory_read_fighter_shield_health(m64py_memory_interface_t* memory, uint32_t fighter_address, uint32_t* shield);
void m64py_memory_read_fighter_shield_break_recovery_timer(m64py_memory_interface_t* memory, uint32_t fighter_address, uint32_t* time_left);
void m64py_memory_read_fighter_percent(m64py_memory_interface_t* memory, uint32_t fighter_address, uint32_t* percent);
void m64py_memory_read_fighter_is_invincible(m64py_memory_interface_t* memory, uint32_t fighter_address, int* is_invincible);
void m64py_memory_read_fighter_is_grounded(m64py_memory_interface_t* memory, uint32_t fighter_address, int* is_grounded);
void m64py_memory_read_fighter_stocks(m64py_memory_interface_t* memory, int fighter_idx, uint8_t* stock_count);

void m64py_memory_read_whispy_wind(m64py_memory_interface_t* memory, float* blowing_direction);

#endif /* M64PY_SSB64_MEMORY_H */
