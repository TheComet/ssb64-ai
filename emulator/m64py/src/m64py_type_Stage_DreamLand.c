#include "m64py_type_Stage_DreamLand.h"
#include "m64py_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
DreamLand_dealloc(m64py_DreamLand* self)
{
    Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
DreamLand_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_DreamLand* self;

    self = (m64py_DreamLand*)type->tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed  : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(WHISPY_DOC,
"float: 0=no wind, -1=blowing left, 1=blowing right. Will ramp up/down as the\n"
"animation begins/ends");
static PyObject*
DreamLand_getwhispy(m64py_DreamLand* self, void* closure)
{
    float wind;
    m64py_memory_read_whispy_wind(self->super.ssb64->memory_interface, &wind);
    return PyFloat_FromDouble(wind);
}
static int
DreamLand_setwhispy(m64py_DreamLand* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Forcing whispy to blow isn't supported (yet?)");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef DreamLand_getset[] = {
    {"whispy", (getter)DreamLand_getwhispy, (setter)DreamLand_setwhispy, WHISPY_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(DREAM_LAND_DOC,
"Provides stage specific state for Dream Land");
PyTypeObject m64py_DreamLandType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64py.DreamLand",
    .tp_basicsize = sizeof(m64py_DreamLand),
    .tp_dealloc = (destructor)DreamLand_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = DREAM_LAND_DOC,
    .tp_getset = DreamLand_getset,
    .tp_new = DreamLand_new,
};

/* ------------------------------------------------------------------------- */
int
m64py_DreamLandType_init(void)
{
    m64py_DreamLandType.tp_base = &m64py_StageType;
    if (PyType_Ready(&m64py_DreamLandType) < 0)
        return -1;
    return 0;
}
