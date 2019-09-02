#include "Python.h"
#include "m64py_type_Emulator.h"
#include "m64py_type_SSB64.h"
#include "m64py_type_Plugin.h"

/* ------------------------------------------------------------------------- */
static void
module_free(void* m)
{
}

/* ------------------------------------------------------------------------- */
static int
init_builtin_types(void)
{
    if (m64py_EmulatorType_init() != 0) return -1;
    if (m64py_SSB64Type_init() != 0)    return -1;
    if (m64py_PluginType_init() != 0)   return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
add_builtin_types_to_module(PyObject* m)
{
    Py_INCREF(&m64py_EmulatorType); if (PyModule_AddObject(m, "Emulator", (PyObject*)&m64py_EmulatorType) != 0) return -1;
    Py_INCREF(&m64py_SSB64Type);    if (PyModule_AddObject(m, "SSB64", (PyObject*)&m64py_SSB64Type) != 0) return -1;
    Py_INCREF(&m64py_PluginType);   if (PyModule_AddObject(m, "Plugin", (PyObject*)&m64py_PluginType) != 0) return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
add_constants_to_module(PyObject* m)
{
    /* Fighters */
    if (PyModule_AddIntConstant(m, "PIKACHU", 0) != 0) return -1;

    /* Stages */
    if (PyModule_AddIntConstant(m, "DREAMLAND", 0) != 0) return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyModuleDef m64py_module = {
    PyModuleDef_HEAD_INIT,
    "m64py",        /* module name */
    NULL,           /* Docstring, may be NULL */
    -1,             /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables */
    NULL,           /* module methods */
    NULL,           /* m_reload */
    NULL,           /* m_traverse */
    NULL,           /* m_clear */
    module_free     /* m_free */
};

/* ------------------------------------------------------------------------- */
PyMODINIT_FUNC PyInit_m64py(void)
{
    PyObject* m;

    m = PyModule_Create(&m64py_module);
    if (m == NULL)
        goto module_alloc_failed;

    if (init_builtin_types() != 0)           goto init_module_failed;
    if (add_builtin_types_to_module(m) != 0) goto init_module_failed;
    if (add_constants_to_module(m) != 0)     goto init_module_failed;

    return m;

    init_module_failed  : Py_DECREF(m);
    module_alloc_failed : return NULL;
}
