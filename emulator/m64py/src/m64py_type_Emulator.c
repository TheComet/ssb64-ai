#include "m64py_type_Emulator.h"
#include "m64py_type_Plugin.h"
#include "m64py_type_SSB64.h"
#include <structmember.h>

#include "osal_dynamiclib.h"

#include <stdint.h>
#include <stdio.h>

#define CORE_API_VERSION 0x020001

static int g_corelib_loaded = 0;

/* ------------------------------------------------------------------------- */
static void
Emulator_dealloc(m64py_Emulator* self)
{
    Py_XDECREF(self->input_plugin);
    Py_XDECREF(self->video_plugin);
    Py_XDECREF(self->video_plugin);
    Py_XDECREF(self->rsp_plugin);

    self->corelib.CoreShutdown();
    osal_dynlib_close(self->corelib.handle);
    g_corelib_loaded = 0;

    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Emulator_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    m64py_Emulator* self;
    const char *corelib_path, *config_path, *data_path;
    m64p_error result;

    /* mupen64plus has global state so enforce only one instance of the Emulator
     * object */
    if (g_corelib_loaded)
    {
        PyErr_SetString(PyExc_RuntimeError, "There can only be one instance of m64py.Emulator (libmupen64plus has static state)");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "sss", &corelib_path, &config_path, &data_path))
        return NULL;

    self = (m64py_Emulator*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    /* Try to load mupen64plus corelib */
    if (osal_dynlib_open(&self->corelib.handle, corelib_path) != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Could not open shared library \"%s\"", corelib_path);
        goto open_corelib_failed;
    }

    /* Load all function pointers */
#define X(name) \
        if ((self->corelib.name = (ptr_##name)osal_dynlib_getproc(self->corelib.handle, #name)) == NULL) \
        { \
            PyErr_Format(PyExc_RuntimeError, "Failed to load function \"%s\" from core library \"%s\"", #name, corelib_path); \
            goto load_corelib_functions_failed; \
        }
    M64PY_CORELIB_FUNCTIONS
#undef X

    /* Init corelib */
    if ((result = self->corelib.CoreStartup(
            CORE_API_VERSION,
            config_path,
            data_path,
            NULL, NULL, /* Debug name/callback */
            NULL, NULL  /* State name/callback */
        )) != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to initialize corelib. Error code %d", result);
        goto corelib_startup_failed;
    }

    self->input_plugin = (Py_INCREF(Py_None), Py_None);
    self->audio_plugin = (Py_INCREF(Py_None), Py_None);
    self->video_plugin = (Py_INCREF(Py_None), Py_None);
    self->rsp_plugin   = (Py_INCREF(Py_None), Py_None);

    g_corelib_loaded = 1;

    return (PyObject*)self;

    corelib_startup_failed        :
    load_corelib_functions_failed : osal_dynlib_close(self->corelib.handle);
    open_corelib_failed           : Py_DECREF(self);
    alloc_self_failed             : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
load_ssb64_rom(m64py_Emulator* self, PyObject* arg)
{
    /* Prepend emulator instance as first argument for SSB64 constructor */
    PyObject* args = PyTuple_New(2);
    if (args == NULL)
        return NULL;
    Py_INCREF(self); PyTuple_SET_ITEM(args, 0, (PyObject*)self);
    Py_INCREF(arg);  PyTuple_SET_ITEM(args, 1, arg);

    m64py_SSB64* game = (m64py_SSB64*)PyObject_CallObject((PyObject*)&m64py_SSB64Type, args);
    Py_DECREF(args);
    return (PyObject*)game;
}

/* ------------------------------------------------------------------------- */
static PyObject*
unload_rom(PyObject* self, PyObject* arg)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
advance_frame(m64py_Emulator* self, PyObject* args)
{
    /*if (self->corelib.CoreDoCommand(M64CMD_ADVANCE_FRAME, 0, NULL) != M64ERR_SUCCESS)*/
    if (self->corelib.CoreDoCommand(M64CMD_EXECUTE, 0, NULL) != M64ERR_SUCCESS)
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
    {"load_ssb64_rom",  (PyCFunction)load_ssb64_rom, METH_O, ""},
    {"unload_rom",      (PyCFunction)unload_rom,     METH_NOARGS, ""},
    {"advance_frame",   (PyCFunction)advance_frame,  METH_NOARGS, ""},
    {"run_macro",       (PyCFunction)run_macro,      METH_O, ""},
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
        case M64PLUGIN_RSP   : pmember = &self->rsp_plugin; break;
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
        m64p_error result;
        m64py_Plugin* new_plugin;
        const char* filepath;

        /* A path to the plugin shared lib was specified. Get path as ASCII
         * string */
        PyObject* bytes = PyUnicode_AsASCIIString(value);
        if (bytes == NULL)
            goto convert_to_ascii_failed;
        filepath = PyBytes_AS_STRING(bytes);

        /* Try to load the shared library and all necessary functions */
        new_plugin = m64py_Plugin_load(filepath, plugin_type, self->corelib.handle);
        if (new_plugin == NULL)
            goto load_plugin_failed;

        /* Don't need ASCII string anymore */
        Py_DECREF(bytes);

        /* Swap new plugin object with current one */
        PyObject* tmp = *pmember;
        *pmember = (PyObject*)new_plugin;
        Py_DECREF(tmp);
        return 0;

        load_plugin_failed      : Py_DECREF(bytes);
        convert_to_ascii_failed : return -1;
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
    return set_plugin_attribute(self, value, M64PLUGIN_RSP);
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

/* ------------------------------------------------------------------------- */
static int
start_plugin(m64py_Emulator* emu, PyObject* maybePlugin)
{
    m64p_error result;
    m64py_Plugin* plugin;

    if (!m64py_Plugin_CheckExact(maybePlugin))
        return 0;
    plugin = (m64py_Plugin*)maybePlugin;

    /* call the plugin's initialization function and make sure it starts okay */
    if ((result = plugin->PluginStartup(emu->corelib.handle, NULL, NULL)) != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Plugin \"%s\" failed to start. Error code %d", plugin->name, result);
        goto start_plugin_failed;
    }

    /* Attach plugin to corelib */
    if ((result = emu->corelib.CoreAttachPlugin(plugin->type, plugin->handle)) != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to attach plugin \"%s\" to core. Error code %d", plugin->name, result);
        goto attach_plugin_failed;
    }

    return 0;

    attach_plugin_failed : plugin->PluginShutdown();
    start_plugin_failed  : return -1;
}

/* ------------------------------------------------------------------------- */
static void
stop_plugin(m64py_Emulator* emu, PyObject* maybePlugin)
{
    m64py_Plugin* plugin;
    if (!m64py_Plugin_CheckExact(maybePlugin))
        return;
    plugin = (m64py_Plugin*)maybePlugin;

    emu->corelib.CoreDetachPlugin(plugin->type);
    plugin->PluginShutdown();
}

/* ------------------------------------------------------------------------- */
int
m64py_Emulator_start_plugins(m64py_Emulator* self)
{
    if (start_plugin(self, self->input_plugin) != 0)
        goto input_plugin_failed;

    if (start_plugin(self, self->audio_plugin) != 0)
        goto audio_plugin_failed;

    if (start_plugin(self, self->video_plugin) != 0)
        goto video_plugin_failed;

    if (start_plugin(self, self->rsp_plugin) != 0)
        goto rsp_plugin_failed;

    return 0;

    rsp_plugin_failed   : stop_plugin(self, self->video_plugin);
    video_plugin_failed : stop_plugin(self, self->audio_plugin);
    audio_plugin_failed : stop_plugin(self, self->input_plugin);
    input_plugin_failed : return -1;
}

/* ------------------------------------------------------------------------- */
void
m64py_Emulator_stop_plugins(m64py_Emulator* self)
{
    stop_plugin(self, self->rsp_plugin);
    stop_plugin(self, self->video_plugin);
    stop_plugin(self, self->audio_plugin);
    stop_plugin(self, self->input_plugin);
}
