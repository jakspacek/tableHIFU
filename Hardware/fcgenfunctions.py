import visa
import time
'''
Function-Generator-related hardware communication functions.
Tested by Gael Margot, 2017.
Collected by JLS (August 2017)
'''
def FUNC_Connection(ipAddress):

    try:
        #trying to open a Visa ressource
        rm = visa.ResourceManager()
        #Connect to VISA Adress
        #LAN Connection Example . 'TCPIP0::xxx.xxx.xxx.xxx::RemotePort::SOCKET'
        #Keep write and read termination with '\n' or it will not work

        myFUNC = rm.open_resource("TCPIP0::" + ipAddress + "::23::SOCKET", write_termination='\n', read_termination='\n')
        myFUNC.delay = 1.2
        myFUNC.timeout = 5000

        return myFUNC

    except visa.VisaIOError:
        return None

def FUNC_ConnectionOff(myinstrument):

    #Closing the Visa connection
    myinstrument.clear()
    myinstrument.close()

def FUNC_Initialization(myinstrument, Frequency, amplitude):
    #Writing the instruction to initialize the device
    myinstrument.write(':INITIATE:CONTINUOUS ON')
    myinstrument.write(':SOUR:FUNC:SHAP SIN')
    myinstrument.write(':FREQ:CW ' + Frequency)
    myinstrument.write(':VOLT:LEV:AMPL ' + amplitude)
    myinstrument.write(':TRIG:SOUR:ADV EXT')
    myinstrument.write(':OUTP:STAT OFF')

def FUNC_SetAmplitude(myinstrument, amplitude):
    #Writing the new amplitude expected
    myinstrument.write(':VOLT:LEV:AMPL ' + str(amplitude))

def FUNC_SetFrequency(instrument, Frequency):
    instrument.write(':FREQ:CW ' + str(Frequency))

def FUNC_TurnOff(myinstrument):
    #Writing turn off the output like expected
    myinstrument.write(':OUTP:STAT OFF')

def FUNC_TurnOn(myinstrument):
    #Writing turn on the output like expected
    myinstrument.write(":OUTP:STAT ON")

if __name__ == '__main__':
    FCGen_Connection = FUNC_Connection('172.16.3.241')
    FUNC_Initialization(FCGen_Connection, '1000000', '0.1')
