#include "m64py_type_SSB64.h"

#define M64P_CORE_PROTOTYPES
#include "m64p_frontend.h"

#include <stdint.h>
#include <stdio.h>

/* ------------------------------------------------------------------------- */
static void
SSB64_dealloc(m64py_SSB64* self)
{
    CoreShutdown();
    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
SSB64_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_SSB64* self;

    self = (m64py_SSB64*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed : return NULL;
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
read_player_position(PyObject* self, PyObject* arg)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
read_player_orientation(PyObject* self, PyObject* arg)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
read_player_anim_state(PyObject* self, PyObject* arg)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
read_player_anim_progress(PyObject* self, PyObject* arg)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
read_whispy_wind(PyObject* self, PyObject* arg)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef SSB64_methods[] = {
    {"set_tournament_rules",      set_tournament_rules,      METH_NOARGS, ""},
    {"set_character",             set_character,             METH_VARARGS, ""},
    {"set_stage",                 set_stage,                 METH_O, ""},
    {"start_game",                start_game,                METH_O, ""},
    {"read_player_position",      read_player_position,      METH_O, ""},
    {"read_player_orientation",   read_player_orientation,   METH_O, ""},
    {"read_player_anim_state",    read_player_anim_state,    METH_O, ""},
    {"read_player_anim_progress", read_player_anim_progress, METH_O, ""},
    {"read_whispy_wind",          read_whispy_wind,          METH_NOARGS, ""},
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
    m64py_SSB64Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&m64py_SSB64Type) < 0)
        return -1;
    return 0;
}
