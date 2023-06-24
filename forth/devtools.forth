( A set of WORDS to help with development )

( is the supplied number within the given range ? )
( i.e. le <= num <= ge )
: WITHIN
  ( n le ge -- num flag )
  2 PICK >= LROT >= AND
;

: ?PRINTABLE
    DUP 0x20 0x7e WITHIN
;

( An enhanced version of DUMP )
( Dump a block of memory in hex, and ptin the characters  )
( address length -- )
: DUMP 
  ( addr length -- end start )
  OVER + SWAP
  ( set BASE for the duration )
  BASE @ HEX LROT
  DO
    ( print the address )
    I . ASPACE EMIT
    ( print 8 bytes )
    8 0 DO
        I J + ( - addr )
        C@ .C ASPACE EMIT
    LOOP
    ( print 8 chars )
    8 0 DO
        I J +
        C@ ?PRINTABLE IF EMIT ELSE DROP ASPACE EMIT THEN ASPACE EMIT
    LOOP CRET
    8 +LOOP
    ( recover the base ) BASE !
;

: [ 0 MODE ! ; IMMEDIATE
: ] 1 MODE ! ;
: UNKNOWN ." WORD UNKNOWN" ;
' UNKNOWN CONSTANT _UNKNOWN

: CA>WA 
    ( try to convert a CA to a Word address )
    ( put a sanity check in place in case we can't find the WA )
    2 - ( skip over the link word )
    64
    SWAP
    BEGIN
        SWAP DUP 0 = NOT 
        ( addr count )
        IF
            ( OK to continue )
            1 - ( addr count-- )
            SWAP 
            2 - DUP @ 0x8000 AND ( got to the header? )
            ( count addr flag)
            IF 1 ELSE 0 THEN
        ELSE
            DROP DROP 0 _UNKNOWN 0
        THEN
    ( count addr flag )
    UNTIL
    SWAP DROP
;

: IS-CODE ( wa -- flag ) 
    WA>CA DUP @ 2 - = 
;

: ?PREV-WORD  ( wa -- wa true || wa -- wa false )
    CURRENT @ @ OVER OVER ( wa pa wa pa )
    ( is the last word the same as the word being looked for? )
    ( if so there is no prev word )
    = IF
        DROP 0
    ELSE
        ( start to work down the dictionary )
        BEGIN 
            DUP NEXT-WORD ( wa pa -- wa pa next-pa )
            DUP 0 = 
            IF ( at the end with nothing found? )
                DROP DROP 0 1
            ELSE
                2 PICK =    ( found the word? )
                IF 1 ROT DROP ELSE NEXT-WORD 0 THEN
            THEN
        UNTIL
    THEN
;

: ['] [ ' *# , ' *# , ] , ' , ; IMMEDIATE

( attempt to print the word at the address )
( if the word is one of the literal handlers )
( for string or numbers, then print the name and value )
( leaves the address of the next word on the stack )
: .WORD-OR-INLINE ( addr -- next-addr )

;
: DECOMPILE ( wa -- )
    DUP .WORD CRET
    DUP IS-CODE
    IF
        ." <CODE>" CRET DROP
    ELSE 
        WA>CB
        BEGIN 
            DUP @ 0x0106 = ( is this semi ? ) 
            NOT IF
                DUP @ CA>WA .WORD CRET
                2+
                0
            ELSE
                DROP
                1
            THEN
        UNTIL
    THEN
;
