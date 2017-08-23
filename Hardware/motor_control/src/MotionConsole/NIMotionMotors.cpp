// NIMotionMotors.cpp : implementation file
//
#include "stdafx.h"
#include "NIMotionMotors.h"
#include "math.h"
#include "LogHandler.h"
#include "Performance_Timers.h"

CNIMotionMotors::CNIMotionMotors()
{
	// Set default values
	
	m_boardID = 0;

	memset(m_axisEncoder, 0, sizeof(m_axisEncoder));
	memset(m_axis, 0, sizeof(m_axis));
	memset(m_axisDAC, 0, sizeof(m_axisDAC));

	m_err = 0;
	m_commandID = 0;
	m_resourceID = 0;
	m_errorCode = 0;
	m_csr = 0;

	memset(m_Conversion_table, 0, sizeof(m_Conversion_table));
	memset(m_convert, 0, sizeof(m_convert));
	memset(m_sCurveTime, 0, sizeof(m_sCurveTime));
	memset(m_velocity_count, 0, sizeof(m_velocity_count));
	memset(m_accelDecel_count, 0, sizeof(m_accelDecel_count));
	memset(m_home_velocity_count, 0, sizeof(m_home_velocity_count));
	memset(m_pos_sw_lim_count, 0, sizeof(m_pos_sw_lim_count));
	memset(m_neg_sw_lim_count, 0, sizeof(m_neg_sw_lim_count));
	m_bAuto_Home_All = false;
	m_bVS_USING = false;

	memset(m_motor_home, 0, sizeof(m_motor_home));
	memset(m_active_axes, 0, sizeof(m_active_axes));
	memset(m_manual_axes, 0, sizeof(m_manual_axes));

	structRange m_pRange = {{CString(_T("")), CString(_T("")), CString(_T(""))}, {CString(_T("")), CString(_T("")), CString(_T(""))}};

	m_axisStatus = 0;
	m_moveComplete = 0;

	m_updatesenabled = false;
	m_hWnd = NULL;
	m_message = 0;
	m_spos = {CString(_T("")), CString(_T("")), CString(_T(""))};
	m_bHoming = false;
	m_bRunning = false;

	memset(m_fInitialFocus, 0, sizeof(m_fInitialFocus));

	m_hWndSts = NULL;
	m_messageSts = 0;

	m_hWndHomeSts = NULL;
	m_messageHomeSts = 0;

	m_hWndIni = NULL;
	m_messageIniBasicEnable = 0;
	m_messageIniBasicDisable = 0;
	m_bIniBasic = false;
	m_messageIniSubjectEnable = 0;
	m_messageIniSubjectDisable = 0;
	m_bIniSubject = false;
	m_messageIniFocusEnable = 0;
	m_messageIniFocusDisable = 0;
	m_bIniFocus = false;

	m_hWndTabs = NULL;
	m_messageEnable = 0;
	m_messageDisable = 0;
	m_bTabs = false;

	m_hWndIndivTab = NULL;
	m_messageIndivTabEnable = 0;
	m_messageIndivTabDisable = 0;
	m_bIndividTab = false;

	m_hWndTargetReached = NULL;
	m_messageTargetReached = 0;
	m_bTargetReached = false;

	m_hWndETime = NULL;
	m_messageElapsedTime = 0;
	m_bElapsedTime = false;

	m_ThreadAxis = 0;
	m_ThreadBitmap = 0;
	m_bitmap = 0;
	m_ThreadRunning = false;
	m_threadSingle = NULL;
	m_bThreadSingleWait = false;
	m_bResumedSingle = false;
	m_SingleMotor_Axis = 0;

	m_vectorSpace = 0;

	m_vMotorPointsCounts.resize(3);
	m_iCycles = 0;
	m_dDuration = 0;
	m_iSonTime = 0;
	m_breakpointPolarity = 0;
	m_breakpointAxis = 0;
	m_bRunBPointThread = false;
	m_bStopBPoint = false;
	m_BPointPolarity = 0;

	memset(m_periodicBreakpointEnabled, 0, sizeof(m_periodicBreakpointEnabled));

	m_iSubjOrien = 0;

	// Set specific values
	
	m_axis[0] = NIMC_AXIS1;
	m_axis[1] = NIMC_NOAXIS;
	m_axis[2] = NIMC_AXIS2;

	m_axisEncoder[0]  = NIMC_ENCODER1;
	m_axisEncoder[1]  = -1;
	m_axisEncoder[2]  = NIMC_ENCODER2;

	m_axisDAC[0]  = NIMC_DAC1;
	m_axisDAC[1]  = -1;
	m_axisDAC[2]  = NIMC_DAC2;
}


//*************************** INITIALIZATION ***************************//
int CNIMotionMotors::InitializeAxes()
{	
	int errors = 0;						//config file read errors
	bool bProcess = true;
	float	home_velocity_mm[3];		// Homing speed
	float	velocity_mm[3];				// Motion velocity
	float	accelDecel_mm[3];			// Motion acceleration
	float	followingError_mm[3]; 		// Following Error in mm
	u16		followingError_count[3];	// Following Error in counts
	float   pos_sw_lim_mm[3];
	float   neg_sw_lim_mm[3];
	
	//Load the Config File
	CString strError;
	UINT	uiErrType;
	strError = _T("");

	// PID parameters for Servo axes
	PID PIDValues_Axis[3];			

	// Set the Conversion Table
	SetConvTable(Supine_HF_LR, Supine_HF_AP, Supine_HF_SI);

	// Set vector space for LR and SI (or not)
	if (m_bVS_USING)
		m_vectorSpace = NIMC_VECTOR_SPACE1;
	
	
	try 
	{
		ConfigFile cf("SystemConfig.inp" );
		
		//Read Board ID
		m_boardID = cf.read<int>( "BoardID", 0 );
		
		//Determine Active Axes
		m_active_axes[0] = cf.read<bool>( "Enabled _Axis1", 0);
		m_active_axes[1] = cf.read<bool>( "Enabled _Axis2", 0);
		m_active_axes[2] = cf.read<bool>( "Enabled _Axis3", 0);

		//Determine Manual Axes
		m_manual_axes[0] = cf.read<bool>( "Manual _Axis1", 0);
		m_manual_axes[1] = cf.read<bool>( "Manual _Axis2", 0);
		m_manual_axes[2] = cf.read<bool>( "Manual _Axis3", 0);


		//Is going home optional?
		m_bAuto_Home_All = cf.read<bool>( "AutoHomeEnable_All", 0);;
		
		// Set parameters for all active axes
		int i;		
		for(i= 0; i < 3; i++)
		{
			std::stringstream ss;
			std::string axis;
			ss << (i+1);
			ss >> axis;
			
			m_convert[i] = 0.0;
			pos_sw_lim_mm[i] = 0.0;
			neg_sw_lim_mm[i] = 0.0;

			if (m_manual_axes[i] || m_active_axes[i])
			{
				// Need these even if the axis is in manual mode in order to see feedback
				m_convert[i] = cf.read<float>( "mmTocount _Axis" + axis, 0.0 );

				pos_sw_lim_mm[i] = cf.read<float>("Range_max _Axis" + axis, 0.0); 
				neg_sw_lim_mm[i] = cf.read<float>("Range_min _Axis" + axis, 0.0); 
				
			}
			
			if(m_active_axes[i])
			{	
				home_velocity_mm[i] = cf.read<float>( "HomeVelocity_Axis" + axis, 0.0 ); 
				followingError_mm[i] = cf.read<float>("followingError_Axis" + axis, 0); 
				velocity_mm[i] = cf.read<float>("velocity_Axis" + axis, 0.0 );
				accelDecel_mm[i] = cf.read<float>("acceleration_Axis" + axis, 0.0 );
				m_sCurveTime[i] = cf.read<int>("sCurveTime_Axis" + axis, 0);
				PIDValues_Axis[i].kp   =  cf.read<int>("kp_Axis" + axis, 0);
				PIDValues_Axis[i].kd   =  cf.read<int>("kd_Axis" + axis, 0);
				PIDValues_Axis[i].ki   =  cf.read<int>("ki_Axis" + axis, 0);
				PIDValues_Axis[i].kv   =  cf.read<int>("kv_Axis" + axis, 0);
				PIDValues_Axis[i].td   =  cf.read<int>("td_Axis" + axis, 0);
				PIDValues_Axis[i].ilim =  cf.read<int>("ilim_Axis" + axis, 0);
				PIDValues_Axis[i].aff  =  cf.read<int>("aff_Axis" + axis, 0);
				PIDValues_Axis[i].vff  =  cf.read<int>("vff_Axis" + axis, 0);
			}
			else
			{
				home_velocity_mm[i] = 0;
				followingError_mm[i] = 0;
				velocity_mm[i] = 0;
				accelDecel_mm[i] = 0;
				m_sCurveTime[i] = 0;
				
				//Set to no gains so that axis is not hard to move
				PIDValues_Axis[i].kp   =  0; 
				PIDValues_Axis[i].kd   =  0; 
				PIDValues_Axis[i].ki   =  0; 
				PIDValues_Axis[i].kv   =  0; 
				PIDValues_Axis[i].td   =  2; 
				PIDValues_Axis[i].ilim =  1000; 	
				PIDValues_Axis[i].aff  =  0; 
				PIDValues_Axis[i].vff  =  0; 
			}
		}
		m_bVS_USING = cf.read<bool>( "VS_USING", 0);
		
		if (m_bVS_USING)
		{
			bProcess = false;
			uiErrType = ERR_CODE_MOTOR_INI;
			strError = _T("Vector space control is not supported.");
		}
		
		if (m_active_axes[0] + m_active_axes[1] + m_active_axes[2] < 2)
			m_bVS_USING = false;
	}
	catch( ConfigFile::file_not_found& e ) 
	{
		++errors;
		strError = e.filename.c_str();

	} 
	catch( ConfigFile::key_not_found& e ) 
	{
		++errors;
		strError = e.key.c_str();

	}
	catch(char *str)
	{
		++errors;
		strError = str;

	}
	if( errors > 0 )
	{
		bProcess = false;
		uiErrType = ERR_CODE_FILEOP;
		strError = CString("Configuration File: SystemConfig.inp: ")+strError;		
	}
	
	//----------------------CONVERSIONS---------------------------------------//
	
	if (bProcess)
	{
		for (int i=0; i<3; i++)
		{
			// Convert Following Error
			followingError_count[i] = (u16)(followingError_mm[i]* m_convert[i]);
			// Convert Velocity
			m_velocity_count[i] = (i32)(velocity_mm[i]* m_convert[i]); 
			// Convert AccelDecel
			m_accelDecel_count[i] = (u32)(accelDecel_mm[i]* m_convert[i]); 
			// Convert Homing Velocity
			m_home_velocity_count[i] = (i32)(home_velocity_mm[i]* m_convert[i]); 
			// Convert software limits
			m_pos_sw_lim_count[i] = (i32)(pos_sw_lim_mm[i]*m_convert[i]);
			m_neg_sw_lim_count[i] = (i32)(neg_sw_lim_mm[i]*m_convert[i]);
		}
	}
	
	//----------------------INITIALIZE CONTROLLER------------------------------//
	i32	status		= NIMC_noError;	        // Return Status of Functions
	u8	enableAxes	= 0x1E;					// Bitmap of axes to enable
	u32 countsPerRev = 1000;				// Quadrature encoder counts per revolution	
	// Bitmap of the polarity for the limit switches, home
	// switches and inhibits - Polarity is being set for inverting
	u8 polarityMap = 0x1E;		
	// Bitmap for the limit switches, home switches and inhibits to enable
	u8 enableMap = 0x1E;	


	if (bProcess)
	{
		// 1. Clear power up reset bit
		bProcess = ResetController();
		if (bProcess == false)
			uiErrType = ERR_CODE_MOTOR_INI;
	}
	

	if (bProcess)
	{
		// 2a. Flush the data return buffer
		m_err = flex_flush_rdb(m_boardID);
		if (m_err!=0)
		{
			bProcess = false;
			uiErrType = ERR_CODE_MOTOR_INI;
			strError = _T("Data return buffer could not be flushed.");
		}
		CheckError(__LINE__);
	}
	

	if (bProcess)
	{
		// 2b. Read the communication status register and check the modal errors
		m_err = flex_read_csr_rtn(m_boardID, &m_csr);
		if (m_err!=0)
		{
			bProcess = false;
			uiErrType = ERR_CODE_MOTOR_READ_STS;
		}
		// Check to see if there were any Modal Errors
		if (m_csr & NIMC_MODAL_ERROR_MSG)
		{ 
			do
			{ 
				//Get the command ID, resource and the error code of the modal
				//	error from the error stack on the board
				flex_read_error_msg_rtn(m_boardID,&m_commandID,&m_resourceID,&m_errorCode);
				flex_read_csr_rtn(m_boardID,&m_csr);
			}
			while(m_csr & NIMC_MODAL_ERROR_MSG);
			m_err = 0;
		}
	}



	if (bProcess)
	{
		// 3. Disable all axes
		StopMotors(1);
		m_err = flex_enable_axis (m_boardID, NIMC_AXIS_CTRL, 0, 0);
		if (m_err!=0)
		{
			bProcess = false;
			uiErrType = ERR_CODE_MOTOR_INI;
			strError = _T("Axes could not be disabled.");
		}
		CheckError(__LINE__);
	}

	// 4. Configure axes

	if (bProcess)
	{
		for (int i=0; i<3 && bProcess; i++)
		{
			if (m_active_axes[i] || m_manual_axes[i])
			{
				// Configure Feedback
				m_err = flex_config_axis (m_boardID, m_axis[i], m_axisEncoder[i], 0, m_axisDAC[i], 0);
				if (m_err!=0)
				{
					bProcess = false;
					uiErrType = ERR_CODE_MOTOR_INI;
					strError.Format(_T("Feedback could not be configured for axis %d"),i+1);
					
				}
				CheckError(__LINE__);
			}
		}
	}

	

	if (bProcess)
	{
		// 5. Enable axes 1-4 with a 250 microsecond PID rate
		m_err = flex_enable_axis (m_boardID, NIMC_AXIS_CTRL, NIMC_PID_RATE_250, 
												enableAxes);
		if (m_err!=0)
		{
			bProcess = false;
			uiErrType = ERR_CODE_MOTOR_INI;
			strError = _T("Axes could not be enabled.");
		}
		CheckError(__LINE__);
	}


	if (bProcess)
	{
		for (int i=0; i<3 && bProcess; i++)
		{
			if (m_active_axes[i] || m_manual_axes[i])
			{
				// Load PID Parameters
				m_err = flex_load_pid_parameters (m_boardID, m_axis[i], &PIDValues_Axis[i], HOST);
				if (m_err!=0)
				{
					bProcess = false;
					uiErrType = ERR_CODE_MOTOR_INI;
					strError = _T("PID parameters could not be loaded. ");
					
				}
				CheckError(__LINE__);
				

				

				if (bProcess)
				{
					// Configure Move Complete Criteria
					m_err = flex_config_mc_criteria(m_boardID, m_axis[i], 21,10,0,0);	//Default settings
					if (m_err!=0)
					{
						uiErrType = ERR_CODE_MOTOR_INI;
						strError = _T("Move complete criteria could not be loaded. ");
						bProcess =false;
					}
					CheckError(__LINE__);
				}
				

				if (bProcess)
				{
					// Encoder counts per revolution
					m_err = flex_load_counts_steps_rev (m_boardID, m_axis[i], NIMC_COUNTS, countsPerRev);
					if (m_err!=0)
					{
						uiErrType = ERR_CODE_MOTOR_INI;
						strError = _T("Encoder counts per revolution could not be loaded. ");
						bProcess =false;
					}
					CheckError(__LINE__);
				}
			}
		}
	}
		


	if (bProcess)
	{
		// 6. Configure inhibit output polarity and enable inhibit outputs 
		m_err = flex_configure_inhibits (m_boardID, polarityMap, enableMap);
		if (m_err!=0)
		{
			uiErrType = ERR_CODE_MOTOR_INI;
			strError = _T("Inhibit output polarity could not be configured. ");
			bProcess =false;
		}
		CheckError(__LINE__);
	}
	if (bProcess)
	{
		m_err = flex_set_limit_polarity (m_boardID, polarityMap, enableMap);
		if (m_err!=0)
		{
			uiErrType = ERR_CODE_MOTOR_INI;
			strError = _T("Limit polarity could not be configured.");
			bProcess =false;
		}
		CheckError(__LINE__);
	}
	if (bProcess)
	{
		m_err = flex_set_home_polarity (m_boardID, polarityMap);
		if (m_err!=0)
		{
			uiErrType = ERR_CODE_MOTOR_INI;
			strError = _T("Home polarity could not be configured. ");
			bProcess =false;
		}
		CheckError(__LINE__);
	}
	if (bProcess)
	{
		m_err = flex_enable_axis_limit(m_boardID, NIMC_LIMIT_INPUTS, 0, 0);
		if (m_err!=0)
		{
			uiErrType = ERR_CODE_MOTOR_INI;
			strError = _T("Axis hardware limits could not be configured. ");
			bProcess =false;
		}
		CheckError(__LINE__);
	}
	// 8. Disable home input switches
	if (bProcess)
	{
		m_err = flex_enable_home_inputs (m_boardID, 0);
		if (m_err!=0)
		{
			uiErrType = ERR_CODE_MOTOR_INI;
			strError = _T("Home input switches could not be configured. ");
			bProcess =false;
		}
		CheckError(__LINE__);
	}
	if (bProcess)
	{
		for (int i=0; i<3 && bProcess; i++)
		{
			if (m_active_axes[i] || m_manual_axes[i])
			{
				// Operation Mode to Absolute
				m_err = flex_set_op_mode (m_boardID, m_axis[i], NIMC_ABSOLUTE_POSITION);
				if (m_err!=0)
				{
					uiErrType = ERR_CODE_MOTOR_INI;
					strError = _T("Operation mode could not be set. ");
					bProcess =false;
				}
				CheckError(__LINE__);

				// Following Error
				if (bProcess)
				{
					m_err = flex_load_follow_err (m_boardID, m_axis[i], followingError_count[i], HOST);
					if (m_err!=0)
					{
						uiErrType = ERR_CODE_MOTOR_INI;
						strError = _T("Following error could not be configured. ");
						bProcess =false;
					}
					CheckError(__LINE__);
				}

				// Default Velocity
				if (bProcess)
				{
					m_err = flex_load_velocity(m_boardID, m_axis[i], m_velocity_count[i], HOST);
					if (m_err!=0)
					{
						uiErrType = ERR_CODE_MOTOR_INI;
						strError = _T("Velocity could not be configured. ");
						bProcess =false;
					}
					CheckError(__LINE__);
				}
				// Default AccelDecel
				if (bProcess)
				{
					m_err = flex_load_acceleration(m_boardID, m_axis[i], NIMC_BOTH, m_accelDecel_count[i], HOST);
					if (m_err!=0)
					{
						uiErrType = ERR_CODE_MOTOR_INI;
						strError = _T("Accel/Decel could not be configured. ");
						bProcess =false;
					}
					CheckError(__LINE__);
				}

				// Set the jerk - scurve time (in sample periods)
				if (bProcess)
				{
					m_err = flex_load_scurve_time(m_boardID, m_axis[i],  m_sCurveTime[i], 0xFF);
					if (m_err!=0)
					{
						uiErrType = ERR_CODE_MOTOR_INI;
						strError = _T("S-Curve time could not be configured. ");
						bProcess =false;
					}
					CheckError(__LINE__);
				}

				// Load Advanced Control Parameters: STATIC_FRICTION_MODE is disabled
				if (bProcess)
				{
					m_err = flex_load_advanced_control_parameter(m_boardID,m_axis[i],NIMC_STATIC_FRICTION_MODE,0,0xFF);
					if (m_err!=0)
					{
						uiErrType = ERR_CODE_MOTOR_INI;
						strError = _T("Static Friction Mode could not be configured. ");
						bProcess =false;
					}
					CheckError(__LINE__);
				}
			}
		}
	}
	
	// 9. Energize axes
	if (bProcess)
	{
		StopMotors(2);		
	}
	
	// End of function
	if (bProcess)
		return 1;
	else
	{
		this->LogHandler.HandleError(uiErrType,__FUNCTIONW__, ERR_LOG, NULL, ERR_NO_MESSAGE, ERR_BOX, ERR_LEVEL_NORMAL,strError);		
		return 0;
	}
}
bool CNIMotionMotors::ResetController()
{
	bool bProcess = true;
	CheckError(__LINE__);
	m_err = flex_clear_pu_status (m_boardID);
	if (m_err!=0)
		bProcess = false;
	CheckError(__LINE__);
	
	//	Wait for the power up reset bit to go low
	if (bProcess)
	{
		do
		{
			//Read the communication Status register
			m_err = flex_read_csr_rtn(m_boardID, &m_csr);
			CheckError(__LINE__);
		}while (m_csr & NIMC_POWER_UP_RESET);
	}
	return bProcess;
	
}
void CNIMotionMotors::SetConvTable(int LR, int AP, int SI)
{
	m_Conversion_table[0][0] = LR;
	m_Conversion_table[0][1] = AP;
	m_Conversion_table[0][2] = SI;

	m_Conversion_table[1][0] = (-1)* m_Conversion_table[0][0];
	m_Conversion_table[1][1] = (-1)* m_Conversion_table[0][1];
	m_Conversion_table[1][2] =       m_Conversion_table[0][2];
	
	m_Conversion_table[2][0] = (-1)*m_Conversion_table[0][0];
	m_Conversion_table[2][1] =      m_Conversion_table[0][1];
	m_Conversion_table[2][2] =      m_Conversion_table[0][2];
	
	m_Conversion_table[3][0] =      m_Conversion_table[0][0];
	m_Conversion_table[3][1] = (-1)*m_Conversion_table[0][1];
	m_Conversion_table[3][2] =      m_Conversion_table[0][2];

	m_Conversion_table[4][0] = (-1)*m_Conversion_table[0][0];
	m_Conversion_table[4][1] =      m_Conversion_table[0][1];
	m_Conversion_table[4][2] = (-1)*m_Conversion_table[0][2];

	m_Conversion_table[5][0] = (-1)* m_Conversion_table[1][0];
	m_Conversion_table[5][1] =       m_Conversion_table[1][1];
	m_Conversion_table[5][2] = (-1)* m_Conversion_table[1][2];

	m_Conversion_table[6][0] =       m_Conversion_table[2][0];
	m_Conversion_table[6][1] =  (-1)*m_Conversion_table[2][1];
	m_Conversion_table[6][2] =  (-1)*m_Conversion_table[2][2];

	m_Conversion_table[7][0] =       m_Conversion_table[3][0];
	m_Conversion_table[7][1] =  (-1)*m_Conversion_table[3][1];
	m_Conversion_table[7][2] =  (-1)*m_Conversion_table[3][2];
}
bool CNIMotionMotors::IsActive(u8 axis)
{
	return m_active_axes[axis-1];
}
bool CNIMotionMotors::IsManual(u8 axis)
{
	return m_manual_axes[axis-1];
}

//*************************** HOMING ***************************//
UINT CNIMotionMotors::HomeSystemThread(LPVOID p)
{
	CString *sMessage = new CString(_T(""));
	((CNIMotionMotors*)(p))->HomeSystemThread(sMessage);
	return 0;
}

void CNIMotionMotors::HomeSystemThread (CString* sMessage)
{ 
	m_bHoming = true;
	UINT uiErrType;
	CString strError;

	m_motor_home[0] = 0;
	m_motor_home[1] = 0;
	m_motor_home[2] = 0;

	bool bProcess = true;
	
	u16 flagLR = 0;
	u16 flagSI = 0;
	u16 flagAP = 0;
	
	// ------------Find home on LR Axis---------------
	if (bProcess)
	{
		if (m_active_axes[0])
		{
			flagLR = HomeAxis(1,m_home_velocity_count[0]);	
		}
		else 
			flagLR = 1;
		// ------------Check Motor Stop--------------------
		if (m_check_stop)
		{
			//This is a no error situation - the user pressed stop
			bProcess = false;
		}
		// Check status after trying to home LR
		if (bProcess && flagLR != 1)
		{
			bProcess = false;
			*sMessage=_T("Problem with Axis L/R...");
			LogHandler.HandleError(ERR_CODE_MOTOR_HOME,__FUNCTIONW__,ERR_LOG,m_hWndHomeSts,m_messageHomeSts,ERR_NO_BOX,ERR_LEVEL_NORMAL, *sMessage);
		}
	}
	
	

	// ------------Find home on SI Axis---------------
	
	if (bProcess)
	{
		if (m_active_axes[2])
		{
			flagSI = HomeAxis(3,m_home_velocity_count[2]);
		}
		else 
			flagSI = 1;
		// ------------Check Motor Stop--------------------
		if (m_check_stop)
		{
			//This is a no error situation - the user pressed stop
			bProcess = false;
		}
		if (bProcess && flagSI != 1)
		{
			bProcess = false;
			*sMessage=_T("Problem with Axis S/I...");
			LogHandler.HandleError(ERR_CODE_MOTOR_HOME,__FUNCTIONW__,ERR_LOG,m_hWndHomeSts,m_messageHomeSts,ERR_NO_BOX,ERR_LEVEL_NORMAL, *sMessage);
		}
	}
	
	// ------------Find home on AP Axis---------------
	if (bProcess)
	{
		if (m_active_axes[1])
		{
			flagAP = HomeAxis(2, m_home_velocity_count[1]);
		}
		else 
			flagAP = 1;
		// ------------Check Motor Stop--------------------
		if (m_check_stop)
		{
			//This is a no error situation - the user pressed stop
			bProcess = false;
		}
		if (bProcess && flagAP!=1)
		{
			bProcess = false;
			*sMessage=_T("Problem with Axis A/P...");
			LogHandler.HandleError(ERR_CODE_MOTOR_HOME,__FUNCTIONW__,ERR_LOG,m_hWndHomeSts,m_messageHomeSts,ERR_NO_BOX,ERR_LEVEL_NORMAL, *sMessage);
		}
	}
	
	//All active axes are home status
	if (bProcess)
	{
		m_motor_home[0] = 0;
		m_motor_home[1] = 0;
		m_motor_home[2] = 0;
	}
	else 
	{
		StopMotors(2); 		
	}
	
	// When thread finishes...
	delete sMessage; //don't need the pointer any more
	m_check_stop = false; //reset the stop flag, in case it was turned true

	// Clean up window handlers
	if(m_bIniBasic)
	{
		m_bIniBasic = false;
		m_hWndIni = NULL;
		m_messageIniBasicDisable = NULL;
		m_messageIniBasicEnable = NULL;
	}
	if(m_bIniSubject)
	{
		m_bIniSubject = false;
		m_hWndIni = NULL;
		m_messageIniSubjectDisable = NULL;
		m_messageIniSubjectEnable = NULL;
	}
	if(m_bIniFocus)
	{
		m_bIniFocus = false;
		m_hWndIni = NULL;
		m_messageIniFocusDisable = NULL;
		m_messageIniFocusEnable = NULL;
	}
	if(m_bTabs)
	{
		m_bTabs = false;
		m_hWndTabs = NULL;
		m_messageDisable = NULL;
		m_messageEnable = NULL;
	}
	if(m_bIndividTab)
	{
		m_bIndividTab = false;
		m_hWndIndivTab = NULL;
		m_messageIndivTabDisable = NULL;
		m_messageIndivTabEnable = NULL;
	}

	DisableHomeStsUpdates();	
	m_bHoming = false;
}



void CNIMotionMotors::HomeSystem()
{
	//AfxBeginThread(HomeSystemThread, this);
	HomeSystemThread(this);
}
int	CNIMotionMotors::HomeAxis(u8 axis, i32 velocity_count)
{
	u16 axisStatus;
	u16 found, finding;
	bool bProcess = true;

	m_check_stop = false;
	axisStatus = 0;
	found = 0;
	finding = 1;
	m_motor_home[axis-1] = 0;

	// Read communication status register
	if (bProcess)
	{
		m_err = flex_read_csr_rtn(m_boardID, &m_csr);
		if (m_err!=0)
		{
			bProcess = false;
		}
		CheckError_Home();
	}

	// Read axis status -> todo: why does this use different variable?
	if (bProcess)
	{
		m_err = flex_read_axis_status_rtn(m_boardID, m_axis[axis-1], &axisStatus);
		if (m_err!=0)
		{
			bProcess = false;
		}
		CheckError_Home();
	}

	//Load velocity to the axis selected
	if (bProcess)
	{
		m_err = flex_load_velocity(m_boardID, m_axis[axis-1], velocity_count, 0xFF);
		if (m_err!=0)
		{
			bProcess = false;
		}
		CheckError_Home();
	}

	//Set enable reset to false
	if (bProcess)
	{
		m_err = flex_load_reference_parameter(m_boardID,m_axis[axis-1],1,NIMC_ENABLE_RESET_POSITION,false);
		if (m_err!=0)
		{
			bProcess = false;
		}
		CheckError_Home();
	}

	//Enable search distance
	if (bProcess)
	{
		m_err = flex_load_reference_parameter(m_boardID,m_axis[axis-1],1,NIMC_ENABLE_SEARCH_DISTANCE, true);
		if (m_err!=0)
		{
			bProcess = false;
		}
		CheckError_Home();
	}

	//TODO: Set search distance: Calculate this distance based on half the distance between limits
	if (bProcess)
	{
		m_err = flex_load_reference_parameter(m_boardID,m_axis[axis-1],1,NIMC_SEARCH_DISTANCE,50000);
		if (m_err!=0)
		{
			bProcess = false;
		}
		CheckError_Home();
	}

	//Set logical state of encoder phase A
	if (bProcess)
	{
		m_err = flex_load_reference_parameter(m_boardID,m_axis[axis-1],1,0xC,1);
		if (m_err!=0)
		{
			bProcess = false;
		}
		CheckError_Home();
	}

	//Set logical state of encoder phase B
	if (bProcess)
	{
		m_err = flex_load_reference_parameter(m_boardID,m_axis[axis-1],1,0xD,1);
		if (m_err!=0)
		{
			bProcess = false;
		}
		CheckError_Home();
	}

	//Set approach speed
	if (bProcess)
	{
		m_err = flex_load_reference_parameter(m_boardID,m_axis[axis-1],1,NIMC_APPROACH_VELOCITY_PERCENT,100);
		if (m_err!=0)
		{
			bProcess = false;
		}
		CheckError_Home();
	}


	//Execute find home
	if (bProcess)
	{
		found = ExecFindHome(true, finding, axisStatus, axis);

		if (!found && !m_check_stop)
		{
			//Find home in reverse direction, since nothing found forward
			StopMotors(2);	
			found = 0;
			finding = 1;
			//Execute find home in the reverse direction
			found = ExecFindHome(false, finding, m_axisStatus, axis);
		}
	}

	//Update axis status at the end
	m_axisStatus = axisStatus;
	if (found)
		m_motor_home[axis-1] = 1;

	return found;
}
u16 CNIMotionMotors::ExecFindHome(bool bDirection, u16 &finding, u16 &axisStatus, u8 axis)
{
	u16 found = 0;
	bool bProcess = true;
	//Set search direction
	if (bProcess)
	{
		m_err = flex_load_reference_parameter(m_boardID,m_axis[axis-1],1,NIMC_INITIAL_SEARCH_DIRECTION,bDirection);
		if (m_err!=0)
			bProcess = false;
		CheckError_Home();
	}

	//Find index: TODO: might have to find limit first
	if (bProcess)
	{
		m_err = flex_find_reference(m_boardID,m_axis[axis-1],0,1);
		if (m_err!=0)
			bProcess = false;
		CheckError_Home();
		
		
	}
	//Wait until search is finished
	while (bProcess && finding && !m_check_stop)
	{
		m_err = flex_check_reference(m_boardID,m_axis[axis-1],0,&found,&finding);
		if (m_err!=0)
			bProcess = false;
		CheckError_Home();
	}
	
	
	//Check for following errors; this used to use m_axisStatus
	if (bProcess)
	{
		m_err = flex_read_axis_status_rtn(m_boardID,m_axis[axis-1],&m_axisStatus);
		if (m_err!=0)
			bProcess = false;
		CheckError_Home();
	}

	// Read the communication status register and check the modal errors
	// do this regardless, in order to clear the modal errors 
	m_err = flex_read_csr_rtn(m_boardID, &m_csr);
	if (m_err!=0)
		bProcess = false;
	else if (m_csr & NIMC_MODAL_ERROR_MSG) // Check for modal errors
	{
		m_err = m_csr & NIMC_MODAL_ERROR_MSG;
		CheckError_Home();
	}
	return found;
}


//*************************** ERROR CHECKING ***************************//
void CNIMotionMotors::CheckError(int line_num, u32 numexceptions, u32* exceptions)
{	
	if (m_err != 0)
	{
		// Check to see if there were any Modal Errors
		if (m_csr & NIMC_MODAL_ERROR_MSG)
		{ 
			do{ 
				//Get the command ID, resource and the error code of the modal
				//	error from the error stack on the board
				flex_read_error_msg_rtn(m_boardID,
					&m_commandID,
					&m_resourceID,
					&m_errorCode);

				DisplayError(line_num,
					m_errorCode,
					m_commandID,
					m_resourceID,
					numexceptions,
					exceptions); 

				//Read the Communication Status Register
				flex_read_csr_rtn(m_boardID,&m_csr);
			}while(m_csr & NIMC_MODAL_ERROR_MSG);
		} 
		else 		// Display regular error 
			DisplayError(line_num,m_err,0,0,numexceptions,exceptions); 
	}
}
void CNIMotionMotors::DisplayError(int line_num, i32 errorCode, u16 commandID, u16 resourceID,
	u32 numexceptions, u32 *exceptions)
{
	char *errorDescription;			//Pointer to i8's -  to get error description
	wchar_t *w_errorDescription;		//Pointer to wide char string
	u32 sizeOfArray;				//Size of error description
	u16 descriptionType;			//The type of description to be printed
	i32 status;						//Error returned by function
	
	if(commandID == 0){
		descriptionType = NIMC_ERROR_ONLY;
	}else{
		descriptionType = NIMC_COMBINED_DESCRIPTION;
	}

	//First get the size for the error description
	sizeOfArray = 0; 
	errorDescription = NULL;//Setting this to NULL returns the size required
	status = flex_get_error_description(descriptionType, 
		errorCode, commandID, resourceID, 
		errorDescription, &sizeOfArray );

	//Allocate memory on the heap for the description
	sizeOfArray++;	 //So that the sizeOfArray is size of description + NULL character
	errorDescription = new char[sizeOfArray];

	// Get Error Description
	status = flex_get_error_description(descriptionType, errorCode, commandID, resourceID, 
													errorDescription, &sizeOfArray );
	
	//Convert error message from ASCII to wide character set
	errorDescription[sizeOfArray-1] = NULL;
	w_errorDescription = new wchar_t[sizeOfArray];
	MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,errorDescription,-1,w_errorDescription,sizeOfArray);

	//Check for exceptions
	bool exception = false;
	if ((exceptions)&&(numexceptions))
	{	for (u32 a = 0; a < numexceptions; a++)
			if (exceptions[a] == errorCode)
				exception = true;
	}
	if (errorDescription != NULL)
	{
		if (!exception)
			LogHandler.HandleError(ERR_CODE_MOTOR_NI,
				_T(""),
				ERR_LOG,
				NULL,
				ERR_NO_MESSAGE,
				ERR_NO_BOX,
				ERR_LEVEL_NORMAL,
				w_errorDescription,
				line_num);

		if (sizeOfArray)
		{
			delete[] errorDescription;
			delete[] w_errorDescription;
		}
	}
	else{
		LogHandler.HandleError(ERR_CODE_SOFTWARE,
			__FUNCTIONW__,
			ERR_LOG,
			NULL,
			ERR_NO_MESSAGE,
			ERR_NO_BOX,
			ERR_LEVEL_NORMAL,
			CString("Memory Allocation Error."),
			line_num);
	}
}
void CNIMotionMotors::CheckError_Home()
{	
	u32 numexceptions = 2;
	u32 exceptions[] = {-70165,-70166};
	CheckError(__LINE__,numexceptions,exceptions);
}



void CNIMotionMotors::StopMotors(int stop_type)
{	
	// Configure a Vector Space comprising of axes 1, 2 and 3;
	u8 vectorSpace = NIMC_VECTOR_SPACE1;
	
	int axis_reordered[3] = {0}; // NOTE: 0 == NIMC_NOAXIS
	
	int curr_index = 0;
	for(int i=0; i<3; i++){
		if(m_axis[i] != NIMC_NOAXIS){
			// we don't want to add NIMC_NOAXIS to the list since it acts as a sentinel stop value
			axis_reordered[curr_index] = m_axis[i];
			curr_index++;
		}
	}
	
	m_err = flex_config_vect_spc(m_boardID, vectorSpace, axis_reordered[0], axis_reordered[1], axis_reordered[2]);
	CheckError(__LINE__);

	switch ( stop_type ) 
	{
	  case 1 : 
		  m_err = flex_stop_motion(m_boardID, vectorSpace, NIMC_KILL_STOP, 0);//stop the motion
		  CheckError(__LINE__);
		  break;

	  case 2 : 
		m_err = flex_stop_motion(m_boardID, vectorSpace, NIMC_HALT_STOP, 0);//stop the motion
		CheckError(__LINE__);
		break;

	  case 3 : 
		  m_err = flex_stop_motion(m_boardID, vectorSpace, NIMC_DECEL_STOP, 0);//stop the motion
		  CheckError(__LINE__);
		  break;

	  default : 
		  //AfxMessageBox(_T("StopMotors function loaded with invalid parameter."));
		  cout << "StopMotors function loaded with invalid parameter." << endl;
		  break;
	}

	
}




//*************************** CONVERSIONS ***************************//
int CNIMotionMotors::ConvertOrienStrToInt(CString sPosition, CString sOrientation)
{
	int iSubjStrToInt = -1;	
	int iTempInitial = 0;
	bool bProcess = true;

	// Grab the initial value to add depending on orientation
	if (sOrientation == "Head First")
		iTempInitial = 0;
	else if (sOrientation == "Feet First")
		iTempInitial = 4;
	else
		bProcess = false;
	
	// Calculate subject integer value corresponding to orientation/position
	if (bProcess)
	{
		if (sPosition == "Supine")
			iSubjStrToInt = iTempInitial+1;
		else if (sPosition == "Prone")
			iSubjStrToInt = iTempInitial+2;
		else if (sPosition == "Left Decub")
			iSubjStrToInt = iTempInitial+3;
		else if (sPosition == "Right Decub")
			iSubjStrToInt = iTempInitial+4;
		else
			bProcess = false;
	}

	if (!bProcess)	
	{

		iSubjStrToInt = -1;
	}
	
	m_iSubjOrien = iSubjStrToInt;
	return iSubjStrToInt;
}

int CNIMotionMotors::compatibleConvertOrienStrToInt(char *cPosition,
	char *cOrientation) {
		CString sPosition = CString(cPosition);
		CString sOrientation = CString(cOrientation);
		return this->ConvertOrienStrToInt(sPosition,sOrientation);
}

bool CNIMotionMotors::ConvertSubjectToMotor(CString subjectposition,  
					 int &motor_axis, float &motor_position)
{
	// Convert from MRI position to motor position in mm  (i.e. sign computation)

	//Find axis label: l,r,a,p,s, or i
	subjectposition.MakeLower();
	TCHAR cSubjectAxis = subjectposition.GetAt(0);
	int iSubjectAxis;
	subjectposition.Delete(0);

	float mri_position = (float) _wtof((LPCWSTR)(subjectposition));
//	USES_CONVERSION;
//	float mri_position = (float) _wtof(T2CW(subjectposition));

	//Find axis and sign
	float sign;
	switch(cSubjectAxis)
	{
		case 'l':	iSubjectAxis = 1;	sign = 1;	break;
		case 'r':	iSubjectAxis = 1;	sign = -1;	break;
		case 'a':	iSubjectAxis = 2;	sign = -1;	break;
		case 'p':	iSubjectAxis = 2;	sign = 1;	break;
		case 's':	iSubjectAxis = 3;	sign = 1;	break;
		case 'i':	iSubjectAxis = 3;	sign = -1;	break;
		default: return false;
	}

	//Check to see if orientation was initialized
	if (m_iSubjOrien == -1)
		return false;

	//Perform conversion
	motor_axis = iSubjectAxis;
	motor_position = sign*m_Conversion_table[m_iSubjOrien-1][iSubjectAxis-1]*mri_position;
	return true;
}

bool CNIMotionMotors::ConvertMotorToSubject(CString &subjectposition,  
					 int iSubjectAxis, float motor_position_mm, int iPrecision)
{
	// Convert from motor position in mm to MRI position (i.e. sign computation)
	TCHAR cSubjectAxis;
	int sign = 0;
	bool bProcess = true;
	float mri_position = 0.0;

	//Check to see if orientation has been done
	if (m_iSubjOrien == -1)
		bProcess = false;

	subjectposition = _T("");
	
	if (iSubjectAxis == 1)
		sign = 1;
	else if (iSubjectAxis == 2)
		sign = -1;
	else if (iSubjectAxis == 3)
		sign = 1;
	else
		bProcess = false;

	if (!m_active_axes[iSubjectAxis-1] && !m_manual_axes[iSubjectAxis-1])
		bProcess = false;

	if (bProcess)
	{
		//Convert position from motor position to subject position in mm
		mri_position = sign*m_Conversion_table[m_iSubjOrien-1][iSubjectAxis-1]*motor_position_mm;

		switch (iSubjectAxis)
		{
		case 1:
			{
				if (mri_position < 0.)
					cSubjectAxis = 'R';
				else
					cSubjectAxis = 'L';
			}
			break;
		case 2:
			{
				if (mri_position < 0.)
					cSubjectAxis = 'P';
				else
					cSubjectAxis = 'A';
			}
			break;
		case 3:
			{
				if (mri_position < 0.)
					cSubjectAxis = 'I';
				else
					cSubjectAxis = 'S';
			}
			break;
		default:
			bProcess = false;
		}
	}
	if (bProcess)
	{
		try
		{
			if ((iPrecision == -1)||(iPrecision < 0))
				subjectposition.Format(_T("%c%f"),cSubjectAxis,fabs(mri_position));
			else
			{
				CString temp;
				temp.Format(_T("%%c%%.%df"),iPrecision);
				subjectposition.Format(temp,cSubjectAxis,fabs(mri_position));
			}
		}
		catch (CException *e)
		{
			e->Delete();
			bProcess = false;
			LogHandler.HandleError(ERR_CODE_SOFTWARE,
				__FUNCTIONW__,
				ERR_LOG,NULL,
				ERR_NO_MESSAGE,
				ERR_NO_BOX,
				ERR_LEVEL_NORMAL,
				_T("Exception while resetting position."));
		}
	}
	if (!bProcess)
		subjectposition = "";
	return bProcess;
}

bool CNIMotionMotors::ResetCurrentFocus(const CStringArray &sNewFocus)
{	
	bool bProcess = true;
	int motor_axis = -1;
	float motor_posn_mm = 0.0;
	int iMaxCount = 0;
	u16 limitMap = 0;
	CString strError;
	UINT uiErrType;

	if (sNewFocus.GetSize() != 3)
	{
		iMaxCount = 0;
		bProcess = false;
		strError = _T("Invalid array size supplied to the function.");
		uiErrType = ERR_CODE_SOFTWARE;
	}
	else
	{
		iMaxCount = 3;
	}

	for (int i=0; i<iMaxCount && bProcess; i++)
	{
		if(m_active_axes[i] || m_manual_axes[i])
		{
			if (ConvertSubjectToMotor(sNewFocus[i], motor_axis, motor_posn_mm))
			{
				try
				{
					m_fInitialFocus[i] = motor_posn_mm;
					ResetPosition(motor_axis, motor_posn_mm);
					
				}
				catch(CException* e)
				{
					bProcess = false;
					e->Delete();
					strError = _T("Exception while resetting position.");
					uiErrType = ERR_CODE_SOFTWARE;
				}
			}
			else
			{
				bProcess = false;
				strError.Format(_T("Error converting subject coordinates to motor coordinates on axis %d."), i+1);	
				uiErrType = ERR_CODE_SOFTWARE;
			}
		} // end active axes
	} // end for loop
	if (bProcess)
	{
		m_ThreadBitmap = 0;
		for(int i=0; i<3 && bProcess; i++)
		{
			// here we assume NIMC_AXIS1==1, NIMC_AXIS2==2, etc...
			m_ThreadBitmap = m_ThreadBitmap|(m_active_axes[i]<<m_axis[i]);
			// if "m_active_axes[i]" is TRUE, then "m_active_axes[i]<<m_axis[i]" simply enables axis "m_axis[i]"
			// for example if m_active_axes[2]==1 and m_axis[2]=NIMC_AXIS1, then m_active_axes[2]<<m_axis[2] == 1<<1 == b01<<1 == b10 -> axis 1 enabled
		}
		
		m_err = flex_enable_axis_limit(m_boardID, NIMC_SOFTWARE_LIMITS, m_ThreadBitmap, m_ThreadBitmap);
		if (m_err!=0)
		{
			bProcess = false;
			strError = _T("Error enabling axis software limits.");
			uiErrType = ERR_CODE_MOTOR_NI;
		}
	}
	if (!bProcess)
		LogHandler.HandleError(uiErrType,__FUNCTIONW__,ERR_LOG,NULL,ERR_NO_MESSAGE,ERR_NO_BOX, ERR_LEVEL_NORMAL,strError);

	return bProcess;
 }

bool CNIMotionMotors::ResetInitialFocus(const CStringArray &sNewFocus)
{	
	bool bProcess = true;
	int motor_axis = -1;
	float motor_focus_mm[3] = {0.0, 0.0, 0.0};
	float motor_posn_mm[3] = {0.0,0.0,0.0};
	CStringArray sCurrentPosition;
	int iMaxCount = 0;
	float offset[3];
	CString strError;
	UINT uiErrType;

	if (sNewFocus.GetSize() != 3)
	{
		iMaxCount = 0;
		bProcess = false;
		strError = _T("Invalid array size supplied to the function.");
		uiErrType = ERR_CODE_SOFTWARE;
	}
	else
	{
		iMaxCount = 3;
	}

	//Convert new focus to motor coordinates
	for (int i=0; i<iMaxCount && bProcess; i++)
	{
		if(m_active_axes[i] || m_manual_axes[i])
		{
			if (!ConvertSubjectToMotor(sNewFocus[i], motor_axis, motor_focus_mm[i]))
			{
				bProcess = false;
				strError.Format(_T("Error converting subject coordinates to motor coordinates on axis %d."), i+1);
				uiErrType = ERR_CODE_SOFTWARE;
			}
		} // end active axes
	} // end for loop
	
	//Read current position 
	if (bProcess)
	{
		if (!ReadCurrentPosition(sCurrentPosition))
		{
			bProcess = false;
			strError = _T("Error reading current position from the motion card.");
			uiErrType = ERR_CODE_MOTOR_NI;
		}
	}

	//Convert current position to motor coordinates
	for (int i=0; i<iMaxCount && bProcess; i++)
	{
		if(m_active_axes[i] || m_manual_axes[i])
		{
			if (!ConvertSubjectToMotor(sCurrentPosition[i], motor_axis, motor_posn_mm[i]))
			{
				bProcess = false;
				strError.Format(_T("Error converting subject coordinates of current position to motor coordinates on axis %d."), i+1);
				uiErrType = ERR_CODE_SOFTWARE;
			}
		} // end active axes
	} // end for loop
	
	
	for (int i=0; i<3 && bProcess; i++)
	{
		if (m_active_axes[i] || m_manual_axes[i])
		{
				//offset = current position - current home
				offset[i] = motor_posn_mm[i] - m_fInitialFocus[i];			
				//new posn = new focus + offset
				motor_posn_mm[i] = motor_focus_mm[i] + offset[i];
				
				m_fInitialFocus[i] = motor_focus_mm[i];

				try
				{
					//reset to new position
					ResetPosition(i+1, motor_posn_mm[i]);						
				}
				catch(CException* e)
				{
					bProcess = false;
					e->Delete();
					strError = _T("Exception while resetting position.");
					uiErrType = ERR_CODE_SOFTWARE;
				}
		}
		else
			motor_posn_mm[i] = 0.0;

	}

	if (!bProcess)
		LogHandler.HandleError(uiErrType,__FUNCTIONW__,ERR_LOG,NULL,ERR_NO_MESSAGE,ERR_NO_BOX, ERR_LEVEL_NORMAL,strError);

	return bProcess;
 }

bool CNIMotionMotors::ReadCurrentPosition(CStringArray &sPosn)
{
	bool bProcess = true;
	float motor_posn_mm;
	bool bRead;
	CString subjectposition;
	CString strError;
	UINT uiErrType;

	if (sPosn.GetSize()<3)
		sPosn.SetSize(3);

	for (int i=0; i<3 && bProcess; i++)
	{
		motor_posn_mm = 0.0;
		subjectposition = "";
		bRead = false;
		if (m_active_axes[i] || m_manual_axes[i])
		{
			try
			{
				// Read position from Motion Controller
				bProcess = ReadPosition(i+1, motor_posn_mm);
			}
			catch(CException* e)
			{
				bProcess = false;
				strError = _T("Exception while reading position.");
				uiErrType = ERR_CODE_MOTOR_NI;
				e->Delete();
			}
			// Convert position to the subject string position
			if (bProcess)
			{
				if (!ConvertMotorToSubject(subjectposition,i+1, motor_posn_mm,1))
				{
					bProcess = false;
					strError.Format(_T("Error converting motor coordinates to subject coordinates on axis %d."), i+1);
					uiErrType = ERR_CODE_SOFTWARE;
				}
			}
			else
			{
				bProcess = false;
				strError = _T("Exception while reading position.");
				uiErrType = ERR_CODE_MOTOR_NI;
			}
		}
		sPosn.SetAt(i,subjectposition);

	}
	if (!bProcess)
		LogHandler.HandleError(uiErrType,__FUNCTIONW__,ERR_LOG,NULL,ERR_NO_MESSAGE,ERR_NO_BOX, ERR_LEVEL_NORMAL,strError);
	return bProcess;
}
//*************************** MOTION ***************************//
bool CNIMotionMotors::MoveToTarget(const CStringArray &sTarget)
{

	bool bProcess = true;
	int iMaxCount = 0;
	float motor_posn_mm[3]={0,0,0};
	int motor_axis[3] = {0,0,0};
	CString strError = _T("");
	UINT uiErrType = 0;

	this->m_check_stop = false;

	if (sTarget.GetSize() != 3)
	{
		iMaxCount = 0;
		bProcess = false;
	}
	else
	{
		iMaxCount = 3;
	}
	
	// Convert position to motor position in mm
	if (bProcess)
	{
		for (int i=0; i<iMaxCount && bProcess; i++)
		{
			if(m_active_axes[i])
			{
				if (!ConvertSubjectToMotor(sTarget[i], motor_axis[i], motor_posn_mm[i]))
				{
					bProcess = false;
					strError.Format(_T("Error converting subject coordinates to motor coordinates on axis %d."), i+1);
					uiErrType = ERR_CODE_SOFTWARE;
				}
			}
		}
	}
	if (bProcess)
	{
		if (m_bVS_USING)
		{
			
			bProcess = MoveVectorSpace(NIMC_ABSOLUTE_POSITION,m_velocity_count[0],
					m_accelDecel_count[0],m_sCurveTime[0],motor_posn_mm);
			if (bProcess)
				bProcess = MoveSingleAxis(sTarget[1], m_velocity_count[1], NIMC_ABSOLUTE_POSITION);

		}
		else
			bProcess = MoveAxes(m_active_axes, motor_posn_mm, m_velocity_count, NIMC_ABSOLUTE_POSITION);
	}

	if (!bProcess)
	{
		if (m_hWndHomeSts != NULL && m_messageSts != NULL)
		{
			CString* sMessage= new CString("Error during move configuration...");
			LogHandler.HandleError(ERR_CODE_MOTOR_MOVE,__FUNCTIONW__,ERR_LOG,m_hWndSts,m_messageSts,ERR_NO_BOX,ERR_LEVEL_NORMAL, *sMessage);
			delete sMessage;
		}
		if (uiErrType!=0) //log internal errors of this function
			LogHandler.HandleError(uiErrType,
									__FUNCTIONW__,
									ERR_LOG,NULL,
									ERR_NO_MESSAGE,
									ERR_NO_BOX,
									ERR_LEVEL_NORMAL,
									strError);
	}

	return bProcess;
}
void CNIMotionMotors::MoveMultiTargets(const vector< vector<CString> > &vsPoints,
	const int &iCycles,
	const double &dDuration,
	const int &iSonTime,
	const u16 &uPolarity) {
	
		bool bProcess = true; //it should get set to true by ConvertSubjectToMotor
		size_t iMaxPoints = vsPoints[0].size()-1;
		UINT i = 0, j = 0;
		bool bActiveAxis = true;
		float motor_posn_mm = 0.0;
		int motor_axis = 0;

		bProcess = ConfigVectSpace(1,3,0,m_vectorSpace);

		if (bProcess)
		{
		bProcess = false;
		for (i=0;i<3;i++)
			m_vMotorPointsCounts[i].resize(iMaxPoints);
		//Convert from string to counts
		do
		{
			i = 0;
			do
			{
				if(m_active_axes[i])
				{
					bProcess = ConvertSubjectToMotor(vsPoints[i][j+1],
						motor_axis,
						motor_posn_mm);

					m_vMotorPointsCounts[i][j] = (i32) (motor_posn_mm*m_convert[i]);					
					bActiveAxis = true;
				}
				else
					m_vMotorPointsCounts[i][j] = 0;
				i++;
			}while(i<3 && bProcess);
			j++;
		}while(j<iMaxPoints && bProcess && bActiveAxis);
		
		CString *sMessage = new CString(_T(""));;
		
		if (!bActiveAxis)
		{
			bProcess = false;
		}
		else if (!bProcess)
		{
			cout << "Conversion of target positions failed." << endl;
		}
		else
		{
			bProcess = LoadMotionProfile(m_vectorSpace,
				m_velocity_count[0],
				m_accelDecel_count[0],
				m_sCurveTime[0]);
			if (bProcess)
			{
				m_iCycles = iCycles;
				m_dDuration = dDuration;
				m_iSonTime = iSonTime;
				m_BPointPolarity = uPolarity;
				//AfxBeginThread(RunMultiTargetThread,this);
				//DEBUG BZ
				cout << "This works" << endl;
				RunMultiTargetThread(this);
			}
		}
		delete sMessage;
	}
	
}
bool CNIMotionMotors::MoveSingleAxis(const CString &sTarget, i32 velocity, u8 opMode)
{
	bool bProcess = true;
	CString strError = _T("");
	UINT uiErrType = 0;

	float motor_posn_mm = 0.0;
	if (!ConvertSubjectToMotor(sTarget, m_SingleMotor_Axis, motor_posn_mm))
	{
		bProcess = false;
		strError.Format(_T("Error converting subject coordinates to motor coordinates on axis %d."), m_SingleMotor_Axis);	
		uiErrType = ERR_CODE_SOFTWARE;
	}

	if (bProcess)
	{
		m_err = flex_set_op_mode(m_boardID, m_axis[m_SingleMotor_Axis-1], opMode);
		if (m_err!=0)
		{
			bProcess = false;
			uiErrType = ERR_CODE_MOTOR_MOVE_CFG;
			strError = _T("An error occurred while setting operation mode.");
		}
		CheckError(__LINE__);
	}

	if (bProcess)
	{
		i32 motor_posn_count = (i32) (motor_posn_mm*m_convert[m_SingleMotor_Axis-1]); 
		m_err = flex_load_target_pos(m_boardID,m_axis[m_SingleMotor_Axis-1],motor_posn_count,HOST);
		if (m_err!=0)
		{
			bProcess = false;
			uiErrType = ERR_CODE_MOTOR_MOVE_CFG;
			strError = _T("An error occurred while setting position.");
		}
		CheckError(__LINE__);
	}
	if (bProcess)
	{
		// Default Velocity
		m_err = flex_load_velocity(m_boardID, m_axis[m_SingleMotor_Axis-1], velocity, HOST);
		if (m_err!=0)
		{
			bProcess = false;
			uiErrType = ERR_CODE_MOTOR_MOVE_CFG;
			strError = _T("An error occurred while setting velocity.");
		}
		CheckError(__LINE__);
	}


	if (bProcess)
	{
		m_bThreadSingleWait = true;
		m_threadSingle = AfxBeginThread(RunMoveThread,this,
			THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED,NULL);
	}

	if (!bProcess)
	{
		//NOTE: The calling public function will deal with status updates

		//Log this function's internal errors
		LogHandler.HandleError(uiErrType,__FUNCTIONW__,ERR_LOG,NULL,ERR_NO_MESSAGE,ERR_NO_BOX, ERR_LEVEL_NORMAL,strError);
	}

	return bProcess; 
}
//*************************** CONTROLLER COMMANDS ***************************//
void CNIMotionMotors::ResetPosition(u8 axis, float motor_posn_mm)
{
	StopMotors(2);

	i32 motor_posn_count = (i32) (motor_posn_mm*m_convert[axis-1]); 
	m_err = flex_reset_pos(m_boardID, m_axis[axis-1], motor_posn_count, 0, 0xFF);
	if (m_err == 0)
		ResetLimits(axis, (i32) (m_fInitialFocus[axis-1]*m_convert[axis-1]));
	else
		CheckError(__LINE__);
	
	
}

void CNIMotionMotors::ResetLimits(u8 axis, i32 motor_home_count)
{
	//Set Software Limits in MAX every time reset position is ran
	i32 forwardLimit = m_pos_sw_lim_count[axis-1] + motor_home_count;
	i32 reverseLimit = motor_home_count - m_neg_sw_lim_count[axis-1];
	m_err = flex_load_sw_lim_pos(m_boardID, m_axis[axis-1], forwardLimit, reverseLimit, HOST);
	CheckError(__LINE__);
	
	CString sLimit;
	ConvertMotorToSubject(sLimit,axis,((float)forwardLimit)/m_convert[axis-1],1);
	m_pRange.sMax[axis-1] = sLimit;
	ConvertMotorToSubject(sLimit,axis,((float)reverseLimit)/m_convert[axis-1],1);
	m_pRange.sMin[axis-1] = sLimit;
}


bool CNIMotionMotors::ReadPosition(u8 axis, float &motor_posn_mm)
{
	i32 motor_posn_count;
	bool bProcess = false;

	//Read the current position of axis
	m_err = flex_read_pos_rtn(m_boardID, m_axis[axis-1], &motor_posn_count);
	CheckError(__LINE__);
	if (m_err == 0)
	{	
		motor_posn_mm = motor_posn_count/m_convert[axis-1];
		bProcess = true;
	}
	return bProcess;
}
bool CNIMotionMotors::MoveAxes(bool axes[3],float motor_posn_mm[3], i32 velocity[3], u8 opMode)
{
	bool bProcess = true;
	UINT uiErrType = 0;
	CString strError = _T("");
	// Set Operation Mode
	if (opMode!= NIMC_ABSOLUTE_POSITION && opMode!=NIMC_RELATIVE_POSITION)
	{
		bProcess = false;
		uiErrType = ERR_CODE_SOFTWARE;
		strError = _T("Invalid operation mode supplied");
	}
	if (bProcess)
	{
		for (int i=0; i<3 && bProcess; i++)
			if (axes[i])
			{
				m_err = flex_set_op_mode(m_boardID, m_axis[i], opMode);
				if (m_err!=0)
				{
					bProcess = false;
					uiErrType = ERR_CODE_MOTOR_MOVE_CFG;
					strError = _T("An error occurred while setting operation mode.");
				}
				CheckError(__LINE__);
			}			
	}
		
	// Load Position
	i32 motor_posn_count[3];
	for (int i=0; i<3 && bProcess; i++)
	{
		if(axes[i])
		{
			motor_posn_count[i] = (i32) (motor_posn_mm[i]*m_convert[i]); 
			m_err = flex_load_target_pos(m_boardID,m_axis[i],motor_posn_count[i],HOST);
			if (m_err!=0)
			{
				bProcess = false;
				uiErrType = ERR_CODE_MOTOR_MOVE_CFG;
				strError = _T("An error occurred while setting position.");
			}
			CheckError(__LINE__);
			// Default Velocity
			if (bProcess)
			{
				m_err = flex_load_velocity(m_boardID, m_axis[i], velocity[i], HOST);
				if (m_err!=0)
				{
					bProcess = false;
					uiErrType = ERR_CODE_MOTOR_MOVE_CFG;
					strError = _T("An error occurred while setting velocity.");
				}
				CheckError(__LINE__);
			}
		}
	}		
	
	if (bProcess)
	{
		m_ThreadAxis = NIMC_AXIS_CTRL;
		m_ThreadBitmap = 0;
		for(int i=0; i<3 && bProcess; i++)
		{
			// here we assume NIMC_AXIS1==1, NIMC_AXIS2==2, etc...
			m_ThreadBitmap = m_ThreadBitmap|(axes[i]<<m_axis[i]);
			// if "axes[i]" is TRUE, then "axes[i]<<m_axis[i]" simply enables axis "m_axis[i]"
			// for example if axes[2]==1 and m_axis[2]=NIMC_AXIS1, then axes[2]<<m_axis[2] == 1<<1 == b01<<1 == b10 -> axis 1 enabled
		}
		//AfxBeginThread(RunMoveThread,this);
		RunMoveThread(this);
	}

	if (!bProcess)
	{
		//NOTE: The calling public function will deal with status updates

		//Log this function's internal errors
		LogHandler.HandleError(uiErrType,__FUNCTIONW__,ERR_LOG,NULL,ERR_NO_MESSAGE,ERR_NO_BOX, ERR_LEVEL_NORMAL,strError);
	}

	return bProcess; 
}
bool CNIMotionMotors::ConfigVectSpace(u8 x, u8 y, u8 z, u8 vectorSpace)
{
	bool bProcess;
	if (x!=y && x!=z && y!=z && x<4 && y<4 && z<4 && !(x==0 && y==0 && z==0))
		bProcess = true;
	else
		bProcess = false;

	if (bProcess)
	{
		m_err = flex_config_vect_spc(m_boardID, vectorSpace, x,y,z);
		CheckError(__LINE__);
	}
	if (m_err!=0)
		bProcess = false;
	return bProcess;
}
bool CNIMotionMotors::MoveVectorSpace(u8 opMode, i32 velocity,
	u32 acceldecel, u16 sCurveTime,float motor_posn_mm[3]) {
		bool bProcess = true;
		UINT uiErrType = 0;
		CString strError = _T("");;

		int x = 1, y = 3, z = 0;
		if (!ConfigVectSpace(x,y,z,m_vectorSpace))
		{
			bProcess = false;
			uiErrType = ERR_CODE_MOTOR_MOVE_CFG;
			strError = _T("Vector space could not be configured.");
		}
		else
		{
			//once the vector space is configured set z to what it really is (AP)
			z = 2; // do this to correctly access the position array
			// Set Operation Mode
			if (opMode!= NIMC_ABSOLUTE_POSITION && opMode!=NIMC_RELATIVE_POSITION)
			{
				bProcess = false;
				uiErrType = ERR_CODE_SOFTWARE;
				strError = _T("Invalid operation mode supplied.");
			}
		}
	
		if (bProcess)
		{
			m_err = flex_set_op_mode(m_boardID, m_vectorSpace, opMode);
			if (m_err!=0)
			{
				bProcess = false;
				uiErrType = ERR_CODE_MOTOR_MOVE_CFG;
				strError = _T("An error occurred while setting operation mode.");
			}
			CheckError(__LINE__);
		}
		if (bProcess)
		{
			//Load velocity to the axis selected
			m_err = flex_load_velocity(m_boardID, m_vectorSpace, velocity, HOST);
			if (m_err!=0)
			{
				bProcess = false;
				uiErrType = ERR_CODE_MOTOR_MOVE_CFG;
				strError = _T("An error occurred while setting velocity.");
			}
			CheckError(__LINE__);
		}
		if (bProcess)
		{
			// Set the acceleration for the move (in counts/sec^2)
			m_err = flex_load_acceleration(m_boardID, m_vectorSpace, NIMC_BOTH, acceldecel, HOST);
			if (m_err!=0)
			{
				bProcess = false;
				uiErrType = ERR_CODE_MOTOR_MOVE_CFG;
				strError = _T("An error occurred while setting acceleration.");
			}
			CheckError(__LINE__);
		}
		if (bProcess)
		{
			// Set the jerk - scurve time (in sample periods)
			m_err = flex_load_scurve_time(m_boardID, m_vectorSpace, sCurveTime, HOST);
			if (m_err!=0)
			{
				bProcess = false;
				uiErrType = ERR_CODE_MOTOR_MOVE_CFG;
				strError = _T("An error occurred while setting s-curve time.");
			}
			CheckError(__LINE__);
		}

		if (bProcess)
		{
			i32 motor_posn_count[3];
			for (int i=0; i<3; i++)
				motor_posn_count[i] = (i32) (motor_posn_mm[i]*m_convert[i]); 

			// Load Vector Space Position
			m_err = flex_load_vs_pos(m_boardID, m_vectorSpace, motor_posn_count[x-1]/*x Position*/, motor_posn_count[y-1]/*y Position*/, motor_posn_count[z-1]/* z Position*/, HOST);
			if (m_err!=0)
			{
				bProcess = false;
				uiErrType = ERR_CODE_MOTOR_MOVE_CFG;
				strError = _T("An error occurred while setting position.");
			}
			CheckError(__LINE__);

		}
	
		if (bProcess)
		{
			m_ThreadAxis = NIMC_VECTOR_SPACE_CTRL; 
			m_ThreadBitmap = 0;
			//AfxBeginThread(RunMoveThread,this);
			RunMoveThread(this);
		}

		if (!bProcess)
		{
			//NOTE: The calling public function will deal with status updates

			//Log this function's internal errors
			LogHandler.HandleError(uiErrType,
				__FUNCTIONW__,
				ERR_LOG,
				NULL,
				ERR_NO_MESSAGE,
				ERR_NO_BOX,
				ERR_LEVEL_NORMAL,
				strError);
		}
		return bProcess; 

}
bool CNIMotionMotors::LoadMotionProfile(int axis_vs, i32 velocity,
	i32 accelDecel, int sCurveTime) {
	
		bool bProcess = true;

		//Load motion profile
		m_err = flex_load_velocity(m_boardID, axis_vs, velocity, HOST);
		if (m_err!= 0)
			bProcess = false;
		CheckError(__LINE__);

		if (bProcess)
			// Set the acceleration for the move (in counts/sec^2)
			m_err = flex_load_acceleration(m_boardID, axis_vs, NIMC_BOTH, accelDecel, HOST);
		if (m_err!=0)
			bProcess = false;
		CheckError(__LINE__);

		if (bProcess)
			// Set the jerk - scurve time (in sample periods)
			m_err = flex_load_scurve_time(m_boardID, axis_vs, sCurveTime, HOST);
		if (m_err!=0)
			bProcess = false;
		CheckError(__LINE__);
		return bProcess;
	}
    
	bool CNIMotionMotors::SetBreakpointOutput(u8 axis, u16 polarity)
	{
		bool bProcess = true;
		if (polarity == 0)
		{
			m_err = flex_configure_breakpoint_output(m_boardID, 
				m_axis[axis-1], 
				NIMC_ACTIVE_HIGH, 
				NIMC_OPEN_COLLECTOR);

			m_err = flex_set_breakpoint_output_momo(m_boardID,
				m_axis[axis-1], 1, 0, 0xFF);

		}
		else if (polarity == 1) {
			m_err = flex_configure_breakpoint_output(m_boardID,
				m_axis[axis-1], NIMC_ACTIVE_LOW, NIMC_OPEN_COLLECTOR);
		}
		if (m_err!=0)
			bProcess = false;
		CheckError(__LINE__);
		return bProcess;
}


//************************** MOTION THREAD *******************************//
UINT CNIMotionMotors::RunMoveThread(LPVOID p)
{
	
	bool bProcess = true;
	UINT uiErrType = 0;
	CString strError;

	CNIMotionMotors *me = (CNIMotionMotors*)(p);
	
	me->m_bRunning = true;
	
	if (me->m_bResumedSingle)
	{
		me->m_bThreadSingleWait = false;
		me->m_bResumedSingle = false;
	}

	if (!me->m_check_stop)
	{
		if (me->m_ThreadAxis == NIMC_VECTOR_SPACE_CTRL)
		{
			// Start the vector space move
			me->m_err = flex_start(me->m_boardID, me->m_vectorSpace, 0);
			if (me->m_err!=0)
			{
				bProcess = false;
				uiErrType = ERR_CODE_MOTOR_MOVE;
				strError = _T("An error occurred starting the VS move.");
			}
			
		}
		else if (me->m_ThreadAxis == NIMC_AXIS_CTRL)
		{
			// Start the axes move
			me->m_err = flex_start(me->m_boardID, NIMC_AXIS_CTRL, me->m_ThreadBitmap);
			if (me->m_err!=0)
			{
				bProcess = false;
				uiErrType = ERR_CODE_MOTOR_MOVE;
				strError = _T("An error occurred starting the Axes move.");
			}
			
		}
		else
		{
			// Start the single axis move
			me->m_err = flex_start(me->m_boardID, me->m_axis[me->m_SingleMotor_Axis-1], 0);
			if (me->m_err!=0)
			{
				bProcess = false;
				uiErrType = ERR_CODE_MOTOR_MOVE;
				strError = _T("An error occurred starting the Single Axis move.");
			}			
		}
		me->CheckError(__LINE__);
	}
	 	
	if (bProcess)
	{	
		CString* sMessage;
		sMessage = me->RunMoveThread();
		delete sMessage;
 
		if (me->m_bThreadSingleWait && me->m_threadSingle!=NULL)
		{			
			me->m_ThreadAxis = me->m_axis[me->m_SingleMotor_Axis-1];
			me->m_ThreadBitmap = 0;
			me->m_bResumedSingle = true;
			me->m_threadSingle->ResumeThread();
		}
	}
	
	if (!bProcess)
	{
		me->StopMotors(2);
		me->LogHandler.HandleError(uiErrType,
			__FUNCTIONW__,
			ERR_LOG,
			NULL,
			ERR_NO_MESSAGE,
			ERR_NO_BOX, 
			ERR_LEVEL_NORMAL,
			strError);
	}

	if (!me->m_bThreadSingleWait)
	{
		if (me->m_check_stop)
			me->m_check_stop = false;
	}		

	return 0;
}
CString* CNIMotionMotors::RunMoveThread()
{
	int iCheckSts = CheckStatus_Loop();
	CString *sMessage = new CString(_T(""));;
	switch (iCheckSts)
	{
		case Check_Unknown:
			*sMessage = _T("Unknown error occurred with motion card. Contact administrator.");
			break;
		case Check_MoveComplete:
			*sMessage = _T("Move complete");
			break;
		case Check_Stop:
			*sMessage = _T("Motion stopped");
			break;
		case Check_AxisTrip:
			*sMessage = _T("Limit reached");
			break;
		default:
			*sMessage = _T("Error during code processing. Contact administrator.");
			break;
	}
	if (iCheckSts!=Check_MoveComplete)
	{
		this->StopMotors(2);
		LogHandler.HandleError(ERR_CODE_MOTOR_MOVE,__FUNCTIONW__,ERR_LOG,NULL,ERR_NO_MESSAGE,ERR_NO_BOX, ERR_LEVEL_NORMAL,*sMessage);
	}
	return sMessage;		
}
UINT CNIMotionMotors::RunMultiTargetThread(LPVOID p)
{
	SetPriorityClass(GetCurrentProcess(),
		REALTIME_PRIORITY_CLASS);

	if (GetPriorityClass(GetCurrentProcess()) != REALTIME_PRIORITY_CLASS)
		cout << "ERROR: could not set process priority class to realtime." << 
		endl << "Timing will be compromised." << endl;

	// all parameters already loaded
	CNIMotionMotors *me = (CNIMotionMotors*)(p);
	me->m_bRunning = true;
	int x = 0, y = 2, z = 1; //(axes 1, 3, and 2 respectively)
	vector <int>::size_type iMaxPoints = me->m_vMotorPointsCounts[0].size();
	bool bProcess = false;
	CString *sMessage = new CString(_T(""));
	
	bProcess = me->Scan3D(iMaxPoints, x, y, z,
		sMessage,
		me->m_iCycles, 
		me->m_dDuration,
		me->m_iSonTime,
		me->m_BPointPolarity);
	
	if (!bProcess)
		cout << "Scan unsuccessful." << endl;

	//clean up
	delete sMessage;

	//Disable elapsed time updates
	if (me->m_bElapsedTime)
		me->SetElapsedTimeMessages(false,NULL,NULL);

	SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
	
	me->m_bRunning = false;
	return 0;
}

bool CNIMotionMotors::Scan3D(const UINT &iMaxPoints, const int &x, const int &y, const int &z, 
							CString* sMessage, const int iCycles, const double &dCycleDur, const int &iSonTime,
							const u16 &polarity)
{
	//NOTE: Even though the points come in as x, y, z, the vector space is 2D and z is ignored
	int iCheckSts;
	UINT iPoint = 0; 
	bool bProcess = true;
	int iCycle = 0;
	int iLastPointIndex = iMaxPoints - 1;
	u32 timeout = (u32)(m_dDuration/(double)m_iCycles)*1000 + 2000;

	LARGE_INTEGER _zero;
	_zero.QuadPart = 0;
	
	LARGE_INTEGER performanceFreq, tTemp0, tTempDuration;
	LARGE_INTEGER tstartLoop, tfinishLoop, tdurationLoop, tWaitUntilLoop;

	double time_secsLoop = 0, tElapsed_secs = 0;

	//Set all timingvariables to zero
	tTemp0 = _zero;			tTempDuration = _zero;
	tstartLoop = _zero;		tfinishLoop = _zero;	tdurationLoop = _zero;		tWaitUntilLoop = _zero;

	if (bProcess && !QueryPerformanceFrequency(&performanceFreq)) //Get performance frequency
	{
		AfxMessageBox(_T("ERROR: could not obtain performance counter frequency"));
		bProcess = false;
	}
	if (bProcess && !QueryPerformanceCounter(&tTemp0)) //Get the start time for the scan
	{
		AfxMessageBox(_T("ERROR: could not obtain performance counter time"));
		bProcess = false;
	}
	if(!QueryPerformanceCounter(&tstartLoop))//Get the start time for this cycle
	{
		AfxMessageBox(_T("ERROR: could not obtain performance counter time"));
		bProcess = false;
	}

	if (bProcess)
	{
		if (polarity == 1)
			SetBreakpointOutput(1,polarity);
		if (!PerformanceTimerMs(m_iSonTime,performanceFreq.QuadPart))
		{
			bProcess = false;
			AfxMessageBox(_T("ERROR: Could not obtain performance counter time and/or frequency"));
		}
		//Turn off breakpoint
		SetBreakpointOutput(1,0);
	}

	if (bProcess)
	{
		iPoint = 0;
		do
		{
			if (bProcess)
			{
				m_err = flex_load_vs_pos(m_boardID, m_vectorSpace, m_vMotorPointsCounts[x][iPoint], m_vMotorPointsCounts[y][iPoint], m_vMotorPointsCounts[z][iPoint], HOST);
				if (m_err!=0)
					bProcess = false;
				CheckError(__LINE__);
			}
			if (bProcess)
			{
				// Start the vector space move
				m_err = flex_start(m_boardID, m_vectorSpace, 0);
				if (m_err!=0)
					bProcess = false;
				CheckError(__LINE__);
			}								
			if (bProcess)
			{
				iCheckSts = 0;
				if (m_messageSts!=NULL && m_hWnd!=NULL)
				{
					iCheckSts = CheckVSpaceStatus_Loop(timeout);										
				}
				else 
				{
					bProcess = false;
					AfxMessageBox(_T("RunMultiMoveThread given a NULL reference. Contact administrator."));
				}
			}
			if (bProcess)
			{
				switch (iCheckSts)
				{
					case Check_Unknown:
						*sMessage = _T("Unknown error occurred with motion card");
						bProcess = false;
						break;
					case Check_MoveComplete:
						break;
					case Check_Stop:
						*sMessage = _T("Motion stopped");
						bProcess = false;
						break;
					case Check_AxisTrip:
						bProcess = false;
						*sMessage = _T("Limit reached on one of the axes");
						break;
					default:
						bProcess = false;
						*sMessage = _T("Error during code processing.");
						break;
				}
			}

			if (iCheckSts!= Check_MoveComplete)
				SendMessage(m_hWndSts,m_messageSts,(WPARAM) sMessage,NULL); 		
			
			if (bProcess && iPoint == iLastPointIndex)
			{
				iCycle++;
	
				if (bProcess && !QueryPerformanceCounter(&tfinishLoop)) //Get finish time
				{
					AfxMessageBox(_T("ERROR: could not obtain performance counter time"));
					bProcess = false;
				}

				if(bProcess)
				{
					//Calculate until when to wait
					tWaitUntilLoop.QuadPart = ((_int64)((dCycleDur)*((double)
						(performanceFreq.QuadPart))))*((_int64)iCycle) + 
						tTemp0.QuadPart;
					CString sTemp;

					//Check if scan successful
					if (tfinishLoop.QuadPart < tWaitUntilLoop.QuadPart) //made the scan time
					{	
						//wait
						WaitUntilTime(tWaitUntilLoop.QuadPart);	

						//Get finish time again
						if (!QueryPerformanceCounter(&tfinishLoop))
						{
							AfxMessageBox(_T("ERROR: could not obtain performance counter time"));
							bProcess = false;
						}				
						sTemp = _T("Success..Previous Scan period: %0.2f sec");
					}
					else
					{									
						sTemp = _T("Error!! Previous Scan period: %0.2f sec");												
						bProcess = false;							
					}	
					tdurationLoop.QuadPart = (tfinishLoop.QuadPart - tstartLoop.QuadPart);
					time_secsLoop = ((double)(tdurationLoop.QuadPart))/((double)(performanceFreq.QuadPart));
					tTempDuration.QuadPart = tfinishLoop.QuadPart - tTemp0.QuadPart;
					tElapsed_secs = ((double)(tTempDuration.QuadPart))/((double)(performanceFreq.QuadPart));
					sMessage->Format(sTemp,time_secsLoop);
					SendMessage(m_hWndSts,m_messageSts,(WPARAM) sMessage,NULL); 	
					//can reuse the same message
					if (m_bElapsedTime)
					{
						sMessage->Format(_T("%0.2f"),tElapsed_secs);
						SendMessage(m_hWndETime,m_messageElapsedTime,(WPARAM) sMessage,NULL); 
					}
					iPoint = 0;	
					if (bProcess)
					{
						if(!QueryPerformanceCounter(&tstartLoop))//Get the start time for this cycle
						{
							AfxMessageBox(_T("ERROR: could not obtain performance counter time"));
							bProcess = false;
						}
					}
				}
			}// end if bProcess and iPoint = iLastPointIndex	
			else					
				iPoint++;

			if (bProcess)
			{
				if (polarity == 1)
					SetBreakpointOutput(1,polarity);
				if (!PerformanceTimerMs(m_iSonTime,performanceFreq.QuadPart))
				{
					bProcess = false;
					AfxMessageBox(_T("ERROR: Could not obtain performance counter time and/or frequency"));
				}
				SetBreakpointOutput(1,0);
			}			
		}while(bProcess && iCycle!=iCycles);
	}

	if (!bProcess)
		StopMotors(2);
	return bProcess;
}
int CNIMotionMotors::CheckStatus_Loop()
{
	u16 bitmap = 0;
	if (m_ThreadAxis == NIMC_AXIS_CTRL)
		bitmap = m_ThreadBitmap;

	m_axisStatus = 0;
	m_moveComplete = 0;
	u16 status;

	while (!m_moveComplete && !(m_axisStatus & NIMC_FOLLOWING_ERROR_BIT)
		&& !(m_axisStatus & NIMC_AXIS_OFF_BIT) && (!m_check_stop))
	{
		m_axisStatus = 0;

		// Check the move complete status
		// For axes -> bitmap must be specified; For vector space, bitmap is 0
		// m_ThreadAxis -> specifies if axis, vector space, or many axes
		if (m_ThreadAxis == NIMC_VECTOR_SPACE_CTRL)
			m_err = flex_check_move_complete_status(m_boardID, m_vectorSpace, bitmap, &m_moveComplete);
		else
			m_err = flex_check_move_complete_status(m_boardID, m_ThreadAxis, bitmap, &m_moveComplete);

		CheckError(__LINE__);
		
		// Check the following error/axis off status 
		if (m_ThreadAxis == NIMC_AXIS_CTRL)
		// For axis control, must read each axis separately
		{
			int iMask = 2;
			status = 0;
			for (int i = 1; i < 4; i++,iMask<<1)
			{
				if (m_active_axes[i-1] || m_manual_axes[i-1])
				{
					if (iMask & bitmap)
					{
						m_err = flex_read_axis_status_rtn(m_boardID,m_axis[i-1],&status);
						CheckError(__LINE__);	
						m_axisStatus |= status;
						
						// Simulate periodic breakpoint w/ modulo breakpoint by resetting on trigger
						if (m_periodicBreakpointEnabled[i-1])
						{
							if(status & NIMC_POS_BREAKPOINT_BIT)
								SetPeriodicBreakpointEnabled(i, true);
						}
					}
				}
			}
		}
		else if (m_ThreadAxis == NIMC_VECTOR_SPACE_CTRL)
		{
			status = 0;
			for (int i=1; i<4; i++)
			{
				if (m_active_axes[i-1] || m_manual_axes[i-1])
				{
					m_err = flex_read_axis_status_rtn(m_boardID, m_axis[i-1], &status);
					CheckError(__LINE__);
					m_axisStatus |= status;
					
					// Simulate periodic breakpoint w/ modulo breakpoint by resetting on trigger
					if (m_periodicBreakpointEnabled[i-1])
					{
						if (status & NIMC_POS_BREAKPOINT_BIT)
							SetPeriodicBreakpointEnabled(i, true);
					}
				}
			}
		}
		else
		{	
			m_err = flex_read_axis_status_rtn(m_boardID, m_ThreadAxis, &m_axisStatus);
			CheckError(__LINE__);
		}

		// Read the communication status register and check the modal errors
		m_err = flex_read_csr_rtn(m_boardID, &m_csr);
		
		// Check the modal errors
		if (m_csr & NIMC_MODAL_ERROR_MSG)
		{
			m_err = m_csr & NIMC_MODAL_ERROR_MSG;
			CheckError(__LINE__);
		}
	}

	int iReturn = 0;
	if (m_moveComplete == 1)
		iReturn = Check_MoveComplete;
	else if (this->m_check_stop)
		iReturn = Check_Stop;
	else if ((m_axisStatus & (NIMC_FOLLOWING_ERROR_BIT)) || (m_axisStatus & (NIMC_FOLLOWING_ERROR_BIT)))
		iReturn = Check_AxisTrip;
	else 
		iReturn = Check_Unknown;
	return iReturn;
	
}
int CNIMotionMotors::CheckBlendStatus_Loop()
{
	u16 bitmap = 0;
	m_axisStatus = 0;
	m_moveComplete = 0;
	u16 status;
	u16 moveComplete = 0;
	bool output = true;
	while (!m_moveComplete && !(m_axisStatus & NIMC_FOLLOWING_ERROR_BIT)
		&& !(m_axisStatus & NIMC_AXIS_OFF_BIT) && (!m_check_stop))
	{
		m_axisStatus = 0;
		
		m_err = flex_check_blend_complete_status(m_boardID, m_vectorSpace, 0, &m_moveComplete);
		CheckError(__LINE__);
		
		status = 0;
		for (int i=1; i<4; i++)
		{
			if (m_active_axes[i-1] || m_manual_axes[i-1])
			{
				m_err = flex_read_axis_status_rtn(m_boardID, m_axis[i-1], &status);
				CheckError(__LINE__);
				m_axisStatus |= status;
			}
		}
		
		// Read the communication status register and check the modal errors
		m_err = flex_read_csr_rtn(m_boardID, &m_csr);
		CheckError(__LINE__);

		// Check the modal errors
		if (m_csr & NIMC_MODAL_ERROR_MSG)
		{
			m_err = m_csr & NIMC_MODAL_ERROR_MSG;
			CheckError(__LINE__);
		}
	}

	int iReturn = 0;
	SetBreakpointOutput(1,0);
	if (m_moveComplete == 1)
		iReturn = Check_MoveComplete;
	else if (this->m_check_stop)
		iReturn = Check_Stop;
	else if ((m_axisStatus & (NIMC_FOLLOWING_ERROR_BIT)) || (m_axisStatus & (NIMC_FOLLOWING_ERROR_BIT)))
		iReturn = Check_AxisTrip;
	else 
		iReturn = Check_Unknown;

	return iReturn;

	
}
int CNIMotionMotors::CheckVSpaceStatus_Loop(const u32 &timeout)
{
	u16 status;
	bool output = true;
	bool bProcess = true;
	m_moveComplete = 0;

	m_err = flex_wait_for_move_complete(m_boardID, m_vectorSpace, 0, timeout, 10, &m_moveComplete);
	if (m_err!=0)
		bProcess = false;
	CheckError(__LINE__);

	if (bProcess)
	{
		m_axisStatus = 0;			
		int i = 1;
		do
		{
			status = 0;
			m_err = flex_read_axis_status_rtn(m_boardID, m_axis[i], &status);
			if (m_err!=0)
				bProcess = false;
			CheckError(__LINE__);
			m_axisStatus |= status;
			i++;
		}while (i<4 && bProcess);
	}

	// Read the communication status register and check the modal errors
	m_err = flex_read_csr_rtn(m_boardID, &m_csr);
	if (m_err!=0)
		bProcess = false;
	CheckError(__LINE__);

	// Check the modal errors
	if (m_csr & NIMC_MODAL_ERROR_MSG)
	{
		m_err = m_csr & NIMC_MODAL_ERROR_MSG;
		if (m_err!=0)
			bProcess = false;
		CheckError(__LINE__);
	}

	int iReturn = 0;
	if (!bProcess)
		m_moveComplete = 0;

	if ((m_axisStatus & (NIMC_FOLLOWING_ERROR_BIT)) || (m_axisStatus & (NIMC_FOLLOWING_ERROR_BIT)))
		iReturn = Check_AxisTrip;
	else if (this->m_check_stop)
		iReturn = Check_Stop;
	else if (m_moveComplete == 1)
		iReturn = Check_MoveComplete;
	else 
		iReturn = Check_Unknown;

	return iReturn;	
}

//************************** MOTION STS UPDATES *******************************//
void CNIMotionMotors::EnableMotionStsUpdates(HWND hWnd, UINT message)
{
	m_hWndSts = hWnd;
	m_messageSts = message;
}
void CNIMotionMotors::DisableMotionStsUpdates()
{
	m_hWndSts = NULL;
	m_messageSts = NULL;
}
//************************** POSITION UPDATES *******************************//
void CNIMotionMotors::EnableUpdates(HWND hWnd, UINT message)
{
	m_hWnd = hWnd;
	m_message = message;
	m_updatesenabled = true;
}
void CNIMotionMotors::StopUpdates()
{
	m_updatesenabled = false;
	m_hWnd = NULL;
	m_message = NULL;
}
void CNIMotionMotors::GetUpdate()
{
	CStringArray  sPosn;
	bool bProcess = true;

	if ((m_updatesenabled)&&(m_hWnd))
	{	
		bProcess = this->ReadCurrentPosition(sPosn);
		if (bProcess)
		{
			m_spos.sLRpos = sPosn[0];
			m_spos.sAPpos = sPosn[1];
			m_spos.sSIpos = sPosn[2];
		}

		//Also check for modal errors unless homing is in progress
		if (!m_bHoming && !m_bRunning)
		{
			// Read the communication status register and check the modal errors
			m_err = flex_read_csr_rtn(m_boardID, &m_csr);
			if (m_err!=0)
			{
				LogHandler.HandleError(ERR_CODE_MOTOR_READ_STS,__FUNCTIONW__,ERR_LOG,NULL,ERR_NO_MESSAGE,ERR_NO_BOX, ERR_LEVEL_NORMAL,_T("Error reading comm status register. "));
			}
			// Check for modal errors
			if (m_csr & NIMC_MODAL_ERROR_MSG)
			{
				m_err = m_csr & NIMC_MODAL_ERROR_MSG;
				CheckError_Home();
			}
		}
	}
}


//************************** HOME UPDATES **************************//
void CNIMotionMotors::EnableHomeStsUpdates(HWND hWnd, UINT message)
{
	m_hWndHomeSts = hWnd;
	m_messageHomeSts = message;
}
void CNIMotionMotors::DisableHomeStsUpdates()
{
	m_hWndHomeSts = NULL;
	m_messageHomeSts = NULL;
}
//************************** INITIAL SETUP CONTROLS **************************//
void CNIMotionMotors::SetIniBasicCtrlMessages(bool bEnable, HWND hWnd, UINT messageEnable, UINT messageDisable)
{
	m_hWndIni = hWnd;
	m_messageIniBasicEnable = messageEnable;
	m_messageIniBasicDisable = messageDisable;
	m_bIniBasic = bEnable;
	
}
void CNIMotionMotors::SetIniSubjectCtrlMessages(bool bEnable, HWND hWnd, UINT messageEnable, UINT messageDisable)
{
	m_hWndIni = hWnd;
	m_messageIniSubjectEnable = messageEnable;
	m_messageIniSubjectDisable = messageDisable;
	m_bIniSubject = bEnable;
}
void CNIMotionMotors::SetIniFocusCtrlMessages(bool bEnable, HWND hWnd, UINT messageEnable, UINT messageDisable)
{
	m_hWndIni = hWnd;
	m_messageIniFocusEnable = messageEnable;
	m_messageIniFocusDisable = messageDisable;
	m_bIniFocus = bEnable;
}
//************************** TAB CONTROLS UPDATES **************************//
void CNIMotionMotors::SetTabsCtrlMessages(HWND hWnd, UINT messageEnable, UINT messageDisable)
{
	m_bTabs = true;
	m_hWndTabs = hWnd;
	m_messageEnable = messageEnable;
	m_messageDisable = messageDisable;
}
void CNIMotionMotors::SetIndivTabCtrlMessages(HWND hWnd, UINT messageEnable, UINT messageDisable)
{
	m_bIndividTab = true;
	m_hWndIndivTab = hWnd;
	m_messageIndivTabEnable = messageEnable;
	m_messageIndivTabDisable = messageDisable;
}
void CNIMotionMotors::SetElapsedTimeMessages(bool bEnable, HWND hWnd, UINT message)
{
	m_bElapsedTime = bEnable;
	m_hWndETime = hWnd;
	m_messageElapsedTime = message;
}
bool CNIMotionMotors::ValidatePosition(CString str[3], CStringArray &sPosn)
{
	// Input: array of positions, output CStringArray of lowered positions and a validation flag
	bool bProcess = false;
	
	for (int i=0; i<3; i++)
		str[i].MakeLower();
	
	if ((str[0].Find('l',0)==0 || str[0].Find('r',0)==0 || !m_active_axes[0]) && 
		(str[1].Find('a',0)==0 || str[1].Find('p',0)==0 || !m_active_axes[1]) && 
		(str[2].Find('s',0)==0 || str[2].Find('i',0)==0 || !m_active_axes[2])) 
	{
		if (sPosn.GetSize()<3)
			sPosn.SetSize(3);
		for (int i= 0; i<3; i++)
			sPosn.SetAt(i,str[i]);

		bProcess = true;
	}

	if (bProcess)
		bProcess = IsPosnWithinLimits(sPosn);

	return bProcess;
}

bool CNIMotionMotors::IsPosnWithinLimits(CStringArray &sPosn)
{
	float motor_posn_mm, range_max_mm, range_min_mm;
	int motor_axis = 0;
	bool bProcess = true;
	for (int i=0; i<3 && bProcess; i++)
	{
		if (m_active_axes[i])
		{
			motor_posn_mm = 0.0;
			bProcess = ConvertSubjectToMotor(sPosn[i], motor_axis, motor_posn_mm);
			if (bProcess)
				bProcess = ConvertSubjectToMotor(m_pRange.sMax[i], motor_axis, range_max_mm);

			if (bProcess)
				bProcess = ConvertSubjectToMotor(m_pRange.sMin[i], motor_axis, range_min_mm);
			if (bProcess)
				if (motor_posn_mm > range_max_mm || motor_posn_mm < range_min_mm)
					bProcess = false;		
		}
	}
	return bProcess;
}

bool CNIMotionMotors::MustHomeSystem()
{
	return m_bAuto_Home_All;
}

structRange CNIMotionMotors::GetRanges()
{
	return m_pRange;
}
structPosition CNIMotionMotors::GetFocus()
{
	structPosition pos;
	ConvertMotorToSubject(pos.sLRpos,1,m_fInitialFocus[0],1);
	ConvertMotorToSubject(pos.sAPpos,2,m_fInitialFocus[1],1);
	ConvertMotorToSubject(pos.sSIpos,3,m_fInitialFocus[2],1);
	return pos;
	
}

bool CNIMotionMotors::compatibleMoveToTarget(char *LR_str, char *SI_str) {
	
	CString motorMovements[3] = {
			CString(LR_str),
			CString("A0.0"),
			CString(SI_str)
	};

	int errorCheck = 0;

	CStringArray motorMovementsStr;
	errorCheck = this->ValidatePosition(motorMovements, 
		motorMovementsStr);
	if (!errorCheck)
		return false;

	errorCheck = this->MoveToTarget(motorMovementsStr);
	if (!errorCheck)
		return false;

	return true;
}

bool CNIMotionMotors::compatibleReadCurrentPosition(double *returnValues){
	// we assume the parameter returnValues is a two element array
	
	CStringArray thePosition;
	thePosition.SetSize(3);
	
	bool success = this->ReadCurrentPosition(thePosition);
	if(!success){
		return false;
	}
	
	char strPtr[100] = {0};
	// we assume 100 characters is enough
	
	// starting R/L value
	
	wcstombs(strPtr, 
		thePosition[0].GetBuffer(thePosition[0].GetLength()),
		sizeof(strPtr)-1);
	thePosition[0].ReleaseBuffer();
	
	if(strlen(strPtr) == 0){
		// the call to ReadCurrentPosition didn't fill in the value
		// did you call initAxes first?
		return false;
	}
	returnValues[0] = atof(strPtr+1);
	
	if(thePosition[0].GetAt(0) == 'L' || thePosition[0].GetAt(0) == 'l'){
		returnValues[0] *= -1;
	}
	
	// starting S/I value
	
	wcstombs(strPtr, 
		thePosition[2].GetBuffer(thePosition[2].GetLength()),
		sizeof(strPtr)-1);
	thePosition[2].ReleaseBuffer();
	
	if(strlen(strPtr) == 0){
		// the call to ReadCurrentPosition didn't fill in the value
		// did you call initAxes first?
		return false;
	}
	returnValues[1] = atof(strPtr+1);

	if(thePosition[2].GetAt(0) == 'I' || thePosition[2].GetAt(0) == 'i'){
		returnValues[1] *= -1;
	}

	return true;
}


bool CNIMotionMotors::compatibleGetRanges(double* returnValues){
	// we assume the parameter returnValues is a two element array
	
	structRange theRange = this->GetRanges();
	
	char strPtr[100] = {0};
	// we assume 100 characters is enough
	
	// starting R/L max
	
	wcstombs(strPtr, 
		theRange.sMax[0].GetBuffer(theRange.sMax[0].GetLength()),
		sizeof(strPtr)-1);
	theRange.sMax[0].ReleaseBuffer();
	
	if(strlen(strPtr) == 0){
		// the call to GetRanges didn't fill in the value
		return false;
	}
	returnValues[0] = atof(strPtr+1);
	
	if(theRange.sMax[0].GetAt(0) == 'L' || theRange.sMax[0].GetAt(0) == 'l'){
		returnValues[0] *= -1;
	}
	
	// starting R/L min
	
	wcstombs(strPtr, 
		theRange.sMin[0].GetBuffer(theRange.sMin[0].GetLength()),
		sizeof(strPtr)-1);
	theRange.sMin[0].ReleaseBuffer();

	if(strlen(strPtr) == 0){
		// the call to GetRanges didn't fill in the value
		return false;
	}
	returnValues[1] = atof(strPtr+1);
	
	if(theRange.sMin[0].GetAt(0) == 'L' || theRange.sMin[0].GetAt(0) == 'l'){
		returnValues[1] *= -1;
	}

	// starting S/I max
	
	wcstombs(strPtr, 
		theRange.sMax[2].GetBuffer(theRange.sMax[2].GetLength()),
		sizeof(strPtr)-1);
	theRange.sMax[2].ReleaseBuffer();

	if(strlen(strPtr) == 0){
		// the call to GetRanges didn't fill in the value
		return false;
	}
	returnValues[2] = atof(strPtr+1);
	
	if(theRange.sMax[2].GetAt(0) == 'I' || theRange.sMax[2].GetAt(0) == 'i'){
		returnValues[2] *= -1;
	}

	// starting S/I min
	
	wcstombs(strPtr, 
		theRange.sMin[2].GetBuffer(theRange.sMin[2].GetLength()),
		sizeof(strPtr)-1);
	theRange.sMin[2].ReleaseBuffer();

	if(strlen(strPtr) == 0){
		// the call to GetRanges didn't fill in the value
		return false;
	}
	returnValues[3] = atof(strPtr+1);
	
	if(theRange.sMin[2].GetAt(0) == 'I' || theRange.sMin[2].GetAt(0) == 'i'){
		returnValues[3] *= -1;
	}

	return true;
}

bool CNIMotionMotors::compatibleResetCurrentFocus(char *LR_str , char *SI_str)
{
	CStringArray initialFocus;
	initialFocus.SetSize(3);
	initialFocus[0] = CString(LR_str);
	initialFocus[1] = CString("A0.0");
	initialFocus[2] = CString(SI_str);

	return this->ResetCurrentFocus(initialFocus);
}

bool CNIMotionMotors::compatibleResetInitialFocus(char *LR_str , char *SI_str)
{
	CStringArray initialFocus;
	initialFocus.SetSize(3);
	initialFocus[0] = CString(LR_str);
	initialFocus[1] = CString("A0.0");
	initialFocus[2] = CString(SI_str);

	return this->ResetInitialFocus(initialFocus);
}

    bool CNIMotionMotors::SetPeriodicBreakpoint(float period_mm, CString mri_position)
    {
        float position_mm = 0;
        int axis = 0;
        
        // Convert MRI position to {axis, position in mm}
        if (!ConvertSubjectToMotor(mri_position, axis, position_mm)) return false;
        
        return SetPeriodicBreakpoint(axis, period_mm, position_mm);
    }
    
    bool CNIMotionMotors::SetPeriodicBreakpoint(u8 axis, float period_mm)
    {
        if (axis < 1 || axis > 3) return false; // Bad axis
        if (!m_active_axes[axis-1] && !m_manual_axes[axis-1]) return false; // Axis is not used
        
        // Use current position
        i32 position = 0;
        float position_mm = 0.0;
        
        m_err = flex_read_pos_rtn(m_boardID, m_axis[axis-1], &position);
        CheckError(__LINE__);
       
        position_mm = ((float)position) / m_convert[axis-1];
        
        return SetPeriodicBreakpoint(axis, period_mm, position_mm);
    }
    
    bool CNIMotionMotors::SetPeriodicBreakpoint(u8 axis, float period_mm, float position_mm, u32 window)
    {
        if (axis < 1 || axis > 3) return false; // Bad axis
        if (!m_active_axes[axis-1] && !m_manual_axes[axis-1]) return false; // Axis is not used
		
		SetPeriodicBreakpointEnabled(false); // Disable the breakpoint while resetting

        // Convert from mm to steps
        period_mm = fabs(period_mm);
        u32 period = (u32) (period_mm * m_convert[axis-1]);
        i32 position = (i32) (position_mm * m_convert[axis-1]);

        // Force position to be < period
        position = position % ((i32)period);

        // Route breakpoint 1 to RTSI line 1
        m_err = flex_select_signal (m_boardID, NIMC_RTSI1, NIMC_BREAKPOINT1);
        CheckError(__LINE__);
        
        // Configure Breakpoint
        m_err = flex_configure_breakpoint(m_boardID, m_axis[axis-1], NIMC_MODULO_BREAKPOINT, NIMC_SET_BREAKPOINT, NIMC_OPERATION_SINGLE);
        CheckError(__LINE__);
        
        // Set the breakpoint window
        m_err = flex_setu32(m_boardID, m_axisEncoder[axis-1], NIMC_BP_WINDOW, window);
        CheckError(__LINE__);

        // Load Breakpoint Modulus - repeat period
        m_err = flex_load_bp_modulus(m_boardID, m_axis[axis-1], period, 0xFF);
        CheckError(__LINE__);
        
        // Set the modulo breakpoint position
        m_err = flex_load_pos_bp(m_boardID, m_axis[axis-1], position, 0xFF);
        CheckError(__LINE__);
        

        return true; // TODO check errorCode
    }
    
    bool CNIMotionMotors::SetPeriodicBreakpointEnabled(u8 axis, bool enable)
    {
        if (axis < 1 || axis > 3) return false; // Bad axis
        if (!m_active_axes[axis-1] && !m_manual_axes[axis-1]) return false; // Axis is not used
                
        // Remember enabledness
        m_periodicBreakpointEnabled[axis-1] = enable;
        
        if (enable)
        {
            // Enable Breakpoint
            m_err = flex_enable_breakpoint(m_boardID, m_axis[axis-1], NIMC_TRUE);
            CheckError(__LINE__);
        }
        else
        {
            // Disable Breakpoint
            m_err = flex_enable_breakpoint(m_boardID, m_axis[axis-1], NIMC_FALSE);
            CheckError(__LINE__);
        }
        
        return true; // TODO check errorCode
    }
