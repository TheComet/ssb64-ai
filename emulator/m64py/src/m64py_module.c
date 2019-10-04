#include "m64py_module.h"
#include "m64py_type_Controller.h"
#include "m64py_type_Emulator.h"
#include "m64py_type_Fighter.h"
#include "m64py_type_Fighter_CaptainFalcon.h"
#include "m64py_type_Fighter_DonkeyKong.h"
#include "m64py_type_Fighter_Fox.h"
#include "m64py_type_Fighter_Jigglypuff.h"
#include "m64py_type_Fighter_Kirby.h"
#include "m64py_type_Fighter_Link.h"
#include "m64py_type_Fighter_Luigi.h"
#include "m64py_type_Fighter_Mario.h"
#include "m64py_type_Fighter_Ness.h"
#include "m64py_type_Fighter_Pikachu.h"
#include "m64py_type_Fighter_Samus.h"
#include "m64py_type_Fighter_Yoshi.h"
#include "m64py_type_Plugin.h"
#include "m64py_type_Plugin_CuckedInputPlugin.h"
#include "m64py_type_SSB64.h"
#include "m64py_type_Stage.h"
#include "m64py_type_Stage_DreamLand.h"
#include "m64py_type_Stage_HyruleCastle.h"
#include "m64py_type_Stage_KongoJungle.h"
#include "m64py_type_Stage_MushroomKingdom.h"
#include "m64py_type_Stage_PeachsCastle.h"
#include "m64py_type_Stage_PlanetZebes.h"
#include "m64py_type_Stage_SaffronCity.h"
#include "m64py_type_Stage_SectorZ.h"
#include "m64py_type_Stage_YoshisIsland.h"

/* ------------------------------------------------------------------------- */
PyObject*
m64py_prepend_module_path_to_filename(const char* filename)
{
    PyObject *m, *module_file, *ospath, *split_tuple, *path_to_module, *py_filename, *result = NULL;

#define JMP_IF_NULL(o, msg) \
    if (o == NULL) goto o##_failed

    m = PyState_FindModule(&m64py_module);  /* borrowed ref */
    JMP_IF_NULL(m, "Failed to get m64py module");

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
    if (m64py_ControllerType_init() != 0)        return -1;
    if (m64py_EmulatorType_init() != 0)          return -1;
    if (m64py_FighterType_init() != 0)           return -1;
    if (m64py_CaptainFalconType_init() != 0)     return -1;
    if (m64py_DonkeyKongType_init() != 0)        return -1;
    if (m64py_FoxType_init() != 0)               return -1;
    if (m64py_JigglypuffType_init() != 0)        return -1;
    if (m64py_KirbyType_init() != 0)             return -1;
    if (m64py_LinkType_init() != 0)              return -1;
    if (m64py_LuigiType_init() != 0)             return -1;
    if (m64py_MarioType_init() != 0)             return -1;
    if (m64py_NessType_init() != 0)              return -1;
    if (m64py_PikachuType_init() != 0)           return -1;
    if (m64py_SamusType_init() != 0)             return -1;
    if (m64py_YoshiType_init() != 0)             return -1;
    if (m64py_PluginType_init() != 0)            return -1;
    if (m64py_CuckedInputPluginType_init() != 0) return -1;
    if (m64py_SSB64Type_init() != 0)             return -1;
    if (m64py_StageType_init() != 0)             return -1;
    if (m64py_DreamLandType_init() != 0)         return -1;
    if (m64py_HyruleCastleType_init() != 0)      return -1;
    if (m64py_KongoJungleType_init() != 0)       return -1;
    if (m64py_MushroomKingdomType_init() != 0)   return -1;
    if (m64py_PeachsCastleType_init() != 0)      return -1;
    if (m64py_PlanetZebesType_init() != 0)       return -1;
    if (m64py_SaffronCityType_init() != 0)       return -1;
    if (m64py_SectorZType_init() != 0)           return -1;
    if (m64py_YoshisIslandType_init() != 0)      return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
static int
add_builtin_types_to_module(PyObject* m)
{
    Py_INCREF(&m64py_ControllerType);        if (PyModule_AddObject(m, "Controller",        (PyObject*)&m64py_ControllerType) != 0)        return -1;
    Py_INCREF(&m64py_EmulatorType);          if (PyModule_AddObject(m, "Emulator",          (PyObject*)&m64py_EmulatorType) != 0)          return -1;
    Py_INCREF(&m64py_FighterType);           if (PyModule_AddObject(m, "Fighter",           (PyObject*)&m64py_FighterType) != 0)           return -1;
    Py_INCREF(&m64py_CaptainFalconType);     if (PyModule_AddObject(m, "CaptainFalcon",     (PyObject*)&m64py_CaptainFalconType) != 0)     return -1;
    Py_INCREF(&m64py_DonkeyKongType);        if (PyModule_AddObject(m, "DonkeyKong",        (PyObject*)&m64py_DonkeyKongType) != 0)        return -1;
    Py_INCREF(&m64py_FoxType);               if (PyModule_AddObject(m, "Fox",               (PyObject*)&m64py_FoxType) != 0)               return -1;
    Py_INCREF(&m64py_JigglypuffType);        if (PyModule_AddObject(m, "Jigglypuff",        (PyObject*)&m64py_JigglypuffType) != 0)        return -1;
    Py_INCREF(&m64py_KirbyType);             if (PyModule_AddObject(m, "Kirby",             (PyObject*)&m64py_KirbyType) != 0)             return -1;
    Py_INCREF(&m64py_LinkType);              if (PyModule_AddObject(m, "Link",              (PyObject*)&m64py_LinkType) != 0)              return -1;
    Py_INCREF(&m64py_LuigiType);             if (PyModule_AddObject(m, "Luigi",             (PyObject*)&m64py_LuigiType) != 0)             return -1;
    Py_INCREF(&m64py_MarioType);             if (PyModule_AddObject(m, "Mario",             (PyObject*)&m64py_MarioType) != 0)             return -1;
    Py_INCREF(&m64py_NessType);              if (PyModule_AddObject(m, "Ness",              (PyObject*)&m64py_NessType) != 0)              return -1;
    Py_INCREF(&m64py_PikachuType);           if (PyModule_AddObject(m, "Pikachu",           (PyObject*)&m64py_PikachuType) != 0)           return -1;
    Py_INCREF(&m64py_SamusType);             if (PyModule_AddObject(m, "Samus",             (PyObject*)&m64py_SamusType) != 0)             return -1;
    Py_INCREF(&m64py_YoshiType);             if (PyModule_AddObject(m, "Yoshi",             (PyObject*)&m64py_YoshiType) != 0)             return -1;
    Py_INCREF(&m64py_PluginType);            if (PyModule_AddObject(m, "Plugin",            (PyObject*)&m64py_PluginType) != 0)            return -1;
    Py_INCREF(&m64py_CuckedInputPluginType); if (PyModule_AddObject(m, "CuckedInputPlugin", (PyObject*)&m64py_CuckedInputPluginType) != 0) return -1;
    Py_INCREF(&m64py_SSB64Type);             if (PyModule_AddObject(m, "SSB64",             (PyObject*)&m64py_SSB64Type) != 0)             return -1;
    Py_INCREF(&m64py_StageType);             if (PyModule_AddObject(m, "Stage",             (PyObject*)&m64py_StageType) != 0)             return -1;
    Py_INCREF(&m64py_DreamLandType);         if (PyModule_AddObject(m, "DreamLand",         (PyObject*)&m64py_DreamLandType) != 0)         return -1;
    Py_INCREF(&m64py_HyruleCastleType);      if (PyModule_AddObject(m, "HyruleCastle",      (PyObject*)&m64py_HyruleCastleType) != 0)      return -1;
    Py_INCREF(&m64py_KongoJungleType);       if (PyModule_AddObject(m, "KongoJungle",       (PyObject*)&m64py_KongoJungleType) != 0)       return -1;
    Py_INCREF(&m64py_MushroomKingdomType);   if (PyModule_AddObject(m, "MushroomKingdom",   (PyObject*)&m64py_MushroomKingdomType) != 0)   return -1;
    Py_INCREF(&m64py_PeachsCastleType);      if (PyModule_AddObject(m, "PeachsCastle",      (PyObject*)&m64py_PeachsCastleType) != 0)      return -1;
    Py_INCREF(&m64py_PlanetZebesType);       if (PyModule_AddObject(m, "PlanetZebes",       (PyObject*)&m64py_PlanetZebesType) != 0)       return -1;
    Py_INCREF(&m64py_SaffronCityType);       if (PyModule_AddObject(m, "SaffronCity",       (PyObject*)&m64py_SaffronCityType) != 0)       return -1;
    Py_INCREF(&m64py_SectorZType);           if (PyModule_AddObject(m, "SectorZ",           (PyObject*)&m64py_SectorZType) != 0)           return -1;
    Py_INCREF(&m64py_YoshisIslandType);      if (PyModule_AddObject(m, "YoshisIsland",      (PyObject*)&m64py_YoshisIslandType) != 0)      return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
PyModuleDef m64py_module = {
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

    return m;

    init_module_failed  : Py_DECREF(m);
    module_alloc_failed : return NULL;
}
