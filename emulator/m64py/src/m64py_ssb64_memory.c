#include "m64py_ssb64_memory.h"
#include "m64py_type_Emulator.h"
#include <stdlib.h>

#define RDRAM_START 0x80000000

/*
enum player_base_addresses_e
{
    BA_PEACHS_CASTLE   = 0x8025E174,
    BA_CONGO_JUNGLE    = 0x8026AEA4,
    BA_HYRULE_CASTLE   = 0x80262F34,
    BA_PLANET_ZEBES    = 0x8026A37C,
    BA_YOSHIS_ISLAND   = 0x8026D7FC,
    BA_DREAMLAND       = 0x80267F14,
    BA_SECTOR_Z        = 0x8026B4CC,
    BA_SAFFRON_CITY    = 0x8026DE6C
};
*/
/*
enum player_offsets_e
{
    OFF_STATE1      = 0x00,
    OFF_STATE2      = 0x04,
    OFF_PERCENT     = 0x08,
    OFF_SHIELD      = 0x10,
    OFF_ORIENTATION = 0x20,
    OFF_SPEED_X     = 0x24,
    OFF_SPEED_Y     = 0x28,
    OFF_LAUNCH_X    = 0x30,
    OFF_LAUNCH_Y    = 0x34,
    OFF_SPEED_X_ABS = 0x3C,
    OFF_POS_X       = 0x5C,
    OFF_POS_Y       = 0x60,
    OFF_FLAGS       = 0xA40
};*/

static const struct memory_t
{
    uint32_t MUSIC;
    uint32_t UNLOCKED_STUFF;
    uint32_t MATCH_SETTINGS_PTR;
    uint32_t HURTBOX_COLOR_RG;
    uint32_t HURTBOX_COLOR_BA;
    uint32_t RED_HITBOX_PATCH;
    uint32_t PURPLE_HITBOX_PATCH;
    uint32_t PLAYER_LIST_PTR;
    uint32_t ITEM_LIST_PTR;
    uint32_t ITEM_HITBOX_OFFSET;
} MEMORY[] = {
    {0x8098BD3, 0x80A28F4, 0x80A30A8, 0,       0,       0,       0,       0x8012E914, 0x80466F0, 0x370},  /* Japan */
    {0x8099833, 0x80A5074, 0x80A5828, 0,       0,       0,       0,       0x80131594, 0x8046E20, 0    },  /* Australia */
    {0x80A2E63, 0x80AD194, 0x80AD948, 0,       0,       0,       0,       0x80139A74, 0x8046E60, 0    },  /* Europe */
    {0x8099113, 0x80A4934, 0x80A50E8, 0xF2786, 0xF279E, 0xF33BC, 0xF2FD0, 0x80130D84, 0x8046700, 0x374},  /* USA */
    {0x8092993, 0x80A4988, 0x80A5C68, 0,       0,       0,       0,       0x80130F04, 0x8098450, 0x374}   /* iQue */
};

static const struct player_field_t
{
    uint32_t CHARACTER;
    uint32_t COSTUME;
    uint32_t MOVEMENT_FRAME;
    uint32_t MOVEMENT_STATE;
    uint32_t PERCENT;
    uint32_t SHIELD_SIZE;
    uint32_t FACING_DIRECTION;
    uint32_t VELOCITY_X;
    uint32_t VELOCITY_Y;
    uint32_t ACCELERATION_X;
    uint32_t ACCELERATION_Y;
    uint32_t POSITION_VECTOR_PTR;
    struct player_position_data_t {
        uint32_t POS_X;
        uint32_t POS_Y;
    } POSITION_VECTOR;
    uint32_t JUMP_COUNTER;
    uint32_t GROUNDED;
    uint32_t CONTROLLER_INPUT_PTR;
    uint32_t SHIELD_BREAK_RECOVERY_TIME;
    uint32_t INVINCIBILITY_STATE;
    /* a bunch more stuff we probably don't care about */
    uint32_t SHOW_HITBOX;
} PLAYER_FIELD = {
    .CHARACTER                  = 0x0B,
    .COSTUME                    = 0x10,
    .MOVEMENT_FRAME             = 0x1C,
    .MOVEMENT_STATE             = 0x26,
    .PERCENT                    = 0x2C,
    .SHIELD_SIZE                = 0x34,
    .FACING_DIRECTION           = 0x44,
    .VELOCITY_X                 = 0x48,
    .VELOCITY_Y                 = 0x4C,
    .ACCELERATION_X             = 0x60,
    .ACCELERATION_Y             = 0x64,
    .POSITION_VECTOR_PTR        = 0x78,
    .POSITION_VECTOR = {
        .POS_X                  = 0x00,
        .POS_Y                  = 0x04
    },
    .JUMP_COUNTER               = 0x148,
    .GROUNDED                   = 0x14C,
    .CONTROLLER_INPUT_PTR       = 0x1B0,
    .SHIELD_BREAK_RECOVERY_TIME = 0x26C,
    .INVINCIBILITY_STATE        = 0x5AC,
    .SHOW_HITBOX                = 0xB4C
};

static const unsigned int STOCK_COUNTERS = 0x801317CC;
static const unsigned int WHISPY_BLOWING = 0x80304BFC;

/* -------------------------------------------------------------------------- */
m64py_memory_interface_t*
m64py_memory_interface_create(m64py_Emulator_corelib_interface* corelib, m64py_region_e region)
{
    m64py_memory_interface_t* memory = malloc(sizeof *memory);
    if (memory == NULL)
        return NULL;
    memory->corelib = corelib;
    memory->region = region;
    return memory;
}

/* -------------------------------------------------------------------------- */
void
m64py_memory_interface_destroy(m64py_memory_interface_t* memory)
{
    free(memory);
}

/* -------------------------------------------------------------------------- */
void
m64py_memory_set_items(m64py_memory_interface_t* memory, int enable)
{

}

/* -------------------------------------------------------------------------- */
void
m64py_memory_unlock_characters(m64py_memory_interface_t* memory)
{

}

/* -------------------------------------------------------------------------- */
void
m64py_memory_set_stocks(m64py_memory_interface_t* memory, int stockcount)
{

}

/* -------------------------------------------------------------------------- */
void
m64py_memory_set_time(m64py_memory_interface_t* memory, int time_in_minutes)
{

}

/* -------------------------------------------------------------------------- */
void
m64py_memory_set_fighter(m64py_memory_interface_t* memory, int player_idx, m64py_fighter_e fighter)
{

}

/* -------------------------------------------------------------------------- */
void
m64py_memory_set_stage(m64py_memory_interface_t* memory, m64py_stage_e stage)
{

}

/* -------------------------------------------------------------------------- */
void
m64py_memory_get_whispy_wind(m64py_memory_interface_t* memory, float* blowing_direction)
{

}

/* -------------------------------------------------------------------------- */
void
m64py_memory_call_start_game(m64py_memory_interface_t* memory)
{
}

/* -------------------------------------------------------------------------- */
int
m64py_memory_get_fighter_address(m64py_memory_interface_t* memory, int fighter_idx, uint32_t* fighter_address, const char** error_msg)
{
    /*
     * There's a variable at a fixed memory location that points to the beginning
     * of the list of fighter structures. Try dereferencing it to get the address
     * of the first fighter
     */
    *fighter_address = memory->corelib->DebugMemRead32(MEMORY[memory->region].PLAYER_LIST_PTR);
    if (*fighter_address < RDRAM_START)
    {
        *error_msg = "Player structures haven't been allocated yet";
        return 0;
    }

    *fighter_address += 0xB50 * fighter_idx;  /* sizeof(Fighter) * fighter_idx */
    return 1;
}


/* -------------------------------------------------------------------------- */
int
m64py_memory_read_fighter_position(m64py_memory_interface_t* memory, uint32_t fighter_address, float* xpos, float* ypos, const char** error_msg)
{
    uint32_t pos_vec_address, xpos_raw, ypos_raw;

    /*
     * The fighter's position vector is allocated separately from the fighter
     * struct itself. Dereference pointer to the position vector to get the
     * address of the position vector.
     */
    pos_vec_address = memory->corelib->DebugMemRead32(fighter_address + PLAYER_FIELD.POSITION_VECTOR_PTR);
    if (pos_vec_address < RDRAM_START)
    {
        *error_msg = "Player position is NULL";
        return 0;
    }

    /* Can now read position vector, which is a ieee754 single precision float */
    xpos_raw = memory->corelib->DebugMemRead32(pos_vec_address + PLAYER_FIELD.POSITION_VECTOR.POS_X);
    ypos_raw = memory->corelib->DebugMemRead32(pos_vec_address + PLAYER_FIELD.POSITION_VECTOR.POS_Y);

    /* reinterpret as float */
    *xpos = *(float*)&xpos_raw;
    *ypos = *(float*)&ypos_raw;

    return 1;
}

/* -------------------------------------------------------------------------- */
void
m64py_memory_read_fighter_velocity(m64py_memory_interface_t* memory, uint32_t fighter_address, float* xvel, float* yvel)
{
    uint32_t xvel_raw, yvel_raw;

    xvel_raw = memory->corelib->DebugMemRead32(fighter_address + PLAYER_FIELD.VELOCITY_X);
    yvel_raw = memory->corelib->DebugMemRead32(fighter_address + PLAYER_FIELD.VELOCITY_Y);

    /* reinterpret as float */
    *xvel = *(float*)&xvel_raw;
    *yvel = *(float*)&yvel_raw;
}

/* -------------------------------------------------------------------------- */
void
m64py_memory_read_fighter_acceleration(m64py_memory_interface_t* memory, uint32_t fighter_address, float* xacc, float* yacc)
{
    uint32_t xacc_raw, yacc_raw;

    xacc_raw = memory->corelib->DebugMemRead32(fighter_address + PLAYER_FIELD.ACCELERATION_X);
    yacc_raw = memory->corelib->DebugMemRead32(fighter_address + PLAYER_FIELD.ACCELERATION_Y);

    /* reinterpret as float */
    *xacc = *(float*)&xacc_raw;
    *yacc = *(float*)&yacc_raw;
}

/* -------------------------------------------------------------------------- */
void
m64py_memory_read_fighter_orientation(m64py_memory_interface_t* memory, uint32_t fighter_address, int* orientation)
{
}

/* -------------------------------------------------------------------------- */
void
m64py_memory_read_fighter_movement_frame(m64py_memory_interface_t* memory, uint32_t fighter_address, uint32_t* frame)
{
}

/* -------------------------------------------------------------------------- */
void
m64py_memory_read_fighter_movement_state(m64py_memory_interface_t* memory, uint32_t fighter_address, int16_t* state)
{
}

/* -------------------------------------------------------------------------- */
void
m64py_memory_read_fighter_shield_health(m64py_memory_interface_t* memory, uint32_t fighter_address, uint32_t* shield)
{
}

/* -------------------------------------------------------------------------- */
void
m64py_memory_read_fighter_shield_break_recovery_timer(m64py_memory_interface_t* memory, uint32_t fighter_address, uint32_t* time_left)
{
}

/* -------------------------------------------------------------------------- */
void
m64py_memory_read_fighter_percent(m64py_memory_interface_t* memory, uint32_t fighter_address, uint16_t* percent)
{
}

/* -------------------------------------------------------------------------- */
void
m64py_memory_read_fighter_is_invincible(m64py_memory_interface_t* memory, uint32_t fighter_address, int* is_invincible)
{
}

/* -------------------------------------------------------------------------- */
void
m64py_memory_read_fighter_is_grounded(m64py_memory_interface_t* memory, uint32_t fighter_address, int* is_grounded)
{
}

/* -------------------------------------------------------------------------- */
void
m64py_memory_read_fighter_stocks(m64py_memory_interface_t* memory, int fighter_idx, uint8_t* stock_count)
{

}
