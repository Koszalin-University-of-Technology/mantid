from __future__ import (absolute_import, division, print_function)


import mantid.simpleapi as mantid

#from Muon import thread_model


class DummyLabelPresenter(object):
    """
    """
    def __init__(self,view,model):
        self.view=view
        self.model=model
        #self.thread = None

    def getWidget(self):
        return self.view

    def updateLabel(self,message):
        self.view.updateLabel(message)
