#ifndef MANTID_DATAHANDLING_LOADSPEC_H_
#define MANTID_DATAHANDLING_LOADSPEC_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/Algorithm.h"
#include "MantidHistogramData/Histogram.h"

namespace Mantid {
namespace DataHandling {
/**
Loads a workspace from an ascii file. Spectra must be stored in columns.

Properties:
<ul>
<li>Filename  - the name of the file to read from.</li>
<li>Workspace - the workspace name that will be created and hold the loaded
data.</li>
<li>Separator - the column separation character: comma
(default),tab,space,colon,semi-colon.</li>
<li>Unit      - the unit to assign to the X axis (default: Energy).</li>
</ul>

@author Roman Tolchenov, Tessella plc
@date 3/07/09

Copyright &copy; 2007-2010 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
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

File change history is stored at: <https://github.com/mantidproject/mantid>.
Code Documentation is available at: <http://doxygen.mantidproject.org>
*/
class DLLExport LoadSpec : public API::Algorithm {
public:
  LoadSpec();
  const std::string name() const override { return "LoadSpec"; }
  /// Summary of algorithms purpose
  const std::string summary() const override {
    return "Loads data from a text file and stores it in a 2D workspace "
           "(Workspace2D class).";
  }

  int version() const override { return 1; }
  const std::vector<std::string> seeAlso() const override {
    return {"LoadSNSspec"};
  }
  const std::string category() const override { return "DataHandling\\Text"; }

private:
  void init() override;
  void exec() override;

  /// Helper method for reading the number of spectra from the file
  size_t readNumberOfSpectra(std::ifstream &file) const;
  /// Helper method for reading a line from the file
  void readLine(const std::string &line, std::vector<double> &buffer) const;
  /// Helper method for reading a single histogram
  void readHistogram(const std::vector<double> &input,
                     HistogramData::Histogram &histogram) const;

  /// Allowed values for the cache property
  std::vector<std::string> m_seperator_options;
  std::map<std::string, const char *> m_separatormap; ///<a map of seperators
  using separator_pair =
      std::pair<std::string, const char *>; ///<serparator pair type def
};

} // namespace DataHandling
} // namespace Mantid

#endif /*  MANTID_DATAHANDLING_LOADSPEC_H_  */
