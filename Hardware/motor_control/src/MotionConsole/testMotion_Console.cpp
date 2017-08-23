// testMotion_Console.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace std;
#include "NIMotionMotors.h"
#include <ctime>

int _tmain(int argc, _TCHAR* argv[])
{

	CNIMotionMotors* Motors = new CNIMotionMotors();
	//Motor Initialization
	int errorCheck = 0;
	//Setting up test case
	errorCheck = Motors->InitializeAxes();
	if (errorCheck) {
		cout << "Motors Initialized" << endl;
	}
	else {
		cout << "There was an error!" << endl;
	}

	//Setup Position & Orientation
	/* Position Choices:
		Prone
		Supine (Default)
		Left Decub
		Right Decub
	*/
	CString oPosition = CString("Supine");

	/* Orientation Choices:
		Head First
		Feet First (Default)
	*/
	CString oOrientation= CString("Feet First");

	errorCheck = Motors->ConvertOrienStrToInt(oPosition, oOrientation);
	if (!errorCheck) {
		cout << "ERROR: Setting up orientation" << endl;
	}
	else {
		cout << "Orientation Initialization Successful" << endl;
	}

	//Homing the system
	Motors->HomeSystem();
	cout << "Done Homing" << endl;

	//Setting the initial focus point
	CStringArray initialFocus;
	initialFocus.SetSize(3);
	initialFocus[0] = CString("R0.0");
	initialFocus[1] = CString("A0.0");
	initialFocus[2] = CString("S0.0");

	Motors->ResetCurrentFocus(initialFocus);


	//Moving the motor
	cout << "Motors - Validating Movement" << endl;

	Motors->EnableMotionStsUpdates();

	Sleep(5000);

	//Motor movements

	//----First Movement----
	CString motorMovements[3] = {_T("L5.0"),_T("A0.0"),_T("S5.0")};

	wcout << "Position";
	for (int i = 0; i < 3; i++) {
		wcout << " : " << motorMovements[i].GetString();
	}
	wcout << endl;

	//Validate and make sure the movement is within the bounds
	CStringArray motorMovementsStr;
	errorCheck = Motors->ValidatePosition(motorMovements, 
		motorMovementsStr);
	if (errorCheck) {
		cout << "Motors - Operation Valid, moving motors" << endl;
	}
	else {
		cout << "Something wrong with the points provided" << endl;
	}

	//Move Motors
	if (errorCheck) 
		errorCheck = Motors->MoveToTarget(motorMovementsStr);

	//Check to see if motor movement is correct
	if (errorCheck) {
		cout << "Motors moved successfully!" << endl;
	}
	else {
		cout << "ERROR: Could not move motors" << endl;
	}

	//---Second Movement---
	motorMovements[0] = _T("L5.0");
	motorMovements[1] =	_T("A0.0");
	motorMovements[2] = _T("I5.0");

	wcout << "Position";
	for (int i = 0; i < 3; i++) {
		wcout << " : " << motorMovements[i].GetString();
	}
	wcout << endl;

	//Validate and make sure the movement is within the bounds
	errorCheck = Motors->ValidatePosition(motorMovements, 
		motorMovementsStr);
	if (errorCheck) {
		cout << "Motors - Operation Valid, moving motors" << endl;
	}
	else {
		cout << "Something wrong with the points provided" << endl;
	}

	//Move Motors
	if (errorCheck) 
		errorCheck = Motors->MoveToTarget(motorMovementsStr);

	//Check to see if motor movement is correct
	if (errorCheck) {
		cout << "Motors moved successfully!" << endl;
	}
	else {
		cout << "ERROR: Could not move motors" << endl;
	}

	CStringArray thePosition;
	thePosition.SetSize(3);
	Motors->ReadCurrentPosition(thePosition);
	wcout << "Current Position -- " << 
		thePosition[0].GetString() <<
		" : " << thePosition[1].GetString() <<
		" : " << thePosition[2].GetString() << endl;

	structRange theRange = Motors->GetRanges();
	wcout << "The Ranges: " << endl << 
		theRange.sMax[0].GetString() << " : " << theRange.sMin[0].GetString() <<
		endl <<
		theRange.sMax[1].GetString() << " : " << theRange.sMin[1].GetString() <<
		endl <<
		theRange.sMax[2].GetString() << " : " << theRange.sMin[2].GetString() <<
		endl;

	Motors->StopMotors(1);

	cout << "\nPress Any Key to continue" << endl;
	cin.get();

	delete Motors;

	return 0;
}

