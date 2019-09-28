#include "m64py_type_Stage.h"
#include "m64py_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
Stage_dealloc(m64py_Stage* self)
{
    Py_XDECREF(self->ssb64);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static char* kwds_names[] = {
    "ssb64",
    NULL
};
static PyObject*
Stage_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_Stage* self = (m64py_Stage*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", kwds_names, &m64py_SSB64Type, &self->ssb64))
        goto parse_args_failed;
    Py_INCREF(self->ssb64);

    return (PyObject*)self;

    parse_args_failed : Py_DECREF(self);
    alloc_self_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(WHISPY_DOC,
"float: 0=no wind, -1=blowing left, 1=blowing right. Will ramp up/down as the\n"
"animation begins/ends");
static PyObject*
Stage_getwhispy(m64py_Stage* self, void* closure)
{
    float wind;
    m64py_memory_get_whispy_wind(self->ssb64->mem_iface, &wind);
    return PyFloat_FromDouble(wind);
}
static int
Stage_setwhispy(m64py_Stage* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Forcing whispy to blow isn't supported (yet?)");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Stage_getset[] = {
    {"whispy", (getter)Stage_getwhispy, (setter)Stage_setwhispy, WHISPY_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
#define STAGE_DOC \
"Represents "
PyTypeObject m64py_StageType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "m64py.Stage",                /* tp_name */
    sizeof(m64py_Stage),          /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)Stage_dealloc,    /* tp_dealloc */
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
    "Stage objects",              /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    0,                            /* tp_methods */
    0,                            /* tp_members */
    Stage_getset,                 /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    0,                            /* tp_init */
    0,                            /* tp_alloc */
    Stage_new,                    /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
m64py_StageType_init(void)
{
    if (PyType_Ready(&m64py_StageType) < 0)
        return -1;
    return 0;
}
