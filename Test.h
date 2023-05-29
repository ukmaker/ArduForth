#ifndef UKMAKER_TEST_H
#define UKMAKER_TEST_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

class Test {

    public:
    Test() {}
    ~Test() {}

    static int tests;
    static int passed;
    static int failed;

    static void reset() {
        tests = 0;
        passed = 0; 
        failed = 0;
    }
};


extern bool should(bool passedTest);

extern void assert(bool t, const char *m);

extern void assertEquals(uint16_t a, uint16_t b, const char *m);

extern void assertString(const char *a, const char *b, const char *m);

#endif