#include "m64pai_type_Stage_HyruleCastle.h"
#include "m64pai_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
HyruleCastle_dealloc(m64pai_HyruleCastle* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
HyruleCastle_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64pai_HyruleCastle* self;

    self = (m64pai_HyruleCastle*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(HYRULE_CASTLE_DOC,
"Provides stage specific state for Dream Land");
PyTypeObject m64pai_HyruleCastleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64pai.HyruleCastle",
    .tp_basicsize = sizeof(m64pai_HyruleCastle),
    .tp_dealloc = (destructor)HyruleCastle_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = HYRULE_CASTLE_DOC,
    .tp_new = HyruleCastle_new,
};

/* ------------------------------------------------------------------------- */
int
m64pai_HyruleCastleType_init(void)
{
    m64pai_HyruleCastleType.tp_base = &m64pai_StageType;
    if (PyType_Ready(&m64pai_HyruleCastleType) < 0)
        return -1;
    return 0;
}
