#include "m64py_type_Emulator.h"
#include "m64py_type_Plugin.h"
#include "m64py_type_SSB64.h"
#include <structmember.h>
#include <frameobject.h>

#include "osal_dynamiclib.h"

#include <stdint.h>
#include <stdio.h>

#define CORE_API_VERSION 0x020001

/* Unfortunately, the frame callback doesn't let you pass in a context pointer,
 * and mupen64 has global state and only supports a single instance of the
 * emulator. So this variable makes sure that there is only one instance of
 * the python Emulator and also allows the frame callback function to propagate
 * the call to python.
 */
static m64py_Emulator* g_emu = NULL;

static int
start_plugin(m64py_Emulator* emu, PyObject* maybePlugin);
static void
stop_plugin(m64py_Emulator* emu, PyObject* maybePlugin);

/* ------------------------------------------------------------------------- */
static void
catch_python_exception(void)
{
    PyObject *exc_type, *exc_value, *exc_tb;

    PyErr_Fetch(&exc_type, &exc_value, &exc_tb);
    PyErr_NormalizeException(&exc_type, &exc_value, &exc_tb);

    if (exc_tb)
    {
        PyTracebackObject* traceback;
        fprintf(stderr, "Traceback (most recent call last):\n");
        for (traceback = (PyTracebackObject*)exc_tb; traceback; traceback = traceback->tb_next)
        {
            PyFrameObject* frame = traceback->tb_frame;
            fprintf(stderr, "  File \"%s\", line %d, in %s\n",
                    PyUnicode_AsUTF8(frame->f_code->co_filename),
                    PyCode_Addr2Line(frame->f_code, frame->f_lasti),
                    PyUnicode_AsUTF8(frame->f_code->co_name)
            );
        }
    }

    if (exc_value)
    {
        PyObject* exc_value_str = PyObject_Str(exc_value);
        if (exc_value_str == NULL)
            goto exc_value_failed;
        fprintf(stderr, "%s: %s\n", Py_TYPE(exc_type)->tp_name, PyUnicode_AsUTF8(exc_value_str));
        Py_DECREF(exc_value_str);
    } exc_value_failed:

    Py_XDECREF(exc_type);
    Py_XDECREF(exc_value);
    Py_XDECREF(exc_tb);
    PyErr_Clear();
}

/* ------------------------------------------------------------------------- */
static void
log_message_callback(m64py_Emulator* self, int level, const char* message)
{
    PyObject *py_level, *py_message, *args, *result;

    if (self->log_message_callback == NULL)
        return;

    if ((py_level = PyLong_FromLong(level)) == NULL)
        goto py_level_failed;
    if ((py_message = PyUnicode_FromString(message)) == NULL)
        goto py_message_failed;
    if ((args = PyTuple_New(2)) == NULL)
        goto py_args_failed;

    PyTuple_SET_ITEM(args, 0, py_level);
    PyTuple_SET_ITEM(args, 1, py_message);
    result = PyObject_CallObject(self->log_message_callback, args);
    Py_DECREF(args);

    if (result != NULL)
        Py_DECREF(result);

    return;

    py_args_failed    : Py_DECREF(py_message);
    py_message_failed : Py_DECREF(py_level);
    py_level_failed   : return;
}

/* ------------------------------------------------------------------------- */
static void
frame_callback(int current_frame)
{
    PyObject *py_frame, *args, *result;

    if (g_emu->frame_callback == NULL)
        return;

    if ((py_frame = PyLong_FromLong(current_frame)) == NULL)
        goto py_frame_failed;
    if ((args = PyTuple_New(1)) == NULL)
        goto py_args_failed;

    PyTuple_SET_ITEM(args, 0, py_frame);
    result = PyObject_CallObject(g_emu->frame_callback, args);
    Py_DECREF(args);

    if (result == NULL)
    {
        /* We want to catch all exceptions except for keyboard interrupts */
        if (!PyErr_ExceptionMatches(PyExc_KeyboardInterrupt))
            catch_python_exception();
        g_emu->corelib.CoreDoCommand(M64CMD_STOP, 0, NULL);
    }
    else
        Py_DECREF(result);

    return;

    py_args_failed  : Py_DECREF(py_frame);
    py_frame_failed : return;
}

/* ------------------------------------------------------------------------- */
static void
Emulator_dealloc(m64py_Emulator* self)
{
    Py_XDECREF(self->log_message_callback);
    Py_XDECREF(self->frame_callback);
    Py_XDECREF(self->input_plugin);
    Py_XDECREF(self->video_plugin);
    Py_XDECREF(self->video_plugin);
    Py_XDECREF(self->rsp_plugin);

    if (self->corelib.handle)
    {
        self->corelib.CoreShutdown();
        osal_dynlib_close(self->corelib.handle);
    }
    g_emu = 0;

    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Emulator_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    const char *corelib_path, *config_path, *data_path;
    m64p_error result;
    static char* kwds_str[] = {
        "corelib_path",
        "config_path",
        "data_path",
        NULL
    };

    /* mupen64plus has global state so enforce only one instance of the Emulator
     * object */
    if (g_emu)
    {
        PyErr_SetString(PyExc_RuntimeError, "There can only be one instance of m64py.Emulator (libmupen64plus has static state)");
        return NULL;
    }

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sss", kwds_str, &corelib_path, &config_path, &data_path))
        return NULL;

    printf("config_path: %s\ndata_path: %s\n", config_path, data_path);

    g_emu = (m64py_Emulator*)type->tp_alloc(type, 0);
    if (g_emu == NULL)
        goto alloc_self_failed;

    /* Try to load mupen64plus corelib */
    if ((result = osal_dynlib_open(&g_emu->corelib.handle, corelib_path) != M64ERR_SUCCESS))
    {
        PyErr_Format(PyExc_RuntimeError, "Could not open shared library \"%s\": %s", corelib_path, osal_dynlib_last_error());
        goto open_corelib_failed;
    }

    /* Load all function pointers */
#define X(name) \
        if ((g_emu->corelib.name = (ptr_##name)osal_dynlib_getproc(g_emu->corelib.handle, #name)) == NULL) \
        { \
            PyErr_Format(PyExc_RuntimeError, "Failed to load function \"%s\" from core library \"%s\"", #name, corelib_path); \
            goto load_corelib_functions_failed; \
        }
    M64PY_CORELIB_FUNCTIONS
#undef X

    /* Init corelib */
    if ((result = g_emu->corelib.CoreStartup(
            CORE_API_VERSION,
            config_path,
            data_path,
            g_emu, (ptr_DebugCallback)log_message_callback,
            NULL, NULL  /* State change callback */
        )) != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to initialize corelib. Error code %d", result);
        goto corelib_startup_failed;
    }

    g_emu->corelib.CoreDoCommand(M64CMD_SET_FRAME_CALLBACK, 0, frame_callback);

    g_emu->input_plugin = (Py_INCREF(Py_None), Py_None);
    g_emu->audio_plugin = (Py_INCREF(Py_None), Py_None);
    g_emu->video_plugin = (Py_INCREF(Py_None), Py_None);
    g_emu->rsp_plugin   = (Py_INCREF(Py_None), Py_None);

    return (PyObject*)g_emu;

    corelib_startup_failed        :
    load_corelib_functions_failed : osal_dynlib_close(g_emu->corelib.handle);
    open_corelib_failed           : Py_DECREF(g_emu); g_emu = NULL;
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
execute(m64py_Emulator* self, PyObject* args)
{
    if (self->corelib.CoreDoCommand(M64CMD_EXECUTE, 0, NULL) != M64ERR_SUCCESS)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to execute: Emulator is in an invalid state");
        return NULL;
    }

    /* User might exit with an exception set, we need to propagate it */
    if (PyErr_Occurred())
        return NULL;

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
stop(m64py_Emulator* self, PyObject* args)
{
    m64p_error result;
    if ((result = self->corelib.CoreDoCommand(M64CMD_STOP, 0, NULL)) != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to stop emulator. Return code %d", result);
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
    {"load_ssb64_rom",     (PyCFunction)load_ssb64_rom,     METH_O, ""},
    {"execute",            (PyCFunction)execute,            METH_NOARGS, ""},
    {"stop",               (PyCFunction)stop,               METH_NOARGS, ""},
    {"run_macro",          (PyCFunction)run_macro,          METH_O, ""},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static int
try_attaching_plugin(m64py_Emulator* self, PyObject* maybe_current_plugin, m64py_Plugin* new_plugin)
{
    /* If there is currently a plugin attached, need to un-attach it */
    stop_plugin(self, maybe_current_plugin);

    /* Try attaching new plugin. If that fails, attach the old one again. If
     * attaching the old one fails, well... That shouldn't ever happen */
    if (start_plugin(self, (PyObject*)new_plugin) == 0)
        return 0;

    /* Start old plugin again */
    if (start_plugin(self, maybe_current_plugin) != 0)
        return -2;
    return -1;
}

/* ------------------------------------------------------------------------- */
/*!
 * Will load the shared library at path_to_plugin and try to start/attach it
 * to the corelib. For this to work, the current plugin pointed to by one of
 * the emu->xxx_plugin members (pointed to by old_maybe_plugin), has to be
 * un-attached temporarily. If the new plugin is successfully attached, then
 * the old plugin is decref'd and the emu->xxx_plugin member is updated to
 * point to the new plugin object. If attaching fails, then the old plugin
 * is attached again. If that fails -- should never happen but who knows --
 * then the current plugin is replaced with Py_None.
 */
static int
try_loading_and_replacing_plugin(m64py_Emulator* self,
                                 PyObject** old_maybe_plugin,
                                 PyObject* path_to_plugin,
                                 m64p_plugin_type plugin_type)
{
    m64py_Plugin* new_plugin;
    const char* filepath_ascii;

    /* A path to the plugin shared lib was specified. Get path as ASCII
     * string */
    PyObject* bytes = PyUnicode_AsASCIIString(path_to_plugin);
    if (bytes == NULL)
        goto convert_to_ascii_failed;
    filepath_ascii = PyBytes_AS_STRING(bytes);

    /* Try to load the shared library and all necessary functions */
    new_plugin = m64py_Plugin_load(filepath_ascii, plugin_type, self->corelib.handle);
    if (new_plugin == NULL)
        goto load_plugin_failed;

    /* Plugin should be attached if emulator is running */
    if (self->is_rom_loaded)
    {
        switch (try_attaching_plugin(self, *old_maybe_plugin, new_plugin))
        {
            case -2 : {
                /* Old plugin failed to start, even though it worked before
                 * In this case I think we have no other option than to set
                 * the current plugin to Py_None */
                PyObject* tmp = *old_maybe_plugin;
                Py_INCREF(Py_None);
                *old_maybe_plugin = Py_None;
                Py_DECREF(tmp);
            } /* fallthrough */

            case -1 :
                /* New plugin failed to start, so old plugin was re-attached
                 * again */
                goto attach_plugin_failed;

            default : break;
        }
    }

    /* Don't need ASCII string anymore */
    Py_DECREF(bytes);

    /* Swap new plugin object with current one */
    PyObject* tmp = *old_maybe_plugin;
    *old_maybe_plugin = (PyObject*)new_plugin;
    Py_DECREF(tmp);
    return 0;

    attach_plugin_failed    : Py_DECREF(new_plugin);
    load_plugin_failed      : Py_DECREF(bytes);
    convert_to_ascii_failed : return -1;
}

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
        PyErr_SetString(PyExc_TypeError, "Cannot delete plugin attribute");
        return -1;
    }

    if (PyUnicode_Check(value))
    {
        return try_loading_and_replacing_plugin(self, pmember, value, plugin_type);
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
static PyObject*
getframe_callback(m64py_Emulator* self, void* closure)
{
    if (self->frame_callback)
        return Py_INCREF(self->frame_callback), self->frame_callback;

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static int
setframe_callback(m64py_Emulator* self, PyObject* callable)
{
    PyObject* tmp;

    if (callable == Py_None)
    {
        Py_XDECREF(self->frame_callback);
        self->frame_callback = NULL;
        return 0;
    }

    if (!PyCallable_Check(callable))
    {
        PyErr_SetString(PyExc_TypeError, "Object is not callable");
        return -1;
    }

    tmp = self->frame_callback;
    Py_INCREF(callable);
    self->frame_callback = callable;
    Py_XDECREF(tmp);

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
getlog_message_callback(m64py_Emulator* self, void* closure)
{
    if (self->log_message_callback)
        return Py_INCREF(self->log_message_callback), self->log_message_callback;

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static int
setlog_message_callback(m64py_Emulator* self, PyObject* callable)
{
    PyObject* tmp;

    if (callable == Py_None)
    {
        Py_XDECREF(self->frame_callback);
        self->frame_callback = NULL;
        return 0;
    }

    if (!PyCallable_Check(callable))
    {
        PyErr_SetString(PyExc_TypeError, "Object is not callable");
        return -1;
    }

    tmp = self->log_message_callback;
    Py_INCREF(callable);
    self->log_message_callback = callable;
    Py_XDECREF(tmp);

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Emulator_getsetters[] = {
    {"input_plugin",         (getter)getinput_plugin,         (setter)setinput_plugin, "", NULL},
    {"audio_plugin",         (getter)getaudio_plugin,         (setter)setaudio_plugin, "", NULL},
    {"video_plugin",         (getter)getvideo_plugin,         (setter)setvideo_plugin, "", NULL},
    {"rsp_plugin",           (getter)getrsp_plugin,           (setter)setrsp_plugin, "", NULL},
    {"frame_callback",       (getter)getframe_callback,       (setter)setframe_callback, "", NULL},
    {"log_message_callback", (getter)getlog_message_callback, (setter)setlog_message_callback, "", NULL},
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
    if (start_plugin(self, self->video_plugin) != 0)
        goto video_plugin_failed;

    if (start_plugin(self, self->audio_plugin) != 0)
        goto audio_plugin_failed;

    if (start_plugin(self, self->input_plugin) != 0)
        goto input_plugin_failed;

    if (start_plugin(self, self->rsp_plugin) != 0)
        goto rsp_plugin_failed;

    return 0;

    rsp_plugin_failed   : stop_plugin(self, self->input_plugin);
    input_plugin_failed : stop_plugin(self, self->audio_plugin);
    audio_plugin_failed : stop_plugin(self, self->video_plugin);
    video_plugin_failed : return -1;
}

/* ------------------------------------------------------------------------- */
void
m64py_Emulator_stop_plugins(m64py_Emulator* self)
{
    stop_plugin(self, self->rsp_plugin);
    stop_plugin(self, self->input_plugin);
    stop_plugin(self, self->audio_plugin);
    stop_plugin(self, self->video_plugin);
}
