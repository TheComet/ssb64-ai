#include "m64py_type_Stage_PlanetZebes.h"
#include "m64py_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
PlanetZebes_dealloc(m64py_PlanetZebes* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
PlanetZebes_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_PlanetZebes* self;

    self = (m64py_PlanetZebes*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(PLANET_ZEBES_DOC,
"Provides stage specific state for Dream Land");
PyTypeObject m64py_PlanetZebesType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64py.PlanetZebes",
    .tp_basicsize = sizeof(m64py_PlanetZebes),
    .tp_dealloc = (destructor)PlanetZebes_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = PLANET_ZEBES_DOC,
    .tp_new = PlanetZebes_new,
};

/* ------------------------------------------------------------------------- */
int
m64py_PlanetZebesType_init(void)
{
    m64py_PlanetZebesType.tp_base = &m64py_StageType;
    if (PyType_Ready(&m64py_PlanetZebesType) < 0)
        return -1;
    return 0;
}
