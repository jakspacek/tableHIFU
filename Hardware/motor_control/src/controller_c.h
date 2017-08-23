#ifndef __PYGPUPOINTER_FUNCTIONS__
#define __PYGPUPOINTER_FUNCTIONS__
//
//Required for including the MFC shared libraries
//from the commandline. (This may not be true)
#define _AFXDLL
//Set MoveToTarget to use threaded version
#define MOVE_THREADED
//
#define STRING_SIZE 64
//
#define SET_LOG()				\
  freopen("flex.log", "a", stdout);
//
static bool initAxes(CNIMotionMotors* motorPtr);
static bool homeSystem(CNIMotionMotors* motorPtr);
static bool moveToTarget(CNIMotionMotors* motorPtr, double r_pos, double s_pos, int wait_time_ms);
static bool orientation(CNIMotionMotors* motorPtr, char* position, char* orientation);
static bool readCurrentPosition(CNIMotionMotors* motorPtr, double *returnValues);
static bool setInitialFocus(CNIMotionMotors* motorPtr, double r_pos, double s_pos);
static bool setFocus(CNIMotionMotors* motorPtr, double r_pos, double s_pos);
static bool stopMotors(CNIMotionMotors* motorPtr);
static bool getRanges(CNIMotionMotors* motorPtr, double *returnValues);
static bool setPeriodicBreakpoint(CNIMotionMotors* motorPtr, int axis, double period_mm);
static bool setPeriodicBreakpointEnabled(CNIMotionMotors* motorPtr, int axis, bool enabled);
//
#endif
