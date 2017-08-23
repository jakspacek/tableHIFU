/*
README: Since all methods used with the Python C API must be static, we must use a SCU (Single Compilation Unit) design. This
means that header files are not used, and all code must be included here once. If you attempt to store code (declarations and
source) in headers (along with include guards), you will run into many circular dependencies, so this approach is not recommended.
*/
//
// include non-project files
#include <Python.h>
//
// include project headers
#include "controller_python.h"
//
static bool pythonLogging = false;
//
// include project source code
#include "controller_python.cpp"
//
static PyObject *motorcontrol_enableLogging(PyObject *self, PyObject *args);
//
static PyObject *motorcontrol_enableLogging(PyObject *self, PyObject *args){
	PyObject *py_object = NULL;
	//
	if(!PyArg_ParseTuple(args, "O!", &PyBool_Type, &py_object)){
		return NULL;
	}
	//
	if(PyObject_IsTrue(py_object)){
		pythonLogging = true;
	}else{
		pythonLogging = false;
	}
	//
	Py_RETURN_NONE;
}
//
//-----------------------------------------------------------------
// Setup Python-C API
//-----------------------------------------------------------------
//
static PyMethodDef _C_Methods[] = {
	{"enable_logging", motorcontrol_enableLogging, METH_VARARGS, "Enable sys.stdout logging? True or False."},
	{NULL, NULL} /* Sentinel - marks the end of this structure */
};
//
PyMODINIT_FUNC initmotorcontrol(){
	if(PyType_Ready(&TableMotorControllerType) < 0){
		return;
	}
	//
	PyObject *module = Py_InitModule("motorcontrol", _C_Methods);
	if(module == NULL){
		return;
	}
	//
	Py_INCREF(&TableMotorControllerType);
	PyModule_AddObject(module, "TableMotorController", (PyObject *)&TableMotorControllerType);
}
