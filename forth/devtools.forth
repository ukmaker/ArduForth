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

: IS-CODE ( ca -- flag ) 
    DUP @ 2 - = 
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

: WORDLEN ( word-header -- word-len )
    0x1fff AND
;

( print the word at addr )
( print a maximum of 64 chars, printable chars only )
: .SAFE-WORD ( addr -- )
    DUP @ WORDLEN ( addr len )
    DUP 64 > ( addr len flag)
    IF 
        DROP 64
    THEN

    OVER 2+ + SWAP 2+ ( addr+2+len addr+2 )
    DO
        I C@ ?PRINTABLE IF EMIT ELSE DROP ASPACE EMIT THEN
    LOOP
;

: &quot 0x22 ;
: &dot 0x2e ;
: &sp 0x20 ;
: .&quot &quot EMIT ;
: .&dot &dot EMIT ;
: .&sp &sp EMIT ;
: .&dotquotsp &dot EMIT &quot EMIT ASPACE EMIT ;

( attempt to print the word at the address )
( if the word is one of the literal handlers )
( for string or numbers, then print the name and value )
( leaves the address of the next word on the stack )
: .XT ( addr -- next-addr )
    DUP @ ( addr instr )
    CASE
        ['] *#        OF 2+ DUP @ .                ENDOF
        ['] *"        OF 2+ DUP .&dotquotsp .SAFE-WORD .&quot DUP @ + ALIGN ENDOF
        ['] *S"       OF 2+ DUP .&dot 0x53 EMIT .&quot .&sp .SAFE-WORD .&quot DUP @ + ALIGN ENDOF
        ['] *+LOOP    OF ." *+LOOP" 2 +            ENDOF
        ['] *LOOP     OF ." *LOOP" 2 +             ENDOF
        ['] *DO       OF ." *DO"                   ENDOF
        ['] *ELSE     OF ." *ELSE " 2 +  DUP @ .   ENDOF
        ['] *IF       OF ." *IF " 2 +   DUP @ .    ENDOF
        ['] *ESAC     OF ." *ESAC"                 ENDOF
        ['] *OF_ENDOF OF ." *OF_ENDOF" 2 +         ENDOF
        ['] *OF       OF ." *OF" 2 +               ENDOF
        ['] *UNTIL    OF ." *UNTIL" 2 +            ENDOF
        ( default )
        CA>WA .SAFE-WORD
    ESAC
    2+
;
: DECOMPILE ( ca -- )
    BASE @ SWAP HEX
    DUP
    CA>WA .WORD CRET
    DUP IS-CODE
    IF
        ." <CODE>" CRET DROP
    ELSE 
        2+
        BEGIN 
            DUP @ 0x0106 = ( is this semi ? ) 
            NOT IF
                DUP . ASPACE EMIT .XT ASPACE EMIT CRET
                0
            ELSE
                . ASPACE EMIT ." ;" CRET
                1
            THEN
        UNTIL
        CRET
    THEN
    BASE !
;

: SEE ' DUP IF DECOMPILE ELSE DROP THEN ;
