#include "m64py_module.h"
#include "m64py_type_Emulator.h"
#include "m64py_type_Plugin.h"
#include "m64py_type_Plugin_CuckedInputPlugin.h"
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

static int
try_loading_and_replacing_plugin(
    m64py_Emulator* self,
    PyObject* path_to_plugin,
    m64p_plugin_type plugin_type,
    PyObject** old_maybe_plugin
);

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

    if (self->log_message_callback == Py_None)
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

    if (g_emu->frame_callback == Py_None)
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
vi_callback(m64py_Emulator* self)
{
    PyObject* result;

    if (g_emu->vi_callback == Py_None)
        return;

    result = PyObject_CallObject(g_emu->vi_callback, NULL);
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
        if (self->corelib.CoreShutdown)
            self->corelib.CoreShutdown();
        osal_dynlib_close(self->corelib.handle);
    }
    g_emu = 0;

    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static int
try_load_corelib(m64py_Emulator* self, const char* config_path, const char* data_path, const char* corelib_file)
{
    m64p_error result;
    PyObject* py_corelib_file = NULL;

    /* If corelib path wasn't provided, load default */
    if (corelib_file == NULL)
    {
        py_corelib_file = m64py_prepend_module_path_to_filename("libmupen64plus.so");
        if (py_corelib_file == NULL)
            return -1;
        corelib_file = PyUnicode_AsUTF8(py_corelib_file);
    }

    /* Try to load mupen64plus corelib */
    result = osal_dynlib_open(&self->corelib.handle, corelib_file);
    Py_XDECREF(py_corelib_file);
    if (result != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Could not open shared library \"%s\": %s", corelib_file, osal_dynlib_last_error());
        return -1;
    }

    /* Load all function pointers */
#define X(name) \
        if ((self->corelib.name = (ptr_##name)osal_dynlib_getproc(self->corelib.handle, #name)) == NULL) \
        { \
            PyErr_Format(PyExc_RuntimeError, "Failed to load function \"%s\" from core library \"%s\"", #name, corelib_file); \
            return -1; \
        }
    M64PY_CORELIB_FUNCTIONS
#undef X

    /* Init corelib */
    if ((result = self->corelib.CoreStartup(
            CORE_API_VERSION,
            config_path,
            data_path,
            self, (ptr_DebugCallback)log_message_callback,
            NULL, NULL  /* State change callback */
        )) != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to initialize corelib. Error code %d", result);
        return -1;
    }

    self->corelib.CoreDoCommand(M64CMD_SET_FRAME_CALLBACK, 0, frame_callback);
    self->corelib.AISetVICallback((void(*)(void*))vi_callback, self);

    return 0;
}

/* ------------------------------------------------------------------------- */
static int
try_load_default_plugins(m64py_Emulator* self)
{
    int success = 0;
    PyObject *input_path, *video_path, *audio_path, *rsp_path;
    if ((input_path = m64py_prepend_module_path_to_filename("mupen64plus-input-sdl.so")) == NULL) goto input_path_failed;
    if ((video_path = m64py_prepend_module_path_to_filename("mupen64plus-video-glide64mk2.so")) == NULL) goto video_path_failed;
    if ((audio_path = m64py_prepend_module_path_to_filename("mupen64plus-audio-sdl.so")) == NULL) goto audio_path_failed;
    if ((rsp_path   = m64py_prepend_module_path_to_filename("mupen64plus-rsp-hle.so")) == NULL) goto rsp_path_failed;

    success = (
        try_loading_and_replacing_plugin(self, input_path, M64PLUGIN_INPUT, &self->input_plugin) == 0 &&
        try_loading_and_replacing_plugin(self, video_path, M64PLUGIN_GFX, &self->video_plugin) == 0 &&
        try_loading_and_replacing_plugin(self, audio_path, M64PLUGIN_AUDIO, &self->audio_plugin) == 0 &&
        try_loading_and_replacing_plugin(self, rsp_path, M64PLUGIN_RSP, &self->rsp_plugin) == 0
    );

                        Py_DECREF(rsp_path);
    rsp_path_failed   : Py_DECREF(audio_path);
    audio_path_failed : Py_DECREF(video_path);
    video_path_failed : Py_DECREF(input_path);
    input_path_failed : return success;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Emulator_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    const char *corelib_path, *config_path, *data_path;
    static char* kwds_str[] = {
        "config_path",
        "data_path",
        "corelib_path",
        NULL
    };

    /* mupen64plus has global state so enforce only one instance of the Emulator
     * object */
    if (g_emu)
    {
        PyErr_SetString(PyExc_RuntimeError, "There can only be one instance of m64py.Emulator (libmupen64plus has static state)");
        return NULL;
    }

    corelib_path = NULL;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ss|s", kwds_str, &config_path, &data_path, &corelib_path))
        return NULL;

    g_emu = (m64py_Emulator*)type->tp_alloc(type, 0);
    if (g_emu == NULL)
        goto alloc_self_failed;

    /* Set these all to valid python objects because log_message_callback cannot
     * handle NULL */
    g_emu->input_plugin         = (Py_INCREF(Py_None), Py_None);
    g_emu->audio_plugin         = (Py_INCREF(Py_None), Py_None);
    g_emu->video_plugin         = (Py_INCREF(Py_None), Py_None);
    g_emu->rsp_plugin           = (Py_INCREF(Py_None), Py_None);
    g_emu->frame_callback       = (Py_INCREF(Py_None), Py_None);
    g_emu->log_message_callback = (Py_INCREF(Py_None), Py_None);
    g_emu->vi_callback          = (Py_INCREF(Py_None), Py_None);

    if (try_load_corelib(g_emu, config_path, data_path, corelib_path) == -1)
        goto init_failed;

    /* Try to load default plugins */
    if (try_load_default_plugins(g_emu) == -1)
        goto init_failed;

    return (PyObject*)g_emu;

    init_failed       : Py_DECREF(g_emu); g_emu = NULL;
    alloc_self_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(LOAD_SSB64_ROM_DOC, "load_ssb64_rom(path_to_rom)\n--\n\n"
"Loads a Super Smash Bros. 64 ROM");
static PyObject*
Emulator_load_ssb64_rom(m64py_Emulator* self, PyObject* arg)
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
PyDoc_STRVAR(EXECUTE_DOC, "execute()\n--\n\n"
"Call this after loading the ROM and setting your callback functions. Will\n"
"enter the emulator's main loop.");
static PyObject*
Emulator_execute(m64py_Emulator* self, PyObject* args)
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
PyDoc_STRVAR(STOP_DOC, "stop()\n--\n\n"
"Stops the emulator. The execute() function will return.");
static PyObject*
Emulator_stop(m64py_Emulator* self, PyObject* args)
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
PyDoc_STRVAR(SAVE_STATE_DOC, "save_state(filename)\n--\n\n"
"Saves the current emulator state to a file. The state can then be loaded back\n"
"at a later point in time with load_state().");
static PyObject*
Emulator_save_state(m64py_Emulator* self, PyObject* py_filename)
{
    m64p_error result;
    const char* filename;
    if (!PyUnicode_CheckExact(py_filename))
    {
        PyErr_SetString(PyExc_TypeError, "save_state(): Expected a filename of type str");
        return NULL;
    }

    filename = PyUnicode_AsUTF8(py_filename);
    if (filename == NULL)
        return NULL;

    result = self->corelib.CoreDoCommand(M64CMD_STATE_SAVE, 1 /* savestates_type_m64p */, (void*)filename);
    if (result != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to save state: Error code %d", result);
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(LOAD_STATE_DOC, "load_state(filename)\n--\n\n"
"Loads a previously saved state.");
static PyObject*
Emulator_load_state(m64py_Emulator* self, PyObject* py_filename)
{
    m64p_error result;
    const char* filename;
    if (!PyUnicode_CheckExact(py_filename))
    {
        PyErr_SetString(PyExc_TypeError, "load_state(): Expected a filename of type str");
        return NULL;
    }

    filename = PyUnicode_AsUTF8(py_filename);
    if (filename == NULL)
        return NULL;

    result = self->corelib.CoreDoCommand(M64CMD_STATE_LOAD, 0, (void*)filename);
    if (result != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to load state: Error code %d", result);
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef Emulator_methods[] = {
    {"load_ssb64_rom",     (PyCFunction)Emulator_load_ssb64_rom,     METH_O,      LOAD_SSB64_ROM_DOC},
    {"execute",            (PyCFunction)Emulator_execute,            METH_NOARGS, EXECUTE_DOC},
    {"stop",               (PyCFunction)Emulator_stop,               METH_NOARGS, STOP_DOC},
    {"save_state",         (PyCFunction)Emulator_save_state,         METH_O,      SAVE_STATE_DOC},
    {"load_state",         (PyCFunction)Emulator_load_state,         METH_O,      LOAD_STATE_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
#define OLD_PLUGIN_STARTED_SUCCESSFULLY  -1
#define OLD_PLUGIN_FAILED_TO_START_AGAIN -2
static int
try_attaching_plugin(m64py_Emulator* self, PyObject* maybe_current_plugin, m64py_Plugin* new_plugin)
{
    /* Try attaching new plugin. If that fails, attach the old one again. If
     * attaching the old one fails, well... That shouldn't ever happen */

    stop_plugin(self, maybe_current_plugin);

    if (start_plugin(self, (PyObject*)new_plugin) == 0)
        return 0;

    if (start_plugin(self, maybe_current_plugin) != 0)
        return OLD_PLUGIN_FAILED_TO_START_AGAIN;
    return OLD_PLUGIN_STARTED_SUCCESSFULLY;
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
try_loading_and_replacing_plugin(
    m64py_Emulator* self,
    PyObject* path_to_plugin,
    m64p_plugin_type plugin_type,
    PyObject** old_maybe_plugin
) {
    PyObject* new_plugin;
    PyObject* py_plugin_type;
    PyObject* py_corelib_handle;
    PyObject* plugin_args;
    PyTypeObject* PluginType;

    /* Build argument list for constructing a plugin */
    py_plugin_type = PyLong_FromLong(plugin_type);
    if (py_plugin_type == NULL)
        goto alloc_plugin_type_failed;
    py_corelib_handle = PyLong_FromVoidPtr(self->corelib.handle);
    if (py_corelib_handle == NULL)
        goto alloc_corelib_handle_failed;
    plugin_args = PyTuple_New(3);
    if (plugin_args == NULL)
        goto alloc_plugin_args_failed;

    Py_INCREF(path_to_plugin);
    PyTuple_SET_ITEM(plugin_args, 0, path_to_plugin);
    PyTuple_SET_ITEM(plugin_args, 1, py_plugin_type);
    PyTuple_SET_ITEM(plugin_args, 2, py_corelib_handle);

    /* Input plugin is handled specially (gets intercepted so AI can take over) */
    if (plugin_type == M64PLUGIN_INPUT)
        PluginType = &m64py_CuckedInputPluginType;
    else
        PluginType = &m64py_PluginType;

    new_plugin = PyObject_CallObject((PyObject*)PluginType, plugin_args);
    Py_DECREF(plugin_args);
    goto fail_if_new_plugin_is_null;

    alloc_plugin_args_failed    : Py_DECREF(py_corelib_handle);
    alloc_corelib_handle_failed : Py_DECREF(py_plugin_type);
    alloc_plugin_type_failed    : return -1;
    fail_if_new_plugin_is_null  : if (new_plugin == NULL) return -1;

    /* Emulator expects all loaded plugins to also be attached */
    if (self->is_rom_loaded)
    {
        switch (try_attaching_plugin(self, *old_maybe_plugin, (m64py_Plugin*)new_plugin))
        {
            case OLD_PLUGIN_FAILED_TO_START_AGAIN : {
                /* In this case I think we have no other option than to set
                 * the current plugin to Py_None */
                Py_DECREF(new_plugin);
                Py_INCREF(Py_None);
                new_plugin = Py_None;
            } break;

            case OLD_PLUGIN_STARTED_SUCCESSFULLY : {
                Py_DECREF(new_plugin);
                return -1;
            }

            default : break;
        }
    }

    /* Replace current plugin with new plugin */
    PyObject* tmp = *old_maybe_plugin;
    *old_maybe_plugin = (PyObject*)new_plugin;
    Py_DECREF(tmp);
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
set_plugin_attribute(m64py_Emulator* self, PyObject* value, m64p_plugin_type plugin_type)
{
    PyObject** pmember;

    /* Can't do this if emulator is running */
    m64p_emu_state emu_state = M64EMU_RUNNING;
    self->corelib.CoreDoCommand(M64CMD_CORE_STATE_QUERY, M64CORE_EMU_STATE, &emu_state);
    if (emu_state != M64EMU_STOPPED)
    {
        PyErr_SetString(PyExc_RuntimeError, "Can't change plugins while emulator is running.");
        return -1;
    }

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
        return try_loading_and_replacing_plugin(self, value, plugin_type, pmember);
    }
    else if (value == Py_None)
    {
        PyObject* tmp = *pmember;
        stop_plugin(self, tmp);
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
PyDoc_STRVAR(AUDIO_PLUGIN_DOC,
"When setting this attribute you should specify a path to a mupen64plus \n"
"compatible audio plugin. To disable the audio plugin (a null-plugin will be\n"
"loaded in its place) assign None instead. Examples:\n\n"
"    emulator.audio_plugin = 'path/to/mupen64plus-audio-sdl.so'\n"
"    emulator.audio_plugin = None  # use a null-plugin instead\n\n"
"When reading this attribute it will instead return a m64py.Plugin object\n"
"representing the currently loaded plugin (or None if the plugin failed to load\n"
"or if no plugin was specified)");
static PyObject*
Emulator_getaudio_plugin(m64py_Emulator* self, void* closure)
{
    return Py_INCREF(self->audio_plugin), self->audio_plugin;
}
static int
Emulator_setaudio_plugin(m64py_Emulator* self, PyObject* value, void* closure)
{
    return set_plugin_attribute(self, value, M64PLUGIN_AUDIO);
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(INPUT_PLUGIN_DOC,
"When setting this attribute you should specify a path to a mupen64plus \n"
"compatible input plugin. To disable the input plugin (a null-plugin will be\n"
"loaded in its place) assign None instead. Examples:\n\n"
"    emulator.input_plugin = 'path/to/mupen64plus-input-sdl.so'\n"
"    emulator.input_plugin = None  # use a null-plugin instead\n\n"
"When reading this attribute it will instead return a m64py.Plugin object\n"
"representing the currently loaded plugin (or None if the plugin failed to load\n"
"or if no plugin was specified)");
static PyObject*
Emulator_getinput_plugin(m64py_Emulator* self, void* closure)
{
    return Py_INCREF(self->input_plugin), self->input_plugin;
}
static int
Emulator_setinput_plugin(m64py_Emulator* self, PyObject* value, void* closure)
{
    return set_plugin_attribute(self, value, M64PLUGIN_INPUT);
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(VIDEO_PLUGIN_DOC,
"When setting this attribute you should specify a path to a mupen64plus \n"
"compatible video plugin. To disable the video plugin (a null-plugin will be\n"
"loaded in its place) assign None instead. Examples:\n\n"
"    emulator.video_plugin = 'path/to/mupen64plus-video-glide64mk2.so'\n"
"    emulator.video_plugin = None  # use a null-plugin instead\n\n"
"When reading this attribute it will instead return a m64py.Plugin object\n"
"representing the currently loaded plugin (or None if the plugin failed to load\n"
"or if no plugin was specified)");
static PyObject*
Emulator_getvideo_plugin(m64py_Emulator* self, void* closure)
{
    return Py_INCREF(self->video_plugin), self->video_plugin;
}
static int
Emulator_setvideo_plugin(m64py_Emulator* self, PyObject* value, void* closure)
{
    return set_plugin_attribute(self, value, M64PLUGIN_GFX);
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(RSP_PLUGIN_DOC,
"When setting this attribute you should specify a path to a mupen64plus \n"
"compatible RSP plugin. To disable the RSP plugin (a null-plugin will be\n"
"loaded in its place) assign None instead. Examples:\n\n"
"    emulator.rsp_plugin = 'path/to/mupen64plus-rsp-cxd4-sse2.so'\n"
"    emulator.rsp_plugin = None  # use a null-plugin instead\n\n"
"When reading this attribute it will instead return a m64py.Plugin object\n"
"representing the currently loaded plugin (or None if the plugin failed to load\n"
"or if no plugin was specified)");
static PyObject*
Emulator_getrsp_plugin(m64py_Emulator* self, void* closure)
{
    return Py_INCREF(self->rsp_plugin), self->rsp_plugin;
}
static int
Emulator_setrsp_plugin(m64py_Emulator* self, PyObject* value, void* closure)
{
    return set_plugin_attribute(self, value, M64PLUGIN_RSP);
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(FRAME_CALLBACK_DOC,
"Specify a function or callable object to be called every frame while the\n"
"emulator is running. The function needs to take a single argument 'frame_idx'\n"
"which is an integer that is incremented by 1 every frame.");
static PyObject*
Emulator_getframe_callback(m64py_Emulator* self, void* closure)
{
    return Py_INCREF(self->frame_callback), self->frame_callback;
}
static int
Emulator_setframe_callback(m64py_Emulator* self, PyObject* callable)
{
    PyObject* tmp;

    if (callable != Py_None && !PyCallable_Check(callable))
    {
        PyErr_SetString(PyExc_TypeError, "Callback needs to be a callable object (it isn't)");
        return -1;
    }

    tmp = self->frame_callback;
    Py_INCREF(callable);
    self->frame_callback = callable;
    Py_DECREF(tmp);

    return 0;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(LOG_MESSAGE_CALLBACK_DOC,
"Specify a function or callable object to listen in on the emulator's log\n"
"messages. The function needs to take two arguments, 'level' and 'message',\n"
"where 'level' is an integer value specifying the severity of the message and \n"
"'message' is a string containing the log message.");
static PyObject*
Emulator_getlog_message_callback(m64py_Emulator* self, void* closure)
{
    if (self->log_message_callback)
        return Py_INCREF(self->log_message_callback), self->log_message_callback;

    Py_RETURN_NONE;
}
static int
Emulator_setlog_message_callback(m64py_Emulator* self, PyObject* callable)
{
    PyObject* tmp;

    if (callable != Py_None &&!PyCallable_Check(callable))
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
PyDoc_STRVAR(VI_CALLBACK_DOC,
"");
static PyObject*
Emulator_getvi_callback(m64py_Emulator* self, void* closure)
{
    return Py_INCREF(self->vi_callback), self->vi_callback;
}
static int
Emulator_setvi_callback(m64py_Emulator* self, PyObject* callable)
{
    PyObject* tmp;

    if (callable != Py_None && !PyCallable_Check(callable))
    {
        PyErr_SetString(PyExc_TypeError, "Callback needs to be a callable object (it isn't)");
        return -1;
    }

    tmp = self->vi_callback;
    Py_INCREF(callable);
    self->vi_callback = callable;
    Py_DECREF(tmp);

    return 0;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(SPEED_LIMITER_DOC,
"Set to True to run the emulator at 60 fps. Set to false to run as fast as possible.");
static PyObject*
Emulator_getspeed_limiter(m64py_Emulator* self, void* closure)
{
    int trueness;
    self->corelib.CoreDoCommand(M64CMD_CORE_STATE_QUERY, M64CORE_SPEED_LIMITER, &trueness);
    if (trueness)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}
static int
Emulator_setspeed_limiter(m64py_Emulator* self, PyObject* value, void* closure)
{
    int trueness = PyObject_IsTrue(value);
    if (trueness == -1)
        return -1;

    self->corelib.CoreDoCommand(M64CMD_CORE_STATE_SET, M64CORE_SPEED_LIMITER, &trueness);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Emulator_getsetters[] = {
    {"input_plugin",         (getter)Emulator_getinput_plugin,         (setter)Emulator_setinput_plugin,         INPUT_PLUGIN_DOC, NULL},
    {"audio_plugin",         (getter)Emulator_getaudio_plugin,         (setter)Emulator_setaudio_plugin,         AUDIO_PLUGIN_DOC, NULL},
    {"video_plugin",         (getter)Emulator_getvideo_plugin,         (setter)Emulator_setvideo_plugin,         VIDEO_PLUGIN_DOC, NULL},
    {"rsp_plugin",           (getter)Emulator_getrsp_plugin,           (setter)Emulator_setrsp_plugin,           RSP_PLUGIN_DOC, NULL},
    {"frame_callback",       (getter)Emulator_getframe_callback,       (setter)Emulator_setframe_callback,       FRAME_CALLBACK_DOC, NULL},
    {"log_message_callback", (getter)Emulator_getlog_message_callback, (setter)Emulator_setlog_message_callback, LOG_MESSAGE_CALLBACK_DOC, NULL},
    {"vi_callback",          (getter)Emulator_getvi_callback,          (setter)Emulator_setvi_callback,          VI_CALLBACK_DOC, NULL},
    {"speed_limiter",        (getter)Emulator_getspeed_limiter,        (setter)Emulator_setspeed_limiter,        SPEED_LIMITER_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(EMULATOR_DOC,
"");
PyTypeObject m64py_EmulatorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "m64py.Emulator",
    .tp_basicsize = sizeof(m64py_Emulator),
    .tp_dealloc = (destructor)Emulator_dealloc, /* tp_dealloc */
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = EMULATOR_DOC,
    .tp_methods = Emulator_methods,
    .tp_getset = Emulator_getsetters,
    .tp_new = Emulator_new
};

/* ------------------------------------------------------------------------- */
int
m64py_EmulatorType_init(void)
{
    if (PyType_Ready((PyTypeObject*)&m64py_EmulatorType) < 0)
        return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
start_plugin(m64py_Emulator* emu, PyObject* maybePlugin)
{
    m64p_error result;
    m64py_Plugin* plugin;

    if (!PyObject_IsInstance(maybePlugin, (PyObject*)&m64py_PluginType))
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
    if (!PyObject_IsInstance(maybePlugin, (PyObject*)&m64py_PluginType))
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
