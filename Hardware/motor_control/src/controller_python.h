#ifndef __MOTORCONTROL_CONTROLLER__
#define __MOTORCONTROL_CONTROLLER__
//
#include <Python.h>
#include <structmember.h>
//
//time in milliseconds upon which the threaded version will run before
//it decides to timeout
#define MOTOR_WAIT_TIME 30000 // TODO make parameter of move functions
//
typedef struct{
	PyObject_HEAD
	PyObject *controller_pointer;
} TableMotorController;
//
static void *getPointer(TableMotorController *py_object);
//
static void TableMotorController_dealloc(TableMotorController* self);
static PyObject *TableMotorController_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int TableMotorController_init(TableMotorController *self, PyObject *args, PyObject *kwds);
static PyObject *TableMotorController_str(TableMotorController *self);
static PyObject *TableMotorController_repr(TableMotorController *self);
//
static PyObject *TableMotorController_init_axes(TableMotorController *self, PyObject *args);
static PyObject *TableMotorController_home_system(TableMotorController *self, PyObject *args);
static PyObject *TableMotorController_move_to_target(TableMotorController *self, PyObject *args, PyObject *kwargs);
static PyObject *TableMotorController_orientation(TableMotorController *self, PyObject *args, PyObject *kwargs);
static PyObject *TableMotorController_read_current_position(TableMotorController *self, PyObject *args, PyObject *kwargs);
static PyObject *TableMotorController_set_initial_focus(TableMotorController *self, PyObject *args, PyObject *kwargs);
static PyObject *TableMotorController_set_focus(TableMotorController *self, PyObject *args, PyObject *kwargs);
static PyObject *TableMotorController_stop_motors(TableMotorController *self, PyObject *args, PyObject *kwargs);
static PyObject *TableMotorController_get_ranges(TableMotorController *self, PyObject *args, PyObject *kwargs);
static PyObject *TableMotorController_set_periodic_breakpoint(TableMotorController *self, PyObject *args, PyObject *kwargs);
static PyObject *TableMotorController_set_periodic_breakpoint_enabled(TableMotorController *self, PyObject *args, PyObject *kwargs);
//
static PyMemberDef TableMotorController_members[] = {
	{"_pointer", T_OBJECT_EX, offsetof(TableMotorController, controller_pointer), 0,
	 "Pointer to the CNIMotionMotors instance"},
	{NULL}  /* Sentinel */
};
//
static PyMethodDef TableMotorController_methods[] = {
	{"init_axes", (PyCFunction)TableMotorController_init_axes, METH_NOARGS, "Init axes."},
	{"home_system", (PyCFunction)TableMotorController_home_system, METH_NOARGS, "Home system."},
	{"move_to_target", (PyCFunction)TableMotorController_move_to_target, METH_KEYWORDS, "Move to target (R, S) position. If multithreading is enabled, the optional arg waittime (ms) is available."},
	{"orientation", (PyCFunction)TableMotorController_orientation, METH_KEYWORDS, ""},
    {"read_current_position", (PyCFunction)TableMotorController_read_current_position, METH_NOARGS, ""},
    {"set_initial_focus", (PyCFunction)TableMotorController_set_initial_focus, METH_KEYWORDS, ""},
    {"set_focus", (PyCFunction)TableMotorController_set_focus, METH_KEYWORDS, ""},
    {"stop_motors", (PyCFunction)TableMotorController_stop_motors, METH_NOARGS, ""},
    {"get_ranges", (PyCFunction)TableMotorController_get_ranges, METH_NOARGS, ""},
    {"set_periodic_breakpoint", (PyCFunction)TableMotorController_set_periodic_breakpoint, METH_KEYWORDS, ""},
    {"set_periodic_breakpoint_enabled", (PyCFunction)TableMotorController_set_periodic_breakpoint_enabled, METH_KEYWORDS, ""},
	{NULL}  /* Sentinel */
};
//
static PyTypeObject TableMotorControllerType = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /*ob_size*/
    "motorcontrol.TableMotorController",         /*tp_name*/
    sizeof(TableMotorController),                /*tp_basicsize*/
    0,                                 /*tp_itemsize*/
    (destructor)TableMotorController_dealloc,    /*tp_dealloc*/
    0,                                 /*tp_print*/
    0,                                 /*tp_getattr*/
    0,                                 /*tp_setattr*/
    0,                                 /*tp_compare*/
    (reprfunc)TableMotorController_repr,         /*tp_repr*/
    0,                                 /*tp_as_number*/
    0,                                 /*tp_as_sequence*/
    0,                                 /*tp_as_mapping*/
    0,                                 /*tp_hash */
    0,                                 /*tp_call*/
    (reprfunc)TableMotorController_str,          /*tp_str*/
    0,                                 /*tp_getattro*/
    0,                                 /*tp_setattro*/
    0,                                 /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "TableMotorController objects contain a pointer to a CNIMotionMotors instance.",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    TableMotorController_methods,    /* tp_methods */
    TableMotorController_members,    /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)TableMotorController_init, /* tp_init */
    0,                         /* tp_alloc */
    TableMotorController_new,            /* tp_new */
};
//
#endif
