import serial
import struct
import time
'''
Amplifier-related hardware communication functions.
Tested by Gael Margot, 2017.
Collected by JLS (August 2017)
'''
def Amplifier_Connection(ComPort, BaudRate, timeout):
    #Trying to connect the computer to the serial port
    try:
        myamplifier = serial.Serial(ComPort, baudrate = BaudRate, timeout = timeout)
        return myamplifier
    #If there is a problem it return None
    #The problem can be :
    #No device connected to the COM port
    #The COM port is already controlled by an other program
    except serial.SerialException:
        return None

def Amplifier_ConnectionOff(Connection):
    #Disconnection of the COM port with our program
    Connection.close()

def Amplifier_GetGain(Connection):
    a = 0
    #We are doing a loop because the device return 1 or 2 times wrong answers
    #These messages are maybe coming from others questions asked to the device
    while a <= 5:
        #Clearing the serial port
        Connection.flush()

        #GetGain is the message to ask to the device "What is the gain ?"
        GetGain = [150,2,20,148]
        #Writing this message in the serial port
        Connection.write(GetGain)
        time.sleep(.500)

        #Checking if there is something to read
        if Connection.read() <> "":
            #if the fram is beggining by "150" (that expected)
            #We check this message
            if struct.unpack_from('B', Connection.read()[0])[0] == 150:
                Gain = [0,0,0,0,0,0]

                Gain[0] = 150
                Gain[1] = struct.unpack_from('B', Connection.read()[0])[0]
                Gain[2] = struct.unpack_from('B', Connection.read()[0])[0]
                Gain[3] = struct.unpack_from('B', Connection.read()[0])[0]
                Gain[4] = struct.unpack_from('B', Connection.read()[0])[0]
                Gain[5] = struct.unpack_from('B', Connection.read()[0])[0]

                #If the third and fourth words are 1 and 244
                #It means that the gain is correct
                #Return True
                if (Gain[3] == 1 and Gain[4] == 244 ):
                    return True
                    break
                else:

                    a = a + 1
                #if the loop fail, return False
                if a == 6:
                    return False
                    break
        else:
            return False
            break

import serial
import struct
import time

def Amplifier_GetStatus(Connection):

    b = 0

    #We are doing a loop because the device return 1 or 2 times wrong answers
    #These messages are maybe coming from others questions asked to the device

    while b <= 5:

        #Clearing the serial port
        Connection.flush()

        #GetStatus is the message to ask to the device "What is the status ?"
        GetStatus = [150,2,23,118]
        #Writing this message in the serial port
        Connection.write(GetStatus)
        time.sleep(.500)

        #Checking if there is something to read
        if Connection.read() <> "":
            #if the fram is beggining by "150" (that expected)
            #We check this message
            if struct.unpack_from('B', Connection.read()[0])[0] == 150:
                Status = [0,0,0,0,0]
                Status[0] = 150
                Status[1] = struct.unpack_from('B', Connection.read()[0])[0]
                Status[2] = struct.unpack_from('B', Connection.read()[0])[0]
                Status[3] = struct.unpack_from('B', Connection.read()[0])[0]
                Status[4] = struct.unpack_from('B', Connection.read()[0])[0]

                #If the third word if 130
                #It means that the status is correct
                #Return True
                if (Status [3] == 130):
                    Status = True
                    return Status
                    break
                else:

                    b = b + 1

                #if the loop fail, return False
                if b == 6:
                    return False
                    break

        else:
            return False
            break

def Amplifier_Initialization(Connection):
    #Clear the serial port
    Connection.flush()
    #Asking the initialization of the device
    Initialization = [150,3,7,130,82]
    Connection.write(Initialization)
    #No answer expected

def Amplifier_RfTurnOff(Connection):
    #Clear the serial port
    Connection.flush()
    #Ask to turn off the RF output
    RfTurnOff = [150,3,7,130,82]
    Connection.write(RfTurnOff)
    #No answer expected

def Amplifier_RfTurnOn(Connection):
    #Clear the serial port
    Connection.flush()
    #Ask to turn on the RF output
    RfTurnOn = [150,3,7,134,51]
    Connection.write(RfTurnOn)
    #No answer expected

def Amplifier_SetGain(Connection):
    #Clear the serial port
    Connection.flush()
    #Ask to set the gain
    SetGain = [150,4,4,1,244,106]
    Connection.write(SetGain)
    #No answer expected


if __name__ == '__main__':
    Connection = Amplifier_Connection('COM6', 19200)
    Amplifier_ConnectionOff(Connection)
