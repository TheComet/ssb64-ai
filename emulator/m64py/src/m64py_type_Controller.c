#include "m64py_type_Controller.h"
#include "m64py_type_Plugin_CuckedInputPlugin.h"
#include "m64py_ssb64_memory.h"
#include <structmember.h>

/* ------------------------------------------------------------------------- */
static void
Controller_dealloc(m64py_Controller* self)
{
    Py_XDECREF(self->emu);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static char* kwds_names[] = {
    "emulator",
    "slot",
    NULL
};
static PyObject*
Controller_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_Controller* self;

    self = (m64py_Controller*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!i", kwds_names, &m64py_EmulatorType, &self->emu, &self->player_slot))
        goto parse_args_failed;
    Py_INCREF(self->emu);

    if (self->player_slot < 1 || self->player_slot > 4)
    {
        PyErr_Format(PyExc_ValueError, "player_slot is out of range : %d", self->player_slot);
        goto parse_args_failed;
    }

    return (PyObject*)self;

    parse_args_failed      : Py_DECREF(self);
    alloc_self_failed      : return NULL;
}

/* ------------------------------------------------------------------------- */
static m64py_CuckedInputPlugin*
try_get_cucked_input(m64py_Controller* self)
{
    if (!m64py_CuckedInputPlugin_CheckExact(self->emu->input_plugin))
    {
        if (m64py_Plugin_CheckExact(self->emu->input_plugin))
            PyErr_SetString(PyExc_RuntimeError, "Input plugin hasn't been cucked. Can't intercept normal input plugin calls.");
        else if (self->emu->input_plugin == Py_None)
            PyErr_SetString(PyExc_RuntimeError, "Input plugin isn't loaded.");
        else
            PyErr_SetString(PyExc_RuntimeError, "Input plugin is an unexpected type (this shouldn't happen -- file a bug report)");
        return NULL;
    }

    return (m64py_CuckedInputPlugin*)self->emu->input_plugin;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(CONTROLLER_OVERRIDE_DOC,
"Set this to True to disable controller inputs from the real plugin and enable\n"
"inputs from code");
static PyObject*
Controller_getoverride(m64py_Controller* self, void* closure)
{
    m64py_CuckedInputPlugin* cuck = try_get_cucked_input(self);
    if (cuck == NULL)
        return NULL;

    if (cuck->GetControllerOverride(self->player_slot - 1))
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}
static int
Controller_setoverride(m64py_Controller* self, PyObject* value, void* closure)
{
    int trueness;
    m64py_CuckedInputPlugin* cuck = try_get_cucked_input(self);
    if (cuck == NULL)
        return -1;

    if ((trueness = PyObject_IsTrue(value)) < 0)
        return -1;

    cuck->SetControllerOverride(self->player_slot - 1, trueness);
    return 0;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(CONTROLLER_A_DOC, "A button: True or False");
static PyObject*
Controller_geta(m64py_Controller* self, void* closure)
{
    m64py_CuckedInputPlugin* cuck = try_get_cucked_input(self);
    if (cuck == NULL)
        return NULL;

    if (cuck->GetControllerButtons(self->player_slot - 1).A_BUTTON)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}
static int
Controller_seta(m64py_Controller* self, PyObject* value, void* closure)
{
    int trueness;
    BUTTONS buttons;
    m64py_CuckedInputPlugin* cuck = try_get_cucked_input(self);
    if (cuck == NULL)
        return -1;

    if ((trueness = PyObject_IsTrue(value)) < 0)
        return -1;

    buttons = cuck->GetControllerButtons(self->player_slot - 1);
    buttons.A_BUTTON = trueness;
    cuck->SetControllerButtons(self->player_slot - 1, buttons);
    return 0;
}

/* ------------------------------------------------------------------------- */
PyGetSetDef Controller_getset[] = {
    {"override", (getter)Controller_getoverride, (setter)Controller_setoverride, CONTROLLER_OVERRIDE_DOC, NULL},
    {"a",        (getter)Controller_geta,        (setter)Controller_seta,        CONTROLLER_A_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(CONTROLLER_DOC,
"Provides character specific state for Captain Falcon.");
PyTypeObject m64py_ControllerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "m64py.Controller",           /* tp_name */
    sizeof(m64py_Controller),     /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)Controller_dealloc, /* tp_dealloc */
    0,                            /* tp_print */
    0,                            /* tp_getattr */
    0,                            /* tp_setattr */
    0,                            /* tp_reserved */
    0,                            /* tp_repr */
    0,                            /* tp_as_number */
    0,                            /* tp_as_sequence */
    0,                            /* tp_as_mapping */
    0,                            /* tp_hash  */
    0,                            /* tp_call */
    0,                            /* tp_str */
    0,                            /* tp_getattro */
    0,                            /* tp_setattro */
    0,                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    CONTROLLER_DOC,               /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    0,                            /* tp_methods */
    0,                            /* tp_members */
    Controller_getset,            /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    0,                            /* tp_init */
    0,                            /* tp_alloc */
    Controller_new,               /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
m64py_ControllerType_init(void)
{
    if (PyType_Ready(&m64py_ControllerType) < 0)
        return -1;
    return 0;
}
