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
    .tp_name = "m64py.SaffronCity",
    .tp_basicsize = sizeof(m64py_SaffronCity),
    .tp_dealloc = (destructor)SaffronCity_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = SAFFRON_CITY_DOC,
    .tp_new = SaffronCity_new,
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
