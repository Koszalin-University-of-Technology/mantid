.. algorithm::

.. summary::

.. relatedalgorithms::

.. properties::

Description
-----------

Fits an \*\_iqt file generated by Iqt using one of the specified
functions. The functions available are either one or two exponentials
(:math:`Intensity \times exp[-(x/\tau)]`), a stretched exponential
(:math:`Intensity \times exp[-(x/\tau)]\beta`) or a combination of both
an exponential and stretched exponential.

This routine was originally part of the MODES package.

Workflow
--------

.. diagram:: IqtFitMultiple-v1_wkflw.dot


Usage
-----

**Example - Running IqtFitMultiple on an reduced workspace.**

.. code-block:: python
    
    #Load in iqt data
    input_ws = Load(Filename='iris26176_graphite002_iqt.nxs')
    function = r'name=LinearBackground,A0=0.027668,A1=0,ties=(A1=0);name=UserFunction,Formula=Intensity*exp(-(x/Tau)^Beta),Intensity=0.972332,Tau=0.0247558,Beta=1;ties=(f1.Intensity=1-f0.A0)'

    #run IqtFitMultiple
    result, params, fit_group = IqtFitMultiple(InputWorkspace=input_ws, Function=function, FitType='1S_s', StartX=0, EndX=0.2, SpecMin=0, SpecMax=16)


.. categories::

.. sourcelink::
