#ifndef M64PY_MODULE_H
#define M64PY_MODULE_H

#include "Python.h"

extern PyModuleDef m64py_module;

/*!
 * @brief Prepends the path to the m64py module with the specified filename.
 */
PyObject*
m64py_prepend_module_path_to_filename(const char* filename);

#endif /* M64PY_MODULE_H */
