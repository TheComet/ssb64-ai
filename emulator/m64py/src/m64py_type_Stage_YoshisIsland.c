#include "m64py_type_Stage_YoshisIsland.h"
#include "m64py_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
YoshisIsland_dealloc(m64py_YoshisIsland* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
YoshisIsland_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_YoshisIsland* self;

    self = (m64py_YoshisIsland*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(YOSHIS_ISLAND_DOC,
"Provides stage specific state for Dream Land");
PyTypeObject m64py_YoshisIslandType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64py.YoshisIsland",
    .tp_basicsize = sizeof(m64py_YoshisIsland),
    .tp_dealloc = (destructor)YoshisIsland_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = YOSHIS_ISLAND_DOC,
    .tp_new = YoshisIsland_new,
};

/* ------------------------------------------------------------------------- */
int
m64py_YoshisIslandType_init(void)
{
    m64py_YoshisIslandType.tp_base = &m64py_StageType;
    if (PyType_Ready(&m64py_YoshisIslandType) < 0)
        return -1;
    return 0;
}
