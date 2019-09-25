#include "m64py_type_Fighter.h"

/* ------------------------------------------------------------------------- */
static void
Fighter_dealloc(m64py_Fighter* self)
{
    Py_XDECREF(self->emu);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static char* kwds_names[] = {
    "emulator",
    NULL
};
static PyObject*
Fighter_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_Fighter* self;

    self = (m64py_Fighter*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", kwds_names, &m64py_EmulatorType, &self->emu))
        goto parse_args_failed;
    Py_INCREF(self->emu);

    return (PyObject*)self;

    parse_args_failed : Py_DECREF(self);
    alloc_self_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
PyTypeObject m64py_SSB64Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "m64py.Fighter",              /* tp_name */
    sizeof(m64py_Fighter),        /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)Fighter_dealloc,  /* tp_dealloc */
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
    "Fighter objects",            /* tp_doc */
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
    Fighter_new,                  /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
m64py_FighterType_init(void)
{
    if (PyType_Ready(&m64py_FighterType) < 0)
        return -1;
    return 0;
}
