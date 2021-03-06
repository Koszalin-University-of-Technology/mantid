#ifndef MANTID_PYTHONINTERFACE_MAPPINGTYPEHANDLER_H
#define MANTID_PYTHONINTERFACE_MAPPINGTYPEHANDLER_H
/*
  Copyright &copy; 2016 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
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
#include "MantidPythonInterface/kernel/Registry/PropertyValueHandler.h"

namespace Mantid {
namespace PythonInterface {
namespace Registry {

/**
  *  Defines a handler class for converting a Python mapping type object
  * to a C++ PropertyManager type.
  */
class MappingTypeHandler final : public PropertyValueHandler {
  void set(Kernel::IPropertyManager *alg, const std::string &name,
           const boost::python::api::object &mapping) const override;
  std::unique_ptr<Kernel::Property>
  create(const std::string &name,
         const boost::python::api::object &defaultValue,
         const boost::python::api::object &validator,
         const unsigned int direction) const override;
};
}
}
}

#endif // MANTID_PYTHONINTERFACE_MAPPINGTYPEHANDLER_H
