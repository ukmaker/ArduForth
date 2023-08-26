( Forth source for core words )

: AT-FIRST
    CA 2+ DP@  =
;

( Words to allow use of local variables )
( Locals are stored on the return stack )
( Space is allocated at the beginning of the word )
( Syntax for using locals is as follows )
( a b c are assigned values off the data stack )
( x y z are initialised to zero )
( : DEF { a b c | x y z } ) 
(   a b + to x )
( ; )
( This compiles to )
( : DEF 3 *INIT-LOCALS 3 *ALLOT-LOCALS )
(   0 *LOCAL@ 1 *LOCAL@ + 3 *LOCAL! )
( ; )
(  )
(  )

8 CONSTANT #MAX-LOCAL-LEN ( in bytes )
8 CONSTANT #MAX-LOCALS
10 CONSTANT #LOCAL-CODE-SIZE

( Size of a local definition in bytes )
#MAX-LOCAL-LEN ALIGN #LOCAL-CODE-SIZE + CONSTANT #LOCAL-SIZE
0 VARIABLE $LOCAL-DEFS #LOCAL-SIZE 2/ #MAX-LOCALS * VEXTEND
0 VARIABLE $NUM-LOCALS 
: NUM-LOCALS $NUM-LOCALS @ ;

: LOCAL-DEF ( num -- addr )
( Returns the address of the nth local )
    DUP #MAX-LOCALS >= 
    IF
        ." Too many locals ( " . ." > " #MAX-LOCALS . ." ) "
        RESTART
    THEN
    #LOCAL-SIZE * $LOCAL-DEFS +
;

: CALLER-FP
    FP 4 + @ 4 + @
;

: *LOCAL
    ( idx -- addr )
    2* CALLER-FP SWAP -
;

: *LOCAL@
    ( idx -- val )
    2* CALLER-FP SWAP - @
;

: *LOCAL!
    ( val idx -- )
    2* CALLER-FP SWAP - !
;

: CALLER
( in case a word needs access to the address )
( of the calling word )
    FP 4 + @
;

: *AT-LOCAL 
( Run by the local at compile-time to compile )
( the runtime action - n *LOCAL@ )
( local-idx -- )
    ['] *# , , 
    ['] *LOCAL@ ,
;

: *TO-LOCAL
    ['] *# , , 
    ['] *LOCAL! ,
;

( During compilation local names are created as )
( definitions in the $LOCAL-DEFS space and temporarily )
( linked to the CURRENT vocabulary so they can be found )
( Move the token at global DP to the locals DP )
: *DP>LOCALS
    DP@ ( source-addr)
    #LOCAL-SIZE ( source-len)
    NUM-LOCALS LOCAL-DEF ( dest-addr)
    COPY
;

: *COMPILE-LOCAL
    DP@ @ 8 > 
    IF 
        ." Local name length > 8 not allowed "
        RESTART
    THEN

    *DP>LOCALS
    NUM-LOCALS 0=
    IF ( first local, make it CURRENT and link to existing CURRENT )
        CURRENT @ @ 
    ELSE ( make current and link to previous local )
        NUM-LOCALS 1- LOCAL-DEF ( WA of next local )
    THEN

    NUM-LOCALS LOCAL-DEF WA>LA !
    NUM-LOCALS LOCAL-DEF CURRENT @ !

    
    ( now compile the compile-time behaviour )
    NUM-LOCALS LOCAL-DEF WA>CA
    ['] *: OVER ! 2+
    ['] *# OVER ! 2+ NUM-LOCALS OVER ! 2+
    ['] *AT-LOCAL OVER ! 2+
    ['] *; OVER ! DROP
    ( make the word immediate )
    NUM-LOCALS LOCAL-DEF DUP @ #IMMEDIATE OR SWAP !
    1 $NUM-LOCALS +!
;

: *COMPILE-INIT-LOCALS ( num-init-locals -- )
    ( assign in reverse order to match the incoming stack order )
    DUP
    IF
        ['] *# , ,
        ['] *INIT-LOCALS ,
    ELSE 
        DROP
    THEN
;

: *COMPILE-ALLOT-LOCALS ( num-allot-locals -- )
    DUP
    IF ( more than 0 to allot )
        ['] *# , ,
        ['] *ALLOT-LOCALS ,
    ELSE
        DROP
    THEN
;

: to 
    ( assign a value to a local )
    ( val [ name] -- )
    ' ( ca )
    4 + ( address of the local idx )
    @ ['] *# , , ['] *LOCAL! ,
; IMMEDIATE

: DP}? S" }" DP@ COMPARE 0= ;
: DP|? S" |" DP@ COMPARE 0= ;

: {
    0 $NUM-LOCALS !
    BEGIN
        &sp; TOKEN 
        DP|? DP}? OR
        IF
            1
        ELSE
            *COMPILE-LOCAL
            0
        THEN
    UNTIL

    DP|?
    
    NUM-LOCALS *COMPILE-INIT-LOCALS


    IF
        NUM-LOCALS
        BEGIN
            &sp; TOKEN 
            DP}?
            IF
                1
            ELSE
                *COMPILE-LOCAL
                0
            THEN
        UNTIL
        NUM-LOCALS SWAP -
        *COMPILE-ALLOT-LOCALS
    THEN
; IMMEDIATE

: [;] [ ' ; , ] ; IMMEDIATE 
: ; ( -- )
( Redefine ; to unlink the local definitions from the dictionary ) 
    NUM-LOCALS
    IF
        0 LOCAL-DEF WA>LA @ CURRENT @ !
        0 $NUM-LOCALS !
    THEN
    [ ' [;] , ]

[;] IMMEDIATE



