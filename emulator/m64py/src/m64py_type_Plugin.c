#include "m64py_type_Plugin.h"
#include "m64py_type_Emulator.h"
#include "m64p_common.h"
#include "osal_dynamiclib.h"
#include <structmember.h>

/* ------------------------------------------------------------------------- */
static void
Plugin_dealloc(m64py_Plugin* self)
{
    if (self->handle)
        osal_dynlib_close(self->handle);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static char* kwds_names[] = {
    "path_to_plugin",
    "type",
    "corelib_handle",
    NULL
};
static PyObject*
Plugin_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    const char* path_to_plugin;
    PyObject* dont_care;
    m64p_plugin_type PluginType = (m64p_plugin_type)0;
    int PluginVersion = 0;

    m64py_Plugin* self = (m64py_Plugin*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "siO", kwds_names, &path_to_plugin, &self->type, &dont_care))
        goto fail;

    /* Try to open shared library */
    if (osal_dynlib_open(&self->handle, path_to_plugin) != M64ERR_SUCCESS)
    {
        PyErr_Format(PyExc_RuntimeError, "Failed to open shared library \"%s\"", path_to_plugin);
        goto fail;
    }

    /* Load function pointers */
#define X(name) \
        if ((self->name = osal_dynlib_getproc(self->handle, #name)) == NULL) \
        { \
            PyErr_Format(PyExc_RuntimeError, "Failed to load function \"%s\" from plugin \"%s\"", #name, path_to_plugin); \
            goto fail; \
        }
    M64PY_PLUGIN_FUNCTIONS
#undef X

    self->PluginGetVersion(&PluginType, &PluginVersion, NULL, &self->name, NULL);
    if (PluginType != self->type)
    {
        PyErr_Format(PyExc_RuntimeError, "Shared library \"%s\" is of type %d, but we expected %d instead.", path_to_plugin, PluginType, self->type);
        goto fail;
    }

    /* the front-end doesn't talk to the plugins, so we don't care about the plugin version or api version */

    return (PyObject*)self;

    fail              : Py_DECREF(self);
    alloc_self_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(PLUGIN_DOC,
"Base class for plugins that the core library loads.");
PyTypeObject m64py_PluginType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "m64py.Plugin",               /* tp_name */
    sizeof(m64py_Plugin),         /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)Plugin_dealloc,   /* tp_dealloc */
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
    PLUGIN_DOC,                   /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    0,                            /* tp_methods */
    0,                            /* tp_members */
    0,                            /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    0,                            /* tp_init */
    0,                            /* tp_alloc */
    Plugin_new,                   /* tp_new */
};

/* ------------------------------------------------------------------------- */
int
m64py_PluginType_init(void)
{
    if (PyType_Ready(&m64py_PluginType) < 0)
        return -1;

    return 0;
}
