#include "m64pai_type_Stage_YoshisIsland.h"
#include "m64pai_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
YoshisIsland_dealloc(m64pai_YoshisIsland* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
YoshisIsland_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64pai_YoshisIsland* self;

    self = (m64pai_YoshisIsland*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(YOSHIS_ISLAND_DOC,
"Provides stage specific state for Dream Land");
PyTypeObject m64pai_YoshisIslandType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64pai.YoshisIsland",
    .tp_basicsize = sizeof(m64pai_YoshisIsland),
    .tp_dealloc = (destructor)YoshisIsland_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = YOSHIS_ISLAND_DOC,
    .tp_new = YoshisIsland_new,
};

/* ------------------------------------------------------------------------- */
int
m64pai_YoshisIslandType_init(void)
{
    m64pai_YoshisIslandType.tp_base = &m64pai_StageType;
    if (PyType_Ready(&m64pai_YoshisIslandType) < 0)
        return -1;
    return 0;
}
