from traits.api import HasTraits, Bool, Str, Button, Property, on_trait_change
from traitsui.api import View, Group, Item, VGroup, HGroup, spring
from traitsui.menu import OKButton, CancelButton, Action

import time

class ConfigWindow(HasTraits):

    useMotors   = Bool(False)
    useFCGen    = Bool(False)
    useAmp      = Bool(False)
    usePMeter   = Bool(False)

    # auxiliary booleans - needed because handlers can only connect in one place.
    useMotorsisTrue = Bool(False)
    usePMeterisTrue = Bool(False)

    # flag for configuration done successfully.
    motorsok    = Bool(True)
    fcgenok     = Bool(True)
    ampok       = Bool(True)
    pmeterok    = Bool(True)

    # Empty buttons - connect to functions!
    MotorsButton    = Button('Initialize Motors')
    PMeterButton    = Button('Set up Power Test')

    messages        = Str('')

    traits_view =   View(
                        HGroup(
                        # left panel should be buttons, right panel should be
                        # message window.
                            VGroup(
                                '10',
                                HGroup(
                                    spring,
                                    Item('useFCGen', label = 'Use Function Generator?')
                                ),
                                Item('_'),
                                HGroup(
                                    spring,
                                    Item('useAmp', label = 'Use Amplifier?')
                                ),
                                Item('_'),
                                HGroup(
                                    spring,
                                    Item('useMotors', label = 'Use Motor Table?'),
                                    Item('MotorsButton',
                                     show_label = False, enabled_when = 'useMotorsisTrue', width = 150,)
                                ),
                                Item('_'),
                                HGroup(
                                    spring,
                                    Item('usePMeter', label = 'Use Power Meter?'),
                                    Item('PMeterButton',
                                    show_label = False, enabled_when = 'usePMeterisTrue', width = 150)
                                ),
                            ),

                            Item('messages', style = 'custom', show_label = False)

                        ),
                        buttons = [Action(name = 'OK',
                enabled_when = 'motorsok and fcgenok and ampok and pmeterok')],
                resizable = True,
                title = 'Experiment Options',
                kind = 'nonmodal'
    )

    def __init__(self, reftocontroller):
        super(ConfigWindow, self).__init__()
        self.controller = reftocontroller

    def printmessage(self, string):
        self.messages = time.strftime('%H')+ ':' + time.strftime('%M') + ':' + time.strftime('%S')+ '  :  ' + string + "\n" + "\n" + self.messages

    @on_trait_change('useFCGen')
    def fcgenboxclicked(self):
        if self.useFCGen == True:
            self.fcgenok = False
            self.printmessage('Trying to connect to function generator...')

            FCGen_Connected = self.controller.initconnectionFCGen()

            if FCGen_Connected <> None:
                self.printmessage('The Function Generator is connected and initialized')
                self.fcgenok = True
            else:
                self.printmessage('Function Generator : Connection denied. Be sure that the device is connected to the computer and that it is not still in remote control. If yes reboot the device')
                self.fcgenok = False
        else:
            self.fcgenok = True
            self.controller.offconnectionFGen()
            self.printmessage('The connection with the Function Generator has been stopped')

    @on_trait_change('useAmp')
    def ampboxclicked(self):
        if self.useAmp == True:
            self.ampok = False
            self.printmessage('Trying to connect to amplifier...')

            Amp_Connected=self.controller.initconnectionAmp()

            if Amp_Connected <> None:
                self.printmessage('The Amplifier is connected and initialized')
                self.ampok = True
            else:
                self.printmessage('Amplifier : Connection denied. Be sure that the device is connected to the computer and that no software are using the communication port')
                self.ampok = False

        else:
            self.ampok = True
            self.controller.offconnectionAmplifier()
            self.printmessage('The connection with the Amplifier has been stopped')

    @on_trait_change('useMotors')
    def motorsboxclicked(self):
        self.useMotorsisTrue = self.useMotors
        if self.useMotors:
            self.motorsok = False
            self.printmessage('Trying to connect to motors...')

            Server_Open=self.controller.OpenServerMotorTable()

            if Server_Open <> None:
                self.printmessage('The motors are connected. You need to initialize Motors')
                self.useMotorsisTrue = self.useMotors
            else:
                self.printmessage('Motors : Connection denied. Be sure that the cables are connected and that the interface is turn On')


        else:
            self.motorsok = True
            self.useMotorsisTrue = self.useMotors
            self.controller.CloseServerMotorTable()
            self.printmessage('The connection with the Motors has been stopped')

    @on_trait_change('usePMeter')
    def pmeterboxclicked(self):
        if self.usePMeter == True:
            self.pmeterok = False
            self.printmessage('Power checking enabled. Click button to configure.')

            PMeter_Connected = self.controller.initconnectionPMeter()

            if PMeter_Connected <> None:
                self.printmessage('The Power Meter is connected, now you need to initialize the device with a calibration')
                self.usePMeterisTrue = self.usePMeter
            else:
                self.printmessage('Power Meter : Connection denied. Be sure that the device is connected to the computer and that no software are using the communication port')
        else:
            self.pmeterok = True
            self.usePMeterisTrue = self.usePMeter
            self.controller.offconnectionPmeter()
            self.printmessage('The connection with the Power Meter has been stopped')

    def _MotorsButton_fired(self):

        Result_homing = self.controller.HomingMotorTable()

        if Result_homing == True:
                self.motorsok = True
                self.printmessage('The motors homing has been succesfully done')
        else:
                self.motorsok = False
                self.printmessage("The motors homing hasn't been succesfully done")

    def _PMeterButton_fired(self):

        Result_calibration = self.controller.CalibrationPMeter()

        if Result_calibration == True:
                self.pmeterok = True
                self.printmessage('The Power Meter calibration has been succesfully done')
        else:
                self.pmeterok = False
                self.printmessage("The Power Meter calibration hasn't been succesfully done")
