#include "m64pai_type_Stage_SaffronCity.h"
#include "m64pai_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
SaffronCity_dealloc(m64pai_SaffronCity* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
SaffronCity_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64pai_SaffronCity* self;

    self = (m64pai_SaffronCity*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(SAFFRON_CITY_DOC,
"Provides stage specific state for Dream Land");
PyTypeObject m64pai_SaffronCityType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64pai.SaffronCity",
    .tp_basicsize = sizeof(m64pai_SaffronCity),
    .tp_dealloc = (destructor)SaffronCity_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = SAFFRON_CITY_DOC,
    .tp_new = SaffronCity_new,
};

/* ------------------------------------------------------------------------- */
int
m64pai_SaffronCityType_init(void)
{
    m64pai_SaffronCityType.tp_base = &m64pai_StageType;
    if (PyType_Ready(&m64pai_SaffronCityType) < 0)
        return -1;
    return 0;
}
