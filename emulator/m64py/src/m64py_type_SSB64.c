#include "m64py_type_SSB64.h"
#include "m64py_type_Fighter_CaptainFalcon.h"
#include "m64py_type_Fighter_DonkeyKong.h"
#include "m64py_type_Fighter_Fox.h"
#include "m64py_type_Fighter_Jigglypuff.h"
#include "m64py_type_Fighter_Kirby.h"
#include "m64py_type_Fighter_Link.h"
#include "m64py_type_Fighter_Luigi.h"
#include "m64py_type_Fighter_Mario.h"
#include "m64py_type_Fighter_Ness.h"
#include "m64py_type_Fighter_Pikachu.h"
#include "m64py_type_Fighter_Samus.h"
#include "m64py_type_Fighter_Yoshi.h"
#include "m64py_type_Stage_DreamLand.h"
#include "m64py_type_Stage_HyruleCastle.h"
#include "m64py_type_Stage_KongoJungle.h"
#include "m64py_type_Stage_MushroomKingdom.h"
#include "m64py_type_Stage_PeachsCastle.h"
#include "m64py_type_Stage_PlanetZebes.h"
#include "m64py_type_Stage_SaffronCity.h"
#include "m64py_type_Stage_SectorZ.h"
#include "m64py_type_Stage_YoshisIsland.h"
#include "m64py_ssb64_memory.h"

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
static char* kwds_strings[] = {
    "emulator",
    "rom_file_name",
    NULL
};
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

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!s", kwds_strings, &m64py_EmulatorType, &self->emu, &rom_file_name))
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

    /* TODO: Figure out region from loaded ROM. For now just assume USA */
    self->memory_interface = m64py_memory_interface_create(&self->emu->corelib, REGION_USA);
    if (self->memory_interface == NULL)
        goto alloc_memory_interface_failed;

    /* With ROM successfully loaded, we can now start all plugins */
    if (m64py_Emulator_start_plugins(self->emu) != 0)
        goto start_plugins_failed;

    /* Cleanup - corelib copies buffer so these aren't needed */
    free(rom_buffer);
    fclose(rom_file);

    self->emu->is_rom_loaded = 1;
    return (PyObject*)self;

    start_plugins_failed          : m64py_memory_interface_destroy(self->memory_interface);
    alloc_memory_interface_failed : self->emu->corelib.CoreDoCommand(M64CMD_ROM_CLOSE, 0, NULL);
    read_rom_failed               : free(rom_buffer);
    malloc_rom_buffer_failed      : fclose(rom_file);
    open_rom_file_failed          :
    parse_args_failed             : Py_DECREF(self);
    alloc_self_failed             : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(GET_FIGHTER_DOC, "get_fighter(slot)\n--\n\n"
"Creates and returns an object that can interface with the fighter's in-game\n"
"state. The slot argument should be an integer between 1 and 4 and corresponds\n"
"to the player's index. For example, if player 1 and 3 are enabled (with 2 and\n"
"4 disabled), then you can get player 3 by calling get_fighter(3).\n\n"
"The returned type depends on the character you selected before starting the\n"
"match\n\n. Must be called after starting the match.");
static PyObject*
SSB64_get_fighter(m64py_SSB64* self, PyObject* idx)
{
    int i;
    int memory_index;
    long player_slot;
    m64py_character_e character[4];
    const char* error_msg;
    PyObject* args;
    PyObject* py_player_slot;
    PyObject* py_memory_index;
    PyTypeObject* FighterType;
    m64py_Fighter* fighter;

    /* Get and validate player index argument */
    if (!PyLong_CheckExact(idx))
    {
        PyErr_SetString(PyExc_TypeError, "get_fighter(): Expected integer between 0 and 3");
        return NULL;
    }
    player_slot = PyLong_AsLong(idx);
    if (player_slot < 1 || player_slot > 4)
    {
        PyErr_Format(PyExc_ValueError, "get_fighter(): Expected integer between 0 and 3 (got %d instead)", player_slot);
        return NULL;
    }

    /* Read match settings to see which player slots are enabled. This affects
     * each player's memory offset */
    for (i = 0; i != 4; ++i)
        if (!m64py_memory_match_settings_get_fighter_character(self->memory_interface, i + 1, &character[i], &error_msg))
        {
            PyErr_Format(PyExc_RuntimeError, "Failed to read match settings: %s\n", error_msg);
            return NULL;
        }

    /* Determine index for memory offset */
    memory_index = 0;
    for (i = 1; i != player_slot; ++i)
        if (character[i-1] != FIGHTER_NONE)
            ++memory_index;

    /* Determine the type of the object we want to create */
    FighterType = NULL;
    switch (character[player_slot - 1])
    {
        case FIGHTER_CAPTAIN_FALCON : FighterType = &m64py_CaptainFalconType; break;
        case FIGHTER_DONKEY_KONG    : FighterType = &m64py_DonkeyKongType;    break;
        case FIGHTER_FOX            : FighterType = &m64py_FoxType;           break;
        case FIGHTER_JIGGLYPUFF     : FighterType = &m64py_JigglypuffType;    break;
        case FIGHTER_KIRBY          : FighterType = &m64py_KirbyType;         break;
        case FIGHTER_LINK           : FighterType = &m64py_LinkType;          break;
        case FIGHTER_LUIGI          : FighterType = &m64py_LuigiType;         break;
        case FIGHTER_MARIO          : FighterType = &m64py_MarioType;         break;
        case FIGHTER_NESS           : FighterType = &m64py_NessType;          break;
        case FIGHTER_PIKACHU        : FighterType = &m64py_PikachuType;       break;
        case FIGHTER_SAMUS          : FighterType = &m64py_SamusType;         break;
        case FIGHTER_YOSHI          : FighterType = &m64py_YoshiType;         break;
        case FIGHTER_NONE    :
            PyErr_Format(PyExc_RuntimeError, "Fighter in slot %d is not fighting!", player_slot);
            return NULL;
        default :
            PyErr_Format(PyExc_RuntimeError, "Fighter character %d is currently not supported. Sorry!", character[player_slot - 1]);
            return NULL;
    }

    /* We have enough info to create the fighter object now */
    py_player_slot = PyLong_FromLong(player_slot);
    if (py_player_slot == NULL)
        goto alloc_player_slot_arg_failed;
    py_memory_index = PyLong_FromLong(memory_index);
    if (py_memory_index == NULL)
        goto alloc_memory_index_arg_failed;
    args = PyTuple_New(3);
    if (args == NULL)
        goto alloc_args_failed;

    Py_INCREF(self);
    PyTuple_SET_ITEM(args, 0, (PyObject*)self);
    PyTuple_SET_ITEM(args, 1, py_player_slot);
    PyTuple_SET_ITEM(args, 2, py_memory_index);
    fighter = (m64py_Fighter*)PyObject_CallObject((PyObject*)FighterType, args);
    Py_DECREF(args);

    return (PyObject*)fighter;

    alloc_args_failed             : Py_DECREF(py_memory_index);
    alloc_memory_index_arg_failed : Py_DECREF(py_player_slot);
    alloc_player_slot_arg_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(GET_STAGE_DOC, "get_stage()\n--\n\n"
"Creates and returns an object that lets you interface with the stage's state.\n"
"The returned type depends on the stage you selected before starting the match.\n\n"
"Must be called after starting the match");
static PyObject*
get_stage_determine_type(m64py_SSB64* self, PyObject* constructor_args)
{
    m64py_stage_e stage = -1;
    const char* error_msg;
    if (!m64py_memory_match_settings_get_stage(self->memory_interface, &stage, &error_msg))
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to read stage type: %s", error_msg);
        return NULL;
    }
    switch (stage)
    {
        case STAGE_DREAM_LAND       : return PyObject_CallObject((PyObject*)&m64py_DreamLandType, constructor_args);
        case STAGE_HYRULE_CASTLE    : return PyObject_CallObject((PyObject*)&m64py_HyruleCastleType, constructor_args);
        case STAGE_KONGO_JUNGLE     : return PyObject_CallObject((PyObject*)&m64py_KongoJungleType, constructor_args);
        case STAGE_MUSHROOM_KINGDOM : return PyObject_CallObject((PyObject*)&m64py_MushroomKingdomType, constructor_args);
        case STAGE_PEACHS_CASTLE    : return PyObject_CallObject((PyObject*)&m64py_PeachsCastleType, constructor_args);
        case STAGE_PLANET_ZEBES     : return PyObject_CallObject((PyObject*)&m64py_PlanetZebesType, constructor_args);
        case STAGE_SAFFRON_CITY     : return PyObject_CallObject((PyObject*)&m64py_SaffronCityType, constructor_args);
        case STAGE_SECTOR_Z         : return PyObject_CallObject((PyObject*)&m64py_SectorZType, constructor_args);
        case STAGE_YOSHIS_ISLAND    : return PyObject_CallObject((PyObject*)&m64py_YoshisIslandType, constructor_args);
        default :
            PyErr_SetString(PyExc_NotImplementedError, "A class for the currently loaded stage is not implemented. Sorry!");
            return NULL;
    }
}
static PyObject*
SSB64_get_stage(m64py_SSB64* self, PyObject* noargs)
{
    PyObject* result;
    PyObject* args = PyTuple_New(1);
    if (args == NULL)
        return NULL;
    Py_INCREF(self);
    PyTuple_SET_ITEM(args, 0, (PyObject*)self);

    result = get_stage_determine_type(self, args);

    Py_DECREF(args);
    return result;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef SSB64_methods[] = {
    {"get_fighter",  (PyCFunction)SSB64_get_fighter,  METH_O,       GET_FIGHTER_DOC},
    {"get_stage",    (PyCFunction)SSB64_get_stage,    METH_NOARGS,  GET_STAGE_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(MATCH_IN_PROGRESS_DOC,
"Returns True as soon as the match begins (after calling start_match()). Returns\n"
"False when one of the fighters wins the match");
static PyObject*
SSB64_getis_match_in_progress(m64py_SSB64* self, void* closure)
{
    if (m64py_memory_is_match_in_progress(self->memory_interface))
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}
static int
SSB64_setis_match_in_progress(m64py_SSB64* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Read only.");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef SSB64_getset[] = {
    {"is_match_in_progress", (getter)SSB64_getis_match_in_progress, (setter)SSB64_setis_match_in_progress, MATCH_IN_PROGRESS_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(SSB64_DOC,
"Object for interfacing with the memory of the game Super Smash Bros. 64");
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
    SSB64_DOC,                    /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    SSB64_methods,                /* tp_methods */
    0,                            /* tp_members */
    SSB64_getset,                 /* tp_getset */
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
