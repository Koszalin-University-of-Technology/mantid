/*WIKI* 


Converts the representation of the vertical axis (the one up the side of a matrix in MantidPlot) of a Workspace2D from its default of holding the spectrum number to the target unit given.

The spectra will be reordered in increasing order by the new unit and duplicates will not be aggregated. Any spectrum for which a detector is not found (i.e. if the instrument definition is incomplete) will not appear in the output workspace.

For units other than <math>\theta</math>, the value placed in the axis is generated by using the [[ConvertUnits]] methods to translate the values of the first and last bin for the current X-data unit into the target unit, then taking the mid-point of these.


*WIKI*/
//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAlgorithms/ConvertSpectrumAxis2.h"
#include "MantidAPI/NumericAxis.h"
#include "MantidKernel/UnitFactory.h"
#include "MantidAPI/WorkspaceValidators.h"
#include "MantidAPI/Run.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <cfloat>
#include "MantidKernel/BoundedValidator.h"
#include "MantidKernel/ListValidator.h"

namespace Mantid
{
namespace Algorithms
{
  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(ConvertSpectrumAxis2)
  using namespace Kernel;
  using namespace API;
  using namespace Geometry;

  /// Sets documentation strings for this algorithm
  void ConvertSpectrumAxis2::initDocs()
  {
    this->setWikiSummary("Converts the axis of a [[Workspace2D]] which normally holds spectrum numbers to one of Q, Q^2 or theta.<p>'''Note''': After running this algorithm, some features will be unavailable on the workspace as it will have lost all connection to the instrument. This includes things like the 3D Instrument Display. ");
    this->setOptionalMessage("Converts the axis of a Workspace2D which normally holds spectrum numbers to one of Q, Q^2 or theta.  'Note': After running this algorithm, some features will be unavailable on the workspace as it will have lost all connection to the instrument. This includes things like the 3D Instrument Display.");
  }
  

  void ConvertSpectrumAxis2::init()
  {
    // Validator for Input Workspace
    auto wsVal = boost::make_shared<CompositeValidator>();
    wsVal->add<HistogramValidator>();
    wsVal->add<SpectraAxisValidator>();
    wsVal->add<InstrumentValidator>();
    
    declareProperty(new WorkspaceProperty<>("InputWorkspace","",Direction::Input, wsVal),"The name of the input workspace.");
    declareProperty(new WorkspaceProperty<>("OutputWorkspace","",Direction::Output),"The name to use for the output workspace.");
    std::vector<std::string> targetOptions(6);
    targetOptions[0] = "Theta";
    targetOptions[1] = "SignedTheta";
    targetOptions[2] = "ElasticQ";
    targetOptions[3] = "ElasticQSquared";
    targetOptions[4] = "theta";
    targetOptions[5] = "signed_theta";

    declareProperty("Target","",boost::make_shared<StringListValidator>(targetOptions),
      "The unit to which spectrum axis is converted to - \"theta\" (for the angle in degrees), Q or Q^2, where elastic Q is evaluated at EFixed. Note that 'theta' and 'signed_theta' are there for compatibility purposes. The are the same as 'Theta' and 'SignedTheta' respectively");
    std::vector<std::string> eModeOptions;
    eModeOptions.push_back("Direct");
    eModeOptions.push_back("Indirect");
    declareProperty("EMode", "Direct",boost::make_shared<StringListValidator>(eModeOptions),
      "Some unit conversions require this value to be set (\"Direct\" or \"Indirect\")");
    auto mustBePositive = boost::make_shared<BoundedValidator<double> >();
    mustBePositive->setLower(0.0);
    declareProperty("EFixed",EMPTY_DBL(),mustBePositive,
    "Value of fixed energy in meV : EI (EMode=Direct) or EF (EMode=Indirect))");
  }

  void ConvertSpectrumAxis2::exec()
  {

  }
} // namespace Algorithms
} // namespace Mantid