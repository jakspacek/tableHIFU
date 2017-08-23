static bool initAxes(CNIMotionMotors* motorPtr){
	int errCode;
	errCode = motorPtr->InitializeAxes();
	//
	if(errCode == 0){
		PyErr_SetString(PyExc_Exception, "Function initAxes: The error code is 0.\n");
        return false;
	}
    //
    return true;
}
//
static bool homeSystem(CNIMotionMotors* motorPtr){
	motorPtr->HomeSystem();
    return true;
}
//
#if defined(MOVE_THREADED)
typedef struct struct_motorParameters{ 
    CNIMotionMotors* motorPtr; 
    char x[STRING_SIZE];
    char y[STRING_SIZE];
    bool bHasError;
    int error;
} motorParameters;
#endif //MOVE_THREADED
//
#if defined(MOVE_THREADED)
DWORD WINAPI moveMotor(LPVOID parameters);
#endif //MOVE_THREADED
//
static bool moveToTarget(CNIMotionMotors* motorPtr, double r_pos, double s_pos, int wait_time_ms){
	if(wait_time_ms < 0){
		PyErr_SetString(PyExc_Exception, "Function moveToTarget: waitTime must be positive.\n");
		return false;
	}
	DWORD waitTime = (DWORD)wait_time_ms;
	//
	char RL_str[STRING_SIZE];
	char SI_str[STRING_SIZE];
	//
	if(r_pos > 0){
		RL_str[0] = 'R';
	}else{
		RL_str[0] = 'L';
	}
	//
	sprintf(RL_str+1, "%f", abs(r_pos));
	//
	if(s_pos > 0){
		SI_str[0] = 'S';
	}else{
		SI_str[0] = 'I';
	}
	//
	sprintf(SI_str+1, "%f", abs(s_pos));
    //
#if defined(MOVE_THREADED)
    //
	/************************************************************
	  Threaded version of moveToTarget allows you to ascertain a
	  timeout period upon which the motors seize to move to its
	  designated spot. The thread is then killed and an error is
	  produced to stop execution while not freezing the entire system.
	*************************************************************/
    //
	//Setup parameters
	motorParameters param;
	param.motorPtr = motorPtr;
	strcpy(param.x, RL_str);
	strcpy(param.y, SI_str);
	param.bHasError = false;
	//
	//Initialize thread handler
	HANDLE threadHandler;
	DWORD dummyID;
	threadHandler = CreateThread(NULL, 0, moveMotor, (LPVOID) &param, 0, (LPDWORD)&dummyID);
    //
	//wait for the thread to complete
	DWORD returnFromWait;
	returnFromWait = WaitForMultipleObjects(1, &threadHandler, TRUE, waitTime);
    //
	//Check to see if it completed successfully, or got to the timeout
	if(returnFromWait == WAIT_TIMEOUT){
        PyErr_SetString(PyExc_Exception, "Function moveToTarget: Error while moving table: exceeded timeout.\n");
        return false;
	}else if(param.bHasError == true){
        PyErr_SetString(PyExc_Exception, "Function moveToTarget: Error while moving table: check your limits or your log file.\n");
        return false;
	}
    //
#endif //MOVE_THREADED
#if !defined(MOVE_THREADED)
    //
	/**************************
	  Non-threaded version
	***************************/
    //
	int errorCheck = 0;
	errorCheck = motorPtr->compatibleMoveToTarget(RL_str, SI_str);
	if(!errorCheck){
        PyErr_SetString(PyExc_Exception, "Function moveToTarget: FLEX: Error when calling MoveToTarget().\n");
        return false;
	}
    //
#endif //!MOVE_THREADED
    //
    return true;
}
//
#if defined(MOVE_THREADED)
DWORD WINAPI moveMotor(LPVOID parameters){
	motorParameters* param = (motorParameters *)parameters;  
	int errorCheck = 0;
	errorCheck = param->motorPtr->compatibleMoveToTarget(param->x, param->y);
	//
	param->error = errorCheck;
	//
	if(!errorCheck){
		param->bHasError = true;
	}
	return 0;
}
#endif //MOVE_THREADED
//
#define STRCMP_ISVALID(strOBJ,strSTR)		\
  errorCheck = strOBJ.Compare(_T(strSTR));	\
  if (errorCheck == 0)						\
    isValid = true;
//
static bool orientation(CNIMotionMotors* motorPtr, char* position, char* orientation){
    int errorCheck = 1;
    bool isValid = false;
    //
    CString sPosition = CString(position);
    //
    STRCMP_ISVALID(sPosition, "Prone");
    STRCMP_ISVALID(sPosition, "Supine");
    STRCMP_ISVALID(sPosition, "Left Decub");
    STRCMP_ISVALID(sPosition, "Right Decub");
    //
    if(!isValid){
        PyErr_SetString(PyExc_Exception, "Function orientation: Incorrect position given.\n");
        return false;
    }
    //
    //reset for next comparison
    isValid = false;
    //
    CString sOrientation = CString(orientation);
    //
    STRCMP_ISVALID(sOrientation, "Head First");
    STRCMP_ISVALID(sOrientation, "Feet First");
    //
    if(!isValid){
        PyErr_SetString(PyExc_Exception, "Function orientation: Incorrect orientation given.\n");
        return false;
    }
    //
    //Everything is valid, set the orientation
    //
    //To get around the issue of incompatible CString types, I created
    //a compatible function which then converts to its CString type
    //and passes to the function of interest.
    errorCheck = motorPtr->compatibleConvertOrienStrToInt(position, orientation);
    //
    if(!errorCheck){
        PyErr_SetString(PyExc_Exception, "Function orientation: Error setting up orientation.\n");
        return false;
    }
    //
    return true;
}
//
static bool readCurrentPosition(CNIMotionMotors* motorPtr, double *returnValues){
    int errorCheck = 1;
    errorCheck = motorPtr->compatibleReadCurrentPosition(returnValues);
    //
    if(!errorCheck){
        PyErr_SetString(PyExc_Exception, "Function readCurrentPosition: Error reading current position.\n");
        return false;
    }
    //
    return true;
}
//
static bool setInitialFocus(CNIMotionMotors* motorPtr, double r_pos, double s_pos){
    char RL_str[STRING_SIZE];
    char SI_str[STRING_SIZE];
    //
    if(r_pos > 0){
        RL_str[0] = 'R';
    }else{
        RL_str[0] = 'L';
    }
    //
    sprintf(RL_str+1, "%f", abs(r_pos));
    //
    if(s_pos > 0){
        SI_str[0] = 'S';
    }else{
        SI_str[0] = 'I';
    }
    //
    sprintf(SI_str+1, "%f", abs(s_pos));
    //
    int errorCheck = 0;
    errorCheck = motorPtr->compatibleResetInitialFocus(RL_str, SI_str);
    if(!errorCheck){
        PyErr_SetString(PyExc_Exception, "Function setInitialFocus: FLEX: Error when calling ResetInitialFocus().\n");
        return false;
    }
    //
    return true;
}
//
static bool setFocus(CNIMotionMotors* motorPtr, double r_pos, double s_pos){
    char RL_str[STRING_SIZE];
    char SI_str[STRING_SIZE];
    //
    if(r_pos > 0){
        RL_str[0] = 'R';
    }else{
        RL_str[0] = 'L';
    }
    //
    sprintf(RL_str+1, "%f", abs(r_pos));
    //
    if(s_pos > 0){
        SI_str[0] = 'S';
    }else{
        SI_str[0] = 'I';
    }
    //
    sprintf(SI_str+1, "%f", abs(s_pos));
    //
    int errorCheck = 0;
    errorCheck = motorPtr->compatibleResetCurrentFocus(RL_str, SI_str);
    if(!errorCheck){
        PyErr_SetString(PyExc_Exception, "Function setFocus: FLEX: Error when calling ResetCurrentFocus().\n");
        return false;
    }
    //
    return true;
}
//
static bool stopMotors(CNIMotionMotors* motorPtr){
    motorPtr->StopMotors(1);
    return true;
}
//
static bool getRanges(CNIMotionMotors* motorPtr, double *returnValues){
    int errorCheck = 1;
    errorCheck = motorPtr->compatibleGetRanges(returnValues);
    //
    if(!errorCheck){
        PyErr_SetString(PyExc_Exception, "Function getRanges: Error reading the ranges.\n");
        return false;
    }
    //
    return true;
}
static bool setPeriodicBreakpoint(CNIMotionMotors* motorPtr, int axis, double period_mm){
	if(axis < 1 || axis > 3){
		PyErr_SetString(PyExc_Exception, "Function setPeriodicBreakpoint: Axis must be an integer between 1 and 3 inclusive.\n");
        return false;
	}
	//
	int errorCheck = 1;
    errorCheck = motorPtr->SetPeriodicBreakpoint(axis, period_mm);
    //
    if(!errorCheck){
        PyErr_SetString(PyExc_Exception, "Function setPeriodicBreakpoint: Error setting the breakpoint.\n");
        return false;
    }
    //
    return true;
}
static bool setPeriodicBreakpointEnabled(CNIMotionMotors* motorPtr, int axis, bool enabled){
	if(axis < 1 || axis > 3){
		PyErr_SetString(PyExc_Exception, "Function setPeriodicBreakpointEnabled: Axis must be an integer between 1 and 3 inclusive.\n");
        return false;
	}
	//
	int errorCheck = 1;
    errorCheck = motorPtr->SetPeriodicBreakpointEnabled(axis, enabled);
    //
    if(!errorCheck){
        PyErr_SetString(PyExc_Exception, "Function setPeriodicBreakpointEnabled: Error enabling/disabling the breakpoint.\n");
        return false;
    }
    //
    return true;
}
