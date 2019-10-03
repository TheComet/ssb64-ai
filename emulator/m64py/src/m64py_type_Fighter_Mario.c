#include "m64py_type_Fighter_Mario.h"
#include "m64py_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
Mario_dealloc(m64py_Mario* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Mario_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_character_e character;
    m64py_Mario* self;

    self = (m64py_Mario*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    /* Double check that we really are the character at this memory location */
    m64py_memory_read_fighter_character(self->super.ssb64->memory_interface, self->super.n64_memory_address, &character);
    if (character != FIGHTER_MARIO)
    {
        PyErr_Format(PyExc_RuntimeError, "Mario class was instantiated, but the fighter in n64 memory is %d", character);
        goto check_character_failed;
    }

    return (PyObject*)self;

    check_character_failed : Py_DECREF(self);
    alloc_self_failed      : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(MARIO_DOC,
"Provides character specific state for Mario.");
PyTypeObject m64py_MarioType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64py.Mario",
    .tp_basicsize = sizeof(m64py_Mario),
    .tp_dealloc = (destructor)Mario_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = MARIO_DOC,
    .tp_new = Mario_new,
};

/* ------------------------------------------------------------------------- */
int
m64py_MarioType_init(void)
{
    m64py_MarioType.tp_base = &m64py_FighterType;
    if (PyType_Ready(&m64py_MarioType) < 0)
        return -1;
    return 0;
}
