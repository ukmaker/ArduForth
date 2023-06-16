#ifndef STREAM_H
#define STREAM_H

#ifndef STM32F401xC

class Stream {

    public:

    Stream(char *line) : _line(line) {

    }

    void flush() {}

    int read() {
        char c = *_line;
        if(c == '\0') return -1;
        _line++;
        return c;
    }


    protected:

    char *_line;
};
#endif
#endif