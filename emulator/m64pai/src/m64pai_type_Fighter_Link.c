#include "m64pai_type_Fighter_Link.h"
#include "m64pai_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
Link_dealloc(m64pai_Link* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Link_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64pai_character_e character;
    m64pai_Link* self;

    self = (m64pai_Link*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    /* Double check that we really are the character at this memory location */
    m64pai_memory_read_fighter_character(self->super.ssb64->memory_interface, self->super.n64_memory_address, &character);
    if (character != FIGHTER_LINK)
    {
        PyErr_Format(PyExc_RuntimeError, "Link class was instantiated, but the fighter in n64 memory is %d", character);
        goto check_character_failed;
    }

    return (PyObject*)self;

    check_character_failed : Py_DECREF(self);
    alloc_self_failed      : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(LINK_DOC,
"Provides character specific state for Link.");
PyTypeObject m64pai_LinkType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64pai.Link",
    .tp_basicsize = sizeof(m64pai_Link),
    .tp_dealloc = (destructor)Link_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = LINK_DOC,
    .tp_new = Link_new,
};

/* ------------------------------------------------------------------------- */
int
m64pai_LinkType_init(void)
{
    m64pai_LinkType.tp_base = &m64pai_FighterType;
    if (PyType_Ready(&m64pai_LinkType) < 0)
        return -1;
    return 0;
}
