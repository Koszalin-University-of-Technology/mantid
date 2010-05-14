//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/AlgorithmProxy.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/AlgorithmObserver.h"

#include <iostream>

using namespace Mantid::Kernel;

namespace Mantid
{
namespace API
{

// Get a reference to the logger
Kernel::Logger& AlgorithmProxy::g_log = Kernel::Logger::get("AlgorithmProxyProxy");

//----------------------------------------------------------------------
// Public methods
//----------------------------------------------------------------------

/// Constructor
AlgorithmProxy::AlgorithmProxy(IAlgorithm_sptr alg) :
  PropertyManagerOwner(),_executeAsync(this,&AlgorithmProxy::executeAsyncImpl),
  m_name(alg->name()),m_category(alg->category()),m_version(alg->version()),m_isExecuted(),m_isLoggingEnabled(true)
{
    Algorithm_sptr a = boost::dynamic_pointer_cast<Algorithm>(alg);
    if (!a)
    {
        g_log.error("Unable to create a proxy algorithm.");
        throw std::logic_error("Unable to create a proxy algorithm.");
    }
    a->initialize();
    copyPropertiesFrom(*a);
}

/// Virtual destructor
AlgorithmProxy::~AlgorithmProxy()
{
}

/** Initialization method invoked by the framework. This method is responsible
 *  for any bookkeeping of initialization required by the framework itself.
 *  It will in turn invoke the init() method of the derived AlgorithmProxy,
 *  and of any sub-AlgorithmProxys which it creates.
 *  @throw runtime_error Thrown if AlgorithmProxy or sub-AlgorithmProxy cannot be initialised
 * 
 */
void AlgorithmProxy::initialize()
{
  // Do nothing as initialization is done in the constructor
  return;
}

/** The actions to be performed by the AlgorithmProxy on a dataset. This method is
 *  invoked for top level AlgorithmProxys by the application manager.
 *  This method invokes exec() method.
 *  For sub-AlgorithmProxys either the execute() method or exec() method
 *  must be EXPLICITLY invoked by  the parent AlgorithmProxy.
 *
 *  @throw runtime_error Thrown if AlgorithmProxy or sub-AlgorithmProxy cannot be executed
 */
bool AlgorithmProxy::execute()
{
    m_alg = boost::dynamic_pointer_cast<Algorithm>(AlgorithmManager::Instance().createUnmanaged(name(),version()));
    m_alg->initializeFromProxy(*this);
    addObservers();
    try
    {
        m_alg->execute();
    }
    catch(...)
    {
        // zero the pointer and rethrow
        m_alg.reset();
        throw;
    }
    m_isExecuted = m_alg->isExecuted();
    m_alg.reset();
    return m_isExecuted;
}

/// True if the algorithm is running asynchronously.
bool AlgorithmProxy::isRunningAsync()
{
    return m_alg? m_alg->isRunningAsync():false;
}

/// True if the algorithm is running.
bool AlgorithmProxy::isRunning()
{
    return m_alg? m_alg->isRunning():false;
}

/// Has the AlgorithmProxy already been initialized
bool AlgorithmProxy::isInitialized() const
{
  return true;//!!!!!!!!!
}

/// Has the AlgorithmProxy already been executed
bool AlgorithmProxy::isExecuted() const
{
  return m_isExecuted;
}
void AlgorithmProxy::cancel()const
{
    if (m_alg)
        m_alg->cancel();
}

/** Add an observer for a notification. If the real algorithm is running
    the observer is added directly. If the algorithm is not running yet
    the observer's address is added to a buffer to be used later when execute/executeAsync
    method is called.
    @param observer Observer
 */
void AlgorithmProxy::addObserver(const Poco::AbstractObserver& observer)const
{
    const Poco::AbstractObserver* obs = &observer;
    if (m_alg) 
    {
        m_alg->addObserver(*obs);
    }
    else
        m_externalObservers.push_back(obs);
}

/** Remove an observer.
    @param observer Observer
 */
void AlgorithmProxy::removeObserver(const Poco::AbstractObserver& observer)const
{
    std::vector<const Poco::AbstractObserver*>::iterator o = 
        std::find(m_externalObservers.begin(),m_externalObservers.end(),&observer);
    if (o != m_externalObservers.end()) m_externalObservers.erase(o);
    if (m_alg) m_alg->removeObserver(observer);
}

//----------------------------------------------------------------------
// Private methods
//----------------------------------------------------------------------
/**
 * Clean up when the real algorithm stops
 */
void AlgorithmProxy::stopped()
{
    m_isExecuted = m_alg->isExecuted();
}

/**
 * Add observers stored previously in m_externalObservers
 */
void AlgorithmProxy::addObservers()
{
    if (!m_alg) return;
    std::vector<const Poco::AbstractObserver*>::reverse_iterator o = m_externalObservers.rbegin();
    for(;o != m_externalObservers.rend();o++)
        m_alg->addObserver(**o);
    m_externalObservers.clear();
}

/** 
 * executeAsync() implementation. Calls execute and when it has finished  deletes the real algorithm.
*/
bool AlgorithmProxy::executeAsyncImpl(const Poco::Void &)
{
  m_alg = boost::dynamic_pointer_cast<Algorithm>(AlgorithmManager::Instance().createUnmanaged(name(),version()));
  m_alg->initializeFromProxy(*this);
  addObservers();
  Poco::ActiveResult<bool> res = m_alg->executeAsync();
  res.wait();
  m_isExecuted = m_alg->isExecuted();
  m_alg.reset();

  try
  {
    return res.data();
  }
  catch(Poco::NullPointerException&)
  {
    return false;
  }
}

} // namespace API
} // namespace Mantid
