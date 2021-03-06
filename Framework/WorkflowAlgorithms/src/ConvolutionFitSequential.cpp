#include "MantidWorkflowAlgorithms/ConvolutionFitSequential.h"

#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/CompositeFunction.h"
#include "MantidAPI/FunctionDomain1D.h"
#include "MantidAPI/FunctionFactory.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidAPI/NumericAxis.h"
#include "MantidAPI/Progress.h"
#include "MantidAPI/Run.h"
#include "MantidAPI/TextAxis.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidAPI/WorkspaceGroup.h"

#include "MantidKernel/BoundedValidator.h"
#include "MantidKernel/ListValidator.h"
#include "MantidKernel/MandatoryValidator.h"
#include "MantidKernel/StringContainsValidator.h"
#include "MantidKernel/VectorHelper.h"

#include <algorithm>
#include <cmath>

namespace {
Mantid::Kernel::Logger g_log("ConvolutionFitSequential");

using namespace Mantid::API;
using namespace Mantid::Kernel;
using Mantid::MantidVec;

std::size_t numberOfFunctions(IFunction_sptr function,
                              const std::string &functionName);

std::size_t numberOfFunctions(CompositeFunction_sptr composite,
                              const std::string &functionName) {
  std::size_t count = 0;
  for (auto i = 0u; i < composite->nFunctions(); ++i)
    count += numberOfFunctions(composite->getFunction(i), functionName);
  return count;
}

std::size_t numberOfFunctions(IFunction_sptr function,
                              const std::string &functionName) {
  const auto composite =
      boost::dynamic_pointer_cast<CompositeFunction>(function);
  if (composite)
    return numberOfFunctions(composite, functionName);
  return function->name() == functionName ? 1 : 0;
}

bool containsFunction(IFunction_sptr function, const std::string &functionName);

bool containsFunction(CompositeFunction_sptr composite,
                      const std::string &functionName) {
  for (auto i = 0u; i < composite->nFunctions(); ++i) {
    if (containsFunction(composite->getFunction(i), functionName))
      return true;
  }
  return false;
}

bool containsFunction(IFunction_sptr function,
                      const std::string &functionName) {
  const auto composite =
      boost::dynamic_pointer_cast<CompositeFunction>(function);
  if (function->name() == functionName)
    return true;
  else if (composite)
    return containsFunction(composite, functionName);
  return false;
}

template <typename T, typename F, typename... Ts>
std::vector<T, Ts...> transformVector(const std::vector<T, Ts...> &vec,
                                      F const &functor) {
  auto target = std::vector<T, Ts...>();
  target.reserve(vec.size());
  std::transform(vec.begin(), vec.end(), std::back_inserter(target), functor);
  return target;
}

template <typename T, typename F, typename... Ts>
std::vector<T, Ts...> combineVectors(const std::vector<T, Ts...> &vec,
                                     const std::vector<T, Ts...> &vec2,
                                     F const &combinator) {
  auto combined = std::vector<T, Ts...>();
  combined.reserve(vec.size());
  std::transform(vec.begin(), vec.end(), vec2.begin(),
                 std::back_inserter(combined), combinator);
  return combined;
}

template <typename T, typename... Ts>
std::vector<T, Ts...> divideVectors(const std::vector<T, Ts...> &dividend,
                                    const std::vector<T, Ts...> &divisor) {
  return combineVectors(dividend, divisor, std::divides<T>());
}

template <typename T, typename... Ts>
std::vector<double> addVectors(const std::vector<T, Ts...> &vec,
                               const std::vector<T, Ts...> &vec2) {
  return combineVectors(vec, vec2, std::plus<T>());
}

template <typename T, typename... Ts>
std::vector<double> multiplyVectors(const std::vector<T, Ts...> &vec,
                                    const std::vector<T, Ts...> &vec2) {
  return combineVectors(vec, vec2, std::multiplies<T>());
}

template <typename T, typename... Ts>
std::vector<T, Ts...> squareVector(const std::vector<T, Ts...> &vec) {
  return transformVector(vec, VectorHelper::Squares<T>());
}

template <typename T, typename... Ts>
std::vector<T, Ts...> squareRootVector(const std::vector<T, Ts...> &vec) {
  return transformVector(vec, static_cast<T (*)(T)>(sqrt));
}

IFunction_sptr extractFirstBackground(IFunction_sptr function);

IFunction_sptr extractFirstBackground(CompositeFunction_sptr composite) {
  for (auto i = 0u; i < composite->nFunctions(); ++i) {
    auto background = extractFirstBackground(composite->getFunction(i));
    if (background)
      return background;
  }
  return nullptr;
}

IFunction_sptr extractFirstBackground(IFunction_sptr function) {
  auto composite = boost::dynamic_pointer_cast<CompositeFunction>(function);

  if (composite)
    return extractFirstBackground(composite);
  else if (function->category() == "Background")
    return function;
  return nullptr;
}

std::string extractBackgroundType(IFunction_sptr function) {
  auto background = extractFirstBackground(function);
  if (!background)
    return "None";

  auto backgroundType = background->name();
  auto position = backgroundType.rfind("Background");

  if (position != std::string::npos)
    backgroundType = backgroundType.substr(0, position);

  if (background->isFixed(0))
    backgroundType = "Fixed " + backgroundType;
  else
    backgroundType = "Fit " + backgroundType;
  return backgroundType;
}

std::vector<std::size_t>
searchForFitParameters(const std::string &suffix,
                       ITableWorkspace_sptr tableWorkspace) {
  auto indices = std::vector<std::size_t>();

  for (auto i = 0u; i < tableWorkspace->columnCount(); ++i) {
    auto name = tableWorkspace->getColumn(i)->name();
    auto position = name.rfind(suffix);
    if (position != std::string::npos &&
        position + suffix.size() == name.size())
      indices.emplace_back(i);
  }
  return indices;
}

std::pair<MantidVec, MantidVec>
calculateEISFAndError(const MantidVec &height, const MantidVec &heightError,
                      const MantidVec &amplitude,
                      const MantidVec &amplitudeError) {
  auto total = addVectors(height, amplitude);
  auto eisfY = divideVectors(height, total);

  auto heightESq = squareVector(heightError);

  auto ampErrSq = squareVector(amplitudeError);
  auto totalErr = addVectors(heightESq, ampErrSq);

  auto heightYSq = squareVector(height);
  auto totalSq = squareVector(total);

  auto errOverTotalSq = divideVectors(totalErr, totalSq);
  auto heightESqOverYSq = divideVectors(heightESq, heightYSq);

  auto sqrtESqOverYSq = squareRootVector(heightESqOverYSq);
  auto eisfYSumRoot = multiplyVectors(eisfY, sqrtESqOverYSq);

  return {eisfY, addVectors(eisfYSumRoot, errOverTotalSq)};
}
} // namespace

namespace Mantid {
namespace Algorithms {

using namespace API;
using namespace Kernel;

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(ConvolutionFitSequential)

//----------------------------------------------------------------------------------------------

/// Algorithms name for identification. @see Algorithm::name
const std::string ConvolutionFitSequential::name() const {
  return "ConvolutionFitSequential";
}

/// Algorithm's version for identification. @see Algorithm::version
int ConvolutionFitSequential::version() const { return 1; }

/// Algorithm's category for identification. @see Algorithm::category
const std::string ConvolutionFitSequential::category() const {
  return "Workflow\\MIDAS";
}

/// Algorithm's summary for use in the GUI and help. @see Algorithm::summary
const std::string ConvolutionFitSequential::summary() const {
  return "Performs a sequential fit for a convolution workspace";
}

/// Algorithm's see also for related algorithms. @see Algorithm::seeAlso
const std::vector<std::string> ConvolutionFitSequential::seeAlso() const {
  return {"QENSFitSequential"};
}

std::map<std::string, std::string> ConvolutionFitSequential::validateInputs() {
  auto errors = QENSFitSequential::validateInputs();
  IFunction_sptr function = getProperty("Function");
  if (!containsFunction(function, "Convolution") ||
      !containsFunction(function, "Resolution"))
    errors["Function"] = "Function provided does not contain convolution with "
                         "a resolution function.";
  return errors;
}

bool ConvolutionFitSequential::throwIfElasticQConversionFails() const {
  return true;
}

bool ConvolutionFitSequential::isFitParameter(const std::string &name) const {
  bool isBackgroundParameter = name.rfind("A0") != std::string::npos ||
                               name.rfind("A1") != std::string::npos;
  return name.rfind("Centre") == std::string::npos && !isBackgroundParameter;
}

ITableWorkspace_sptr
ConvolutionFitSequential::performFit(const std::string &input,
                                     const std::string &output) {
  auto parameterWorkspace = QENSFitSequential::performFit(input, output);
  IFunction_sptr function = getProperty("Function");
  m_deltaUsed = containsFunction(function, "DeltaFunction");
  if (m_deltaUsed)
    calculateEISF(parameterWorkspace);
  return parameterWorkspace;
}

std::map<std::string, std::string>
ConvolutionFitSequential::getAdditionalLogStrings() const {
  IFunction_sptr function = getProperty("Function");
  auto logs = QENSFitSequential::getAdditionalLogStrings();
  logs["delta_function"] = m_deltaUsed ? "true" : "false";
  logs["background"] = extractBackgroundType(function);
  return logs;
}

std::map<std::string, std::string>
ConvolutionFitSequential::getAdditionalLogNumbers() const {
  auto logs = QENSFitSequential::getAdditionalLogNumbers();
  IFunction_sptr function = getProperty("Function");
  logs["lorentzians"] = boost::lexical_cast<std::string>(
      numberOfFunctions(function, "Lorentzian"));
  return logs;
}

/**
 * Calculates the EISF if the fit includes a Delta function
 * @param tableWs - The TableWorkspace to append the EISF calculation to
 */
void ConvolutionFitSequential::calculateEISF(ITableWorkspace_sptr &tableWs) {
  // Get height data from parameter table
  const auto height = searchForFitParameters("Height", tableWs).at(0);
  const auto heightErr = searchForFitParameters("Height_Err", tableWs).at(0);
  auto heightY = tableWs->getColumn(height)->numeric_fill<>();
  auto heightE = tableWs->getColumn(heightErr)->numeric_fill<>();

  // Get amplitude column names
  const auto ampIndices = searchForFitParameters("Amplitude", tableWs);
  const auto ampErrorIndices = searchForFitParameters("Amplitude_Err", tableWs);

  // For each lorentzian, calculate EISF
  auto maxSize = ampIndices.size();
  if (ampErrorIndices.size() > maxSize)
    maxSize = ampErrorIndices.size();

  for (auto i = 0u; i < maxSize; ++i) {
    // Get amplitude from column in table workspace
    auto ampY = tableWs->getColumn(ampIndices[i])->numeric_fill<>();
    auto ampErr = tableWs->getColumn(ampErrorIndices[i])->numeric_fill<>();
    auto eisfAndError = calculateEISFAndError(heightY, heightE, ampY, ampErr);

    // Append the calculated values to the table workspace
    auto ampName = tableWs->getColumn(ampIndices[i])->name();
    auto ampErrorName = tableWs->getColumn(ampErrorIndices[i])->name();
    auto columnName =
        ampName.substr(0, (ampName.size() - std::string("Amplitude").size()));
    columnName += "EISF";
    auto errorColumnName = ampErrorName.substr(
        0, (ampErrorName.size() - std::string("Amplitude_Err").size()));
    errorColumnName += "EISF_Err";

    tableWs->addColumn("double", columnName);
    tableWs->addColumn("double", errorColumnName);
    auto maxEisf = eisfAndError.first.size();
    if (eisfAndError.second.size() > maxEisf) {
      maxEisf = eisfAndError.second.size();
    }

    auto col = tableWs->getColumn(columnName);
    auto errCol = tableWs->getColumn(errorColumnName);
    for (auto j = 0u; j < maxEisf; j++) {
      col->cell<double>(j) = eisfAndError.first.at(j);
      errCol->cell<double>(j) = eisfAndError.second.at(j);
    }
  }
}

} // namespace Algorithms
} // namespace Mantid
