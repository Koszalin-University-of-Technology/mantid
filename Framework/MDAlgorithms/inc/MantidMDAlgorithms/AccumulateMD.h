#ifndef MANTID_MDALGORITHMS_ACCUMULATEMD_H_
#define MANTID_MDALGORITHMS_ACCUMULATEMD_H_

#include "MantidKernel/System.h"
#include "MantidAPI/Algorithm.h"
#include "MantidAPI/WorkspaceHistory.h"
#include "MantidAPI/IMDEventWorkspace.h"
#include <set>

namespace {}

namespace Mantid {
namespace API {
class IMDHistoWorkspace;
}

namespace MDAlgorithms {

/// Reduce the vector of input data to only data files and workspaces which can
/// be found
void filterToExistingSources(std::vector<std::string> &input_data,
                             std::vector<double> &psi, std::vector<double> &gl,
                             std::vector<double> &gs,
                             std::vector<double> &efix);

/// Check if the named data source is an existing workspace or file
bool dataExists(const std::string &data_name);

/// Reduce the vector of input data to only data files and workspaces which are
/// not found in the vector of data currently in the workspace
void filterToNew(std::vector<std::string> &input_data,
                 std::vector<std::string> &current_data,
                 std::vector<double> &psi, std::vector<double> &gl,
                 std::vector<double> &gs, std::vector<double> &efix);

/// Check if the named data source is in the vector of data currently in the
/// workspace
bool appearsInCurrentData(const std::string &input_data,
                          std::vector<std::string> &current_data);

/// Return a vector of the names of files and workspaces which have been
/// previously added to the workspace
std::vector<std::string>
getHistoricalDataSources(const API::WorkspaceHistory &ws_history);

/// Extract names of data sources from workspace history and form a set of
/// historical data sources
void insertDataSources(const std::string &data_sources,
                       std::set<std::string> &historical_data_sources);

/// Test if a file with the given full path name exists
bool fileExists(const std::string &filename);

/// Pad vector of parameters to given length
extern void padParameterVector(std::vector<double> &param_vector,
                        const size_t grow_to_size);

/** AccumulateMD : Algorithm for appending new data to a MDHistoWorkspace

  Copyright &copy; 2015 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
  National Laboratory & European Spallation Source

  This file is part of Mantid.

  Mantid is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Mantid is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  File change history is stored at: <https://github.com/mantidproject/mantid>
  Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
class DLLExport AccumulateMD : public API::Algorithm {
public:
  AccumulateMD();
  virtual ~AccumulateMD();

  virtual const std::string name() const;
  virtual int version() const;
  virtual const std::string category() const;
  virtual const std::string summary() const;

private:
  void init();
  void exec();

  /// Use the CreateMD algorithm to create an MD workspace
  Mantid::API::IMDEventWorkspace_sptr createMDWorkspace(
      const std::vector<std::string> &data_sources,
      const std::vector<double> &psi, const std::vector<double> &gl,
      const std::vector<double> &gs, const std::vector<double> &efix);

  std::map<std::string, std::string> validateInputs();
};

} // namespace MDAlgorithms
} // namespace Mantid

#endif /* MANTID_MDALGORITHMS_ACCUMULATEMD_H_ */
