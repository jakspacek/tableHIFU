'''
2017, Thunder Bay Regional Health Research Institute
Jak Loree-Spacek
This software is a cheap, simplified ripoff of the amazing work done by Samuel
Pichardo, Benjamin Zaporzan and co. in 2012.
Also uses gratuitous snippets from Steven Engler's motor_server.py.

This software is to be used only for research purposes
in a preclinical setting.
'''

from __future__ import print_function, absolute_import

try:
	from twisted.internet import reactor, protocol
except ImportError:
	print('issue importing twisted.internet package for socket comms. \n'
	'Your miniconda environment might be set up wrong.\n')
	raise

from Hardware.InstrumentManager import InstrumentManager

import time
import sys
from threading import Thread
from Queue import Queue
import socket, select
import logging
import json

if sys.maxsize > 2**32:
	raise Exception('You must use a 32-bit version of Python to use the motor controls.')

DEFAULT_PORT = 8888
DEFAULT_BUFFER_SIZE = 4096
DEFAULT_MESSAGE_DELIMITER = '!EOF!'

LATENCY_BUFFER = 1.00
SAFETY_TIMEOUT = 5.00

REFLECTED_POWER_SPIKE_THRESHOLD = 0.10 # 10% by default.

class SafetyManager(Thread):
	'''
	SafetyManager watches two things.
	The first is client timeout (watch for disconnection or crash.)
	The second is reflected power, as long as the power meter is connected.
	'''
	def __init__(self, backchannel, cmdqueue, instruments):
		# safety manager is instantiated as soon as the server is.
		# Probably too early for anything relevant in the __init__.
		Thread.__init__(self)
		self.daemon = True
		self.bRunning = False
		self.backchannel = backchannel
		self.instruments = instruments
		self.queue = cmdqueue
		self.__lasttrigger = None
		self._lastpowerreadings = []

	def connectionSTART(self):
		logger.debug('Starting safety thread. Instructing client to send refreshes '
			'every %d seconds.' % (SAFETY_TIMEOUT - LATENCY_BUFFER))
		self.backchannel.write(json.dumps({'REFRESH_RATE':SAFETY_TIMEOUT - LATENCY_BUFFER}))
		self.bRunning = True
		self.__lasttrigger = time.time()
		self.start()

	def connectionENDED(self):
		logger.debug('Pausing/stopping the safety thread.')
		self.bRunning = False

	def run(self):
		while self.bRunning:
			if time.time() - self.__lasttrigger > SAFETY_TIMEOUT:
				logger.critical('Safety timeout triggered!! Client is probably frozen/crashed.')
				self._halt()
				self.bRunning = False
			if self._lastpowerreadings[2] > REFLECTED_POWER_SPIKE_THRESHOLD*self._lastpowerreadings[0]:
				logger.critical('Reflected power spike detected!! Instruments will be halted.')
				self._halt()
				self.bRunning = False
			# hackiest way imaginable to perform a thing every 0.5 seconds.
			# (doesnt waste CPU cycles though, and doesnt require another thread, and
			# doesnt clog up the queue, so I'm happy with it.)
			tensdigit = round(round(time.time(), 1)%1, 1)*10
			if tensdigit==5 or tensdigit==0:
				if self.instruments.instrumentsconnected[3]:
					print(self._lastpowerreadings)
					self.queue.put(('pmeter_send_to_safety', {}))

	def acceptRefresh(self):
		self.__lasttrigger = time.time()

	def _halt(self):
		# Turn off all the instruments suddenly
		pass
#===============================================================================

class Server(protocol.Protocol):
	def __init__(self):
		self.cmdqueue = Queue()
		self.argListing = {
			# these are all the instructions that the client can
			# request the server performs.
			#'open_init_dialog' : [], nope, not anymore...
			# INITIALIZATION. ====
			'fcgen_init' : [],
			'amp_init' : [],
			'pmeter_init' : [],
			'pmeter_calibrate_step1' : [],
			'pmeter_calibrate_step2' : [],
			'pmeter_calibrate_step3' : [],
			'motors_init' : [],
			'motors_homing' : [],

			# OPERATION ====
			'motors_set_focus' : ['xval', 'yval'], # this is needed for cone locking.
			'move_blocking' : ['xpos', 'ypos'],
			'move_async' : ['xpos','ypos'],
			'exposure_timed' : ['amplitude', 'duration'],
			'update_voltage' : ['amplitude'],
			'request_meter_readings' : [],
			'request_table_position' : [],
			'request_instrument_statuses' : [],
			# for refreshing. it's a nop.
			'refresh' : [],
			# stop everything
			'halt' : []
		}

	def connectionMade(self):
		logger.info('Connection established!')
		# start our two threaded managers.
		self.Instruments = InstrumentManager(self.transport, self.cmdqueue)
		self.Safety = SafetyManager(self.transport, self.cmdqueue, self.Instruments)
		self.Instruments.setSafetyRef(self.Safety)
		self.Safety.connectionSTART()

	def connectionLost(self, reason):
		logger.warning('Client disconnected.')
		self.Safety.connectionENDED()
		self.Instruments.bRunning = False

	def dataReceived(self, data):
		# Something was gotten over the channel! Refresh the safety.
		self.Safety.acceptRefresh()
		try:
			message = json.loads(data)
			commandName = message['commandName']; inputArgs = message['commandArgs']
			logger.debug('Message decoded OK. command name '+commandName)
			try:
				#
				if commandName in self.argListing.keys():
					# good! at least the command exists.
					if self.checkMatch(self.argListing[commandName] , inputArgs):
						# input argument list also makes sense!
						self.delegate(commandName, inputArgs)
					#
				#
				else:
					logger.error('Received command name does not match an understood tableHIFU operation.'
						'Protocol will be ignored.')
			except AttributeError:
				logger.error('Message has unusual contents. Version issue?')
				raise
		except ValueError:
			logger.error('Could not decode JSON from the received string.')
			raise
		except:
			logger.error('Exception occurred somewhere else in the data receipt process.')
			raise

	def checkMatch(self, list1, list2):
		# list 1 is list of NEEDED arguments...
		# list 2 is list of arguments we actually got.
		decide = []
		for i in list1:
			if i not in list2:
				logger.error('Argument list for receieved command is not compatible. Ignoring request.')
				return False
		for j in list2:
			decide.append(True if j in list1 else False)
		if all(decide): return True

		logger.warning('Received command is compatible, but contains extra fields. '
			'Are you sure you\'re using the right version of client AND server?')
		return True

	def delegate(self, name, args):
		self.cmdqueue.put((name, args))

	'''
	===================================================
	===================================================
	'''

def InitiateServer(tcp_port=DEFAULT_PORT):
	factory = protocol.ServerFactory()
	factory.protocol = Server
	logger.info('Starting the server on port '+ str(tcp_port))
	reactor.listenTCP(tcp_port, factory)
	reactor.run()

if __name__ == '__main__':

	root_logger = logging.getLogger()
	#root_logger.setLevel(logging.DEBUG)
	root_logger.setLevel(logging.INFO)
	import Extras.color_stream_handler
	streamHandler = Extras.color_stream_handler.ColorStreamHandler()
	streamHandler.setFormatter(logging.Formatter('%(levelname)-8s : %(name)-10s : %(message)s') )
	root_logger.addHandler(streamHandler)
	logger = logging.getLogger(__name__)

	server2 = InitiateServer()

	sys.exit(0)
