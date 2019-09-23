#include "m64py_type_SSB64.h"

#define M64P_CORE_PROTOTYPES

#include <stdint.h>
#include <stdio.h>

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
#define READ_PLAYER_POSITION_DOC \
"-> Tuple[float,float]: Absolute position of player"
static PyObject*
read_player_position(PyObject* self, PyObject* arg)
{
    PyObject *xpos, *ypos, *args;

    if ((xpos = PyFloat_FromDouble(0.0)) == NULL)
        goto alloc_xpos_failed;

    if ((ypos = PyFloat_FromDouble(0.0)) == NULL)
        goto alloc_ypos_failed;

    if ((args = PyTuple_New(2)) == NULL)
        goto alloc_args_failed;

    PyTuple_SET_ITEM(args, 0, xpos);
    PyTuple_SET_ITEM(args, 1, ypos);
    return args;

    alloc_args_failed : Py_DECREF(ypos);
    alloc_ypos_failed : Py_DECREF(xpos);
    alloc_xpos_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
#define READ_PLAYER_ORIENTATION_DOC \
"-> int: -1=facing left, 1=facing right"
static PyObject*
read_player_orientation(m64py_SSB64* self, PyObject* arg)
{
    int player_idx;

    if (!PyLong_Check(arg))
    {
        PyErr_SetString(PyExc_TypeError, "Expected player index [0-3] as an integer");
        return NULL;
    }
    player_idx = PyLong_AS_LONG(arg);
    if (player_idx < 0 || player_idx > 3)
    {
        PyErr_Format(PyExc_ValueError, "Player index is out of bounds (%d)", player_idx);
        return NULL;
    }

    int direction_facing = self->emu->corelib.DebugMemRead32(0x80267F34);
    return PyLong_FromLong(direction_facing);
}

/* ------------------------------------------------------------------------- */
#define READ_PLAYER_ANIM_STATE_DOC \
"-> int: The currently active animation, a number between 0-80 or something"
static PyObject*
read_player_anim_state(PyObject* self, PyObject* arg)
{
    return PyLong_FromLong(0);
}

/* ------------------------------------------------------------------------- */
#define READ_PLAYER_ANIM_PROGRESS_DOC \
"-> float: Progress of the currently active animation from 0-1"
static PyObject*
read_player_anim_progress(PyObject* self, PyObject* arg)
{
    return PyFloat_FromDouble(0.0);
}

/* ------------------------------------------------------------------------- */
#define READ_PLAYER_SHIELD_HEALTH_DOC \
"-> float: How much shield the player currently has from 0-1"
static PyObject*
read_player_shield_health(PyObject* self, PyObject* arg)
{
    return PyFloat_FromDouble(1.0);
}

/* ------------------------------------------------------------------------- */
#define READ_PLAYER_DAMAGE_DOC \
"-> int: Current \"percentage\" of the player from 0-999"
static PyObject*
read_player_damage(PyObject* self, PyObject* arg)
{
    return PyLong_FromLong(0);
}

/* ------------------------------------------------------------------------- */
#define READ_WHISPY_WIND_DOC \
"-> float: 0=no wind, -1=blowing left, 1=blowing right. Will ramp up/down as the animation begins/ends"
static PyObject*
read_whispy_wind(PyObject* self, PyObject* arg)
{
    return PyFloat_FromDouble(0.0);
}

/* ------------------------------------------------------------------------- */
static PyMethodDef SSB64_methods[] = {
    {"set_tournament_rules",      set_tournament_rules,                   METH_NOARGS, ""},
    {"set_character",             set_character,                          METH_VARARGS, ""},
    {"set_stage",                 set_stage,                              METH_O, ""},
    {"start_game",                start_game,                             METH_NOARGS, ""},
    {"is_running",                is_running,                             METH_NOARGS, ""},
    {"read_player_position",      read_player_position,                   METH_O, READ_PLAYER_POSITION_DOC},
    {"read_player_orientation",   (PyCFunction)read_player_orientation,   METH_O, READ_PLAYER_ORIENTATION_DOC},
    {"read_player_anim_state",    read_player_anim_state,                 METH_O, READ_PLAYER_ANIM_STATE_DOC},
    {"read_player_anim_progress", read_player_anim_progress,              METH_O, READ_PLAYER_ANIM_PROGRESS_DOC},
    {"read_player_shield_health", read_player_shield_health,              METH_O, READ_PLAYER_SHIELD_HEALTH_DOC},
    {"read_player_damage",        read_player_damage,                     METH_O, READ_PLAYER_DAMAGE_DOC},
    {"read_whispy_wind",          read_whispy_wind,                       METH_NOARGS, READ_WHISPY_WIND_DOC},
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
