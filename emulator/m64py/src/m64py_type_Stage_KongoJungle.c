#include "m64py_type_Stage_KongoJungle.h"
#include "m64py_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
KongoJungle_dealloc(m64py_KongoJungle* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
KongoJungle_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_KongoJungle* self;

    self = (m64py_KongoJungle*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(KONGO_JUNGLE_DOC,
"Provides stage specific state for Dream Land");
PyTypeObject m64py_KongoJungleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64py.KongoJungle",
    .tp_basicsize = sizeof(m64py_KongoJungle),
    .tp_dealloc = (destructor)KongoJungle_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = KONGO_JUNGLE_DOC,
    .tp_new = KongoJungle_new,
};

/* ------------------------------------------------------------------------- */
int
m64py_KongoJungleType_init(void)
{
    m64py_KongoJungleType.tp_base = &m64py_StageType;
    if (PyType_Ready(&m64py_KongoJungleType) < 0)
        return -1;
    return 0;
}
