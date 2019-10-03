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
"inputs from code\n\n"
"Note that overriding controls only works if the fighter is being controlled\n"
"by a human player.");
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
#define GETSET_BUTTONS_LIST                                                   \
    X(A_BUTTON, a, "A button")                                                \
    X(B_BUTTON, b, "B button")                                                \
    X(L_CBUTTON, cl, "C Left")                                                \
    X(R_CBUTTON, cr, "C Right")                                               \
    X(U_CBUTTON, cu, "C Up")                                                  \
    X(D_CBUTTON, cd, "C Down")                                                \
    X(L_DPAD, dl, "D-Pad Left")                                               \
    X(R_DPAD, dr, "D-Pad Right")                                              \
    X(U_DPAD, du, "D-Pad Up")                                                 \
    X(D_DPAD, dd, "D-Pad Down")                                               \
    X(L_TRIG, l, "Left Trigger")                                              \
    X(R_TRIG, r, "Right Trigger")                                             \
    X(START_BUTTON, s, "Start button")                                        \
    X(Z_TRIG, z, "Z Trigger")

#define GET_BUTTON(name)                                                      \
    m64py_CuckedInputPlugin* cuck = try_get_cucked_input(self);               \
    if (cuck == NULL)                                                         \
        return NULL;                                                          \
                                                                              \
    if (cuck->GetControllerButtons(self->player_slot - 1).name)               \
        Py_RETURN_TRUE;                                                       \
    Py_RETURN_FALSE;

#define SET_BUTTON(name)                                                      \
    int trueness;                                                             \
    BUTTONS buttons;                                                          \
    m64py_CuckedInputPlugin* cuck = try_get_cucked_input(self);               \
    if (cuck == NULL)                                                         \
        return -1;                                                            \
                                                                              \
    if ((trueness = PyObject_IsTrue(value)) < 0)                              \
        return -1;                                                            \
                                                                              \
    buttons = cuck->GetControllerButtons(self->player_slot - 1);              \
    buttons.name = trueness;                                                  \
    cuck->SetControllerButtons(self->player_slot - 1, buttons);               \
    return 0;

#define X(mupen, python, desc)                                                \
    PyDoc_STRVAR(CONTROLLER_##python##_DOC, desc ": True or False");          \
    static PyObject* Controller_get##python(m64py_Controller* self, void* closure) { GET_BUTTON(mupen); } \
    static int Controller_set##python(m64py_Controller* self, PyObject* value, void* closure) { SET_BUTTON(mupen); }
GETSET_BUTTONS_LIST
#undef X

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(CONTROLLER_JOY_X_DOC,
"X axis of the joystick, a value between -1.0 and 1.0.\n\n"
"Note: Internally, the value is stored as a signed 8-bit value, which means the\n"
"full range is from -128 to 127. To fix this asymmetry, the value is clamped\n"
"to -127 to 127 before converting it to a float.");
static PyObject*
Controller_getx(m64py_Controller* self, void* closure)
{
    int8_t x_axis;
    m64py_CuckedInputPlugin* cuck = try_get_cucked_input(self);
    if (cuck == NULL)
        return NULL;

    x_axis = cuck->GetControllerButtons(self->player_slot - 1).X_AXIS;
    if (x_axis < -127)
        x_axis = -127;
    return PyFloat_FromDouble((double)x_axis / 127.0);
}
static int
Controller_setx(m64py_Controller* self, PyObject* py_value, void* closure)
{
    double value;
    BUTTONS buttons;
    m64py_CuckedInputPlugin* cuck = try_get_cucked_input(self);
    if (cuck == NULL)
        return -1;

    if ((value = PyFloat_AsDouble(py_value)) == -1.0 && PyErr_Occurred())
        return -1;

    buttons = cuck->GetControllerButtons(self->player_slot - 1);
    buttons.X_AXIS = (int8_t)(value * 127);
    cuck->SetControllerButtons(self->player_slot - 1, buttons);
    return 0;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(CONTROLLER_JOY_Y_DOC,
"Y axis of the joystick, a value between -1.0 and 1.0.\n\n"
"Note: Internally, the value is stored as a signed 8-bit value, which means the\n"
"full range is from -128 to 127. To fix this asymmetry, the value is clamped\n"
"to -127 to 127 before converting it to a float.");
static PyObject*
Controller_gety(m64py_Controller* self, void* closure)
{
    int8_t y_axis;
    m64py_CuckedInputPlugin* cuck = try_get_cucked_input(self);
    if (cuck == NULL)
        return NULL;

    y_axis = cuck->GetControllerButtons(self->player_slot - 1).Y_AXIS;
    if (y_axis < -127)
        y_axis = -127;
    return PyFloat_FromDouble((double)y_axis / 127.0);
}
static int
Controller_sety(m64py_Controller* self, PyObject* py_value, void* closure)
{
    double value;
    BUTTONS buttons;
    m64py_CuckedInputPlugin* cuck = try_get_cucked_input(self);
    if (cuck == NULL)
        return -1;

    if ((value = PyFloat_AsDouble(py_value)) == -1.0 && PyErr_Occurred())
        return -1;

    buttons = cuck->GetControllerButtons(self->player_slot - 1);
    buttons.Y_AXIS = (int8_t)(value * 127);
    cuck->SetControllerButtons(self->player_slot - 1, buttons);
    return 0;
}

/* ------------------------------------------------------------------------- */
PyGetSetDef Controller_getset[] = {
#define X(mupen, python, desc) \
    {#python,    (getter)Controller_get##python, (setter)Controller_set##python, CONTROLLER_##python##_DOC, NULL},
    GETSET_BUTTONS_LIST
#undef X
    {"override", (getter)Controller_getoverride, (setter)Controller_setoverride, CONTROLLER_OVERRIDE_DOC, NULL},
    {"x",        (getter)Controller_getx,        (setter)Controller_setx,        CONTROLLER_JOY_X_DOC, NULL},
    {"y",        (getter)Controller_gety,        (setter)Controller_sety,        CONTROLLER_JOY_Y_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(CONTROLLER_DOC,
"Provides a method to override the controls of a fighter. Note that the fighter\n"
"must have been selected as a human player for this to work. The human's inputs\n"
"are replaced with those set by code when setting Controller.override = True");
PyTypeObject m64py_ControllerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64py.Controller",
    .tp_basicsize = sizeof(m64py_Controller),
    .tp_dealloc = (destructor)Controller_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = CONTROLLER_DOC,
    .tp_getset = Controller_getset,
    .tp_new = Controller_new,
};

/* ------------------------------------------------------------------------- */
int
m64py_ControllerType_init(void)
{
    if (PyType_Ready(&m64py_ControllerType) < 0)
        return -1;
    return 0;
}
