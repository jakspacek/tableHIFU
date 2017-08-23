#pragma once

bool PerformanceTimerMs(double Milliseconds, _int64 PerformanceCounterFreq );
//Waits for a given amount of time
//	Milliseconds: time to wait (in milliseconds)
//	PerformanceCounterFreq: performance counter frequency (-1 is default: automatically finds counter frequency)
//	->Return: returns true if successful, false if unsuccessful

bool PerformanceTimerCounts(_int64 Counts, _int64 PerformanceCounterFreq );
//Waits for a given amount of time
//	Counts: time to wait (in counts)
//	PerformanceCounterFreq: performance counter frequency (-1 is default: automatically finds counter frequency)
//	->Return: returns true if successful, false if unsuccessful

bool WaitUntilTime(_int64 WaitUntilCounts);
//Waits until the performance counter reaches a certain time (in performance counter counts)
//	WaitUntilCounts: the timer will wait until the performance counter reaches WaitUntilCounts