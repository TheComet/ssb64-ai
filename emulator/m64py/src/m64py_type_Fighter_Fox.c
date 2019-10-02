#include "m64py_type_Fighter_Fox.h"
#include "m64py_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
Fox_dealloc(m64py_Fox* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Fox_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_character_e character;
    m64py_Fox* self;

    self = (m64py_Fox*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    /* Double check that we really are the character at this memory location */
    m64py_memory_read_fighter_character(self->super.ssb64->memory_interface, self->super.n64_memory_address, &character);
    if (character != FIGHTER_FOX)
    {
        PyErr_Format(PyExc_RuntimeError, "Fox class was instantiated, but the fighter in n64 memory is %d", character);
        goto check_character_failed;
    }

    return (PyObject*)self;

    check_character_failed : Py_DECREF(self);
    alloc_self_failed      : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FOX_DOC,
"Provides character specific state for Fox.");
PyTypeObject m64py_FoxType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "m64py.Fox",                  /* tp_name */
    sizeof(m64py_Fox),            /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)Fox_dealloc,      /* tp_dealloc */
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
    FOX_DOC,                      /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    0,                            /* tp_methods */
    0,                            /* tp_members */
    0,                            /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    0,                            /* tp_init */
    0,                            /* tp_alloc */
    Fox_new,                      /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
m64py_FoxType_init(void)
{
    m64py_FoxType.tp_base = &m64py_FighterType;
    if (PyType_Ready(&m64py_FoxType) < 0)
        return -1;
    return 0;
}
