/* Generated file, do not edit */

#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#define _CXXTEST_HAVE_STD
#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>
#include <cxxtest/TestMain.h>
#include <cxxtest/ErrorPrinter.h>

int main( int argc, char *argv[] ) {
 int status;
    CxxTest::ErrorPrinter tmp;
    CxxTest::RealWorldDescription::_worldName = "cxxtest";
    status = CxxTest::Main< CxxTest::ErrorPrinter >( tmp, argc, argv );
    return status;
}
bool suite_mytestsuite_init = false;
#include "test.h"

static mytestsuite suite_mytestsuite;

static CxxTest::List Tests_mytestsuite = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_mytestsuite( "test.h", 2, "mytestsuite", suite_mytestsuite, Tests_mytestsuite );

static class TestDescription_suite_mytestsuite_testAddition : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_mytestsuite_testAddition() : CxxTest::RealTestDescription( Tests_mytestsuite, suiteDescription_mytestsuite, 4, "testAddition" ) {}
 void runTest() { suite_mytestsuite.testAddition(); }
} testDescription_suite_mytestsuite_testAddition;

#include <cxxtest/Root.cpp>
const char* CxxTest::RealWorldDescription::_worldName = "cxxtest";
