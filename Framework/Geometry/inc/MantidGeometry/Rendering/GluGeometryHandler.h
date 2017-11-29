#ifndef GLU_GEOMETRYHANDLER_H
#define GLU_GEOMETRYHANDLER_H

#include "MantidGeometry/DllConfig.h"
#include "MantidGeometry/Rendering/GeometryHandler.h"
#include "MantidGeometry/Rendering/ShapeInfo.h"

namespace Mantid {
namespace Kernel {
class V3D;
}
namespace Geometry {
class GeometryHandler;
class IObjComponent;
class CSGObject;
/**
   \class GluGeometryHandler
   \brief Place holder for geometry triangulation and rendering with special
   cases of cube, sphere, cone and cylinder.
   \author Srikanth Nagella
   \date December 2008
   \version 1.0

   This is an implementation class for handling geometry without triangulating
   and using opengl glu methods.
   This class can render cube, sphere, cone and cylinder.

   Copyright &copy; 2008 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
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
*/
class MANTID_GEOMETRY_DLL GluGeometryHandler : public GeometryHandler {
private:
  static Kernel::Logger &PLog;                   ///< The official logger
  /// Object/ObjComponent
  std::vector<Kernel::V3D> m_points;
  double radius; ///<Radius for the sphere, cone and cylinder
  double height; ///<height for cone and cylinder;
  detail::ShapeInfo m_shapeInfo;
public:
  GluGeometryHandler(const GluGeometryHandler &other);
  GluGeometryHandler(IObjComponent *comp);              ///< Constructor
  GluGeometryHandler(boost::shared_ptr<CSGObject> obj); ///< Constructor
  GluGeometryHandler(CSGObject *obj);                   ///< Constructor
  boost::shared_ptr<GeometryHandler> clone() const override;
  ~GluGeometryHandler() override; ///< Destructor
  GeometryHandler *createInstance(IObjComponent *comp) override;
  GeometryHandler *createInstance(boost::shared_ptr<CSGObject> obj) override;
  GeometryHandler *createInstance(CSGObject *) override;
  /// sets the geometry handler for a cuboid
  void setShapeInfo(detail::ShapeInfo &&shapeInfo);
  void Triangulate() override;
  void Render() override;
  void Initialize() override;
  void GetObjectGeom(int &mytype, std::vector<Kernel::V3D> &vectors,
                     double &myradius, double &myheight) override;
};

} // NAMESPACE Geometry

} // NAMESPACE Mantid

#endif
