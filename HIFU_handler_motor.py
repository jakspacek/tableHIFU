import Queue
from threading import Thread

class HandlerMotor(Thread):
    def __init__(self):
        Thread.__init__(self)
        self.bRunning = True
        self.daemon = True
        self.actionqueue = Queue.Queue()
        self.start()

    def run(self):
        while self.bRunning:
            pass
