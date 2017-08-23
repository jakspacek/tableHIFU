#include "Performance_Timers.h"
#include "stdafx.h"

bool PerformanceTimerMs(double Milliseconds, _int64 PerformanceCounterFreq = -1)
{	
	LARGE_INTEGER starttime, currenttime, endtime;
	if (!QueryPerformanceCounter(&starttime))
		return false;
	
	LARGE_INTEGER performancefreq;
	if (PerformanceCounterFreq == -1)	{
		if (!QueryPerformanceFrequency(&performancefreq))
			return false;	}
	else
		performancefreq.QuadPart = PerformanceCounterFreq;
	endtime.QuadPart = starttime.QuadPart + (LONGLONG)((Milliseconds/1000.0)*((double)performancefreq.QuadPart));	
	currenttime = starttime;
	//while (((currenttime .QuadPart >= starttime.QuadPart)&&(currenttime.QuadPart < endtime.QuadPart))||
	//	((currenttime.QuadPart < starttime.QuadPart)&&(currenttime.QuadPart < endtime.QuadPart)))
	while (currenttime.QuadPart < endtime.QuadPart)
	{
		if (!QueryPerformanceCounter(&currenttime))
			return false;
	}
	/*CString str;
	str.Format("%0.15f",((double)(currenttime.QuadPart - starttime.QuadPart))/((double)(performancefreq.QuadPart)));
	AfxMessageBox(str);*/
	return true;
}

bool PerformanceTimerCounts(_int64 Counts, _int64 PerformanceCounterFreq = -1)
{
	LARGE_INTEGER performancefreq;
	performancefreq.QuadPart = PerformanceCounterFreq;
	if (performancefreq.QuadPart == -1)
		if (!QueryPerformanceFrequency(&performancefreq))
			return false;
	LARGE_INTEGER starttime, currenttime, endtime;
	if (!QueryPerformanceCounter(&starttime))
		return false;
	endtime.QuadPart = starttime.QuadPart + Counts;
	currenttime.QuadPart = starttime.QuadPart;
	while (((currenttime .QuadPart >= starttime.QuadPart)&&(currenttime.QuadPart < endtime.QuadPart))||
		((currenttime.QuadPart < starttime.QuadPart)&&(currenttime.QuadPart < endtime.QuadPart)))
	{
		if (!QueryPerformanceCounter(&currenttime))
			return false;
	}
	return true;
}

bool WaitUntilTime(_int64 WaitUntilCounts)
{
	LARGE_INTEGER currenttime;
	if (!QueryPerformanceCounter(&currenttime))
		return false;
	while (currenttime.QuadPart < WaitUntilCounts)
	{
		if (!QueryPerformanceCounter(&currenttime))
			return false;
	}
	return true;
}
