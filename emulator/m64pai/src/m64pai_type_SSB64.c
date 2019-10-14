#include "m64pai_type_SSB64.h"
#include "m64pai_type_Fighter_CaptainFalcon.h"
#include "m64pai_type_Fighter_DonkeyKong.h"
#include "m64pai_type_Fighter_Fox.h"
#include "m64pai_type_Fighter_Jigglypuff.h"
#include "m64pai_type_Fighter_Kirby.h"
#include "m64pai_type_Fighter_Link.h"
#include "m64pai_type_Fighter_Luigi.h"
#include "m64pai_type_Fighter_Mario.h"
#include "m64pai_type_Fighter_Ness.h"
#include "m64pai_type_Fighter_Pikachu.h"
#include "m64pai_type_Fighter_Samus.h"
#include "m64pai_type_Fighter_Yoshi.h"
#include "m64pai_type_Stage_DreamLand.h"
#include "m64pai_type_Stage_HyruleCastle.h"
#include "m64pai_type_Stage_KongoJungle.h"
#include "m64pai_type_Stage_MushroomKingdom.h"
#include "m64pai_type_Stage_PeachsCastle.h"
#include "m64pai_type_Stage_PlanetZebes.h"
#include "m64pai_type_Stage_SaffronCity.h"
#include "m64pai_type_Stage_SectorZ.h"
#include "m64pai_type_Stage_YoshisIsland.h"
#include "m64pai_ssb64_memory.h"

#include <stdint.h>
#include <stdio.h>

struct cheat_t
{
    const char* name;
    m64p_cheat_code code[2];
    int code_length;
};

static struct cheat_t cheats_usa[] = {
    {"Unlock Everything", {{0x800A4937, 0x00FF}, {0x810A4938, 0x0FF0}}, 2},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static void
SSB64_dealloc(m64pai_SSB64* self)
{
    m64pai_Emulator_stop_plugins(self->emu);
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
    m64pai_SSB64* self;
    const char* rom_file_name;
    FILE* rom_file;
    long rom_size;
    void* rom_buffer;
    m64p_error result;
    m64p_rom_settings rom_settings;
    struct cheat_t* cheat;

    self = (m64pai_SSB64*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!s", kwds_strings, &m64pai_EmulatorType, &self->emu, &rom_file_name))
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

    /* Make sure ROM is a SSB64 (U) and allocate the memory interace for this region */
    if ((result = self->emu->corelib.CoreDoCommand(M64CMD_ROM_GET_SETTINGS, sizeof(rom_settings), &rom_settings)) != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to get rom settings. Error code %d", result);
        goto alloc_memory_interface_failed;
    }
    if (strcmp(rom_settings.MD5, "F7C52568A31AADF26E14DC2B6416B2ED") != 0)
    {
        PyErr_Format(PyExc_RuntimeError, "ROM MD5 \"%s\" does not match expected \"F7C52568A31AADF26E14DC2B6416B2ED\". Only Super Smash Bros. (U) [!] is currently supported.", rom_settings.MD5);
        goto alloc_memory_interface_failed;
    }
    self->memory_interface = m64pai_memory_interface_create(&self->emu->corelib, REGION_USA);
    if (self->memory_interface == NULL)
        goto alloc_memory_interface_failed;

    /* With ROM successfully loaded, we can now start all plugins */
    if (m64pai_Emulator_start_plugins(self->emu) != 0)
        goto start_plugins_failed;

    for (cheat = cheats_usa; cheat->name; ++cheat)
        if (self->emu->corelib.CoreAddCheat(cheat->name, cheat->code, cheat->code_length) != M64ERR_SUCCESS)
        {
            PyErr_Format(PyExc_RuntimeError, "Failed to add cheat %s", cheat->name);
            goto start_plugins_failed;
        }
    for (cheat = cheats_usa; cheat->name; ++cheat)
        if (self->emu->corelib.CoreCheatEnabled(cheat->name, 1) != M64ERR_SUCCESS)
        {
            PyErr_Format(PyExc_RuntimeError, "Failed to enable cheat %s", cheat->name);
            goto start_plugins_failed;
        }

    /* Cleanup - corelib copies buffer so these aren't needed */
    free(rom_buffer);
    fclose(rom_file);

    self->emu->is_rom_loaded = 1;
    return (PyObject*)self;

    start_plugins_failed          : m64pai_memory_interface_destroy(self->memory_interface);
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
SSB64_get_fighter(m64pai_SSB64* self, PyObject* idx)
{
    int i;
    int memory_index;
    long player_slot;
    m64pai_character_e character[4];
    const char* error_msg;
    PyObject* args;
    PyObject* py_player_slot;
    PyObject* py_memory_index;
    PyTypeObject* FighterType;
    m64pai_Fighter* fighter;

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
        if (!m64pai_memory_match_settings_get_fighter_character(self->memory_interface, i + 1, &character[i], &error_msg))
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
        case FIGHTER_CAPTAIN_FALCON : FighterType = &m64pai_CaptainFalconType; break;
        case FIGHTER_DONKEY_KONG    : FighterType = &m64pai_DonkeyKongType;    break;
        case FIGHTER_FOX            : FighterType = &m64pai_FoxType;           break;
        case FIGHTER_JIGGLYPUFF     : FighterType = &m64pai_JigglypuffType;    break;
        case FIGHTER_KIRBY          : FighterType = &m64pai_KirbyType;         break;
        case FIGHTER_LINK           : FighterType = &m64pai_LinkType;          break;
        case FIGHTER_LUIGI          : FighterType = &m64pai_LuigiType;         break;
        case FIGHTER_MARIO          : FighterType = &m64pai_MarioType;         break;
        case FIGHTER_NESS           : FighterType = &m64pai_NessType;          break;
        case FIGHTER_PIKACHU        : FighterType = &m64pai_PikachuType;       break;
        case FIGHTER_SAMUS          : FighterType = &m64pai_SamusType;         break;
        case FIGHTER_YOSHI          : FighterType = &m64pai_YoshiType;         break;
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
    fighter = (m64pai_Fighter*)PyObject_CallObject((PyObject*)FighterType, args);
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
get_stage_determine_type(m64pai_SSB64* self, PyObject* constructor_args)
{
    m64pai_stage_e stage = -1;
    const char* error_msg;
    if (!m64pai_memory_match_settings_get_stage(self->memory_interface, &stage, &error_msg))
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to read stage type: %s", error_msg);
        return NULL;
    }
    switch (stage)
    {
        case STAGE_DREAM_LAND       : return PyObject_CallObject((PyObject*)&m64pai_DreamLandType, constructor_args);
        case STAGE_HYRULE_CASTLE    : return PyObject_CallObject((PyObject*)&m64pai_HyruleCastleType, constructor_args);
        case STAGE_KONGO_JUNGLE     : return PyObject_CallObject((PyObject*)&m64pai_KongoJungleType, constructor_args);
        case STAGE_MUSHROOM_KINGDOM : return PyObject_CallObject((PyObject*)&m64pai_MushroomKingdomType, constructor_args);
        case STAGE_PEACHS_CASTLE    : return PyObject_CallObject((PyObject*)&m64pai_PeachsCastleType, constructor_args);
        case STAGE_PLANET_ZEBES     : return PyObject_CallObject((PyObject*)&m64pai_PlanetZebesType, constructor_args);
        case STAGE_SAFFRON_CITY     : return PyObject_CallObject((PyObject*)&m64pai_SaffronCityType, constructor_args);
        case STAGE_SECTOR_Z         : return PyObject_CallObject((PyObject*)&m64pai_SectorZType, constructor_args);
        case STAGE_YOSHIS_ISLAND    : return PyObject_CallObject((PyObject*)&m64pai_YoshisIslandType, constructor_args);
        default :
            PyErr_SetString(PyExc_NotImplementedError, "A class for the currently loaded stage is not implemented. Sorry!");
            return NULL;
    }
}
static PyObject*
SSB64_get_stage(m64pai_SSB64* self, PyObject* noargs)
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
SSB64_getis_match_in_progress(m64pai_SSB64* self, void* closure)
{
    if (m64pai_memory_is_match_in_progress(self->memory_interface))
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}
static int
SSB64_setis_match_in_progress(m64pai_SSB64* self, PyObject* value, void* closure)
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
PyTypeObject m64pai_SSB64Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64pai.SSB64",
    .tp_basicsize = sizeof(m64pai_SSB64),
    .tp_dealloc = (destructor)SSB64_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = SSB64_DOC,
    .tp_methods = SSB64_methods,
    .tp_getset = SSB64_getset,
    .tp_new = SSB64_new,
};

/* ------------------------------------------------------------------------- */
int
m64pai_SSB64Type_init(void)
{
    if (PyType_Ready(&m64pai_SSB64Type) < 0)
        return -1;
    return 0;
}
