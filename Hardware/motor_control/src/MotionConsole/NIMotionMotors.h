#pragma once

#if !defined _NIMotionMotors_h
#define _NIMotionMotors_h
//--------------MECH SYSTEM ORIENTATION VS DIRECTION MAPPING--------------
// These values correspond to motor directions L/A/S for HeadFirst/Supine combination
// The rest of the orientations are derived from these values
#if !defined Supine_HF_LR
#define Supine_HF_LR -1
#endif

#if !defined Supine_HF_AP
#define Supine_HF_AP 1
#endif

#if !defined Supine_HF_SI
#define Supine_HF_SI 1
#endif

//-------------HOST DEFINITION--------------
#if !defined HOST
#define HOST	0xFF
#endif

//-------------ERRORS--------------//
#if !defined Err_Posn_Convert
#define Err_Posn_Convert 1000
#endif

#if !defined Err_MAX_ResetPosn
#define Err_MAX_ResetPosn 1001
#endif

#if !defined Err_MAX_ReadPosn
#define Err_MAX_ReadPosn 1002
#endif

#if !defined Err_Mtr2Subj_Convert
#define Err_Mtr2Subj_Convert 1003
#endif
#if !defined Err_Invalid_Arr
#define Err_Invalid_Arr 1004
#endif

#if !defined Err_Invalid_OpMode
#define Err_Invalid_OpMode 1005
#endif

#if !defined Err_General
#define Err_General 1006
#endif

#if !defined Err_Vect_Space
#define Err_Vect_Space 1007
#endif
//-----------END ERRORS-----------//

//-----------CHECK STATUS-----------//
#if !defined Check_MoveComplete
#define Check_MoveComplete 1
#endif

#if !defined Check_Stop
#define Check_Stop 2
#endif

#if !defined Check_AxisTrip
#define Check_AxisTrip 3
#endif

#if !defined Check_Unknown
#define Check_Unknown 0
#endif	
//-----------END CHECK STATUS-----------//

// Include libraries
#include "stdafx.h"
#include "ConfigFile.h"
#include "flexmotn.h"
#include <vector>

struct structPosition
{
	CString sLRpos, sAPpos, sSIpos;
};
struct structRange
{
	CString sMax[3];
	CString sMin[3];
};

using namespace std;

class CNIMotionMotors
{
	// From Config file set during InitializeAxes
	u8		m_boardID;					// Board identification number	
	
    // Axis configuration
    int m_axisEncoder[3];
    int m_axis[3];
    int m_axisDAC[3];

	//Variables for modal error handling
	i32		m_err;						// Error status from NI Motion
	u16		m_commandID;				// The commandID of the function
	u16		m_resourceID;				// The resource ID
	i32		m_errorCode;				// Error code
	u16		m_csr ;						// Communication status register
	
	

	//Config variables
	int		m_Conversion_table[8][3];	// Conversion table for different systems
	float	m_convert[3];				// Conversion matrix
	int 	m_sCurveTime[3];			// s-Curve Time
	i32		m_velocity_count[3];		// Velocity in counts
	u32		m_accelDecel_count[3];		// AccelDecel in counts
	i32		m_home_velocity_count[3];	// Homing Velocity in counts
	i32		m_pos_sw_lim_count[3];		// Positive software limit
	i32		m_neg_sw_lim_count[3];		// Negative software limit
	bool	m_bAuto_Home_All;			// Auto Home All
	bool	m_bVS_USING;				// Vector space using

	u16		m_motor_home[3];
	bool	m_active_axes[3];			// Active Axes array
	bool	m_manual_axes[3];			// Active Axes array
	
	structRange m_pRange;

	//Axis Status
	u16		m_axisStatus;		// Axis status
	u16		m_moveComplete;		// Move complete

	
	//Variables for position updates
	bool m_updatesenabled;		//Update enabled flag
	HWND m_hWnd;				//Handle to window that updates are sent to
	UINT m_message;				//Message sent to window
	structPosition m_spos;		//Subject position
	bool	m_bHoming;
	bool    m_bRunning;

	//Initial Focus
	float m_fInitialFocus[3];

	//Variables for status updates
	//Motion
	HWND m_hWndSts;
	UINT m_messageSts;

	//Home status
	HWND m_hWndHomeSts;
	UINT m_messageHomeSts;

	//Initial Setup Dlg Controls
	HWND m_hWndIni;
	UINT m_messageIniBasicEnable;
	UINT m_messageIniBasicDisable;
	bool m_bIniBasic;
	UINT m_messageIniSubjectEnable;
	UINT m_messageIniSubjectDisable;
	bool m_bIniSubject;
	UINT m_messageIniFocusEnable;
	UINT m_messageIniFocusDisable;
	bool m_bIniFocus;


	HWND m_hWndTabs;
	UINT m_messageEnable;
	UINT m_messageDisable;
	bool m_bTabs;

	HWND m_hWndIndivTab;
	UINT m_messageIndivTabEnable;
	UINT m_messageIndivTabDisable;
	bool m_bIndividTab;

	HWND m_hWndTargetReached;
	UINT m_messageTargetReached;
	bool m_bTargetReached;
	
	HWND m_hWndETime;
	UINT m_messageElapsedTime;
	bool m_bElapsedTime;

	//Threading variables
	u8 m_ThreadAxis;
	u16 m_ThreadBitmap;
	u16 m_bitmap;
	bool m_ThreadRunning;
	CWinThread* m_threadSingle;
	bool m_bThreadSingleWait;
	bool m_bResumedSingle;
	int m_SingleMotor_Axis;


	//Motion variables
	u8 m_vectorSpace;
	
	//Log Handler
	CLogHandler LogHandler;

	//Note: Rows represent L/R,A/P,S/I, and columns points 1,2,3...
	//The vector should contain the first point at the end if one is to move back there
	vector< vector< i32 > > m_vMotorPointsCounts;
	int m_iCycles;
	double m_dDuration;
	int m_iSonTime;
	u16 m_breakpointPolarity;
	u8 m_breakpointAxis;
	bool m_bRunBPointThread;
	bool m_bStopBPoint;
	u16 m_BPointPolarity;

    // Periodic breakpoint
    bool m_periodicBreakpointEnabled[3]; //!< Remember if a breakpoint is enabled (for each axis)
    
	//Subject setup
	int			m_iSubjOrien;				//subject orientation


	//************************** CONVERSIONS **************************//	
	void	SetConvTable(int LR, int AP, int SI);
	bool	ConvertSubjectToMotor(CString subjectposition, int &motor_axis, 
				float &motor_position);
	bool	ConvertMotorToSubject(CString &subjectposition, int iSubjectAxis, 
				float motor_position_mm, int iPrecision = -1);
	//************************** CONTROLLER **************************//	
	void	ResetPosition(u8 axis, float motor_posn_mm);
	void    ResetLimits(u8 axis, i32 motor_home_count);
	bool	ReadPosition(u8 axis, float &motor_posn_mm);
	bool    LoadMotionProfile(int axis_vs, i32 velocity, i32 accelDecel, int sCurveTime);
	bool    LoadBreakpoints();
	bool	SetBreakpointOutput(u8 axis, u16 polarity);
	//************************** ERROR CHECKING **************************//
	// Checks for errors
		//numexceptions: number of error exceptions
		//exceptions: an array containing errors not to display an error message for
		//			   (i.e. error exceptions)
	void	CheckError(int line_num, u32 numexceptions = 0, u32* exceptions = NULL);
	// Displays errors
	void	DisplayError(int line_num, i32 errorCode, u16 commandID, u16 resourceID, u32 numexceptions, u32* exceptions);
	// Set exceptions during homing
	void	CheckError_Home();
	//************************** HOMING **************************//
	// Execute home finding and error checking during execution
	static	UINT	HomeSystemThread(LPVOID p);
	void	HomeSystemThread(CString* sMessage);
	int		HomeAxis(u8 axis, i32 velocity_count);	
	u16		ExecFindHome(bool bDirection, u16 &finding, u16 &axisStatus, u8 axis);
	//************************** MOTION **************************//
	static UINT RunMoveThread(LPVOID p);
	CString* RunMoveThread();
	int CheckStatus_Loop();
	bool MoveAxes(bool axes[3],float motor_posn_mm[3], i32 velocity [3], u8 opMode);
	bool MoveVectorSpace(u8 opMode, i32 velocity, u32 acceldecel, u16 sCurveTime,float motor_posn_mm[3]);
	bool ConfigVectSpace(u8 x, u8 y, u8 z, u8 vectorSpace);
	bool ConfigureBreakpoints(const u8 &axis, u8 &buffer, const UINT &points);


	static UINT RunMultiTargetThread(LPVOID p);
	static UINT RunBreakpointThread(LPVOID p);
	bool Scan3D(const UINT &iMaxPoints, const int &x, const int &y, 
		const int &z, CString* sMessage, const int iCycles, const double &dCycleDur, const int &iSonTime,
		const u16 &polarity);
	int CheckBlendStatus_Loop();
	int CheckVSpaceStatus_Loop(const u32 &timeout);
	
	bool IsPosnWithinLimits(CStringArray &sPosn);

	

public:
	CNIMotionMotors();	
	//************************** INITIALIZATION **************************//
	int			InitializeAxes();
	bool		ResetController();
	bool		IsActive(u8 axis);
	bool		IsManual(u8 axis);
	//************************** HOMING **************************//
	void			HomeSystem();			// Default uses home_velocity
	
	//************************** ERROR CHECKING **************************//
	void		StopMotors(int stop_type);
	//************************** CONVERSIONS **************************//	
	int			ConvertOrienStrToInt(CString sPosition, CString sOrientation);

	bool		ResetCurrentFocus(const CStringArray &sNewFocus);

	bool		ResetInitialFocus(const CStringArray &sNewFocus);
	bool		ReadCurrentPosition(CStringArray &sPosn);
	//************************** POSITION UPDATES **************************//
	void EnableUpdates(HWND hWnd, UINT message);
		//Enables position updates to a window
		//MESSAGE FORMAT:
		//		msg: "UINT message" parameter sent to EnableUpdates
		//		wParam: pointer to a subject_position structure (position in motor coordinates)
		//		lParam: pointer to a motor_position structure (position in motor coordinates)
	void StopUpdates();
		//Stops position updates to a window

	void GetUpdate();
		//Forces NIMotionMotors to send a position update

	//************************** HOME UPDATES **************************//
	//Made up numbers. Will fix later when I understand the code better
	void EnableHomeStsUpdates(HWND hWnd = (HWND) 100, UINT message = 1);
	void DisableHomeStsUpdates();
	//************************** INITIAL SETUP CONTROLS **************************//
	void SetIniBasicCtrlMessages(bool bEnable, HWND hWnd, UINT messageEnable, UINT messageDisable);
	void SetIniSubjectCtrlMessages(bool bEnable, HWND hWnd, UINT messageEnable, UINT messageDisable);
	void SetIniFocusCtrlMessages(bool bEnable, HWND hWnd, UINT messageEnable, UINT messageDisable);
	//************************** STATUS UPDATES **************************//
	void EnableMotionStsUpdates(HWND hWnd = (HWND) 0x001, UINT message = 1);
	void DisableMotionStsUpdates();
	//************************** TAB CONTROLS UPDATES **************************//
	void SetTabsCtrlMessages(HWND hWnd, UINT messageEnable, UINT messageDisable);
	//********************* INDIVIDUAL TAB CONTROLS UPDATES **********************//
	void SetIndivTabCtrlMessages(HWND hWnd, UINT messageEnable, UINT messageDisable);
	//********************* ELAPSED TIME UPDATES **********************//
	void SetElapsedTimeMessages(bool bEnable, HWND hWnd, UINT message);
	//************************** MOTION **************************//
	bool MoveToTarget(const CStringArray &sTarget);
	bool MoveSingleAxis(const CString &sTarget, i32 velocity, u8 opMode);
	void MoveMultiTargets(const vector< vector<CString> > &vsPoints, const int &iCycles, 
		const double &dDuration, const int &dSonTime, const u16 &polarity);
	//************************** POSITION VALIDATION **************************//
	bool ValidatePosition(CString str[3], CStringArray &sPosn);
	
	structRange GetRanges();
	structPosition GetFocus();
	
	bool MustHomeSystem();

	//************************** PUBLIC MEMBERS **************************//
	bool		m_check_stop;

	//************************** MEX COMPATIBLE **************************//
	bool compatibleMoveToTarget(char*, char*);
	bool compatibleReadCurrentPosition(double*);
	//Expects a double array with 4 values
	//the values returned are:
	//[ max L, max R, max S, max I ]
	bool compatibleGetRanges(double*);
	bool compatibleResetCurrentFocus(char*, char*);
	bool compatibleResetInitialFocus(char*, char*);
	//Required to ensure compatibilty in mex functions
	int	compatibleConvertOrienStrToInt(char *cPosition, char *cOrientation);

    //********************** PERIODIC BREAKPOINTS ***********************//
    bool SetPeriodicBreakpointEnabled(u8 axis, bool enable = true);
    
    bool SetPeriodicBreakpoint(u8 axis, float period_mm); //!< Set the breakpoint from current position on axis
    bool SetPeriodicBreakpoint(u8 axis, float period_mm, float position_mm, u32 window = 5); //!< Set breakpoint at position (in mm) on axis
    bool SetPeriodicBreakpoint(float period_mm, CString mri_position); //!< Set the breakpoint at subject/MRI position
    
protected:

};
#endif //#if !defined _NIMotionMotors_h
