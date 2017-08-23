#include "NIMotionMotors.h"
#include "controller_c.h"
#include "controller_c.cpp"
//
static void *getPointer(TableMotorController *py_object){
	return PyCapsule_GetPointer(py_object->controller_pointer, NULL);
}
//
//////////////////////////////////////
//
static void
TableMotorController_dealloc(TableMotorController* self){
	delete getPointer(self);
	Py_XDECREF(self->controller_pointer);
	if(pythonLogging){
		printf("TableMotorController: Deleted pointer.\n");
	}
	self->ob_type->tp_free((PyObject*)self);
}
//
static PyObject *
TableMotorController_new(PyTypeObject *type, PyObject *args, PyObject *kwds){
	TableMotorController *self = (TableMotorController *)type->tp_alloc(type, 0);
	//
	if(self == NULL){
		PyErr_SetString(PyExc_Exception, "Could not initialize.");
		return NULL;
	}
	//
	void *pointer = new CNIMotionMotors();
	if(pointer == NULL){
		Py_DECREF(self);
		PyErr_SetString(PyExc_Exception, "Could not initialize the CNIMotionMotors.");
		return NULL;
	}
	//
	self->controller_pointer = PyCapsule_New(pointer, NULL, NULL);
	if(self->controller_pointer == NULL){
		Py_DECREF(self);
		PyErr_SetString(PyExc_Exception, "Could not initialize the PyCapsule pointer.");
		return NULL;
	}
	//
	if(pythonLogging){
		printf("TableMotorController: Initialized.\n");
	}
	return (PyObject *)self;
}
//
static int
TableMotorController_init(TableMotorController *self, PyObject *args, PyObject *kwds){
	return 0;
}
//
static PyObject *
TableMotorController_str(TableMotorController *self){
	PyObject* str = PyString_FromFormat("motorcontrol.TableMotorController (pointer: %p)", getPointer(self));
	return str;
}
//
static PyObject *
TableMotorController_repr(TableMotorController *self){
	return TableMotorController_str(self);
}
//
//////////////////////////////////////
//
static PyObject *TableMotorController_init_axes(TableMotorController *self, PyObject *args){
	CNIMotionMotors* motorPtr = (CNIMotionMotors*)getPointer(self);
	if(!initAxes(motorPtr)){
        // PyErr should already be set
        return NULL;
    }
    //
	Py_RETURN_NONE;
}
//
static PyObject *TableMotorController_home_system(TableMotorController *self, PyObject *args){
	CNIMotionMotors* motorPtr = (CNIMotionMotors*)getPointer(self);
	if(!homeSystem(motorPtr)){
        // PyErr should already be set
        return NULL;
    }
    //
	Py_RETURN_NONE;
}
//
static PyObject *TableMotorController_move_to_target(TableMotorController *self, PyObject *args, PyObject *kwargs){
	double r_pos = 0.0;
	double s_pos = 0.0;
	int wait_time_ms = MOTOR_WAIT_TIME;
	//
	static char *kwlist[] = {"r_pos", "s_pos", "wait_time_ms", NULL};
	//
	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "dd|i", kwlist, &r_pos, &s_pos, &wait_time_ms)){
		PyErr_SetString(PyExc_Exception, "Could not parse args and kwargs.\n");
		return NULL;
	}
    if(pythonLogging){
        printf("move_to_target: (r_pos, s_pos, wait_time_ms) = (%f %f %d)\n", r_pos, s_pos, wait_time_ms);
    }
	//
	CNIMotionMotors* motorPtr = (CNIMotionMotors*)getPointer(self);
	if(!moveToTarget(motorPtr, r_pos, s_pos, wait_time_ms)){
        // PyErr should already be set
        return NULL;
    }
    //
	Py_RETURN_NONE;
}
//
static PyObject *TableMotorController_orientation(TableMotorController *self, PyObject *args, PyObject *kwargs){
    char *position_str = NULL;
    char *orientation_str = NULL;
	//
	static char *kwlist[] = {"position", "orientation", NULL};
    //
    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "zz", kwlist, &position_str, &orientation_str)){
		//PyErr_SetString(PyExc_Exception, "Could not parse args and kwargs.\n");
		return NULL;
	}
    if(pythonLogging){
        printf("orientation: (position, orientation) = (%s, %s)\n", position_str, orientation_str);
    }
	//
	CNIMotionMotors* motorPtr = (CNIMotionMotors*)getPointer(self);
	if(!orientation(motorPtr, position_str, orientation_str)){
        // PyErr should already be set
        return NULL;
    }
    //
	Py_RETURN_NONE;
}
//
static PyObject *TableMotorController_read_current_position(TableMotorController *self, PyObject *args, PyObject *kwargs){
    double returnValues[2];
    //
    CNIMotionMotors* motorPtr = (CNIMotionMotors*)getPointer(self);
    if(!readCurrentPosition(motorPtr, returnValues)){
        // PyErr should already be set
        return NULL;
    }
    if(pythonLogging){
        printf("read_current_position: Building position.\n");
    }
    if(pythonLogging){
        printf("read_current_position: (val[0], val[1]) = (%f %f)\n", returnValues[0], returnValues[1]);
    }
    //
    PyObject* returnVal = Py_BuildValue("[d,d]", returnValues[0], returnValues[1]);
    //
	return returnVal;
}
//
static PyObject *TableMotorController_set_initial_focus(TableMotorController *self, PyObject *args, PyObject *kwargs){
    double r_pos = 0.0;
	double s_pos = 0.0;
	//
	static char *kwlist[] = {"r_pos", "s_pos", NULL};
	//
	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "dd", kwlist, &r_pos, &s_pos)){
		PyErr_SetString(PyExc_Exception, "Could not parse args and kwargs.\n");
		return NULL;
	}
    if(pythonLogging){
        printf("set_initial_focus: (r_pos, s_pos) = (%f %f)\n", r_pos, s_pos);
    }
	//
	CNIMotionMotors* motorPtr = (CNIMotionMotors*)getPointer(self);
	if(!setInitialFocus(motorPtr, r_pos, s_pos)){
        // PyErr should already be set
        return NULL;
    }
    //
	Py_RETURN_NONE;
}
//
static PyObject *TableMotorController_set_focus(TableMotorController *self, PyObject *args, PyObject *kwargs){
    double r_pos = 0.0;
	double s_pos = 0.0;
	//
	static char *kwlist[] = {"r_pos", "s_pos", NULL};
	//
	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "dd", kwlist, &r_pos, &s_pos)){
		PyErr_SetString(PyExc_Exception, "Could not parse args and kwargs.\n");
		return NULL;
	}
    if(pythonLogging){
        printf("set_focus: (r_pos, s_pos) = (%f %f)\n", r_pos, s_pos);
    }
	//
	CNIMotionMotors* motorPtr = (CNIMotionMotors*)getPointer(self);
	if(!setFocus(motorPtr, r_pos, s_pos)){
        // PyErr should already be set
        return NULL;
    }
    //
	Py_RETURN_NONE;
}
//
static PyObject *TableMotorController_stop_motors(TableMotorController *self, PyObject *args, PyObject *kwargs){
	CNIMotionMotors* motorPtr = (CNIMotionMotors*)getPointer(self);
	if(!stopMotors(motorPtr)){
        // PyErr should already be set
        return NULL;
    }
    //
	Py_RETURN_NONE;
}
//
static PyObject *TableMotorController_get_ranges(TableMotorController *self, PyObject *args, PyObject *kwargs){
    double returnValues[4];
    //
    CNIMotionMotors* motorPtr = (CNIMotionMotors*)getPointer(self);
    if(!getRanges(motorPtr, returnValues)){
        // PyErr should already be set
        return NULL;
    }
    if(pythonLogging){
        printf("get_ranges: (val[0], val[1], val[2], val[3]) = (%f %f %f %f)\n", returnValues[0], returnValues[1], returnValues[2], returnValues[3]);
    }
    //
    PyObject* returnVal = Py_BuildValue("[d,d,d,d]", returnValues[0], returnValues[1], returnValues[2], returnValues[3]);
    //
	return returnVal;
}
//
static PyObject *TableMotorController_set_periodic_breakpoint(TableMotorController *self, PyObject *args, PyObject *kwargs){
	int axis = 0;
	double period_mm = 0.0;
	//
	static char *kwlist[] = {"axis", "period_mm", NULL};
	//
	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "id", kwlist, &axis, &period_mm)){
		PyErr_SetString(PyExc_Exception, "Could not parse args and kwargs.\n");
		return NULL;
	}
	//
	CNIMotionMotors* motorPtr = (CNIMotionMotors*)getPointer(self);
	if(!setPeriodicBreakpoint(motorPtr, axis, period_mm)){
        // PyErr should already be set
        return NULL;
    }
    //
	Py_RETURN_NONE;
}
//
static PyObject *TableMotorController_set_periodic_breakpoint_enabled(TableMotorController *self, PyObject *args, PyObject *kwargs){
	int axis = 0;
	PyObject* py_enabled = NULL;
	bool enabled = false;
	//
	static char *kwlist[] = {"axis", "enabled", NULL};
	//
	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "iO!", kwlist, &axis, &PyBool_Type, &py_enabled)){
		PyErr_SetString(PyExc_Exception, "Could not parse args and kwargs.\n");
		return NULL;
	}
	enabled = PyObject_IsTrue(py_enabled);
	//
	CNIMotionMotors* motorPtr = (CNIMotionMotors*)getPointer(self);
	if(!setPeriodicBreakpointEnabled(motorPtr, axis, enabled)){
        // PyErr should already be set
        return NULL;
    }
    //
	Py_RETURN_NONE;
}
