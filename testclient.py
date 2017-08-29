import socket
import json
import time

#with open('samplejson.json') as datafile:
#    jsondata = json.load(datafile)

# trying straight socket comms.

instruction = {'commandName':'pmeter_init',
    'commandArgs':{

    }
    }
sendthis = json.dumps(instruction)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('127.0.0.1', 8888))
getsomething = s.recv(1024)
print(getsomething)

print('client: sent the instruction.')
s.send(sendthis); print(s.recv(1024))
time.sleep(3)
s.send(json.dumps({'commandName':'refresh','commandArgs':{}}))
time.sleep(3)
s.send(json.dumps({'commandName':'refresh','commandArgs':{}}))

s.close()
