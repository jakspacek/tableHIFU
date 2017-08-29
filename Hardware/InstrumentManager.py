from __future__ import print_function, absolute_import
import time
import logging
from threading import Thread
from Queue import Queue, Empty

logger = logging.getLogger(__name__)

import Hardware.ampfunctions as AmpLib
import Hardware.fcgenfunctions as FCGenLib
import Hardware.powermeterfunctions as PMeterLib
#import Hardware.motorfunctions as MotorsLib
# ***Add motors later - we dont have a compiled binary on this laptop.

# ORGANIZATION TIPS FOR MYSELF. -------------
# Server takes exactly one command via the socket. (Suppose it's a non-blocking one.)
# Delegates it here to the InstrumentManager. The InstrumentManager calls the appropriate
# function, then sends a return message (either instantly OR after the thing is complete.)
# It MUST run in a different thread so that the server is free to keep receiving stuff (Including halts).

class InstrumentManager(Thread):
    def __init__(self, backchannel, queue):
        Thread.__init__(self)
        self.bRunning = False
        self.queue = queue
        self.backchannel = backchannel
        self.instrumentsconnected = [False, False, False, False]
        # **alphabetical. Amp, FC Gen, Motors, Powermeter.
        self.bRunning = True
        self.start()

    def setSafetyRef(self, safetyobj):
        self.Safety = safetyobj

    def run(self):
        while self.bRunning:
            try:
                function, kwargs = self.queue.get(timeout = 5.0)
                eval('self.'+str(function)+'(**kwargs)')
            except Empty: # just try again.
                pass

    def issueHandshake(self, message):
        self.backchannel.write(message)

    #===========================================================================
    # Internal use functions. Mostly setup.

    def fcgen_init(self, **kwargs):
        self.FCGen = FCGenLib.FUNC_Connection('172.16.3.241')
        if not self.FCGen:
            logger.error('Function generator connection was not made. Sending error message to client.')
            self.issueHandshake('uh oh')
        else:
            FCGenLib.FUNC_Initialization(self.FCGen, '1e6', '110mV')
            self.issueHandshake('sure i guess')

    def amp_init(self, **kwargs):
        self.Amp = AmpLib.Amplifier_Connection('COM6', 19200, 5)
        if not self.Amp:
            logger.error('Amplifier connection was not made. Sending error message to client.')
            self.issueHandshake('uh oh')
        else:
            AmpLib.Amplifier_Initialization(self.Amp)
            AmpLib.Amplifier_SetGain(self.Amp)
            self.issueHandshake(AmpLib.Amplifier_GetGain(self.Amp),
                AmpLib.Amplifier_GetStatus(self.Amp))

    def pmeter_init(self, **kwargs):
        self.PMeter = PMeterLib.Power_Meter_Connection('COM3', 9600, 2)
        if not self.PMeter:
            logger.error('Power meter connection was not made. Sending error message to client.')
            self.issueHandshake('uh oh')
        else:
            PMeterLib.Power_Meter_Initialization(self.PMeter)
            self.instrumentsconnected[3] = True
            self.Safety._queryagainflag = True
            self.issueHandshake('go ahead')

    def pmeter_calibrate_zeroboth(self, **kwargs):
        self.PMeter.write('CAL1:ZERO:AUTO ONCE\n')
        time.sleep(12)
        self.PMeter.write('CAL2:ZERO:AUTO ONCE\n')
        time.sleep(12)
        self.issueHandshake('go ahead')

    def pmeter_calibrate_cal1(self, **kwargs):
        self.PMeter.write('CAL1:AUTO ONCE\n')
        time.sleep(12)
        self.issueHandshake('go ahead')

    def pmeter_calibrate_cal2(self, **kwargs):
        self.PMeter.write('CAL2:AUTO ONCE\n')
        time.sleep(12)
        self.issueHandshake('go ahead')

    def pmeter_send_to_safety(self):
        vals = PMeterLib.Power_Meter_Reading(self.PMeter)
        self.Safety._lastpowerreadings = vals
        self.Safety._queryagainflag = True

    def motors_init(self, **kwargs):
        self.Motors = motorcontrol.TableMotorController()
        if not self.Motors:
            logger.error('Motor table connection was not made. Sending error message to client.')
            self.issueHandshake('uh oh')
        else:
            self.Motors.init_axes()
            self.issueHandshake('go ahead :)')

    def motors_set_focus(self, xval, yval, **kwargs):
        Motors.set_focus(xval, yval)
        self.issueHandshake('Done')

    def motors_homing(self, **kwargs):
        Motors.stop_motors()
        Motors.orientation('Supine', 'Head First')
        p = Motors.read_current_position()
        # move Y axis
        Motors.set_initial_focus(p[0], p[1])
        Motors.set_focus(p[0],-11.17)
        Motors.move_to_target(p[0],0)
        # move X axis
        p = Motors.read_current_position()
        Motors.set_focus(-25.3,p[1])
        Motors.move_to_target(0,p[1])
        Motors.stop_motors()
        p = Motors.read_current_position()
        if p[0] < 0.5 and p[0] > -0.5 and p[1] < 0.5 and p[1] > -0.5:
            # in range.
            self.issueHandshake('Homing done OK')
        else:
            self.issueHandshake('Homing NOT done OK')

    def clean_close(self):
        if self.instrumentsconnected[0]:
            self.Amp.close()
        if self.instrumentsconnected[1]:
            self.FCGen.clear()
            self.FCGen.close()
        if self.instrumentsconnected[2]:
            self.Motors.stop_motors()
            # no real way to disconnect, weirdly.
        if self.instrumentsconnected[3]:
            self.PMeter.close()
        self.instrumentsconnected = [False, False, False, False]
        self.bRunning = False

    #===========================================================================
    # Exposed Functions.

    def move_blocking(self, xpos, ypos, **kwargs):
        logger.debug('move-blocking in the instrument manager.')

        self.issueHandshake('OK')

    def move_async(self, xpos, ypos, **kwargs):
        pass

    def exposure_timed(self, amplitude, duration, **kwargs):
        "Run in main InstruManager Thread. Loop until timeout or halting signal. "
        "Halting signal can originate from Safety or from a server command."

        pass

    def update_voltage(self, amplitude, **kwargs):
        "The PID controller has decided a new voltage. Update the "
        "Note that we're relying on the Safety thread to catch issues - there "
        "will be NO timing done here...!"
        FCGenLib.FUNC_SetAmplitude(self.FCGen, amplitude)
        self.issueHandshake('done')

    def request_meter_readings(self, **kwargs):
        "Ask the power meter, send the numbers by backchannel. I guess."
        pass

    def request_table_position(self, **kwargs):
        "Send the answer by backchannel."
        self.issueHandshake(self.Motors.read_current_position())

    def request_instrument_statuses(self, **kwargs):
        "Return the instruments that were actually configured."
        "Send them by backchannel as JSON, I guess."
        self.issueHandshake('they exist in the current universe.')

    def refresh(self, **kwargs):
        "This should actually be a NOP. The actual refreshing is done way earlier."
        pass

    def halt(self, **kwargs):
        # end the instrument management thread.
        # halt other things too.
        self.bRunning = False
