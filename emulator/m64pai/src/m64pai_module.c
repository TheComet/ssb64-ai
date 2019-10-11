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
m64pai_prepend_module_path_to_filename(const char* filename)
{
    PyObject *m, *module_file, *ospath, *split_tuple, *path_to_module, *py_filename, *result = NULL;

#define JMP_IF_NULL(o, msg) \
    if (o == NULL) goto o##_failed

    m = PyState_FindModule(&m64pai_module);  /* borrowed ref */
    JMP_IF_NULL(m, "Failed to get m64pai module");

    module_file = PyModule_GetFilenameObject(m);  /* new ref */
    JMP_IF_NULL(module_file, "Failed to get filename object from module");

    ospath = PyImport_ImportModule("os.path");  /* new ref */
    JMP_IF_NULL(ospath, "Failed to get filename object from module");

    split_tuple = PyObject_CallMethod(ospath, "split", "(O)", module_file); /* new ref */
    JMP_IF_NULL(split_tuple, "Failed to get filename object from module");

    path_to_module = PyTuple_GET_ITEM(split_tuple, 0); /* borrowed ref */

    py_filename = PyUnicode_FromString(filename); /* new ref */
    JMP_IF_NULL(py_filename, "Failed to get filename object from module");

    result = PyObject_CallMethod(ospath, "join", "(OO)", path_to_module, py_filename);

    result_failed         : Py_DECREF(py_filename);
    py_filename_failed    : Py_DECREF(split_tuple);
    split_tuple_failed    : Py_DECREF(ospath);
    ospath_failed         : Py_DECREF(module_file);
    module_file_failed    :
    m_failed              : return result;
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
PyDoc_STRVAR(M64PAI_MODULE_DOC,
"Provides an API for communicating with N64 games, specifically for AI training\n"
"purposes.");
PyModuleDef m64pai_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "m64pai",
    .m_doc = M64PAI_MODULE_DOC,
    .m_size = -1,  /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables */
    .m_methods = NULL,
    .m_slots = NULL,
    .m_traverse = NULL,
    .m_clear = NULL,
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
