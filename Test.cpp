#include "Test.h"


     int Test::tests;
     int Test::passed;
     int Test::failed;

bool should(bool passedTest) {
    Test::tests++;
    if(passedTest) {
        Test::passed++;
    } else {
        Test::failed++;
    }
    return passedTest;
}

void assert(bool t, const char *m) {
    if(should(t)) {
        printf("[OK    ]");
    } else {
        printf("[FAILED]");
    }

    printf(" %s - \n", m);

}

void assertEquals(uint16_t a, uint16_t b, const char *m) {

    if(should(a == b)) {
        printf("[OK    ]");
        printf(" [%d == %d] %s\n", a, b, m);
    } else {
        printf("[FAILED]");
        printf(" [%d != %d] %s\n", a, b, m);
    }
}

void assertString(const char *a, const char *b, const char *m) {
    if(a == NULL && b == NULL) {
        printf("[OK    ] %s\n", m);
        should(true);
    } else if(a == NULL) {
        printf("[FAILED]");
        printf(" [NULL != %s] %s\n", b, m);
        should(false);
    } else if(b == NULL) {
        printf("[FAILED]");
        printf(" [%s != NULL] %s\n", a, m);
        should(false);
    } else if(strcmp(a,b) != 0) {
        printf("[FAILED]");
        printf(" [%s != %s] %s\n", a, b, m);
        should(false);
    } else {
        printf("[OK    ]");
        printf(" [%s == %s] %s\n", a, b, m);
        should(true);
    }
}