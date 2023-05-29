#include <stdint.h>

#define FLAG_IMMEDIATE 1
#define FLAG_PRIMITIVE 2

/**
 * Word layout
 * FLAGS <- Header address
 * LEN
 * LINKH.LINKL <- Link address
 * TOKEN[0] <- Token address
 * ..
 * ..
 * TOKEN[LEN-1]
 * CODE ADDRESS H.L <- Word Address = HA + 4 + LEN
 * BODY
 * ..
 * ..
 * ..
 * NEXT or SEMI
*/
#define HEADER_ADDRESS 0
#define FLAGS_ADDRESS 0
#define LENGTH_ADDRESS 1
#define LINK_ADDRESS 2
#define TOKEN_ADDRESS 4

class Dictionary {

public:
    Dictionary(char *data, size_t size)
    : _data(data), _size(size)
    {
        _ptr = _data;
        _current = _data;
        _context = _data;
        _top = _data;
    }

    ~Dictionary() { }

    int size()
    {
        return _size;
    }

    int len()
    {
        return _ptr - _data;
    }

    void reset()
    {
        _ptr = _data;
    }

    char* end()
    {
        return _ptr;
    }

    bool full()
    {
        return (_ptr - _data) > _size;
    }

    bool append(char c)
    {
        if (full()) {
            return false;
        }
        *_ptr = c;
        _ptr++;
        setTokenLength(_top - _data, getTokenLength(_top - _data) + 1);
        return true;
    }

    /**
     * Reserve 3 words
     * First word holds the length of the word name and any flags
     * Second word holds the link address of the next word in the vocabulary or zero
     * Third word (the Word Address) holds the code address of the word
     * @return The header address
     */
    int reserveToken()
    {
        if (_ptr + TOKEN_ADDRESS - _data > _size) {
            return -1;
        }

        for(int i=0; i<TOKEN_ADDRESS; i++) {
            *_ptr++ = 0;
        }

        return _top - _data;
    }

    void setTokenLength(int addr, uint8_t len)
    {
        _set8(addr + LENGTH_ADDRESS, len);
    }

    uint8_t getTokenLength(int addr)
    {
        return _get8(addr + LENGTH_ADDRESS);
    }

    int getTokenAddress() {
        return _ptr - _data;
    }
    /*
    * returns the indexed character of the token currently sitting on the top
    */
    char getChar(int addr, int i) {
        return _get8(addr + TOKEN_ADDRESS + i);
    }

    void setFlags(int addr, uint8_t len)
    {
        _set8(addr + FLAGS_ADDRESS, len);
    }

    uint8_t getFlags(int addr)
    {
        return _get8(addr + FLAGS_ADDRESS);
    }

    void setCodeAddress(int addr, int ca)
    {
        _set(getWA(addr), ca);
    }

    int getWA(int addr)
    {
        return _get(addr + LENGTH_ADDRESS + getTokenLength(addr));
    }

    void setLink(int addr, int link)
    {
        _set(addr + LINK_ADDRESS, link);
    }

    int getLink(int addr)
    {
        return _get(addr + LINK_ADDRESS);
    }

    void linkToken() {
        // find the top of the current vocabulary and link it to the 
        // token on top. 
        int p1 = getLink(_current - _data);
        int p2;
        while(p2 = getLink(p1) != 0) {
            p1 = p2;
        }
        setLink(p1, _top - _data);
        _top = _ptr;
    }

    /*
     * Returns the word address of the token currently at the end of the dictionary
     * by searching the vocabulary pointed to by CURRENT
     * Returns -1 if nothing is found
     */
    int search()
    {
        int ptr = 0;
        int matched;
        while ((matched = tokenMatch(ptr)) != -1)
            ;

        return matched;
    }

    int tokenMatch(int p)
    {
        // token to be matched lives at _top
        // *p points to a word definition
        // compare lengths first
        int t = _top - _data;
        int a = getTokenLength(p);
        int b = getTokenLength(t);

        if (a != b) {
            // advance p to the next token
            p = getLink(p);
            if(p == 0) { // no more words in this vocabulary
                return -1;
            }
            return tokenMatch(p);
        }

        for (int i = 0; i < a; i++) {
            if (getChar(a,i) != getChar(b,i)) {
                if(p == 0) { // no more words in this vocabulary
                    return -1;
                }
                return tokenMatch(p);
            }
        }

        return getWA(p);
    }

    void enclose(int wa) {
        _set(_top - _data, wa);
        _top += 2;
        _ptr=_top;
    }

    uint16_t addPrimitive(char *name) {
        int tok = reserveToken();
        while(*name != '\0') {
            append(*name);
            name++;
        }
        setFlags(tok, FLAG_PRIMITIVE);
        setCodeAddress(tok, getWA(tok) + 2);
        _primitives++;
        enclose(_primitives);
        enclose(0); // this is the next() primitive by definition
        return _primitives;
    }

protected:
    char* _data;
    char* _ptr;
    char* _current;
    char* _context;
    char *_top;
    size_t _size;
    uint16_t _primitives = 0;

    void _set8(int addr, uint8_t data)
    {
        _data[addr] = data;
    }

    uint8_t _get8(int addr)
    {
        return _data[addr];
    }
    
    void _set(int addr, int data)
    {
        _data[addr] = (char)(data / 256);
        _data[addr + 1] = (char)(data & 255);
    }

    int _get(int addr)
    {
        return _data[addr] * 256 + _data[addr + 1];
    }
};