#ifndef UKMAKER_TESTSUITE_H
#define UKMAKER_TESTSUITE_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

class TestSuite {

    public:
    TestSuite() {}
    ~TestSuite() {}

     int tests;
     int passed;
     int failed;

    void reset() {
        tests = 0;
        passed = 0; 
        failed = 0;
    }
};

#endif