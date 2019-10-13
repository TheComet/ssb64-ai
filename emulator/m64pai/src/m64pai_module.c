#include "m64pai_types_list.h"
#include "m64pai_module.h"
#include "m64pai_type_Controller.h"
#include "m64pai_type_Emulator.h"
#include "m64pai_type_Fighter.h"
#include "m64pai_type_Fighter_CaptainFalcon.h"
#include "m64pai_type_Fighter_DonkeyKong.h"
#include "m64pai_type_Fighter_Fox.h"
#include "m64pai_type_Fighter_Jigglypuff.h"
#include "m64pai_type_Fighter_Kirby.h"
#include "m64pai_type_Fighter_Link.h"
#include "m64pai_type_Fighter_Luigi.h"
#include "m64pai_type_Fighter_Mario.h"
#include "m64pai_type_Fighter_Ness.h"
#include "m64pai_type_Fighter_Pikachu.h"
#include "m64pai_type_Fighter_Samus.h"
#include "m64pai_type_Fighter_Yoshi.h"
#include "m64pai_type_Plugin.h"
#include "m64pai_type_Plugin_CuckedInputPlugin.h"
#include "m64pai_type_SSB64.h"
#include "m64pai_type_Stage.h"
#include "m64pai_type_Stage_DreamLand.h"
#include "m64pai_type_Stage_HyruleCastle.h"
#include "m64pai_type_Stage_KongoJungle.h"
#include "m64pai_type_Stage_MushroomKingdom.h"
#include "m64pai_type_Stage_PeachsCastle.h"
#include "m64pai_type_Stage_PlanetZebes.h"
#include "m64pai_type_Stage_SaffronCity.h"
#include "m64pai_type_Stage_SectorZ.h"
#include "m64pai_type_Stage_YoshisIsland.h"

/* ------------------------------------------------------------------------- */
PyObject*
m64pai_get_path_to_module(void)
{
    PyObject *m, *mfile, *ospathmod, *split_result;
    PyObject* result = NULL;

#define FAIL_IF_NULL(o, msg) \
    if (o == NULL) { \
        PyErr_SetString(PyExc_RuntimeError, msg); \
        goto o##_failed; \
    }

    m = PyState_FindModule(&m64pai_module);  /* borrowed ref */
    FAIL_IF_NULL(m, "Failed to get m64pai module");

    mfile = PyModule_GetFilenameObject(m);  /* new ref */
    FAIL_IF_NULL(mfile, "Failed to get filename object from module");

    ospathmod = PyImport_ImportModule("os.path"); /* new ref */
    if (ospathmod == NULL) goto ospathmod_failed;

    split_result = PyObject_CallMethod(ospathmod, "split", "(O)", mfile); /* new ref */
    if (split_result == NULL) goto split_failed;

    result = PyTuple_GET_ITEM(split_result, 0);
    Py_INCREF(result);

                       Py_DECREF(split_result);
    split_failed     : Py_DECREF(ospathmod);
    ospathmod_failed : Py_DECREF(mfile);
    mfile_failed     :
    m_failed         : return result;
#undef FAIL_IF_NULL
}

/* ------------------------------------------------------------------------- */
PyObject*
m64pai_prepend_module_path_to_filename(const char* filename)
{
    PyObject *ospathmod, *mpath, *py_filename;
    PyObject* result = NULL;

    ospathmod = PyImport_ImportModule("os.path");  /* new ref */
    if (ospathmod == NULL) goto ospathmod_failed;

    mpath = m64pai_get_path_to_module(); /* new ref */
    if (mpath == NULL) goto mpath_failed;

    py_filename = PyUnicode_FromString(filename); /* new ref */
    if (py_filename == NULL) goto py_filename_failed;

    result = PyObject_CallMethod(ospathmod, "join", "(OO)", mpath, py_filename);

                         Py_DECREF(py_filename);
    py_filename_failed : Py_DECREF(mpath);
    mpath_failed       : Py_DECREF(ospathmod);
    ospathmod_failed   : return result;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(GET_DATA_PATH_DOC, "get_data_path()\n--\n\n"
"Returns an absolute path to the 'data' folder of the m64pai module. This\n"
"folder contains config files and savestates, among other things.\n"
"\n"
"The Emulator's default config_path and data_path are initialized to\n"
"  os.path.join(m64pai.get_data_path(), 'config')\n");
static PyObject*
m64pai_get_data_path(PyObject* m, PyObject* args)
{
    return m64pai_prepend_module_path_to_filename("share/m64pai/data");
}

/* ------------------------------------------------------------------------- */
static void
module_free(void* m)
{
}

/* ------------------------------------------------------------------------- */
static int
init_builtin_types(void)
{
#define X(type) \
    if (m64pai_##type##Type_init() != 0) \
        return -1;
    M64PAI_TYPES_LIST
#undef X
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
add_builtin_types_to_module(PyObject* m)
{
#define X(type) \
    Py_INCREF(&m64pai_##type##Type); \
    if (PyModule_AddObject(m, #type, (PyObject*)&m64pai_##type##Type) != 0) \
        return -1;
    M64PAI_TYPES_LIST
#undef X

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef m64pai_methods[] = {
    {"get_data_path", m64pai_get_data_path, METH_NOARGS, GET_DATA_PATH_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(M64PAI_MODULE_DOC,
"Provides an API for communicating with N64 games, specifically for AI training\n"
"purposes.");
PyModuleDef m64pai_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "m64pai",
    .m_doc = M64PAI_MODULE_DOC,
    .m_size = -1,  /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables */
    .m_methods = m64pai_methods,
    .m_free = module_free
};

/* ------------------------------------------------------------------------- */
PyMODINIT_FUNC PyInit_m64pai(void)
{
    PyObject* m;

    m = PyModule_Create(&m64pai_module);
    if (m == NULL)
        goto module_alloc_failed;

    if (init_builtin_types() != 0)           goto init_module_failed;
    if (add_builtin_types_to_module(m) != 0) goto init_module_failed;

    return m;

    init_module_failed  : Py_DECREF(m);
    module_alloc_failed : return NULL;
}
