'''
2017, Thunder Bay Regional Health Research Institute
Jak Loree-Spacek
This software is a cheap, simplified ripoff of the amazing work done by Samuel
Pichardo, Benjamin Zaporzan and co. in 2012.
Also uses gratuitous snippets from Steven Engler's motor_server.py.

This software is to be used only for research purposes
in a preclinical setting.
'''

from __future__ import print_function

try:
	from twisted.internet import reactor, protocol
except ImportError:
	print('issue importing twisted.internet package for socket comms. \n'
	'Your miniconda environment might be set up wrong.\n')
	raise

from HIFU_handler_motor import HandlerMotor
from HIFU_handler_pmeter import HandlerPMeter
from HIFU_handler_fcgen import HandlerFCGen
from HIFU_handler_amp import HandlerAmp

import time
import sys
from threading import Thread
import socket, select
import logging
import json

if sys.maxsize > 2**32:
	raise Exception('You must use a 32-bit version of Python to use the motor controls.')

DEFAULT_PORT = 3000
DEFAULT_HOST = ''
MAX_BACKLOG = 5
DEFAULT_BUFFER_SIZE = 4096
DEFAULT_SELECT_TIMEOUT = None
DEFAULT_MESSAGE_DELIMITER = '!EOF!'
# FROM MATHIFU: ===========
#'None' makes it blocking communication. DEFAULT_TIMEOUT only affects
#the socket recv after it's begun receiving information from the
#client
DEFAULT_TIMEOUT = None

LATENCY_BUFFER = 1.00
SAFETY_TIMEOUT = 5.00

class SafetyManager(Thread):
	'''
	SafetyManager watches two things.
	The first is client timeout (watch for disconnection or crash.)
	The second is reflected power, as long as the power meter is connected.
	'''
	def __init__(self, backchannel):
		# safety manager is instantiated as soon as the server is.
		# Probably too early for anything relevant in the __init__.
		Thread.__init__(self)
		self.daemon = True
		self.bRunning = False
		self.backchannel = backchannel

	def connectionSTART(self):
		logger.debug('Starting safety thread. Instructing client to send refreshes'
			'every %d seconds.' % (SAFETY_TIMEOUT - LATENCY_BUFFER))
		self.backchannel.write(json.dumps({'REFRESH_RATE':SAFETY_TIMEOUT - LATENCY_BUFFER}))
		self.bRunning = True
		self.start()

	def run(self):
		while self.bRunning:
			pass

	def acceptRefresh(self):
		pass
	def _halt(self):
		pass

class Server(protocol.Protocol):
	def __init__(self):
		self.LHandlerMotor = HandlerMotor()
		self.LHandlerPMeter = HandlerPMeter()
		self.LHandlerFCGen = HandlerFCGen()
		self.LHandlerAmp = HandlerAmp()
		self.argListing = {
			# these are all the high-level instructions that the client can
			# request the server performs.
			'open_init_dialog' : [],
			'move_blocking' : ['xpos', 'ypos'],
			'move_async' : ['xpos','ypos'],
			'exposure_timed' : ['amplitude', 'duration'],
			'update_voltage' : ['amplitude'],
			'request_meter_readings' : [],
			'request_instrument_statuses' : [],
			# for refreshing
			'refresh' : []
		}

	def connectionMade(self):
		logger.info('Connection established!')
		self.Safety = SafetyManager(self.transport)
		self.Safety.connectionSTART()
	def connectionLost(self, reason):
		logger.warning('Client disconnected.')

	def dataReceived(self, data):
		try:
			message = json.loads(data)
			logger.debug('Message decoded OK. JSON interpretation is:'+str(message))
			commandName = message['commandName']; inputArgs = message['command']
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
			logger.error('Exception occurred somewhere in the data receipt process.')
			raise
		#time.sleep(4)
		#self.transport.write('here\'s something for you!')

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
		pass

	'''
	===================================================
	===================================================
	'''

def InitiateServer(tcp_port=8888):
	factory = protocol.ServerFactory()
	factory.protocol = Server
	logger.info('Starting the server on port '+ str(tcp_port))
	reactor.listenTCP(tcp_port, factory)
	reactor.run()

if __name__ == '__main__':
	root_logger = logging.getLogger()
	root_logger.setLevel(logging.DEBUG)
	import color_stream_handler
	streamHandler = color_stream_handler.ColorStreamHandler()
	streamHandler.setFormatter(logging.Formatter('%(levelname)-8s : %(name)-10s : %(message)s') )
	root_logger.addHandler(streamHandler)
	logger = logging.getLogger(__name__)

	server2 = InitiateServer()

	sys.exit(0)
