#ifndef M64PAI_MODULE_H
#define M64PAI_MODULE_H

#include "Python.h"

extern PyModuleDef m64pai_module;

/*!
 * @brief Prepends the path to the m64pai module with the specified filename.
 */
PyObject*
m64pai_prepend_module_path_to_filename(const char* filename);

#endif /* M64PAI_MODULE_H */
