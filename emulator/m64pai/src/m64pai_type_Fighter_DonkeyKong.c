#include "m64pai_type_Fighter_DonkeyKong.h"
#include "m64pai_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
DonkeyKong_dealloc(m64pai_DonkeyKong* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
DonkeyKong_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64pai_character_e character;
    m64pai_DonkeyKong* self;

    self = (m64pai_DonkeyKong*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    /* Double check that we really are the character at this memory location */
    m64pai_memory_read_fighter_character(self->super.ssb64->memory_interface, self->super.n64_memory_address, &character);
    if (character != FIGHTER_DONKEY_KONG)
    {
        PyErr_Format(PyExc_RuntimeError, "DonkeyKong class was instantiated, but the fighter in n64 memory is %d", character);
        goto check_character_failed;
    }

    return (PyObject*)self;

    check_character_failed : Py_DECREF(self);
    alloc_self_failed      : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(DONKEY_KONG_DOC,
"Provides character specific state for Donkey Kong.");
PyTypeObject m64pai_DonkeyKongType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64pai.DonkeyKong",
    .tp_basicsize = sizeof(m64pai_DonkeyKong),
    .tp_dealloc = (destructor)DonkeyKong_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = DONKEY_KONG_DOC,
    .tp_new = DonkeyKong_new,
};

/* ------------------------------------------------------------------------- */
int
m64pai_DonkeyKongType_init(void)
{
    m64pai_DonkeyKongType.tp_base = &m64pai_FighterType;
    if (PyType_Ready(&m64pai_DonkeyKongType) < 0)
        return -1;
    return 0;
}
