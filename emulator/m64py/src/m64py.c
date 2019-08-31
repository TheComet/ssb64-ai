#include "Python.h"

#define M64P_CORE_PROTOTYPES
#include "m64p_frontend.h"

#include <stdint.h>
#include <stdio.h>

#define CORE_API_VERSION 0x020001

/* ------------------------------------------------------------------------- */
static PyObject*
set_plugins(PyObject* m, PyObject* args, PyObject* kwds)
{
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
load_ssb64_rom(PyObject* m, PyObject* args)
{
    fprintf(stderr, "fuck you\n");
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
advance_frame(PyObject* m, PyObject* args)
{
    fprintf(stderr, "lolnope\n");
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static void
module_free(void* x)
{
    CoreShutdown();
}

static PyMethodDef m64py_methods[] = {
    {"set_plugins", (PyCFunction)set_plugins, METH_VARARGS | METH_KEYWORDS, ""},
    {"load_ssb64_rom", load_ssb64_rom, METH_VARARGS, ""},
    {"advance_frame", advance_frame, METH_NOARGS, ""},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyModuleDef m64py_module = {
    PyModuleDef_HEAD_INIT,
    "m64py",        /* module name */
    NULL,           /* Docstring, may be NULL */
    -1,             /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables */
    m64py_methods,  /* module methods */
    NULL,           /* m_reload */
    NULL,           /* m_traverse */
    NULL,           /* m_clear */
    module_free     /* m_free */
};

/* ------------------------------------------------------------------------- */
PyMODINIT_FUNC PyInit_m64py(void)
{
    PyObject* m;

    if (CoreStartup(CORE_API_VERSION, "", "", NULL, NULL, NULL, NULL) != M64ERR_SUCCESS)
        goto libmupen_init_failed;

    m = PyModule_Create(&m64py_module);
    if (m == NULL)
        goto module_alloc_failed;

    return m;

    module_alloc_failed  : CoreShutdown();
    libmupen_init_failed : return NULL;
}
