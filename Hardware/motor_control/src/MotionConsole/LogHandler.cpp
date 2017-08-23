#include "StdAfx.h"
#include "LogHandler.h"
#include "afx.h"



CLogHandler::CLogHandler(void)
{
	
	m_fileName = _T("");
	Initialize();
}

CLogHandler::~CLogHandler(void)
{

}

UINT CLogHandler::Initialize() // Sets the file name upon initialization 
{
	UINT returnCode = ERR_CODE_SUCCESS;
	try
	{
		CTime t = CTime::GetCurrentTime();
		m_fileName = t.Format("Error Log %Y-%m-%d.txt");
		if (m_fileLog.Open(m_fileName, CFile::modeRead))
			m_fileLog.Close();
		else if (m_fileLog.Open (m_fileName, CFile::modeCreate|CFile::modeWrite)) {
			wcout << "File: " << m_fileName.GetString() << " Created Successfully" << endl;
			m_fileLog.Close();
		}
		else {
			returnCode = ERR_CODE_FILEOP;
		}
	}
	catch(CException *e)
	{
		delete e;
		returnCode = ERR_CODE_UNKNOWN;
	}
	if (returnCode!=ERR_CODE_SUCCESS)
		HandleError(returnCode,__FUNCTIONW__ , ERR_NO_LOG, NULL,ERR_NO_MESSAGE,ERR_BOX, ERR_LEVEL_CLOSING,m_fileName);
	return returnCode;
}

void CLogHandler::HandleError(UINT uiError, CString sFunc, UINT uiLog, HWND hwndSts, UINT uiSendMsg,  UINT uiAfxBox, UINT uiClosing, CString sOptionalMsg, int line_num)
{
	CString sTimeStamp = CTime::GetCurrentTime().Format("[%Y-%m-%d %H:%M:%S]");

	//Print a simple error message to the screen
//	wcout << "ERROR: " << sOptionalMsg.GetString() << endl;
	wcout << "[LINENUM=" << line_num << "] ";
	wcout << sTimeStamp << " ERROR: " << sOptionalMsg.GetString() << endl;

//	CString sErrMessage = _T("Error: ");
	CString sErrMessage =  sTimeStamp + _T(" Error: ");

	UINT uiErrType;
	switch (uiError)
	{
	case ERR_CODE_UNKNOWN:
		sErrMessage += "Unknown error occurred. Contact the software administrator. ";
		uiErrType = INFO_ERROR_SOFTWARE;
		break;
	case ERR_CODE_FILEOP:
		sErrMessage += "An error occurred while opening a file. ";
		if (sOptionalMsg.IsEmpty()==0)
			sErrMessage += "\nFile: ";
		uiErrType = INFO_ERROR_SOFTWARE;
		break;
	case ERR_CODE_MOTOR_INI:
		sErrMessage += "An error occurred while initializing the motion card. ";
		uiErrType = INFO_ERROR_MOTOR;
		break;
	case ERR_CODE_MOTOR_NI:
		uiErrType = INFO_ERROR_NI;
		break;
	case ERR_CODE_MOTOR_MOVE_CFG:
		uiErrType = INFO_ERROR_MOTOR;
		sErrMessage += "Move configuration error. ";
		break;
	case ERR_CODE_SOFTWARE:
		uiErrType = INFO_ERROR_SOFTWARE;
		break;	
	case ERR_CODE_MOTOR_MOVE: //for all motion status updates
		uiErrType = INFO_ERROR_MOTOR;
		break;
	case ERR_CODE_MOTOR_HOME:
		uiErrType = INFO_ERROR_MOTOR;
		sErrMessage += "Homing error ocurred. ";
		break;	
	case ERR_CODE_USER_ENTRY:
		uiLog = ERR_NO_LOG;
		sErrMessage += "Invalid Entry.\n\n";
		break;
	case ERR_CODE_FGEN:
		sErrMessage = "Warning: Problem communicating with Function Generator. ";
		uiErrType = INFO_ERROR_FGEN;
		break;
	default:
		sErrMessage += "Error handler received undefined error. Contact the software administrator. ";
		uiErrType = INFO_ERROR_SOFTWARE;
		break;
		
	}
	if (sOptionalMsg.IsEmpty()==0)
	{
		sErrMessage+= sOptionalMsg;
	}
	if (sFunc.IsEmpty()==0)
		sErrMessage += _T("\n\nFunction: ") + sFunc;

	if (uiLog == ERR_LOG)
		LogInfo(uiErrType, sErrMessage);

	if (uiClosing == ERR_LEVEL_CLOSING)
		sErrMessage += _T("\n\nTHE PROGRAM MUST CLOSE. ");


	if (uiSendMsg!=ERR_NO_MESSAGE)
	{
		if (uiError == ERR_CODE_MOTOR_HOME && sOptionalMsg.IsEmpty() == 0) {
			cout << "Problem with homing motors" << endl;
		}
		else {
			cout << "Problem homing motors" << endl;
		}
	}

	//Changed to print to wcout
	if (uiAfxBox == ERR_BOX)
		wcout << sErrMessage.GetString() << endl;
		
}


UINT CLogHandler::LogInfo(UINT uiInfoType, CString sOptionalMsg )
{
	CString sLog;
	UINT returnCode = ERR_CODE_SUCCESS;
	CTime t = CTime::GetCurrentTime();
	sLog = t.Format("\n%Y/%m/%d %H:%M:%S ");
	if (sOptionalMsg.IsEmpty()==0)
		sOptionalMsg.Replace('\n',' ');
	switch (uiInfoType)
	{
	case INFO_APP_START:
		sLog += _T("APPLICATION START");
		break;
	case INFO_APP_EXIT:
		sLog += _T("APPLICATION EXIT ");
		if (sOptionalMsg.IsEmpty()!=0)
			sLog += _T("\n");
		else
			sOptionalMsg += _T("\n");
		break;
	case INFO_APP_ABORT:
		sLog += _T("APPLICATION ABORT ");
		if (sOptionalMsg.IsEmpty()!=0)
			sLog += _T("\n");
		else
			sOptionalMsg += _T("\n");
		break;
	case INFO_ERROR_GUI:
		sLog += _T("ERROR GUI ");
		break;
	case INFO_ERROR_MOTOR:
		sLog += _T("ERROR MOTOR ");
		break;
	case INFO_ERROR_SOFTWARE:
		sLog += _T("ERROR SOFTWARE ");
		break;
	case INFO_ERROR_NI:
		sLog += _T("ERROR NI MOTION ");
		break;
	default:
		sLog = _T(" ");
		break;
	}
	sLog += sOptionalMsg;

	if (sLog.IsEmpty()==0)
	{
		if(m_fileLog.Open (m_fileName, CFile::modeWrite))
		{
			m_fileLog.SeekToEnd();
			m_fileLog.WriteString(sLog);
			m_fileLog.Close();
		}
		else
		{
			returnCode = ERR_CODE_FILEOP;
			HandleError(returnCode,__FUNCTIONW__ ,ERR_NO_LOG,NULL,ERR_NO_MESSAGE,ERR_BOX, ERR_LEVEL_NORMAL, m_fileName);
		}
	}
	return returnCode;
}