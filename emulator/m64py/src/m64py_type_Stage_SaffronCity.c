#include "m64py_type_Stage_SaffronCity.h"
#include "m64py_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
SaffronCity_dealloc(m64py_SaffronCity* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
SaffronCity_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_SaffronCity* self;

    self = (m64py_SaffronCity*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(SAFFRON_CITY_DOC,
"Provides stage specific state for Dream Land");
PyTypeObject m64py_SaffronCityType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "m64py.SaffronCity",          /* tp_name */
    sizeof(m64py_SaffronCity),    /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)SaffronCity_dealloc,/* tp_dealloc */
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
    SAFFRON_CITY_DOC,             /* tp_doc */
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
    SaffronCity_new,              /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
m64py_SaffronCityType_init(void)
{
    m64py_SaffronCityType.tp_base = &m64py_StageType;
    if (PyType_Ready(&m64py_SaffronCityType) < 0)
        return -1;
    return 0;
}
