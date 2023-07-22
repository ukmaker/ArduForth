( Forth source for core words )

: ALLOT 0 DO 0 , LOOP ;
: DONE ( Leave a DO LOOP )
    R> R> R> SWAP DROP DUP >R >R >R
;

: 2DROP DROP DROP ;

: CA ENTRY WA>CA ;

: AT-FIRST
    CA 2+ HERE  =
;

: VALLOT 
( allocate space in the variables heap )
( size-in-words -- start-addr )
    2 *
    $USER-TOP @ SWAP ( size -- current-address size )
    $USER-TOP +!
;

: VEXTEND 
( Extend a variable to the size on the stack in words )
    1 - VALLOT
;

: ARRAY 
    ( size-in-words -- ) 
    ( -- addr )
    <BUILDS DUP VALLOT , , DOES>
;

: ARRAY-SIZE 2+ @ ;

: ARRAY-BOUNDS? ( addr index -- addr index flag )
    ( is the index within the bounds of the array ? )
    DDUP SWAP ARRAY-SIZE <
    DUP NOT IF ." Index out of bounds " THEN
;

: ARRAY! ( value array-addr index -- )
    ARRAY-BOUNDS?
    IF
        2* SWAP @ + !
    ELSE
        2DROP DROP
    THEN
;


: ARRAY@ ( addr index -- value )
    ARRAY-BOUNDS?
    IF
        2* SWAP @ + @
    ELSE
        2DROP 0
    THEN
;

: .ARRAY 
    DUP ARRAY-SIZE  0
    ?DO 
        DUP I ARRAY@ I . &sp; EMIT . CRET 
    LOOP 
    DROP 
;

: STACK 
    ( Create a stack )
    ( stack size -- )
    1+ ARRAY 0 CA 4 + @ @ ! 
;
: STACK-SP  @ @ ;
: STACK-SIZE  2+ @ 1- ;
: STACK-SP++ @ DUP @ 1+ SWAP ! ;
: STACK-SP-- @ DUP @ 1- SWAP ! ;
: STACK! 1+ ARRAY! ;
: STACK@ 1+ ARRAY@ ;

: STACK-PUT
    ( put data at the nth position down on the stack ) 
    ( v pos -- )
    DUP STACK-SP - STACK!
;

: STACK-GET
    ( get the data at the nth position down on the stack ) 
    ( pos -- v )
    DUP STACK-SP - STACK@
;

: PUSH 
    DUP STACK-SP OVER STACK-SIZE
    < IF
        SWAP OVER DUP STACK-SP STACK!
        STACK-SP++
    ELSE
        ." Stack overflow "
        2DROP
    THEN
;

: POP ( addr -- val )
    DUP STACK-SP 0
    > IF
        DUP STACK-SP--
        DUP STACK-SP STACK@
    ELSE
        ." Stack underflow "
        DROP
    THEN
;

: .RS #RSTOP BEGIN
    DUP . .&sp; DUP @ . CRET
    2- DUP ?RS = UNTIL DROP
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
( )
( )

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
        ." Too many locals " 
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

: *DP>LOCALS
    DP@ #LOCAL-SIZE $NUM-LOCALS @ LOCAL-DEF COPY
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
    ['] *# , ,
    ['] *INIT-LOCALS ,
;

: *COMPILE-ALLOT-LOCALS ( num-allot-locals -- )
    ['] *# , ,
    ['] *ALLOT-LOCALS ,
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
        DP|? DUP
        DP}?
        OR
        IF
            1
        ELSE
            *COMPILE-LOCAL
            0
        THEN
    UNTIL

    NUM-LOCALS *COMPILE-INIT-LOCALS

    IF ( S" |" DP@ COMPARE 0=  From above )
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



