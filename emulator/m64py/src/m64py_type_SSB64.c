#include "m64py_type_SSB64.h"
#include "m64py_type_Fighter.h"
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
    self->mem_iface = m64py_memory_interface_create(&self->emu->corelib, REGION_USA);
    if (self->mem_iface == NULL)
        goto alloc_memory_interface_failed;

    /* With ROM successfully loaded, we can now start all plugins */
    if (m64py_Emulator_start_plugins(self->emu) != 0)
        goto start_plugins_failed;

    /* Cleanup - corelib copies buffer so these aren't needed */
    free(rom_buffer);
    fclose(rom_file);

    self->emu->is_rom_loaded = 1;
    return (PyObject*)self;

    start_plugins_failed          : m64py_memory_interface_destroy(self->mem_iface);
    alloc_memory_interface_failed : self->emu->corelib.CoreDoCommand(M64CMD_ROM_CLOSE, 0, NULL);
    read_rom_failed               : free(rom_buffer);
    malloc_rom_buffer_failed      : fclose(rom_file);
    open_rom_file_failed          :
    parse_args_failed             : Py_DECREF(self);
    alloc_self_failed             : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(START_MATCH_DOC, "start_match()\n--\n\n"
"Starts the match. This should be called once you've configured the ruleset,\n"
"selected the stage, and selected the fighters.");
static PyObject*
SSB64_start_match(m64py_SSB64* self, PyObject* arg)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(GET_FIGHTER_DOC, "get_fighter(index)\n--\n\n"
"Creates and returns an object that can interface with the fighter's in-game\n"
"state. The returned type depends on the character you selected before starting\n"
"the match\n\n"
"Must be called after starting the match.");
static PyObject*
SSB64_get_fighter(m64py_SSB64* self, PyObject* idx)
{
    PyObject* args = PyTuple_New(2);
    if (args == NULL)
        return NULL;

    Py_INCREF(self); PyTuple_SET_ITEM(args, 0, (PyObject*)self);
    Py_INCREF(idx);  PyTuple_SET_ITEM(args, 1, idx);

    m64py_Fighter* fighter = (m64py_Fighter*)PyObject_CallObject((PyObject*)&m64py_FighterType, args);
    Py_DECREF(args);
    return (PyObject*)fighter;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(SET_FIGHTERS_DOC, "set_fighters(fighter1, fighter2, fighter3, fighter4)\n--\n\n"
"Similar to the character select screen, use this to select each character by\n"
"specifying Fighter.CHARACTER (e.g. Fighter.PIKACHU) or if you want to disable\n"
"a fighter, you can specify None instead.\n\n"
"Must be called before start_match().");
static PyObject*
SSB64_set_fighters(m64py_SSB64* self, PyObject* args)
{
    unsigned long fighter[4];
    int i;

    if (PyTuple_GET_SIZE(args) != 4)
    {
        PyErr_Format(PyExc_TypeError, "set_fighters() takes exactly 4 arguments but %d were given. Arguments must be of type Fighter.CHARACTER, or can be None if fighter should be disabled.", PyTuple_GET_SIZE(args));
        return NULL;
    }

    for (i = 0; i != 4; ++i)
    {
        PyObject* arg = PyTuple_GET_ITEM(args, i);
        if (PyLong_CheckExact(arg))
        {
            fighter[i] = PyLong_AsUnsignedLong(arg);
            if (fighter[i] > FIGHTER_GIANT_DONKEY_KONG)
            {
                PyErr_Format(PyExc_ValueError, "Argument %d: Fighter ID is out of range: %d", i+1, fighter[i]);
                return NULL;
            }
        }
        else if (arg == Py_None)
        {
            fighter[i] = FIGHTER_NONE;
        }
        else
        {
            PyErr_Format(PyExc_TypeError, "set_fighters() argument %d: Type must be either Fighter.CHARACTER or None if fighter should be disabled", i);
            return NULL;
        }
    }

    for (i = 0; i != 4; ++i)
        m64py_memory_set_fighter(self->mem_iface, i, fighter[i]);

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(GET_STAGE_DOC, "get_stage()\n--\n\n"
"Creates and returns an object that lets you interface with the stage's state.\n"
"The returned type depends on the stage you selected before starting the match.\n\n"
"Must be called after starting the match");
static PyObject*
SSB64_get_stage(m64py_SSB64* self, PyObject* args)
{
    Py_RETURN_NONE;
}

PyDoc_STRVAR(SET_STAGE_DOC, "set_stage(enum)\n--\n\n"
"Similar to the stage select screen, use this to select which stage to play on\n"
"by specifying Stage.NAME (e.g. Stage.DREAM_LAND).\n\n"
"Must be called before start_match()");
/* ------------------------------------------------------------------------- */
static PyObject*
SSB64_set_stage(m64py_SSB64* self, PyObject* py_stage)
{
    unsigned long stage;
    if (!PyLong_CheckExact(py_stage))
    {
        PyErr_SetString(PyExc_TypeError, "set_stage() expects a value of type Stage.NAME, e.g. Stage.DREAM_LAND");
        return NULL;
    }

    stage = PyLong_AsUnsignedLong(py_stage);
    if (stage > STAGE_BTP_NESS)
    {
        PyErr_Format(PyExc_ValueError, "set_stage(): Stage ID is out of range: %d", stage);
        return NULL;
    }

    m64py_memory_set_stage(self->mem_iface, stage);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef SSB64_methods[] = {
    {"start_match",  (PyCFunction)SSB64_start_match,  METH_NOARGS,  START_MATCH_DOC},
    {"get_fighter",  (PyCFunction)SSB64_get_fighter,  METH_O,       GET_FIGHTER_DOC},
    {"set_fighters", (PyCFunction)SSB64_set_fighters, METH_VARARGS, SET_FIGHTERS_DOC},
    {"get_stage",    (PyCFunction)SSB64_get_stage,    METH_NOARGS,  GET_STAGE_DOC},
    {"set_stage",    (PyCFunction)SSB64_set_stage,    METH_O,       SET_STAGE_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(MATCH_IN_PROGRESS_DOC,
"Returns True as soon as the match begins (after calling start_match()). Returns\n"
"False when one of the fighters wins the match");
static PyObject*
SSB64_getis_match_in_progress(m64py_SSB64* self, void* closure)
{
    Py_RETURN_TRUE;
}
static int
SSB64_setis_match_in_progress(m64py_SSB64* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Read only. Use start_match() to start the match.");
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
