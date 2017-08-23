#pragma once
#if !defined LogHandler_H
#define LogHandler_H

#define ERR_CODE_SUCCESS			1000
#define ERR_CODE_UNKNOWN			ERR_CODE_SUCCESS+1	
#define ERR_CODE_FILEOP				ERR_CODE_UNKNOWN+1
#define	ERR_CODE_MOTOR_INI			ERR_CODE_FILEOP+1
#define	ERR_CODE_MOTOR_NI			ERR_CODE_MOTOR_INI+1
#define	ERR_CODE_SOFTWARE			ERR_CODE_MOTOR_NI+1
#define	ERR_CODE_MOTOR_READ_STS		ERR_CODE_SOFTWARE+1
#define	ERR_CODE_MOTOR_HOME			ERR_CODE_MOTOR_READ_STS+1
#define	ERR_CODE_USER_ENTRY			ERR_CODE_MOTOR_HOME+1
#define ERR_CODE_MOTOR_MOVE_CFG		ERR_CODE_USER_ENTRY+1		//internal logging
#define	ERR_CODE_MOTOR_MOVE			ERR_CODE_MOTOR_MOVE_CFG+1	//external status box
#define	ERR_CODE_FGEN				ERR_CODE_MOTOR_MOVE+1	//ALL FGEN RELATED errors

#define ERR_BOX					1
#define ERR_NO_BOX				0
#define	ERR_NO_MESSAGE			0
#define ERR_NO_LOG				0
#define ERR_LOG					1

#define	ERR_LEVEL_CLOSING		2000
#define	ERR_LEVEL_NORMAL		2001

#define INFO_APP_START			3000
#define	INFO_APP_EXIT			INFO_APP_START + 1
#define	INFO_APP_ABORT			INFO_APP_EXIT + 1
#define	INFO_ERROR_GUI			INFO_APP_ABORT + 1
#define INFO_ERROR_MOTOR		INFO_ERROR_GUI + 1
#define INFO_ERROR_SOFTWARE		INFO_ERROR_MOTOR + 1
#define INFO_ERROR_NI			INFO_ERROR_SOFTWARE + 1
#define INFO_ERROR_FGEN			INFO_ERROR_NI + 1

class CLogHandler
{
	CStdioFile m_fileLog; // Log file pointer
	CString m_fileName;
	

public:
	CLogHandler(void);
	~CLogHandler(void);

	UINT Initialize(); // Sets the file name upon initialization 
	
	void HandleError(UINT uiError, CString sFunc, UINT uiLog = ERR_NO_LOG, HWND hwndSts = NULL,UINT uiSendMsg = 0,UINT uiAfxBox = 0, UINT uiClosing = ERR_LEVEL_NORMAL, CString sOptionalMsg = _T(""), int line_num = -1);

	UINT LogInfo(UINT uiInfoType, CString sOptionalMsg = _T(""));
	

	
};

#endif