
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
