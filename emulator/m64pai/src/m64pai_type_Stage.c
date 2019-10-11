#include "m64pai_type_Stage.h"

/* ------------------------------------------------------------------------- */
static void
Stage_dealloc(m64pai_Stage* self)
{
    Py_XDECREF(self->ssb64);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static char* kwds_names[] = {
    "ssb64",
    NULL
};
static PyObject*
Stage_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64pai_Stage* self = (m64pai_Stage*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", kwds_names, &m64pai_SSB64Type, &self->ssb64))
        goto parse_args_failed;
    Py_INCREF(self->ssb64);

    return (PyObject*)self;

    parse_args_failed : Py_DECREF(self);
    alloc_self_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
#define STAGE_DOC \
"Represents "
PyTypeObject m64pai_StageType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64pai.Stage",
    .tp_basicsize = sizeof(m64pai_Stage),
    .tp_dealloc = (destructor)Stage_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = STAGE_DOC,
    .tp_new = Stage_new,
};

/* ------------------------------------------------------------------------- */
int
m64pai_StageType_init(void)
{
    if (PyType_Ready(&m64pai_StageType) < 0)
        return -1;
    return 0;
}
