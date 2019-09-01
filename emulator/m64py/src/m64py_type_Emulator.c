#include "m64py_type_Emulator.h"

#define M64P_CORE_PROTOTYPES
#include "m64p_frontend.h"

#include <stdint.h>
#include <stdio.h>

#define CORE_API_VERSION 0x020001

/* ------------------------------------------------------------------------- */
static void
Emulator_dealloc(m64py_Emulator* self)
{
    CoreShutdown();
    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Emulator_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_Emulator* self;
    const char *config_path, *data_path;

    if (!PyArg_ParseTuple(args, "ss", &config_path, &data_path))
        return NULL;

    /* This function checks if it was already initialized, so it's fine to call
     * it again */
    if (CoreStartup(CORE_API_VERSION,
                    config_path,
                    data_path,
                    NULL, NULL, /* Debug name/callback */
                    NULL, NULL  /* State name/callback */
            ) != M64ERR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, "There can only be one instance of m64py.Emulator (libmupen64plus has static state)");
        goto libmupen_init_failed;
    }

    self = (m64py_Emulator*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    return (PyObject*)self;

    alloc_self_failed    : CoreShutdown();
    libmupen_init_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
set_input_plugin(PyObject* self, PyObject* args)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
set_audio_plugin(PyObject* self, PyObject* args)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
set_video_plugin(PyObject* self, PyObject* args)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
set_rsp_plugin(PyObject* self, PyObject* args)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
load_ai_plugin(PyObject* self, PyObject* args)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
load_ssb64_rom(PyObject* self, PyObject* args)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
unload_rom(PyObject* self, PyObject* arg)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
advance_frame(PyObject* self, PyObject* args)
{
    if (CoreDoCommand(M64CMD_ADVANCE_FRAME, 0, NULL) != M64ERR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to advance frame: Emulator is in an invalid state");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
run_macro(PyObject* self, PyObject* arg)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef Emulator_methods[] = {
    {"set_input_plugin",  set_input_plugin, METH_O, ""},
    {"set_audio_plugin",  set_audio_plugin, METH_O, ""},
    {"set_video_plugin",  set_video_plugin, METH_O, ""},
    {"set_rsp_plugin",    set_rsp_plugin,   METH_O, ""},
    {"load_ssb64_rom",    load_ssb64_rom,    METH_O, ""},
    {"unload_rom",        unload_rom,        METH_NOARGS, ""},
    {"advance_frame",     advance_frame,     METH_NOARGS, ""},
    {"run_macro",         run_macro,         METH_O, ""},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyTypeObject m64py_EmulatorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "m64py.Emulator",             /* tp_name */
    sizeof(m64py_Emulator),       /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)Emulator_dealloc, /* tp_dealloc */
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
    "Emulator objects",           /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    Emulator_methods,             /* tp_methods */
    0,                            /* tp_members */
    0,                            /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    0,                            /* tp_init */
    0,                            /* tp_alloc */
    Emulator_new,                 /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
m64py_EmulatorType_init(void)
{
    if (PyType_Ready(&m64py_EmulatorType) < 0)
        return -1;
    return 0;
}
