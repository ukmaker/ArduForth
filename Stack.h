#ifndef UKMAKER_STACK_H
#define UKMAKER_STACK_H

#include "Arduino.h"

class Stack {

    public:

    Stack(uint16_t *data, size_t size) : _stack(data), _size(size)
        {
            _ptr = _stack;
        }

    int size() {
        return _size;
    }

    void reset() {
        _ptr = _stack;
    }

    bool push(uint16_t v) {
        if(_ptr - _stack > _size) {
            return false;
        }

        *_ptr = v;
        _ptr++;

        return true;
    }

    bool canPop() {
        return _ptr > _stack;
    }

    uint16_t top() {
        if(canPop()) {
            return *_ptr;
        }

        return 0;        
    }

    uint16_t pop() {
        if(canPop()) {
            _ptr--;
            return *_ptr;
        }

        return 0;
    }

    bool drop() {
        if(canPop()) {
            pop();
            return true;
        }
        return false;
    }

    bool dup() {
        if(!canPop()) {
            return false;
        }

        uint16_t v = pop();
        push(v);
        return push(v);
    }

    bool over() {
        uint16_t v1, v2;

        if(!canPop()) {
            return false;
        }

        v1 = pop();

        if(!canPop()) {
            return false;
        }

        v2 = pop();

        return push(v2) ? (push(v1) ? push(v2) : 0) : 0 ;
    }


    protected:
        uint16_t *_stack;
        uint16_t *_ptr;
        size_t _size;
};

#endif