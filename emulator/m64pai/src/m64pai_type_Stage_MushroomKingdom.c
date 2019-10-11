#include "m64pai_type_Stage_MushroomKingdom.h"
#include "m64pai_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
MushroomKingdom_dealloc(m64pai_MushroomKingdom* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
MushroomKingdom_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64pai_MushroomKingdom* self;

    self = (m64pai_MushroomKingdom*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(MUSHROOM_KINGDOM_DOC,
"Provides stage specific state for Dream Land");
PyTypeObject m64pai_MushroomKingdomType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64pai.MushroomKingdom",
    .tp_basicsize = sizeof(m64pai_MushroomKingdom),
    .tp_dealloc = (destructor)MushroomKingdom_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = MUSHROOM_KINGDOM_DOC,
    .tp_new = MushroomKingdom_new,
};

/* ------------------------------------------------------------------------- */
int
m64pai_MushroomKingdomType_init(void)
{
    m64pai_MushroomKingdomType.tp_base = &m64pai_StageType;
    if (PyType_Ready(&m64pai_MushroomKingdomType) < 0)
        return -1;
    return 0;
}
