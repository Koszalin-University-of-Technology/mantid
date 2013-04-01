#ifndef AlgorithmManagerTest_H_
#define AlgorithmManagerTest_H_

#include <cxxtest/TestSuite.h>

#include "MantidAPI/Algorithm.h"
#include "MantidAPI/AlgorithmProxy.h"
#include "MantidAPI/AlgorithmManager.h"
#include "MantidKernel/ConfigService.h"
#include <stdexcept>
#include <vector>
#include <Poco/Thread.h>

using namespace Mantid::API;

class AlgTest : public Algorithm
{
public:

  AlgTest() : Algorithm() {}
  virtual ~AlgTest() {}
  void init() { }
  void exec() { }
  virtual const std::string name() const {return "AlgTest";}
  virtual int version() const {return(1);}
  virtual const std::string category() const {return("Cat1");}
};

class AlgTestFail : public Algorithm
{
public:

  AlgTestFail() : Algorithm() {}
  virtual ~AlgTestFail() {}
  void init() { }
  void exec() { }
  virtual const std::string name() const {return "AlgTest";}
  virtual int version() const {return(1);}
  virtual const std::string category() const {return("Cat2");}

};

class AlgTestPass : public Algorithm
{
public:

  AlgTestPass() : Algorithm() {}
  virtual ~AlgTestPass() {}
  void init() { }
  void exec() { }
  virtual const std::string name() const {return "AlgTest";}
  virtual int version() const {return(2);}
  virtual const std::string category() const {return("Cat4");}

};

class AlgTestSecond : public Algorithm
{
public:

  AlgTestSecond() : Algorithm() {}
  virtual ~AlgTestSecond() {}
  void init() { }
  void exec() { }
  virtual const std::string name() const {return "AlgTestSecond";}
  virtual int version() const {return(1);}
  virtual const std::string category() const {return("Cat3");}
};


/** Algorithm that always says it's running if asked */
class AlgRunsForever : public Algorithm
{
public:
  AlgRunsForever() : Algorithm() {}
  virtual ~AlgRunsForever() {}
  void init() { }
  void exec() { }
  virtual const std::string name() const {return "AlgRunsForever";}
  virtual int version() const {return(1);}
  virtual const std::string category() const {return("Cat1");}
  // Override method so we can manipulate whether it appears to be running
  virtual bool isRunning() { return true; }
};


DECLARE_ALGORITHM(AlgTest)
DECLARE_ALGORITHM(AlgRunsForever)
DECLARE_ALGORITHM(AlgTestSecond)

class AlgorithmManagerTest : public CxxTest::TestSuite
{
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static AlgorithmManagerTest *createSuite() { return new AlgorithmManagerTest(); }
  static void destroySuite( AlgorithmManagerTest *suite ) { delete suite; }

  AlgorithmManagerTest() 
  {
    // A test fails unless algorithms.retained is big enough
    Mantid::Kernel::ConfigService::Instance().setString("algorithms.retained","5");
  }

  void testVersionFail()
  {
    const size_t nalgs = AlgorithmFactory::Instance().getKeys().size();
    TS_ASSERT_THROWS_NOTHING(AlgorithmFactory::Instance().subscribe<AlgTestFail>());
    // Size should be the same
    TS_ASSERT_EQUALS(AlgorithmFactory::Instance().getKeys().size(), nalgs);
  }

  void testVersionPass()
  {
    TS_ASSERT_THROWS_NOTHING(AlgorithmFactory::Instance().subscribe<AlgTestPass>());
  }

  void testInstance()
  {
    TS_ASSERT_THROWS_NOTHING( AlgorithmManager::Instance().create("AlgTest") );
    TS_ASSERT_THROWS( AlgorithmManager::Instance().create("AlgTest",3), std::runtime_error );
    TS_ASSERT_THROWS(AlgorithmManager::Instance().create("aaaaaa"), std::runtime_error );
  }

  void testGetNamesAndCategories()
  {
    AlgorithmManager::Instance().clear();
    TS_ASSERT_THROWS_NOTHING( AlgorithmManager::Instance().create("AlgTest") );
    TS_ASSERT_THROWS_NOTHING(AlgorithmManager::Instance().create("AlgTestSecond") );
    std::vector<std::pair<std::string,std::string> > names = AlgorithmManager::Instance().getNamesAndCategories();
    TS_ASSERT_EQUALS(names.size(), 2);
    if (names.size() > 0)
    {
      TS_ASSERT_EQUALS(names[0].first, "AlgTest");
      TS_ASSERT_EQUALS(names[0].second, "Cat4");
    }
    if (names.size() > 1)
    {
      TS_ASSERT_EQUALS(names[1].first, "AlgTestSecond");
      TS_ASSERT_EQUALS(names[1].second, "Cat3");
    }

  }

  void testClear()
  {
    AlgorithmManager::Instance().clear();
    TS_ASSERT_THROWS_NOTHING( AlgorithmManager::Instance().create("AlgTest") );
    TS_ASSERT_THROWS_NOTHING(AlgorithmManager::Instance().create("AlgTestSecond") );
    TS_ASSERT_EQUALS(AlgorithmManager::Instance().size(),2);
    AlgorithmManager::Instance().clear();
    TS_ASSERT_EQUALS(AlgorithmManager::Instance().size(),0);
  }

  void testReturnType()
  {
    AlgorithmManager::Instance().clear();
    IAlgorithm_sptr alg;
    TS_ASSERT_THROWS_NOTHING( alg = AlgorithmManager::Instance().create("AlgTest",1) );
    TS_ASSERT_DIFFERS(dynamic_cast<AlgorithmProxy*>(alg.get()),static_cast<AlgorithmProxy*>(0));
    TS_ASSERT_THROWS_NOTHING( alg = AlgorithmManager::Instance().create("AlgTestSecond",1) );
    TS_ASSERT_DIFFERS(dynamic_cast<AlgorithmProxy*>(alg.get()),static_cast<AlgorithmProxy*>(0));
    TS_ASSERT_DIFFERS(dynamic_cast<IAlgorithm*>(alg.get()),static_cast<IAlgorithm*>(0));
    TS_ASSERT_EQUALS(AlgorithmManager::Instance().size(),2);   // To check that crea is called on local objects
  }

  void testManagedType()
  {
    AlgorithmManager::Instance().clear();
    IAlgorithm_sptr Aptr, Bptr;
    Aptr=AlgorithmManager::Instance().create("AlgTest");
    Bptr=AlgorithmManager::Instance().createUnmanaged("AlgTest");
    TS_ASSERT_DIFFERS(Aptr,Bptr);
    TS_ASSERT_EQUALS(AlgorithmManager::Instance().size(),1);
    TS_ASSERT_DIFFERS(Aptr.get(),static_cast<Algorithm*>(0));
    TS_ASSERT_DIFFERS(Bptr.get(),static_cast<Algorithm*>(0));
  }

  void testCreateNoProxy()
  {
    AlgorithmManager::Instance().clear();
    IAlgorithm_sptr Aptr, Bptr;
    Aptr=AlgorithmManager::Instance().create("AlgTest", -1, true);
    Bptr=AlgorithmManager::Instance().create("AlgTest", -1, false);
    TSM_ASSERT("Was created as a AlgorithmProxy", dynamic_cast<AlgorithmProxy*>(Aptr.get()) );
    TSM_ASSERT("Was NOT created as a AlgorithmProxy", dynamic_cast<AlgorithmProxy*>(Bptr.get())==NULL );
  }

  // This will be called back when an algo starts
  void handleAlgorithmStartingNotification(const Poco::AutoPtr<AlgorithmStartingNotification>& /*pNf*/)
  {
    m_notificationValue = 12345;
  }


  /** When running an algorithm in async mode, the
   * AlgorithmManager needs to send out a notification
   */
  void testStartingNotification()
  {
    AlgorithmManager::Instance().clear();
    Poco::NObserver<AlgorithmManagerTest, Mantid::API::AlgorithmStartingNotification>
      my_observer(*this, &AlgorithmManagerTest::handleAlgorithmStartingNotification);
    AlgorithmManager::Instance().notificationCenter.addObserver(my_observer);

    IAlgorithm_sptr Aptr, Bptr;
    Aptr=AlgorithmManager::Instance().create("AlgTest", -1, true);
    Bptr=AlgorithmManager::Instance().create("AlgTest", -1, false);

    m_notificationValue = 0;
    Poco::ActiveResult<bool> resB = Bptr->executeAsync();
    resB.wait();
    TSM_ASSERT_EQUALS( "Notification was received.", m_notificationValue, 12345 );

    m_notificationValue = 0;
    Poco::ActiveResult<bool> resA = Aptr->executeAsync();
    resA.wait();
    TSM_ASSERT_EQUALS( "Notification was received (proxy).", m_notificationValue, 12345 );

  }

  /** Keep the right number of algorithms in the list */
  void testDroppingOldOnes()
  {
    AlgorithmManager::Instance().clear();
    TS_ASSERT_EQUALS(AlgorithmManager::Instance().size(),0);

    IAlgorithm_sptr first = AlgorithmManager::Instance().create("AlgTest");
    // Fill up the list
    for (size_t i=1; i<5; i++)
      AlgorithmManager::Instance().create("AlgTest");
    TS_ASSERT_EQUALS(AlgorithmManager::Instance().size(),5);

    // The first one is still in the list
    TS_ASSERT(AlgorithmManager::Instance().getAlgorithm(first->getAlgorithmID()) == first);

    // Add one more, drops the oldest one
    AlgorithmManager::Instance().create("AlgTest");
    TS_ASSERT_EQUALS(AlgorithmManager::Instance().size(),5);
    TS_ASSERT( ! AlgorithmManager::Instance().getAlgorithm(first->getAlgorithmID()) );
  }


  /** Keep one algorithm running, drop the second-oldest one etc. */
  void testDroppingOldOnes_whenAnAlgorithmIsStillRunning()
  {
    AlgorithmManager::Instance().clear();
    TS_ASSERT_EQUALS(AlgorithmManager::Instance().size(),0);

    // Create one algorithm that appears never to stop
    IAlgorithm_sptr first = AlgorithmManager::Instance().create("AlgRunsForever");

    IAlgorithm_sptr second = AlgorithmManager::Instance().create("AlgTest");

    // Another long-running algo
    IAlgorithm_sptr third = AlgorithmManager::Instance().create("AlgRunsForever");

    for (size_t i=3; i<5; i++)
      AlgorithmManager::Instance().create("AlgTest");
    TS_ASSERT_EQUALS(AlgorithmManager::Instance().size(),5);

    // The first three created are in the list
    TS_ASSERT(AlgorithmManager::Instance().getAlgorithm(first->getAlgorithmID()) == first);
    TS_ASSERT(AlgorithmManager::Instance().getAlgorithm(second->getAlgorithmID()) == second);
    TS_ASSERT(AlgorithmManager::Instance().getAlgorithm(third->getAlgorithmID()) == third);

    // Add one more, drops the SECOND oldest one
    AlgorithmManager::Instance().create("AlgTest");
    TS_ASSERT_EQUALS(AlgorithmManager::Instance().size(), 5);

    TSM_ASSERT("The oldest algorithm (is still running) so it is still there",
        AlgorithmManager::Instance().getAlgorithm(first->getAlgorithmID()) == first);
    TSM_ASSERT("The second oldest was popped, so trying to get it should return null",
        !AlgorithmManager::Instance().getAlgorithm(second->getAlgorithmID()) );

    // One more time
    AlgorithmManager::Instance().create("AlgTest");
    TS_ASSERT_EQUALS(AlgorithmManager::Instance().size(), 5);

    // The right ones are at the front
    TSM_ASSERT("The oldest algorithm (is still running) so it is still there",
        AlgorithmManager::Instance().getAlgorithm(first->getAlgorithmID()) == first);
    TSM_ASSERT("The third algorithm (is still running) so it is still there",
        AlgorithmManager::Instance().getAlgorithm(third->getAlgorithmID()) == third);

  }

  /**
   * Disabled due to random failures that cannot be pinned down and are most likely timing issues.
   * This test has never failed legitimately and only serves to cause confusion when it fails
   * due to completely unrelated changes.
   */
  void xtestDroppingOldOnes_extremeCase()
  {
  /** Extreme case where your queue fills up and all algos are running */
    AlgorithmManager::Instance().clear();
    std::vector<Poco::ActiveResult<bool>> results;
    std::vector<IAlgorithm_sptr> algs;
    for (size_t i=0; i<5; i++)
    {
      IAlgorithm_sptr alg = AlgorithmManager::Instance().create("AlgRunsForever");
      algs.push_back(alg);
      results.push_back(alg->executeAsync());
    }
    // give it some time to start
    Poco::Thread::sleep(100);

    TS_ASSERT_EQUALS(AlgorithmManager::Instance().size(), 5);
    AlgorithmManager::Instance().create("AlgTest");
    TS_ASSERT_EQUALS(AlgorithmManager::Instance().size(), 6);

    for (size_t i=0; i<5; i++)
    {
      algs[i]->cancel();
      results[i].wait();
    }
  }

  void testThreadSafety()
  {
    PARALLEL_FOR_NO_WSP_CHECK()
    for (int i=0; i < 5000; i++)
    {
      AlgorithmManager::Instance().create("AlgTest");
    }
  }

  void test_runningInstancesOf()
  {
    AlgorithmManager::Instance().clear();
    // Had better return empty at this point
    TS_ASSERT( AlgorithmManager::Instance().runningInstancesOf("AlgTest").empty() )
    // Create an algorithm, but don't start it
    AlgorithmManager::Instance().create("AlgTest");
    // Still empty
    TS_ASSERT( AlgorithmManager::Instance().runningInstancesOf("AlgTest").empty() )
    // Create the 'runs forever' algorithm
    AlgorithmManager::Instance().create("AlgRunsForever");
    auto runningAlgorithms = AlgorithmManager::Instance().runningInstancesOf("AlgRunsForever");
    TS_ASSERT_EQUALS( runningAlgorithms.size(), 1 );
    TS_ASSERT_EQUALS( runningAlgorithms.at(0)->name(), "AlgRunsForever" );
    // Create another 'runs forever' algorithm and another 'normal' one
    auto aRunningAlgorithm = AlgorithmManager::Instance().create("AlgRunsForever");
    TS_ASSERT( AlgorithmManager::Instance().runningInstancesOf("AlgTest").empty() )
    TS_ASSERT_EQUALS( AlgorithmManager::Instance().runningInstancesOf("AlgRunsForever").size(), 2);
    // TODO: Test the count drops back when the algorithm has finished running
    TS_ASSERT( AlgorithmManager::Instance().runningInstancesOf("AlgTest").empty() )
    TS_ASSERT_EQUALS( AlgorithmManager::Instance().size(), 3 );
  }

int m_notificationValue;
};

#endif /* AlgorithmManagerTest_H_*/
