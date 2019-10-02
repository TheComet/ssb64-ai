#include "m64py_type_Fighter.h"
#include "m64py_ssb64_memory.h"

/* ------------------------------------------------------------------------- */
static void
Fighter_dealloc(m64py_Fighter* self)
{
    Py_XDECREF(self->controller);
    Py_XDECREF(self->ssb64);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static char* kwds_names[] = {
    "ssb64",
    "memory_index",
    "player_slot",
    NULL
};
static PyObject*
Fighter_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_Fighter* self;
    const char* error;
    PyObject* controller_args;
    PyObject* py_player_slot;

    self = (m64py_Fighter*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!ii", kwds_names, &m64py_SSB64Type, &self->ssb64, &self->player_slot, &self->memory_index))
        goto parse_args_failed;
    Py_INCREF(self->ssb64);

    /* These checks shouldn't be necessary since the SSB64 class does them already */
    assert(self->memory_index >= 0 && self->memory_index <= 3);
    assert(self->player_slot >= 1 && self->player_slot <= 4);

    /* Cache fighter's memory address so we don't read it every frame */
    if (!m64py_memory_get_fighter_address(self->ssb64->memory_interface, self->memory_index, &self->n64_memory_address, &error))
    {
        PyErr_SetString(PyExc_RuntimeError, error);
        goto get_address_failed;
    }

    controller_args = PyTuple_New(2);
    if (controller_args == NULL)
        goto alloc_controller_failed;

    Py_INCREF(self->ssb64->emu);
    PyTuple_SET_ITEM(controller_args, 0, (PyObject*)self->ssb64->emu);
    py_player_slot = PyTuple_GET_ITEM(args, 1);
    Py_INCREF(py_player_slot);
    PyTuple_SET_ITEM(controller_args, 1, py_player_slot);

    self->controller = (m64py_Controller*)PyObject_CallObject((PyObject*)&m64py_ControllerType, controller_args);
    Py_DECREF(controller_args);
    if (self->controller == NULL)
        goto alloc_controller_failed;

    return (PyObject*)self;

    alloc_controller_failed      :
    get_address_failed           :
    parse_args_failed            : Py_DECREF(self);
    alloc_self_failed            : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_CHARACTER_DOC,
"Which character type this fighter is (Fighter.CHARACTER, e.g. Fighter.PIKACHU)");
static PyObject*
Fighter_getcharacter(m64py_Fighter* self, void* closure)
{
    m64py_character_e character;
    m64py_memory_read_fighter_character(self->ssb64->memory_interface, self->n64_memory_address, &character);
    return PyLong_FromLong(character);
}
static int
Fighter_setcharacter(m64py_Fighter* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Character type is read only.");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_POSITION_DOC,
"Tuple[float,float]: Absolute position of player");
static PyObject*
Fighter_getposition(m64py_Fighter* self, void* closure)
{
    PyObject *py_xpos, *py_ypos, *args;
    float xpos, ypos;
    const char* error;

    if (!m64py_memory_read_fighter_position(self->ssb64->memory_interface, self->n64_memory_address, &xpos, &ypos, &error))
    {
        PyErr_SetString(PyExc_RuntimeError, error);
        return NULL;
    }

    if ((py_xpos = PyFloat_FromDouble(xpos)) == NULL)
        goto alloc_xpos_failed;
    if ((py_ypos = PyFloat_FromDouble(ypos)) == NULL)
        goto alloc_ypos_failed;
    if ((args = PyTuple_New(2)) == NULL)
        goto alloc_args_failed;
    PyTuple_SET_ITEM(args, 0, py_xpos);
    PyTuple_SET_ITEM(args, 1, py_ypos);

    return args;

    alloc_args_failed : Py_DECREF(py_ypos);
    alloc_ypos_failed : Py_DECREF(py_xpos);
    alloc_xpos_failed : return NULL;
}
static int
Fighter_setposition(m64py_Fighter* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Position is read only.");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_VELOCITY_DOC,
"Tuple[float,float]: Speed at which the player is currently being launched\n"
"(e.g. after being hit by the opponent)");
static PyObject*
Fighter_getvelocity(m64py_Fighter* self, void* closure)
{
    PyObject *py_xvel, *py_yvel, *args;
    float xvel, yvel;

    m64py_memory_read_fighter_velocity(self->ssb64->memory_interface, self->n64_memory_address, &xvel, &yvel);

    if ((py_xvel = PyFloat_FromDouble(xvel)) == NULL)
        goto alloc_xpos_failed;
    if ((py_yvel = PyFloat_FromDouble(yvel)) == NULL)
        goto alloc_ypos_failed;
    if ((args = PyTuple_New(2)) == NULL)
        goto alloc_args_failed;
    PyTuple_SET_ITEM(args, 0, py_xvel);
    PyTuple_SET_ITEM(args, 1, py_yvel);

    return args;

    alloc_args_failed : Py_DECREF(py_yvel);
    alloc_ypos_failed : Py_DECREF(py_xvel);
    alloc_xpos_failed : return NULL;
}
static int
Fighter_setvelocity(m64py_Fighter* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Velocity is read only.");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_ACCELERATION_DOC,
"Tuple[float,float]: Accleration at which the player is currently being\n"
"launched (e.g. after being hit by the opponent)");
static PyObject*
Fighter_getacceleration(m64py_Fighter* self, void* closure)
{
    PyObject *py_xacc, *py_yacc, *args;
    float xacc, yacc;
    const char* error;

    m64py_memory_read_fighter_velocity(self->ssb64->memory_interface, self->n64_memory_address, &xacc, &yacc);

    if ((py_xacc = PyFloat_FromDouble(xacc)) == NULL)
        goto alloc_xpos_failed;
    if ((py_yacc = PyFloat_FromDouble(yacc)) == NULL)
        goto alloc_ypos_failed;
    if ((args = PyTuple_New(2)) == NULL)
        goto alloc_args_failed;
    PyTuple_SET_ITEM(args, 0, py_xacc);
    PyTuple_SET_ITEM(args, 1, py_yacc);

    return args;

    alloc_args_failed : Py_DECREF(py_yacc);
    alloc_ypos_failed : Py_DECREF(py_xacc);
    alloc_xpos_failed : return NULL;
}
static int
Fighter_setacceleration(m64py_Fighter* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Acceleration is read only.");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_ORIENTATION_DOC,
"int: -1=facing left, 1=facing right");
static PyObject*
Fighter_getorientation(m64py_Fighter* self, void* closure)
{
    int orientation;
    m64py_memory_read_fighter_orientation(self->ssb64->memory_interface, self->n64_memory_address, &orientation);
    return PyLong_FromLong(orientation);
}
static int
Fighter_setorientation(m64py_Fighter* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Orientation is read only.");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_MOVEMENT_FRAME_DOC,
"int: Counter that resets every time animation state changes. This could be\n"
"interpreted as 'animation progress', but it doesn't reset when the animation\n"
"loops.");
static PyObject*
Fighter_getmovement_frame(m64py_Fighter* self, void* closure)
{
    uint32_t frame;
    m64py_memory_read_fighter_movement_frame(self->ssb64->memory_interface, self->n64_memory_address, &frame);
    return PyLong_FromLong(frame);
}
static int
Fighter_setmovement_frame(m64py_Fighter* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Movement frame is read only.");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_MOVEMENT_STATE_DOC,
"int: The current state of the fighter. This is a number that uniquely identifies\n"
"every animation, even across characters.");
static PyObject*
Fighter_getmovement_state(m64py_Fighter* self, void* closure)
{
    int16_t state;
    m64py_memory_read_fighter_movement_state(self->ssb64->memory_interface, self->n64_memory_address, &state);
    return PyLong_FromLong(state);
}
static int
Fighter_setmovement_state(m64py_Fighter* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Movement state is read only.");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_SHIELD_HEALTH_DOC,
"int: How much shield the player currently has. Some characters have more\n"
"shield health than others. Pikachu's is 55, for example.");
static PyObject*
Fighter_getshield_health(m64py_Fighter* self, void* closure)
{
    uint32_t shield;
    m64py_memory_read_fighter_shield_health(self->ssb64->memory_interface, self->n64_memory_address, &shield);
    return PyLong_FromLong(shield);
}
static int
Fighter_setshield_health(m64py_Fighter* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Shield health is read only.");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_SHIELD_BREAK_RECOVERY_TIMER_DOC,
"int: After a shield break, this counts down to 0 while the player is stunned.");
static PyObject*
Fighter_getshield_break_recovery_timer(m64py_Fighter* self, void* closure)
{
    uint32_t time_left;
    m64py_memory_read_fighter_shield_break_recovery_timer(self->ssb64->memory_interface, self->n64_memory_address, &time_left);
    return PyLong_FromLong(time_left);
}
static int
Fighter_setshield_break_recovery_timer(m64py_Fighter* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Shield break recovery timer is read only.");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_PERCENT_DOC,
"int: Current \"percentage\" of the player from 0-999");
static PyObject*
Fighter_getpercent(m64py_Fighter* self, void* closure)
{
    uint32_t percent;
    m64py_memory_read_fighter_percent(self->ssb64->memory_interface, self->n64_memory_address, &percent);
    return PyLong_FromLong(percent);
}
static int
Fighter_setpercent(m64py_Fighter* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Percent is read only.");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_IS_INVINCIBLE_DOC,
"bool: True if the player is invincible (TODO: Not entirely sure what this means. Invincibility frames?)");
static PyObject*
Fighter_getis_invincible(m64py_Fighter* self, void* closure)
{
    int is_invincible;
    m64py_memory_read_fighter_is_invincible(self->ssb64->memory_interface, self->n64_memory_address, &is_invincible);
    if (is_invincible)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}
static int
Fighter_setis_invincible(m64py_Fighter* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Invincibility state is read only.");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_IS_GROUNDED_DOC,
"bool: True if the player is standing on the ground or on a platform");
static PyObject*
Fighter_getis_grounded(m64py_Fighter* self, void* closure)
{
    int is_grounded;
    m64py_memory_read_fighter_is_grounded(self->ssb64->memory_interface, self->n64_memory_address, &is_grounded);
    if (is_grounded)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}
static int
Fighter_setis_grounded(m64py_Fighter* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Grounded state is read only.");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_STOCKS_DOC,
"int: Number of stocks left");
static PyObject*
Fighter_getstocks(m64py_Fighter* self, void* closure)
{
    uint8_t stocks;
    m64py_memory_read_fighter_stocks(self->ssb64->memory_interface, self->memory_index, &stocks);
    return PyLong_FromLong(stocks);
}
static int
Fighter_setstocks(m64py_Fighter* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Number of stocks is read only.");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_CONTROLLER_DOC,
"m64py.Controller: An object that can control the player's button and joystick inputs");
static m64py_Controller*
Fighter_getcontroller(m64py_Fighter* self, void* closure)
{
    return Py_INCREF(self->controller), self->controller;
}
static int
Fighter_setcontroller(m64py_Fighter* self, PyObject* value, void* closure)
{
    PyErr_SetString(PyExc_AttributeError, "Controller is read-only");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Fighter_getset[] = {
    {"character",                   (getter)Fighter_getcharacter,                   (setter)Fighter_setcharacter,                   FIGHTER_CHARACTER_DOC, NULL},
    {"position",                    (getter)Fighter_getposition,                    (setter)Fighter_setposition,                    FIGHTER_POSITION_DOC, NULL},
    {"velocity",                    (getter)Fighter_getvelocity,                    (setter)Fighter_setvelocity,                    FIGHTER_VELOCITY_DOC, NULL},
    {"acceleration",                (getter)Fighter_getacceleration,                (setter)Fighter_setacceleration,                FIGHTER_ACCELERATION_DOC, NULL},
    {"orientation",                 (getter)Fighter_getorientation,                 (setter)Fighter_setorientation,                 FIGHTER_ORIENTATION_DOC, NULL},
    {"movement_frame",              (getter)Fighter_getmovement_frame,              (setter)Fighter_setmovement_frame,              FIGHTER_MOVEMENT_FRAME_DOC, NULL},
    {"movement_state",              (getter)Fighter_getmovement_state,              (setter)Fighter_setmovement_state,              FIGHTER_MOVEMENT_STATE_DOC, NULL},
    {"shield_health",               (getter)Fighter_getshield_health,               (setter)Fighter_setshield_health,               FIGHTER_SHIELD_HEALTH_DOC, NULL},
    {"shield_break_recovery_timer", (getter)Fighter_getshield_break_recovery_timer, (setter)Fighter_setshield_break_recovery_timer, FIGHTER_SHIELD_BREAK_RECOVERY_TIMER_DOC, NULL},
    {"percent",                     (getter)Fighter_getpercent,                     (setter)Fighter_setpercent,                     FIGHTER_PERCENT_DOC, NULL},
    {"is_invincible",               (getter)Fighter_getis_invincible,               (setter)Fighter_setis_invincible,               FIGHTER_IS_INVINCIBLE_DOC, NULL},
    {"is_grounded",                 (getter)Fighter_getis_grounded,                 (setter)Fighter_setis_grounded,                 FIGHTER_IS_GROUNDED_DOC, NULL},
    {"stocks",                      (getter)Fighter_getstocks,                      (setter)Fighter_setstocks,                      FIGHTER_STOCKS_DOC, NULL},
    {"controller",                  (getter)Fighter_getcontroller,                  (setter)Fighter_setcontroller,                  FIGHTER_CONTROLLER_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FIGHTER_DOC,
"Base class for all fighters. Provides methods for reading common state, such\n"
"as position, percentage, stock count, etc.");
PyTypeObject m64py_FighterType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "m64py.Fighter",              /* tp_name */
    sizeof(m64py_Fighter),        /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)Fighter_dealloc,  /* tp_dealloc */
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  /* tp_flags */
    FIGHTER_DOC,                  /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    0,                            /* tp_methods */
    0,                            /* tp_members */
    Fighter_getset,               /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    0,                            /* tp_init */
    0,                            /* tp_alloc */
    Fighter_new,                  /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
m64py_FighterType_init(void)
{
    if (PyType_Ready(&m64py_FighterType) < 0)
        return -1;
    return 0;
}
