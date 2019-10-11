#include "m64pai_type_Stage_SectorZ.h"
#include "m64pai_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
SectorZ_dealloc(m64pai_SectorZ* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
SectorZ_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64pai_SectorZ* self;

    self = (m64pai_SectorZ*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(SECTOR_Z_DOC,
"Provides stage specific state for Dream Land");
PyTypeObject m64pai_SectorZType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64pai.SectorZ",
    .tp_basicsize = sizeof(m64pai_SectorZ),
    .tp_dealloc = (destructor)SectorZ_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = SECTOR_Z_DOC,
    .tp_new = SectorZ_new,
};

/* ------------------------------------------------------------------------- */
int
m64pai_SectorZType_init(void)
{
    m64pai_SectorZType.tp_base = &m64pai_StageType;
    if (PyType_Ready(&m64pai_SectorZType) < 0)
        return -1;
    return 0;
}
