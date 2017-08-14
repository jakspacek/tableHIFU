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
	'Try typing \'pip install twisted.internet\' in your console.\n')
	#raise

	#TODO: figure out how to get this working with Conda environment...?

from HIFU_handler_motor import HandlerMotor
from HIFU_handler_pmeter import HandlerPMeter
from HIFU_handler_fcgen import HandlerFCGen
from HIFU_handler_amp import HandlerAmp

import time
import sys
from threading import Thread
import socket, select
import logging
#import yaml

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

#plagiarized from MatHIFU
class HIFU_ThreadedHandler(Thread):
	def __init__(self,
                 parent, #server object
                 socketSession):
		Thread.__init__(self)
		logger.info('Connection established. ')
		self.daemon = False
		self.bRunning = False
		self.parent = parent

		self.socketObject, self.socketAddress = socketSession

		self.socketObject.settimeout(DEFAULT_TIMEOUT)

		self.socket_commandListing = {
            'ping' : self.pong,
            'close' : self.close,
            'quit' : self.stop,
            }

		self.handlerListing = {
            COMMAND_TYPE_SOCKET : self.handlerSocket, #function
            COMMAND_TYPE_MOTOR : HandlerStatic(), #Class Instantiation
            COMMAND_TYPE_PMETER : HandlerDynamic(), #Class Instantiation
			COMMAND_TYPE_FCGEN: HandlerDynamic(), #Class Instantiation
			COMMAND_TYPE_AMP : HandlerDynamic(), #Class Instantiation
            }
		self.start()

	def run(self):
		self.bRunning = True
		while self.bRunning:
			try:
				clientMessage = self.receiveClientMessage()
			except:
				#The server couldn't parse the message, so we send a generic error
				logger.error('Received an unknown message from the client')
			if clientMessage:
				self.handleClientMessage(clientMessage)

	def receiveClientMessage(self):
		messageString = ''
		while DEFAULT_MESSAGE_DELIMITER not in messageString:
			try:
				messageString += self.socketObject.recv(DEFAULT_BUFFER_SIZE)
			except socket.timeout:
				logger.error("TIMEOUT (%d seconds)" % DEFAULT_TIMEOUT)
				logger.error("ERROR: Could not receive the entire message from the client in time.")
				logger.error("SOCKET: " + str(self.socketAddress))
				logger.error("MESSAGE: " + str(messageString))
				return None
			except:
				logger.error('Some other error was obtained from recv... Weird')

		if not messageString:
			logger.info('Receiving blank data, assuming socket connection has been severed')
			logger.info('Closing Socket...')
			self.close()
			return None

		#remove delimiter after receiving
		messageString = messageString[:(-1 * len(DEFAULT_MESSAGE_DELIMITER))]
		logger.debug("SUCCESS: Message Received")
		logger.debug("R_MESSAGE: " + str(messageString))

		# create and return a JSON data object corresponding to the receieved JSON data!
		try:
			dataObject = socketMessageProtocol.JSON_MessageProtocol().fromString(messageString)
		except:
			logger.error("ERROR: Message could not be parsed, are you sure it's a JSON object?")
			logger.error("MESSAGE: " + str(messageString))
			return None
		return dataObject

	def handleClientMessage(self, message):
		responseMessage = None
		if message.Contains(COMMAND_TYPE_SOCKET):
			cmdHandler = self.handlerListing[COMMAND_TYPE_SOCKET]
		elif message.Contains(COMMAND_TYPE_STATIC):
			cmdHandler = self.handlerListing[COMMAND_TYPE_STATIC]
		elif message.Contains(COMMAND_TYPE_DYNAMIC):
			cmdHandler = self.handlerListing[COMMAND_TYPE_DYNAMIC]
		else:
			logger.error('Unknown client command received. Message will'
				'be ignored.')
			logger.info('message:' + str(message))

#also plagiarized from MatHIFU
class Server(object):
    def __init__(self, host = DEFAULT_HOST, port = DEFAULT_PORT):
        self.bRunning = False
        self.threadList = []
        self.socketList = []

        self.__call__ = self.run
        self.__del__ = self.stop

        self.listening_socket = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
        self.listening_socket.bind( (host, port) )
        self.listening_socket.listen(MAX_BACKLOG)

    def run(self):
        self.bRunning = True
        logger.info('Waiting for connections...')
        while self.bRunning:
            self._select()
        logger.info('Server has stopped running.')

    def stop(self):
        for thread in self.threadList:
            thread.close()
        self.bRunning = False

    def _select(self):
		pass
        #try:
        #    readList, writeList, execList = select.select(
        #        [ self.listening_socket, ], [], [],
        #        DEFAULT_SELECT_TIMEOUT,
        #    )
        #except (KeyboardInterrupt, SystemExit):
        #    print('at least this works...')
        #finally:
        #    pass

if __name__ == '__main__':
	root_logger = logging.getLogger()
	root_logger.setLevel(logging.DEBUG)
	import color_stream_handler
	streamHandler = color_stream_handler.ColorStreamHandler()
	streamHandler.setFormatter(logging.Formatter('%(levelname)-8s : %(name)-10s : %(message)s') )
	root_logger.addHandler(streamHandler)
	logger = logging.getLogger(__name__)

	server = Server()
	dummythread = HIFU_ThreadedHandler(server, None)
	server.run()
	sys.exit(0)
