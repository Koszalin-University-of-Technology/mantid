#pylint: disable=no-init

from __future__ import (absolute_import, division, print_function)
import stresstesting
from mantid.simpleapi import *
from ISISCommandInterface import *


class SANS2DLimitEventsTime(stresstesting.MantidStressTest):

    def runTest(self):
        SANS2D()
        MaskFile('MaskSANS2DReductionGUI_LimitEventsTime.txt')
        AssignSample('22048')
        WavRangeReduction()

    def validate(self):
        self.disableChecking.append('SpectraMap')
        self.disableChecking.append('Axes')
        self.disableChecking.append('Instrument')
        return '22048rear_1D_1.5_12.5','SANSReductionGUI_LimitEventsTime.nxs'
