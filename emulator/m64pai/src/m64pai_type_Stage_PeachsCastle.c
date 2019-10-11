#include "m64pai_type_Stage_PeachsCastle.h"
#include "m64pai_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
PeachsCastle_dealloc(m64pai_PeachsCastle* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
PeachsCastle_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64pai_PeachsCastle* self;

    self = (m64pai_PeachsCastle*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(PEACHS_CASTLE_DOC,
"Provides stage specific state for Dream Land");
PyTypeObject m64pai_PeachsCastleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64pai.PeachsCastle",
    .tp_basicsize = sizeof(m64pai_PeachsCastle),
    .tp_dealloc = (destructor)PeachsCastle_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = PEACHS_CASTLE_DOC,
    .tp_new = PeachsCastle_new,
};

/* ------------------------------------------------------------------------- */
int
m64pai_PeachsCastleType_init(void)
{
    m64pai_PeachsCastleType.tp_base = &m64pai_StageType;
    if (PyType_Ready(&m64pai_PeachsCastleType) < 0)
        return -1;
    return 0;
}
