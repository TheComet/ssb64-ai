#include "m64py_type_SSB64.h"

#define M64P_CORE_PROTOTYPES

#include <stdint.h>
#include <stdio.h>

enum regions_e
{
    REGION_JAPAN,
    REGION_AUSTRALIA,
    REGION_EUROPE,
    REGION_USA,
    REGION_IQUE
};
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

static const unsigned int ADDR_STOCK_COUNTERS = 0x801317CC;
static const unsigned int ADDR_WHISPY_BLOWING = 0x80304BFC;

/* ------------------------------------------------------------------------- */
static void
SSB64_dealloc(m64py_SSB64* self)
{
    m64py_Emulator_stop_plugins(self->emu);
    self->emu->corelib.CoreDoCommand(M64CMD_ROM_CLOSE, 0, NULL);
    self->emu->is_rom_loaded = 0;
    Py_XDECREF(self->emu);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
SSB64_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_SSB64* self;
    const char* rom_file_name;
    FILE* rom_file;
    long rom_size;
    void* rom_buffer;
    m64p_error result;

    self = (m64py_SSB64*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    if (!PyArg_ParseTuple(args, "O!s", &m64py_EmulatorType, &self->emu, &rom_file_name))
        goto parse_args_failed;
    Py_INCREF(self->emu);

    /* Corelib expects a memory blob containing the ROM, so we have to load
     * the file into memory */
    rom_file = fopen(rom_file_name, "rb");
    if (rom_file == NULL)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to open file \"%s\"", rom_file_name);
        goto open_rom_file_failed;
    }

    /* Determine length */
    fseek(rom_file, 0L, SEEK_END);
    rom_size = ftell(rom_file);
    fseek(rom_file, 0L, SEEK_SET);

    /* Copy ROM into memory */
    rom_buffer = malloc(rom_size);
    if (rom_buffer == NULL)
    {
        PyErr_NoMemory();
        goto malloc_rom_buffer_failed;
    }
    if (fread(rom_buffer, 1, rom_size, rom_file) != rom_size)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to read ROM file into memory");
        goto read_rom_failed;
    }

    /* Give ROM image to corelib */
    if ((result = self->emu->corelib.CoreDoCommand(M64CMD_ROM_OPEN, rom_size, rom_buffer)) != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to open ROM image: Error code %d", result);
        goto read_rom_failed;
    }

    /* With ROM successfully loaded, we can now start all plugins */
    if (m64py_Emulator_start_plugins(self->emu) != 0)
        goto start_plugins_failed;

    /* Cleanup - corelib copies buffer so these aren't needed */
    free(rom_buffer);
    fclose(rom_file);

    self->emu->is_rom_loaded = 1;
    return (PyObject*)self;

    start_plugins_failed     : self->emu->corelib.CoreDoCommand(M64CMD_ROM_CLOSE, 0, NULL);
    read_rom_failed          : free(rom_buffer);
    malloc_rom_buffer_failed : fclose(rom_file);
    open_rom_file_failed     :
    parse_args_failed        : Py_DECREF(self);
    alloc_self_failed        : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
set_tournament_rules(PyObject* self, PyObject* arg)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
set_character(PyObject* self, PyObject* arg)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
set_stage(PyObject* self, PyObject* arg)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
start_game(PyObject* self, PyObject* arg)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
is_running(PyObject* self, PyObject* arg)
{
    Py_RETURN_TRUE;
}

/* ------------------------------------------------------------------------- */
static int parse_player_idx(PyObject* arg)
{
    int player_idx;

    if (!PyLong_Check(arg))
    {
        PyErr_SetString(PyExc_TypeError, "Expected player index [0-3] as an integer");
        return -1;
    }
    player_idx = PyLong_AS_LONG(arg);
    if (player_idx < 0 || player_idx > 3)
    {
        PyErr_Format(PyExc_ValueError, "Player index is out of bounds (%d)", player_idx);
        return -1;
    }

    return player_idx;
}

/* ------------------------------------------------------------------------- */
#define READ_PLAYER_POSITION_DOC \
"-> Tuple[float,float]: Absolute position of player"
static PyObject*
read_player_position(m64py_SSB64* self, PyObject* arg)
{
    PyObject *py_xpos, *py_ypos, *args;
    int player_idx;
    unsigned int player_base_addr, pos_vec_address, xpos_raw, ypos_raw;
    float xpos, ypos;

    if ((player_idx = parse_player_idx(arg)) < 0)
        return NULL;

    player_base_addr = self->emu->corelib.DebugMemRead32(MEMORY[REGION_USA].PLAYER_LIST_PTR);
    if (player_base_addr == 0x0)
    {
        PyErr_SetString(PyExc_RuntimeError, "Player structures haven't been allocated yet");
        return NULL;
    }
    player_base_addr += 0xB50 * player_idx;  /* sizeof(Fighter) * player_idx */

    pos_vec_address = self->emu->corelib.DebugMemRead32(player_base_addr + PLAYER_FIELD.POSITION_VECTOR_PTR);
    if (pos_vec_address == 0x0)
    {
        PyErr_SetString(PyExc_RuntimeError, "Player position is NULL");
        return NULL;
    }

    xpos_raw = self->emu->corelib.DebugMemRead32(pos_vec_address + PLAYER_FIELD.POSITION_VECTOR.POS_X);
    ypos_raw = self->emu->corelib.DebugMemRead32(pos_vec_address + PLAYER_FIELD.POSITION_VECTOR.POS_X);

    /* It is a ieee754 float, hopefully this never gets compiled on a computer
     * that has a different fp implementation */
    xpos = *(float*)&xpos_raw;
    ypos = *(float*)&ypos_raw;

    if ((py_xpos = PyFloat_FromDouble(xpos)) == NULL)
        goto alloc_xpos_failed;
    if ((py_ypos = PyFloat_FromDouble(ypos)) == NULL)
        goto alloc_ypos_failed;
    if ((args = PyTuple_New(2)) == NULL)
        goto alloc_args_failed;
    PyTuple_SET_ITEM(args, 0, py_xpos);
    PyTuple_SET_ITEM(args, 1, py_ypos);

    return args;

    alloc_args_failed : Py_DECREF(py_ypos);
    alloc_ypos_failed : Py_DECREF(py_xpos);
    alloc_xpos_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
#define READ_PLAYER_LAUNCH_VELOCITY_DOC \
"-> Tuple[float,float]: Speed at which the player is currently being launched (e.g. after being hit by the opponent)"
static PyObject*
read_player_launch_velocity(m64py_SSB64* self, PyObject* arg)
{
    PyObject *py_xlaunch, *py_ylaunch, *args;
    int player_idx;
    unsigned int xlaunch_address, ylaunch_address, xlaunch_int, ylaunch_int;
    float xlaunch, ylaunch;

    if ((player_idx = parse_player_idx(arg)) < 0)
        return NULL;
/*
    xlaunch_address = ADDR_DREAMLAND_PLAYER[player_idx] + OFF_LAUNCH_X;
    ylaunch_address = ADDR_DREAMLAND_PLAYER[player_idx] + OFF_LAUNCH_Y;
    xlaunch_int = self->emu->corelib.DebugMemRead32(xlaunch_address);
    ylaunch_int = self->emu->corelib.DebugMemRead32(ylaunch_address);*/

    /* It is a ieee754 float, hopefully this never gets compiled on a computer
     * that has a different fp implementation */
    xlaunch = *(float*)&xlaunch_int;
    ylaunch = *(float*)&ylaunch_int;

    if ((py_xlaunch = PyFloat_FromDouble(xlaunch)) == NULL)
        goto alloc_xpos_failed;
    if ((py_ylaunch = PyFloat_FromDouble(ylaunch)) == NULL)
        goto alloc_ypos_failed;
    if ((args = PyTuple_New(2)) == NULL)
        goto alloc_args_failed;
    PyTuple_SET_ITEM(args, 0, py_xlaunch);
    PyTuple_SET_ITEM(args, 1, py_ylaunch);

    return args;

    alloc_args_failed : Py_DECREF(py_ylaunch);
    alloc_ypos_failed : Py_DECREF(py_xlaunch);
    alloc_xpos_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
#define READ_PLAYER_ORIENTATION_DOC \
"-> int: -1=facing left, 1=facing right"
static PyObject*
read_player_orientation(m64py_SSB64* self, PyObject* arg)
{
    int player_idx, orientation;
    unsigned int address;
    if ((player_idx = parse_player_idx(arg)) < 0)
        return NULL;
/*
    address = ADDR_DREAMLAND_PLAYER[player_idx] + OFF_ORIENTATION;
    orientation = self->emu->corelib.DebugMemRead32(address);*/

    return PyLong_FromLong(orientation);
}

/* ------------------------------------------------------------------------- */
#define READ_PLAYER_ANIM_STATE_DOC \
"-> int: The currently active animation, a number between 0-250 or something"
static PyObject*
read_player_anim_state(m64py_SSB64* self, PyObject* arg)
{
    int player_idx;
    unsigned int address, state;
    if ((player_idx = parse_player_idx(arg)) < 0)
        return NULL;
/*
    address = ADDR_DREAMLAND_PLAYER[player_idx] + OFF_STATE1;
    state = self->emu->corelib.DebugMemRead32(address);*/

    return PyLong_FromLong(state);
}

/* ------------------------------------------------------------------------- */
#define READ_PLAYER_ANIM_PROGRESS_DOC \
"-> float: Progress of the currently active animation from 0-1"
static PyObject*
read_player_anim_progress(m64py_SSB64* self, PyObject* arg)
{
    return PyFloat_FromDouble(0.0);
}

/* ------------------------------------------------------------------------- */
#define READ_PLAYER_SHIELD_HEALTH_DOC \
"-> int: How much shield the player currently has. Some characters have more shield health than others. Pikachu's is 55, for example.'"
static PyObject*
read_player_shield_health(m64py_SSB64* self, PyObject* arg)
{
    int player_idx;
    unsigned int address, shield;
    if ((player_idx = parse_player_idx(arg)) < 0)
        return NULL;
/*
    address = ADDR_DREAMLAND_PLAYER[player_idx] + OFF_SHIELD;
    shield = self->emu->corelib.DebugMemRead32(address);*/

    return PyLong_FromLong(shield);
}

/* ------------------------------------------------------------------------- */
#define READ_PLAYER_DAMAGE_DOC \
"-> int: Current \"percentage\" of the player from 0-999"
static PyObject*
read_player_damage(m64py_SSB64* self, PyObject* arg)
{
    int player_idx;
    unsigned int address, percent;
    if ((player_idx = parse_player_idx(arg)) < 0)
        return NULL;
/*
    address = ADDR_DREAMLAND_PLAYER[player_idx] + OFF_PERCENT;
    percent = self->emu->corelib.DebugMemRead32(address);*/

    return PyLong_FromLong(percent);
}

/* ------------------------------------------------------------------------- */
#define READ_WHISPY_WIND_DOC \
"-> float: 0=no wind, -1=blowing left, 1=blowing right. Will ramp up/down as the animation begins/ends"
static PyObject*
read_whispy_wind(m64py_SSB64* self, PyObject* arg)
{
    unsigned int wind_int;
    float wind;

    wind_int = self->emu->corelib.DebugMemRead32(ADDR_WHISPY_BLOWING);
    wind = *(float*)&wind_int;

    return PyFloat_FromDouble(wind);
}

/* ------------------------------------------------------------------------- */
static PyMethodDef SSB64_methods[] = {
    {"set_tournament_rules",        set_tournament_rules,                     METH_NOARGS, ""},
    {"set_character",               set_character,                            METH_VARARGS, ""},
    {"set_stage",                   set_stage,                                METH_O, ""},
    {"start_game",                  start_game,                               METH_NOARGS, ""},
    {"is_running",                  is_running,                               METH_NOARGS, ""},
    {"read_player_position",        (PyCFunction)read_player_position,        METH_O, READ_PLAYER_POSITION_DOC},
    {"read_player_launch_velocity", (PyCFunction)read_player_launch_velocity, METH_O, READ_PLAYER_LAUNCH_VELOCITY_DOC},
    {"read_player_orientation",     (PyCFunction)read_player_orientation,     METH_O, READ_PLAYER_ORIENTATION_DOC},
    {"read_player_anim_state",      (PyCFunction)read_player_anim_state,      METH_O, READ_PLAYER_ANIM_STATE_DOC},
    {"read_player_anim_progress",   (PyCFunction)read_player_anim_progress,   METH_O, READ_PLAYER_ANIM_PROGRESS_DOC},
    {"read_player_shield_health",   (PyCFunction)read_player_shield_health,   METH_O, READ_PLAYER_SHIELD_HEALTH_DOC},
    {"read_player_damage",          (PyCFunction)read_player_damage,          METH_O, READ_PLAYER_DAMAGE_DOC},
    {"read_whispy_wind",            (PyCFunction)read_whispy_wind,            METH_NOARGS, READ_WHISPY_WIND_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyTypeObject m64py_SSB64Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "m64py.SSB64",                /* tp_name */
    sizeof(m64py_SSB64),          /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)SSB64_dealloc,    /* tp_dealloc */
    0,                            /* tp_print */
    0,                            /* tp_getattr */
    0,                            /* tp_setattr */
    0,                            /* tp_reserved */
    0,                            /* tp_repr */
    0,                            /* tp_as_number */
    0,                            /* tp_as_sequence */
    0,                            /* tp_as_mapping */
    0,                            /* tp_hash  */
    0,                            /* tp_call */
    0,                            /* tp_str */
    0,                            /* tp_getattro */
    0,                            /* tp_setattro */
    0,                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  /* tp_flags */
    "SSB64 objects",              /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    SSB64_methods,                /* tp_methods */
    0,                            /* tp_members */
    0,                            /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    0,                            /* tp_init */
    0,                            /* tp_alloc */
    SSB64_new,                    /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
m64py_SSB64Type_init(void)
{
    if (PyType_Ready(&m64py_SSB64Type) < 0)
        return -1;
    return 0;
}
