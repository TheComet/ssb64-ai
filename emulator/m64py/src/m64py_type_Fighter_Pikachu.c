#include "m64py_type_Fighter_Pikachu.h"
#include "m64py_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
Pikachu_dealloc(m64py_Pikachu* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Pikachu_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_character_e character;
    m64py_Pikachu* self;

    self = (m64py_Pikachu*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    /* Double check that we really are the character at this memory location */
    m64py_memory_read_fighter_character(self->super.ssb64->memory_interface, self->super.n64_memory_address, &character);
    if (character != FIGHTER_PIKACHU)
    {
        PyErr_Format(PyExc_RuntimeError, "Pikachu class was instantiated, but the fighter in n64 memory is %d", character);
        goto check_character_failed;
    }

    return (PyObject*)self;

    check_character_failed : Py_DECREF(self);
    alloc_self_failed      : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(PIKACHU_DOC,
"Provides character specific state for Pikachu.");
PyTypeObject m64py_PikachuType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64py.Pikachu",
    .tp_basicsize = sizeof(m64py_Pikachu),
    .tp_dealloc = (destructor)Pikachu_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = PIKACHU_DOC,
    .tp_new = Pikachu_new,
};

/* ------------------------------------------------------------------------- */
int
m64py_PikachuType_init(void)
{
    m64py_PikachuType.tp_base = &m64py_FighterType;
    if (PyType_Ready(&m64py_PikachuType) < 0)
        return -1;
    return 0;
}
