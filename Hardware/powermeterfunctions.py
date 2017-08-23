import serial
from PySide.QtGui import QMessageBox
import pdb
import sys
import time

def Power_Meter_Calibration(mypowermeter):
    #codecalibration will be False if the calibration is not done properly
    #codecalibration will be True if the calibration is done properly
    codecalibration = False

    #Open a pop up window to inform the operator that he has to keep A and B connected
    reply = QMessageBox.question(None, 'Please Confirm.', 'Zeroing in Process \n DO NOT REMOVE "A" OR "B" FROM DUAL DIRECTIONAL COUPLER', QMessageBox.Ok, QMessageBox.Cancel)

    #User choose 'Ok'
    if reply == QMessageBox.Ok:
        #Zeroing the 2 inputs
        mypowermeter.write('CAL1:ZERO:AUTO ONCE\n')
        time.sleep(12)
        mypowermeter.write('CAL2:ZERO:AUTO ONCE\n')
        time.sleep(12)

        #Open a pop up window to inform the operator that he has to remove A and connect it to Power Ref
        reply = QMessageBox.question(None, 'Please Confirm.', 'Remove "A" from dual directional coupler and Connect "A" to "Power Ref', QMessageBox.Ok, QMessageBox.Cancel)
        if reply == QMessageBox.Ok:
            #Calibration of A
            mypowermeter.write('CAL1:AUTO ONCE\n')
            time.sleep(12)

            #Open a pop up window to inform the operator that he has to reconnect A and connect B to Power Ref
            reply = QMessageBox.question(None, 'Please Confirm.', 'Reconnect "A" to dual directional coupler, Remove "B" from dual directional coupler and Connect "B" to "Power Ref', QMessageBox.Ok, QMessageBox.Cancel)
            if reply == QMessageBox.Ok:
                #Calibration of A
                mypowermeter.write('CAL2:AUTO ONCE\n')
                time.sleep(12)

                #Open a pop up window to inform the operator that he has to reconnect B
                reply = QMessageBox.question(None, 'Please Confirm.', 'Reconnect "B" to the coupler.', QMessageBox.Ok, QMessageBox.Cancel)
                if reply == QMessageBox.Ok:
                    #Zeroing the 2 inputs
                    mypowermeter.write('CAL1:ZERO:AUTO ONCE\n')
                    time.sleep(12)
                    mypowermeter.write('CAL2:ZERO:AUTO ONCE\n')
                    time.sleep(12)
                    #The calibration is finished, then codecalibration is up to 'True'
                    codecalibration = True

                else:  # user chose Cancel
                    print 'Calibration stopped'
                    codecalibration = False
            else:  # user chose Cancel
                print 'Calibration stopped'
                codecalibration = False
        else:  # user chose Cancel
            print 'Calibration stopped'
            codecalibration = False
    else:  # user chose Cancel
        print 'Calibration stopped'
        codecalibration = False

    return codecalibration

def Power_Meter_Connection(Port, Baudrate):
    #trying to establish a connection with the serial port of Power Meter
    #If there is a problem the exception will return None
    #It can be because the serial port is still used by an other software
    try:
        mypowermeter = serial.Serial(port=Port, baudrate=Baudrate)
        mypowermeter.timeout = 2
        mypowermeter.write('*IDN?\n')
        DeviceName = mypowermeter.readline()

        #This device return an IDN
        #Then we are trying to read the answer after asking this
        #If there is an IDN it means that the device is connected then we return the connection
        #Else we return None
        if DeviceName == "":
            return None
        else:
            return mypowermeter

    except serial.SerialException:
        return None

def Power_Meter_Connection_Off(mypowermeter):
    #Closing the connection
    mypowermeter.close()

def Power_Meter_Initialization(mypowermeter):

    #Asking some instruction to initialize the system
    mypowermeter.write('SYST:LANG SCPI\n')
    time.sleep(0.5)
    mypowermeter.write('ABOR1\n')
    time.sleep(1)
    mypowermeter.write('ABOR2\n')
    time.sleep(1)
    mypowermeter.write('CONF1:POW:AC DEF,(@1)\n')
    time.sleep(0.3)
    mypowermeter.write('CONF2:POW:AC:DIFF DEF,DEF,(@1),(@2)\n')
    time.sleep(0.3)
    mypowermeter.write('SENS1:AVER:COUN 1\n')
    time.sleep(0.3)
    mypowermeter.write('SENS2:AVER:COUN 1\n')
    time.sleep(0.3)
    mypowermeter.write('TRIG1:DEL:AUTO ON\n')
    time.sleep(0.3)
    mypowermeter.write('TRIG2:DEL:AUTO ON\n')
    time.sleep(0.3)
    mypowermeter.write('INIT1:CONT ON\n')
    time.sleep(0.1)
    mypowermeter.write('INIT2:CONT ON\n')

def Power_Meter_Reading(mypowermeter):
    #For each reading values we need to freeze the windows
    #Then asking to the device to read the needed values
    #When the answers complete, unfreeze the windows
    mypowermeter.write('INIT1:CONT OFF\n')
    mypowermeter.write('INIT2:CONT OFF\n')

    mypowermeter.write('READ1:POW:AC?\n')
    powerA = mypowermeter.readline()
    print powerA

    mypowermeter.write('READ2:POW:AC?\n')
    powerB = mypowermeter.readline()
    print powerB

    mypowermeter.write('READ2:POW:AC:DIFF?\n')
    powerreading = mypowermeter.readline()
    print powerreading

    mypowermeter.write('INIT1:CONT ON\n')
    mypowermeter.write('INIT2:CONT ON\n')

if __name__ == '__main__':
    mypowermeter = Power_Meter_Connection('COM3', 9600)
