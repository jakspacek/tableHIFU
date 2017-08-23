'''
Moving away from the Pyro4 server. We are the server now.
wish me luck. JLS, 2017
Enormous credit to Steven Engler. I'm just re-wrapping the wrappers.
'''
from PySide import QtGui, QtCore
import time

import threading

# Non-package-style import just to minimize changes.
import sys
sys.path.append('motor_control/lib')
import motorcontrol

class popupMoveWindow(QtGui.QDialog):
    #Window who permit to the operator to check the current position of the table
    #here we need to reach the position [0,0]
    def __init__(self, controller, parent = None):
        super(popupMoveWindow, self).__init__(parent)
        self.controller = controller
        self.endthreadflag = False
        self.Spinner = '|'

        self.setWindowTitle('Motor table - manual repositioning')
        self.setModal(True)
        self.Xdisplay = QtGui.QLabel()
        self.Ydisplay = QtGui.QLabel()
        self.Spinnerlabel = QtGui.QLabel()
        self.Xfloat = 0.0
        self.Yfloat = 0.0
        layout = QtGui.QGridLayout()
        layout.addWidget(QtGui.QLabel('Move motor table until homed (0, 0)'), 0,0)
        layout.addWidget(self.Spinnerlabel, 0,1)
        layout.addWidget(QtGui.QLabel('X position:'), 1,0)
        layout.addWidget(QtGui.QLabel('Y position:'), 2,0)
        layout.addWidget(self.Xdisplay, 1,1)
        layout.addWidget(self.Ydisplay, 2,1)
        self.buttons = QtGui.QDialogButtonBox(
            QtGui.QDialogButtonBox.Ok | QtGui.QDialogButtonBox.Cancel,
            QtCore.Qt.Horizontal, self)
        self.buttons.accepted.connect(self.accept)
        self.buttons.rejected.connect(self.reject)
        layout.addWidget(self.buttons, 3,0,1,2)

        self.setLayout(layout)
        self.update_xy()

        t = threading.Thread(target = self.refreshnumbers, args = ())
        t.start()

    def reject(self):
        self.endthreadflag = True
        QtGui.QDialog.reject(self)
    def accept(self):
        self.endthreadflag = True
        QtGui.QDialog.accept(self)

    def update_xy(self):
        self.Xdisplay.setText('%.1f' % self.Xfloat)
        self.Ydisplay.setText('%.1f' % self.Yfloat)
        self.Spinnerlabel.setText(self.Spinner)
        if self.Xfloat < 0.5 and self.Xfloat > -0.5 and self.Yfloat < 0.5 and self.Yfloat > -0.5:
            self.buttons.buttons()[0].setEnabled(True)
        else:
            self.buttons.buttons()[0].setEnabled(False)

    def refreshnumbers(self):
        while not self.endthreadflag:
            time.sleep(0.10)
            SpinnerChars = ['/','-','\\', '|']
            for index in range(len(SpinnerChars)):
                if self.Spinner == SpinnerChars[index]:
                    if index == 3:
                        self.Spinner = SpinnerChars[0];break
                    else:
                        self.Spinner = SpinnerChars[index+1];break

            Positions = self.controller.GetValueMotorTable()
            self.Xfloat = Positions[0]
            self.Yfloat = Positions[1]
            self.update_xy()

def Motors_Homing(connection, controller):

        result = False

        motors = connection
        #Stop the regulation of the motors
        motors.stop_motors()

        #Display a window popup which ask the operator to move the support at the position B
        reply = QtGui.QMessageBox.question(None, 'Please Confirm.', 'This code will perform the homing procedure \n\n Please make sure the motors have moved to corner B', QtGui.QMessageBox.Ok, QtGui.QMessageBox.Cancel)
        #The operator answer Ok
        if reply == QtGui.QMessageBox.Ok:
            motors.stop_motors()
            motors.orientation('Supine', 'Head First')

            #Update of the current position of the 2 axis
            p = motors.read_current_position()
            motors.set_initial_focus(p[0], p[1])
            #Set the new value of the Y axis
            motors.set_focus(p[0],-11.17)
            #Ask the axis Y to move to reach the position 0
            motors.move_to_target(p[0],0)

            #Update of the current position of the 2 axis
            p = motors.read_current_position()
            #Set the new value of the X axis
            motors.set_focus(-25.3,p[1])
            #Ask the axis X to move to reach the position 0
            motors.move_to_target(0,p[1])
            motors.stop_motors()

            p = motors.read_current_position()

            #Test if the homing is done
            if p[0] < 0.5 and p[0] > -0.5 and p[1] < 0.5 and p[1] > -0.5:
                motors.stop_motors()
                #Information to the operator to say that the homing has been done
                reply = QtGui.QMessageBox.question(None, 'Information', "The homing has been done", QtGui.QMessageBox.Ok)
                return True
            else:
                motors.stop_motors()
                #Inform that the homing hasnt been correctly done
                reply = QtGui.QMessageBox.question(None, 'Please Confirm.', "The homing has a problem and has'nt been done \n\n Please check if the interface is turn on and if all the cables are well connected \n\n Click Ok to do this manually or Cancel to retry in automatic", QtGui.QMessageBox.Ok, QtGui.QMessageBox.Cancel)
                #operator click Ok
                if reply == QtGui.QMessageBox.Ok:
                    #ask the operator to move the manually the support on B position
                    reply = QtGui.QMessageBox.question(None, 'Please Confirm.', "Please make sure the motors have moved to corner B", QtGui.QMessageBox.Ok, QtGui.QMessageBox.Cancel)
                    if reply == QtGui.QMessageBox.Ok:
                        #Set new values on the 2 axis
                        motors.set_focus(-25.3,-11.17)
                        motors.stop_motors()

                        #Call of the popup window to control the current position of the table
                        w = popupMoveWindow(controller)
                        returnval = w.exec_()

                        #If the function return 1 it means that the position was near to 0 when the operator click on OK
                        #+/- 0.5 mm
                        return returnval == 1

                    else: # user chose Cancel
                        motors.stop_motors()
                        return False
                else: # user chose Cancel
                    motors.stop_motors()
                    return False

        else: # user chose Cancel
            motors.stop_motors()
            return False

def Motors_Get_Value(connection):
    motors = connection
    #p will return an array : p[0] and p[1]
    #p[0] = X
    #p[1] = Y
    p = motors.read_current_position()
    return p

def Motors_Move_To_Target(connection, Target_X, Target_Y, timeout_ms):

    motors = connection
    motors.stop_motors()

    motors.orientation('Supine', 'Head First')
    p = motors.read_current_position()
    motors.move_to_target(Target_X,p[1],timeout_ms)
    p = motors.read_current_position()
    motors.move_to_target(p[0],Target_Y,timeout_ms)
    p = motors.read_current_position()
    motors.stop_motors()

    #Check if the target has been reached (+/- 0.5 mm)
    return p[0] < (Target_X + 0.5) and p[0] > (Target_X - 0.5) and p[1] < (Target_Y + 0.5) and p[1] > (Target_Y - 0.5)

def Motors_Set_Focus(connection, X,Y):
    connection.stop_motors()
    connection.set_focus(X,Y)

if __name__ == '__main__':
    controller = motorcontrol.TableMotorController()
    controller.init_axes()
    try:
        pass
    except:
        controller.stop_motors()
        raise
