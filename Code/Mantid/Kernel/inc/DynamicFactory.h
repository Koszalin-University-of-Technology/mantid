#ifndef MANTID_DYNAMICFACTORY_H_
#define MANTID_DYNAMICFACTORY_H_

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "Instantiator.h"
#include <map>
#include <stdexcept>

namespace Mantid
{
/** @class DynamicFactory DynamicFactory.h Kernel/DynamicFactory.h

    The dynamic factory is a base dynamic factory for serving up objects in response to requests from other classes.
    
    @author Nick Draper, Tessella Support Services plc
    @date 10/10/2007
    
    Copyright &copy; 2007 ???RAL???

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

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>    
*/
template <class Base>
class DynamicFactory
{
public:
  /// A typedef for the instantiator
  typedef AbstractInstantiator<Base> AbstractFactory;
  
  /// Destroys the DynamicFactory and deletes the instantiators for 
  /// all registered classes.
  virtual ~DynamicFactory()
  {
    for (typename FactoryMap::iterator it = _map.begin(); it != _map.end(); ++it)
    {
      delete it->second;
    }
  }
  
  /// Creates a new instance of the class with the given name.
  /// The class must have been registered with registerClass.
  /// If the class name is unknown, a NotFoundException is thrown.
  Base* create(const std::string& className) const
  {
    
    typename FactoryMap::const_iterator it = _map.find(className);
    if (it != _map.end())
      return it->second->createInstance();
    else
      throw std::runtime_error(className + " is not registered.");
  }
  
  /// Registers the instantiator for the given class with the DynamicFactory.
  /// The DynamicFactory takes ownership of the instantiator and deletes
  /// it when it's no longer used.
  /// If the class has already been registered, an ExistsException is thrown
  /// and the instantiator is deleted.
  template <class C> 
  void subscribe(const std::string& className)
  {
    subscribe(className, new Instantiator<C, Base>);
  }
  
  /// Registers the instantiator for the given class with the DynamicFactory.
  /// The DynamicFactory takes ownership of the instantiator and deletes
  /// it when it's no longer used.
  /// If the class has already been registered, an ExistsException is thrown
  /// and the instantiator is deleted.
  void subscribe(const std::string& className, AbstractFactory* pAbstractFactory)
  {
    std::auto_ptr<AbstractFactory> ptr(pAbstractFactory);
    typename FactoryMap::iterator it = _map.find(className);
    if (it == _map.end())
      _map[className] = ptr.release();
    else
      throw std::runtime_error(className + "is already registered.");
  }
  
  /// Unregisters the given class and deletes the instantiator
  /// for the class.
  /// Throws a NotFoundException if the class has not been registered.
  void unsubscribe(const std::string& className)
  {
    typename FactoryMap::iterator it = _map.find(className);
    if (it != _map.end())
    {
      delete it->second;
      _map.erase(it);
    }
    else 
      throw std::runtime_error(className + " is not registered.");
  }
  
  /// Returns true if the given class is currently registered.
  bool exists(const std::string& className) const
  {
    return _map.find(className) != _map.end();
  }
  
protected:
  /// Protected constructor for base class
  DynamicFactory()
  {
  }  
  
private:
  /// Private copy constructor - NO COPY ALLOWED
  DynamicFactory(const DynamicFactory&);
  /// Private assignment operator - NO ASSIGNMENT ALLOWED
  DynamicFactory& operator = (const DynamicFactory&);

  /// A typedef for the map of registered classes
  typedef std::map<std::string, AbstractFactory*> FactoryMap;
  /// The map holding the registered class names and their instantiators
  FactoryMap _map;
};

} // Namespace Mantid

#endif /*MANTID_DYNAMICFACTORY_H_*/

