#include "m64py_type_Fighter_Samus.h"
#include "m64py_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
Samus_dealloc(m64py_Samus* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Samus_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_character_e character;
    m64py_Samus* self;

    self = (m64py_Samus*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    /* Double check that we really are the character at this memory location */
    m64py_memory_read_fighter_character(self->super.ssb64->memory_interface, self->super.n64_memory_address, &character);
    if (character != FIGHTER_SAMUS)
    {
        PyErr_Format(PyExc_RuntimeError, "Samus class was instantiated, but the fighter in n64 memory is %d", character);
        goto check_character_failed;
    }

    return (PyObject*)self;

    check_character_failed : Py_DECREF(self);
    alloc_self_failed      : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(SAMUS_DOC,
"Provides character specific state for Samus.");
PyTypeObject m64py_SamusType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64py.Samus",
    .tp_basicsize = sizeof(m64py_Samus),
    .tp_dealloc = (destructor)Samus_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = SAMUS_DOC,
    .tp_new = Samus_new,
};

/* ------------------------------------------------------------------------- */
int
m64py_SamusType_init(void)
{
    m64py_SamusType.tp_base = &m64py_FighterType;
    if (PyType_Ready(&m64py_SamusType) < 0)
        return -1;
    return 0;
}
