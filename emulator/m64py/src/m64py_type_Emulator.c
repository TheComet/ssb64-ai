#include "m64py_type_Emulator.h"
#include "m64py_type_Plugin.h"
#include <structmember.h>

#define M64P_CORE_PROTOTYPES
#include "m64p_frontend.h"
#include "osal_dynamiclib.h"

#include <stdint.h>
#include <stdio.h>

#define CORE_API_VERSION 0x020001

/* ------------------------------------------------------------------------- */
static void
Emulator_dealloc(m64py_Emulator* self)
{
    Py_XDECREF(self->input_plugin);
    Py_XDECREF(self->video_plugin);
    Py_XDECREF(self->video_plugin);
    Py_XDECREF(self->rsp_plugin);
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

    self->input_plugin = (Py_INCREF(Py_None), Py_None);
    self->video_plugin = (Py_INCREF(Py_None), Py_None);
    self->video_plugin = (Py_INCREF(Py_None), Py_None);
    self->rsp_plugin   = (Py_INCREF(Py_None), Py_None);

    return (PyObject*)self;

    alloc_self_failed    : CoreShutdown();
    libmupen_init_failed : return NULL;
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
    {"load_ssb64_rom",    load_ssb64_rom,   METH_VARARGS, ""},
    {"unload_rom",        unload_rom,       METH_NOARGS, ""},
    {"advance_frame",     advance_frame,    METH_NOARGS, ""},
    {"run_macro",         run_macro,        METH_O, ""},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static int
set_plugin_attribute(m64py_Emulator* self, PyObject* value, m64p_plugin_type plugin_type)
{
    PyObject** pmember;
    switch (plugin_type)
    {
        case M64PLUGIN_INPUT : pmember = &self->input_plugin; break;
        case M64PLUGIN_AUDIO : pmember = &self->audio_plugin; break;
        case M64PLUGIN_GFX   : pmember = &self->video_plugin; break;
        case M64PLUGIN_RSP   : pmember = &self->video_plugin; break;
        default :
            PyErr_SetString(PyExc_RuntimeError, "Unsupported plugin type was set (this shouldn't happen)");
            return -1;
    }

    if (value == NULL)
    {
        PyErr_SetString(PyExc_TypeError, "Cannot delete input plugin attribute");
        return -1;
    }

    if (PyUnicode_Check(value))
    {
        m64py_Plugin* new_plugin;
        PyObject* bytes = PyUnicode_AsASCIIString(value);
        if (bytes == NULL)
            return -1;

        const char* filepath = PyBytes_AS_STRING(bytes);
        new_plugin = m64py_Plugin_load(self, filepath, plugin_type);
        Py_DECREF(bytes);
        if (new_plugin == NULL)
            return -1;

        PyObject* tmp = *pmember;
        *pmember = value;
        Py_DECREF(tmp);
    }
    else if (value == Py_None)
    {
        PyObject* tmp = *pmember;
        Py_INCREF(value);
        *pmember = value;
        Py_DECREF(tmp);
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Value must be either None or a path to a plugin to load");
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
getinput_plugin(m64py_Emulator* self, void* closure)
{
    return Py_INCREF(self->input_plugin), self->input_plugin;
}

/* ------------------------------------------------------------------------- */
static int
setinput_plugin(m64py_Emulator* self, PyObject* value, void* closure)
{
    return set_plugin_attribute(self, value, M64PLUGIN_INPUT);
}

/* ------------------------------------------------------------------------- */
static PyObject*
getaudio_plugin(m64py_Emulator* self, void* closure)
{
    return Py_INCREF(self->audio_plugin), self->audio_plugin;
}

/* ------------------------------------------------------------------------- */
static int
setaudio_plugin(m64py_Emulator* self, PyObject* value, void* closure)
{
    return set_plugin_attribute(self, value, M64PLUGIN_AUDIO);
}

/* ------------------------------------------------------------------------- */
static PyObject*
getvideo_plugin(m64py_Emulator* self, void* closure)
{
    return Py_INCREF(self->video_plugin), self->video_plugin;
}

/* ------------------------------------------------------------------------- */
static int
setvideo_plugin(m64py_Emulator* self, PyObject* value, void* closure)
{
    return set_plugin_attribute(self, value, M64PLUGIN_GFX);
}

/* ------------------------------------------------------------------------- */
static PyObject*
getrsp_plugin(m64py_Emulator* self, void* closure)
{
    return Py_INCREF(self->rsp_plugin), self->rsp_plugin;
}

/* ------------------------------------------------------------------------- */
static int
setrsp_plugin(m64py_Emulator* self, PyObject* value, void* closure)
{
    return set_plugin_attribute(self, value, M64PLUGIN_GFX);
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Emulator_getsetters[] = {
    {"input_plugin", (getter)getinput_plugin, (setter)setinput_plugin, "", NULL},
    {"audio_plugin", (getter)getaudio_plugin, (setter)setaudio_plugin, "", NULL},
    {"video_plugin", (getter)getvideo_plugin, (setter)setvideo_plugin, "", NULL},
    {"rsp_plugin",   (getter)getrsp_plugin,   (setter)setrsp_plugin, "", NULL},
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
    Emulator_getsetters,          /* tp_getset */
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
