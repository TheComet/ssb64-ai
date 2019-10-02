#include "m64py_type_Stage_DreamLand.h"
#include "m64py_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
DreamLand_dealloc(m64py_DreamLand* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
DreamLand_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_DreamLand* self;

    self = (m64py_DreamLand*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(WHISPY_DOC,
"float: 0=no wind, -1=blowing left, 1=blowing right. Will ramp up/down as the\n"
"animation begins/ends");
static PyObject*
DreamLand_getwhispy(m64py_DreamLand* self, void* closure)
{
    float wind;
    m64py_memory_read_whispy_wind(self->super.ssb64->memory_interface, &wind);
    return PyFloat_FromDouble(wind);
}
static int
DreamLand_setwhispy(m64py_DreamLand* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Forcing whispy to blow isn't supported (yet?)");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef DreamLand_getset[] = {
    {"whispy", (getter)DreamLand_getwhispy, (setter)DreamLand_setwhispy, WHISPY_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(DREAM_LAND_DOC,
"Provides stage specific state for Dream Land");
PyTypeObject m64py_DreamLandType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "m64py.DreamLand",            /* tp_name */
    sizeof(m64py_DreamLand),      /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)DreamLand_dealloc,/* tp_dealloc */
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
    DREAM_LAND_DOC,               /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    0,                            /* tp_methods */
    0,                            /* tp_members */
    DreamLand_getset,             /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    0,                            /* tp_init */
    0,                            /* tp_alloc */
    DreamLand_new,                /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
m64py_DreamLandType_init(void)
{
    m64py_DreamLandType.tp_base = &m64py_StageType;
    if (PyType_Ready(&m64py_DreamLandType) < 0)
        return -1;
    return 0;
}
