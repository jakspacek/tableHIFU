from __future__ import print_function
import time
import logging
from threading import Thread
from Queue import Queue, Empty

logger = logging.getLogger(__name__)

import Hardware.ampfunctions as Amp
import Hardware.fcgenfunctions as FCGen
import Hardware.powermeterfunctions as PMeter
#import Hardware.motorfunctions as Motors
# Add motors later - we dont have a compiled binary on this laptop.

# ORGANIZATION TIPS FOR MYSELF. -------------
# Server takes exactly one command via the socket. (Suppose it's a non-blocking one.)
# Delegates it here to the InstrumentManager. The InstrumentManager calls the appropriate
# function, then sends a return message (either instantly OR after the thing is complete.)
# It MUST run in a different thread so that the server is free to keep receiving stuff (Including halts).

class InstrumentManager(Thread):
    def __init__(self, queue, backchannel):
        Thread.__init__(self)
        self.bRunning = False
        self.queue = queue
        self.backchannel = backchannel

        self.bRunning = True
        self.start()

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
    # Other internal use functions.


    #===========================================================================
    # Exposed Functions.

    def move_blocking(self, xpos, ypos):
        logger.debug('move-blocking in the instrument manager.')


        self.issueHandshake('OK')

    def move_async(self, xpos, ypos):
        pass

    def exposure_timed(self, amplitude, duration):
        "Run in main InstruManager Thread. Loop until timeout or halting signal. "
        "Halting signal can originate from Safety or from a server command."

        pass

    def update_voltage(self, amplitude):
        "The PID controller has decided a new voltage. Update the "
        "Note that we're relying on the Safety thread to catch issues - there "
        "will be NO timing done here...!"
        pass

    def request_meter_readings(self):
        "Ask the power meter, send the numbers by backchannel. I guess."
        "MIGHT NOT NEED THIS?"
        pass

    def request_table_position(self):
        "Send the answer by backchannel."
        pass

    def request_instrument_statuses(self):
        "Return the instruments that were actually configured."
        "Send them by backchannel as JSON, I guess."
        pass

    def refresh(self):
        "This should actually be a NOP. The actual refreshing is done way earlier."
        pass

    def halt(self):
        # end the instrument management thread.
        # halt other things too.
        self.bRunning = False
