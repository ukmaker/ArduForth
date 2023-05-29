#ifndef UKMAKER_BUF_H
#define UKMAKER_BUF_H

#include "Arduino.h"

class Buffer {

    public:

    Buffer(char *data, size_t size) : _buf(data), _size(size)
    { 
        reset();
    }

    int size() {
        return _size;
    }

    void reset() {
        _ptr = _buf;
        _end = _buf;
    }

   void rewind() {
        _ptr = _buf;
    }

    bool append(char c) {
        if(_ptr - _buf > _size) {
            return false;
        }

        *_ptr = c;
        _ptr++;
        _end++;

        return true;
    }

    bool backspace() {
        if(_ptr > _buf) {
            _ptr--;
            return true;
        }

        return false;
    }

    uint16_t len() {
        return _end - _buf;
    }

    char get(uint16_t pos) {
        return _buf[pos];
    }

    int next() {
        if(_ptr >= _end) {
            return -1;
        }
        char c = *_ptr;
        _ptr++;
        return c;
    }

    protected:

    char *_buf;
    char *_ptr;
    char *_end;
    size_t _size;
};
#endif