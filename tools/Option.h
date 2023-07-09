#ifndef UKMAKER_OPTON_H
#define UKMAKER_OPTION_H
#include <stdint.h>
#include <string.h>

class Option
{

    public:

    Option(const char *n, uint16_t v) : name(n), value(v) {}
    ~Option() {}

    const char *name = NULL;
    uint16_t value = 0;
    Option *next = NULL;

};
#endif